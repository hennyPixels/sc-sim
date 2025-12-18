# Parts Database Application Architecture

## Overview

A cross-platform parts database application with:
1. **Blazor Hybrid .NET MAUI** - Portable GUI for Windows, macOS, Linux, iOS, Android
2. **C23 CLI Extension** - Lightweight text-based interface for Linux and embedded systems
3. **Shared Database Layer** - Common data access via SQLite

---

## Database Selection: SQLite

### Why SQLite?

| Consideration | SQLite | PostgreSQL | LiteDB |
|---------------|--------|------------|--------|
| Portability | ✅ Single file | ❌ Server required | ✅ Single file |
| Embedded support | ✅ Excellent | ❌ Not suitable | ❌ .NET only |
| C interop | ✅ Native C API | ⚠️ libpq required | ❌ Not available |
| Memory footprint | ✅ ~600KB | ❌ Heavy | ⚠️ Medium |
| BLOB storage | ✅ Efficient | ✅ Good | ⚠️ Document-based |
| Cross-platform | ✅ All platforms | ⚠️ Complex setup | ⚠️ .NET only |

**Decision: SQLite** - Best balance of portability, C interop for embedded, and efficient BLOB storage for images.

---

## Project Structure

```
sc-sim/
├── src/
│   ├── PartsDb.Shared/           # Shared .NET library
│   │   ├── Models/               # Data models
│   │   ├── Services/             # Business logic
│   │   ├── Data/                 # Database context
│   │   └── ImageProcessing/      # Image compression
│   │
│   ├── PartsDb.Maui/             # Blazor Hybrid MAUI app
│   │   ├── Components/           # Blazor components
│   │   ├── Pages/                # Razor pages
│   │   └── Platforms/            # Platform-specific code
│   │
│   └── partsdb-cli/              # C23 CLI extension
│       ├── src/
│       │   ├── main.c            # Entry point
│       │   ├── database.c        # SQLite operations
│       │   ├── image.c           # Image compression (stb_image)
│       │   └── ui.c              # Text UI (ncurses optional)
│       ├── include/
│       └── Makefile
│
├── database/
│   └── schema.sql                # Database schema
│
├── tools/
│   └── image-import/             # Automated import scripts
│
└── tests/
    ├── PartsDb.Tests/            # .NET tests
    └── cli-tests/                # C tests
```

---

## Image Compression Strategy

### For Blazor Hybrid (.NET)
- **Library**: ImageSharp (cross-platform, no native dependencies)
- **Format**: WebP (best compression/quality ratio)
- **Thumbnail**: 150x150px, quality 75%
- **Full image**: Max 1024px, quality 85%

### For C23 CLI (Embedded)
- **Library**: stb_image (single-header, public domain)
- **Format**: Raw compressed (custom RLE or stored JPEG)
- **Memory mode**: Stream processing, no full image in RAM
- **Thumbnail only**: 64x64px, 4-bit grayscale for extreme constraints

### Compression Pipeline
```
Original Image
     │
     ▼
┌─────────────────┐
│ Import Process  │
│ (Automated)     │
└────────┬────────┘
         │
    ┌────┴────┐
    ▼         ▼
┌───────┐ ┌───────────┐
│ .NET  │ │ Embedded  │
│ WebP  │ │ Compressed│
│ ~50KB │ │ ~2-5KB    │
└───────┘ └───────────┘
```

---

## Database Schema

```sql
-- Parts table
CREATE TABLE parts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    part_number TEXT UNIQUE NOT NULL,
    name TEXT NOT NULL,
    description TEXT,
    category_id INTEGER,
    manufacturer TEXT,
    quantity INTEGER DEFAULT 0,
    unit_price REAL,
    location TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Categories table
CREATE TABLE categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    parent_id INTEGER,
    FOREIGN KEY (parent_id) REFERENCES categories(id)
);

-- Images table (separate for flexibility)
CREATE TABLE part_images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    part_id INTEGER NOT NULL,
    image_full BLOB,           -- WebP for .NET apps
    image_thumbnail BLOB,      -- WebP thumbnail
    image_embedded BLOB,       -- Compressed for C23/embedded
    original_filename TEXT,
    mime_type TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (part_id) REFERENCES parts(id) ON DELETE CASCADE
);

-- Full-text search
CREATE VIRTUAL TABLE parts_fts USING fts5(
    part_number, name, description, manufacturer,
    content='parts', content_rowid='id'
);
```

---

## API Design

### .NET Service Interface
```csharp
public interface IPartsService
{
    Task<IEnumerable<Part>> SearchPartsAsync(string query);
    Task<Part?> GetPartByIdAsync(int id);
    Task<Part?> GetPartByNumberAsync(string partNumber);
    Task<int> CreatePartAsync(Part part);
    Task UpdatePartAsync(Part part);
    Task DeletePartAsync(int id);
    Task ImportImageAsync(int partId, Stream imageStream, string filename);
    Task<byte[]?> GetImageAsync(int partId, ImageSize size);
}
```

### C23 API
```c
// Database operations
int partsdb_open(const char *path, partsdb_t **db);
void partsdb_close(partsdb_t *db);

// Part operations
int partsdb_search(partsdb_t *db, const char *query, part_t **results, size_t *count);
int partsdb_get_by_id(partsdb_t *db, int64_t id, part_t *part);
int partsdb_get_image_embedded(partsdb_t *db, int64_t part_id, uint8_t **data, size_t *size);

// Memory management
void partsdb_free_parts(part_t *parts, size_t count);
void partsdb_free_image(uint8_t *data);
```

---

## Automated Image Import

### Features
- Watch folder for new images
- Auto-detect part number from filename (e.g., `PN12345_front.jpg`)
- Generate all compression variants
- Update database automatically

### Process
```bash
./import-images --watch ./incoming --db ./parts.db
```

---

## Build Requirements

### .NET (Blazor Hybrid)
- .NET 8.0 SDK
- MAUI workload

### C23 CLI
- GCC 13+ or Clang 16+ (C23 support)
- SQLite3 development libraries
- Optional: ncurses for TUI

---

## Memory Considerations (Embedded)

| Component | RAM Usage |
|-----------|-----------|
| SQLite | ~256KB |
| Image buffer | ~16KB (64x64x4) |
| UI buffer | ~8KB |
| Query cache | ~32KB |
| **Total** | **~312KB** |

Target: Systems with 512KB+ RAM

