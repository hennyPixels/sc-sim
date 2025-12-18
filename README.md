# PartsDB - Cross-Platform Vehicle Parts Inventory System

A comprehensive parts database application with cross-platform GUI (Blazor Hybrid/.NET MAUI) and lightweight CLI (C23) for Linux/embedded systems. Designed for managing vehicle parts inventory with support for image imports from various sources.

---

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Building from Source](#building-from-source)
  - [.NET/C# Version](#netc-version)
  - [C23 CLI Version](#c23-cli-version)
- [Creating Executables](#creating-executables)
  - [.NET Portable Executable](#net-portable-executable)
  - [C Native Executable](#c-native-executable)
- [Running the Application](#running-the-application)
- [Importing Parts Data](#importing-parts-data)
  - [Using APIs](#using-apis)
  - [Manual Web Scraping](#manual-web-scraping)
- [Image Management](#image-management)
  - [Image Import Pipeline](#image-import-pipeline)
  - [ML-Based Image Generation](#ml-based-image-generation)
- [Configuration](#configuration)
- [Documentation](#documentation)
- [License](#license)

---

## Features

- **Cross-Platform GUI**: Runs on Windows, macOS, Linux, iOS, and Android
- **Lightweight CLI**: C23-based CLI for servers and embedded systems
- **SQLite Database**: Portable, single-file database with full-text search
- **Image Management**: Multi-resolution image storage with compression
- **Offline-First**: Works without internet connection
- **Import Tools**: API integrations and scraping utilities for parts data

---

## Quick Start

```bash
# Clone the repository
git clone https://github.com/hennyPixels/sc-sim.git
cd sc-sim

# Option 1: Run with .NET (requires .NET 10 SDK)
dotnet run --project src/PartsDb.Maui

# Option 2: Build and run C CLI (requires GCC 13+ and SQLite3)
cd src/partsdb-cli
make
./bin/partsdb
```

---

## Building from Source

### .NET/C# Version

#### Prerequisites

| Requirement | Version | Download |
|-------------|---------|----------|
| .NET SDK | 10.0+ | [dotnet.microsoft.com](https://dotnet.microsoft.com/download) |
| Visual Studio (optional) | 2022+ | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/) |
| VS Code (optional) | Latest | [code.visualstudio.com](https://code.visualstudio.com/) |

#### Install .NET SDK

**Windows (winget):**
```powershell
winget install Microsoft.DotNet.SDK.10
```

**macOS (Homebrew):**
```bash
brew install dotnet@10
```

**Linux (Ubuntu/Debian):**
```bash
# Add Microsoft package repository
wget https://packages.microsoft.com/config/ubuntu/24.04/packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
sudo apt update

# Install .NET SDK
sudo apt install dotnet-sdk-10.0
```

**Linux (Fedora):**
```bash
sudo dnf install dotnet-sdk-10.0
```

#### Build Commands

```bash
# Restore dependencies
dotnet restore

# Build all projects (Debug)
dotnet build

# Build for Release
dotnet build -c Release

# Run tests
dotnet test

# Run the application
dotnet run --project src/PartsDb.Maui
```

#### Build Specific Platforms

```bash
# Windows
dotnet build src/PartsDb.Maui -f net10.0-windows10.0.19041.0

# macOS
dotnet build src/PartsDb.Maui -f net10.0-maccatalyst

# Android
dotnet build src/PartsDb.Maui -f net10.0-android

# iOS (requires Mac with Xcode)
dotnet build src/PartsDb.Maui -f net10.0-ios
```

---

### C23 CLI Version

#### Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| GCC | 13+ | C23 support required |
| Clang | 16+ | Alternative compiler |
| SQLite3 | 3.35+ | Development libraries |
| Make | Any | Build system |

#### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential gcc-13 libsqlite3-dev
```

**Fedora:**
```bash
sudo dnf install gcc sqlite-devel make
```

**macOS:**
```bash
brew install gcc sqlite3
```

**Arch Linux:**
```bash
sudo pacman -S gcc sqlite make
```

#### Build Commands

```bash
cd src/partsdb-cli

# Standard build (optimized)
make

# Debug build (with sanitizers)
make debug

# Embedded build (minimal size)
make embedded

# Clean build artifacts
make clean

# Show help
make help
```

#### Build Options

```bash
# Use specific compiler
CC=clang make

# Custom SQLite path
CFLAGS="-I/opt/sqlite/include" LDFLAGS="-L/opt/sqlite/lib" make

# Cross-compile for ARM
CC=arm-linux-gnueabihf-gcc make embedded
```

---

## Creating Executables

### .NET Portable Executable

#### Single-File Executable (Recommended)

Creates a single executable with all dependencies bundled:

```bash
# Windows x64
dotnet publish src/PartsDb.Maui -c Release \
  -r win-x64 \
  --self-contained true \
  -p:PublishSingleFile=true \
  -p:IncludeNativeLibrariesForSelfExtract=true \
  -o ./publish/win-x64

# Linux x64
dotnet publish src/PartsDb.Maui -c Release \
  -r linux-x64 \
  --self-contained true \
  -p:PublishSingleFile=true \
  -o ./publish/linux-x64

# macOS x64
dotnet publish src/PartsDb.Maui -c Release \
  -r osx-x64 \
  --self-contained true \
  -p:PublishSingleFile=true \
  -o ./publish/osx-x64

# macOS ARM64 (Apple Silicon)
dotnet publish src/PartsDb.Maui -c Release \
  -r osx-arm64 \
  --self-contained true \
  -p:PublishSingleFile=true \
  -o ./publish/osx-arm64
```

#### Trimmed Executable (Smaller Size)

```bash
# Trimmed single-file (removes unused code)
dotnet publish src/PartsDb.Maui -c Release \
  -r win-x64 \
  --self-contained true \
  -p:PublishSingleFile=true \
  -p:PublishTrimmed=true \
  -p:TrimMode=link \
  -o ./publish/win-x64-trimmed
```

#### Framework-Dependent (Requires .NET Runtime)

```bash
# Smaller package, requires .NET runtime installed
dotnet publish src/PartsDb.Maui -c Release \
  --self-contained false \
  -o ./publish/framework-dependent
```

#### Output Locations

| Platform | Output Path | Executable |
|----------|-------------|------------|
| Windows x64 | `./publish/win-x64/` | `PartsDb.Maui.exe` |
| Linux x64 | `./publish/linux-x64/` | `PartsDb.Maui` |
| macOS x64 | `./publish/osx-x64/` | `PartsDb.Maui` |
| macOS ARM64 | `./publish/osx-arm64/` | `PartsDb.Maui` |

---

### C Native Executable

#### Standard Build

```bash
cd src/partsdb-cli

# Release build
make clean && make
# Output: bin/partsdb

# Check the binary
file bin/partsdb
ldd bin/partsdb  # Show dependencies
```

#### Static Build (No Dependencies)

```bash
# Fully static binary (no shared libraries needed)
make embedded

# Output: bin/partsdb (statically linked)
ls -lh bin/partsdb  # Check size
```

#### Cross-Compilation

**For ARM (Raspberry Pi, embedded):**
```bash
# Install cross-compiler
sudo apt install gcc-arm-linux-gnueabihf

# Build for ARM
CC=arm-linux-gnueabihf-gcc make embedded
```

**For ARM64:**
```bash
sudo apt install gcc-aarch64-linux-gnu
CC=aarch64-linux-gnu-gcc make embedded
```

**For Windows (MinGW):**
```bash
sudo apt install mingw-w64
CC=x86_64-w64-mingw32-gcc make
# Output: bin/partsdb.exe
```

#### Installation

```bash
# Install to system (requires root)
sudo make install
# Installs to /usr/local/bin/partsdb

# Install to custom location
DESTDIR=/opt/partsdb make install
```

---

## Running the Application

### .NET GUI Application

```bash
# From source
dotnet run --project src/PartsDb.Maui

# From published executable
./publish/linux-x64/PartsDb.Maui

# With custom database path
./PartsDb.Maui --database /path/to/parts.db
```

### C CLI Application

```bash
# Interactive mode (default database: ./parts.db)
./bin/partsdb

# Specify database path
./bin/partsdb -d /path/to/inventory.db

# Show help
./bin/partsdb --help
```

#### CLI Commands

```
partsdb> help

Commands:
  list [limit]       - List all parts
  search <query>     - Search parts by name/number/mfr
  show <id|number>   - Show part details
  add                - Add new part (interactive)
  qty <id> <+/-n>    - Adjust quantity
  low                - Show low stock items
  cat                - List categories
  img <id>           - Show part image (ASCII)
  help               - Show this help
  quit               - Exit program
```

---

## Importing Parts Data

PartsDB supports importing vehicle parts data from various online sources through APIs and web scraping.

### Using APIs

#### Supported API Sources

| Source | API Type | Coverage | Rate Limit |
|--------|----------|----------|------------|
| [NHTSA vPIC](https://vpic.nhtsa.dot.gov/api/) | REST (Free) | US Vehicles | 5 req/sec |
| [PartsLink24](https://www.partslink24.com/) | REST (Commercial) | OEM Parts | Varies |
| [TecDoc](https://www.tecalliance.net/) | SOAP/REST (Commercial) | Aftermarket | Varies |
| [AutoZone API](https://www.autozone.com/) | REST (Partner) | Retail Parts | Partner only |
| [RockAuto](https://www.rockauto.com/) | Unofficial | Aftermarket | Scraping only |

#### NHTSA API Integration (Free)

The NHTSA Vehicle Product Information Catalog (vPIC) provides free access to vehicle and parts data.

**Setup:**
```bash
# No API key required for NHTSA

# Create import script
cat > tools/import-nhtsa.py << 'EOF'
#!/usr/bin/env python3
"""
NHTSA vPIC API Importer
Imports vehicle and parts data from the NHTSA database.
"""

import requests
import sqlite3
import json
import time
from pathlib import Path

NHTSA_BASE_URL = "https://vpic.nhtsa.dot.gov/api/vehicles"

def get_vehicle_parts(make: str, model: str, year: int) -> list:
    """Fetch parts data for a specific vehicle."""
    # Get vehicle ID
    url = f"{NHTSA_BASE_URL}/GetModelsForMakeYear/make/{make}/modelyear/{year}?format=json"
    response = requests.get(url)
    response.raise_for_status()

    data = response.json()
    return data.get("Results", [])

def get_parts_by_type(part_type: str) -> list:
    """Fetch parts by type from NHTSA."""
    url = f"{NHTSA_BASE_URL}/GetParts?type={part_type}&format=json"
    response = requests.get(url)
    response.raise_for_status()

    return response.json().get("Results", [])

def import_to_database(parts: list, db_path: str):
    """Import parts into PartsDB SQLite database."""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    for part in parts:
        cursor.execute("""
            INSERT OR IGNORE INTO parts
            (part_number, name, description, manufacturer)
            VALUES (?, ?, ?, ?)
        """, (
            part.get("PartNumber", ""),
            part.get("Name", ""),
            part.get("Description", ""),
            part.get("Manufacturer", "")
        ))

    conn.commit()
    conn.close()
    print(f"Imported {len(parts)} parts")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Import NHTSA parts data")
    parser.add_argument("--make", help="Vehicle make (e.g., Ford)")
    parser.add_argument("--model", help="Vehicle model (e.g., F-150)")
    parser.add_argument("--year", type=int, help="Vehicle year")
    parser.add_argument("--db", default="parts.db", help="Database path")
    args = parser.parse_args()

    parts = get_vehicle_parts(args.make, args.model, args.year)
    import_to_database(parts, args.db)
EOF

chmod +x tools/import-nhtsa.py
```

**Usage:**
```bash
# Import parts for a specific vehicle
python3 tools/import-nhtsa.py --make Ford --model F-150 --year 2023 --db parts.db

# Import all brake parts
python3 tools/import-nhtsa.py --type brake --db parts.db
```

#### TecDoc API Integration (Commercial)

TecDoc provides comprehensive aftermarket parts data.

**Setup:**
```bash
cat > tools/import-tecdoc.py << 'EOF'
#!/usr/bin/env python3
"""
TecDoc API Importer
Requires commercial API access from TecAlliance.
"""

import requests
import sqlite3
import os

TECDOC_API_URL = "https://webservice.tecalliance.services/pegasus-3-0/services"

class TecDocClient:
    def __init__(self, api_key: str, provider_id: int):
        self.api_key = api_key
        self.provider_id = provider_id
        self.session = requests.Session()
        self.session.headers.update({
            "X-API-Key": api_key,
            "Content-Type": "application/json"
        })

    def search_articles(self, search_term: str, page: int = 1) -> dict:
        """Search for parts/articles."""
        payload = {
            "articleCountry": "US",
            "lang": "en",
            "provider": self.provider_id,
            "searchQuery": search_term,
            "searchType": 0,
            "page": page,
            "perPage": 100
        }

        response = self.session.post(
            f"{TECDOC_API_URL}/article/search",
            json=payload
        )
        response.raise_for_status()
        return response.json()

    def get_article_details(self, article_id: int) -> dict:
        """Get detailed article information including images."""
        payload = {
            "articleCountry": "US",
            "lang": "en",
            "provider": self.provider_id,
            "articleId": article_id,
            "includeImages": True,
            "includeAttributes": True
        }

        response = self.session.post(
            f"{TECDOC_API_URL}/article/details",
            json=payload
        )
        response.raise_for_status()
        return response.json()

    def download_image(self, image_url: str, save_path: str):
        """Download part image."""
        response = self.session.get(image_url)
        response.raise_for_status()

        with open(save_path, "wb") as f:
            f.write(response.content)

def import_tecdoc_parts(client: TecDocClient, search_term: str, db_path: str):
    """Import parts from TecDoc into database."""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    results = client.search_articles(search_term)

    for article in results.get("articles", []):
        # Get full details
        details = client.get_article_details(article["articleId"])

        # Insert part
        cursor.execute("""
            INSERT OR REPLACE INTO parts
            (part_number, name, description, manufacturer, datasheet_url)
            VALUES (?, ?, ?, ?, ?)
        """, (
            details.get("articleNumber"),
            details.get("genericArticleDescription"),
            details.get("articleText"),
            details.get("brandName"),
            details.get("datasheetUrl")
        ))

        part_id = cursor.lastrowid

        # Download and save images
        for img in details.get("images", []):
            img_path = f"images/{part_id}_{img['imageId']}.jpg"
            client.download_image(img["imageUrl"], img_path)

            # TODO: Process image and store in database

    conn.commit()
    conn.close()

if __name__ == "__main__":
    api_key = os.environ.get("TECDOC_API_KEY")
    provider_id = int(os.environ.get("TECDOC_PROVIDER_ID", 0))

    if not api_key:
        print("Error: Set TECDOC_API_KEY environment variable")
        exit(1)

    client = TecDocClient(api_key, provider_id)
    import_tecdoc_parts(client, "brake pad", "parts.db")
EOF
```

**Usage:**
```bash
# Set credentials
export TECDOC_API_KEY="your-api-key"
export TECDOC_PROVIDER_ID="your-provider-id"

# Import brake pads
python3 tools/import-tecdoc.py --search "brake pad" --db parts.db
```

---

### Manual Web Scraping

For sources without APIs, you can use web scraping. **Always respect robots.txt and terms of service.**

#### Scraping Setup

```bash
# Install scraping dependencies
pip install requests beautifulsoup4 selenium playwright lxml

# Install Playwright browsers
playwright install chromium
```

#### RockAuto Scraper Example

```bash
cat > tools/scrape-rockauto.py << 'EOF'
#!/usr/bin/env python3
"""
RockAuto Parts Scraper
Scrapes parts data and images from RockAuto.com

WARNING: Check robots.txt and ToS before scraping.
Use responsibly with rate limiting.
"""

import requests
from bs4 import BeautifulSoup
import sqlite3
import time
import os
import re
from urllib.parse import urljoin
from pathlib import Path

class RockAutoScraper:
    BASE_URL = "https://www.rockauto.com"

    def __init__(self, db_path: str, image_dir: str = "images"):
        self.session = requests.Session()
        self.session.headers.update({
            "User-Agent": "Mozilla/5.0 (compatible; PartsDB/1.0; +https://github.com/yourrepo)",
            "Accept": "text/html,application/xhtml+xml",
            "Accept-Language": "en-US,en;q=0.9"
        })
        self.db_path = db_path
        self.image_dir = Path(image_dir)
        self.image_dir.mkdir(exist_ok=True)
        self.rate_limit = 2.0  # Seconds between requests

    def _request(self, url: str) -> BeautifulSoup:
        """Make rate-limited request and parse HTML."""
        time.sleep(self.rate_limit)  # Respect the server

        response = self.session.get(url)
        response.raise_for_status()

        return BeautifulSoup(response.text, "lxml")

    def search_parts(self, make: str, model: str, year: int, category: str) -> list:
        """Search for parts by vehicle and category."""
        # Build search URL (structure varies by site)
        search_url = f"{self.BASE_URL}/catalog/{make},{model},{year},{category}"

        soup = self._request(search_url)
        parts = []

        # Find part listings (selector depends on site structure)
        for listing in soup.select(".listing-container .part-row"):
            part = {
                "part_number": listing.select_one(".part-number").text.strip(),
                "name": listing.select_one(".part-name").text.strip(),
                "manufacturer": listing.select_one(".brand-name").text.strip(),
                "price": self._parse_price(listing.select_one(".price")),
                "image_url": listing.select_one("img").get("src"),
                "detail_url": listing.select_one("a").get("href")
            }
            parts.append(part)

        return parts

    def get_part_details(self, detail_url: str) -> dict:
        """Get detailed part information."""
        soup = self._request(urljoin(self.BASE_URL, detail_url))

        details = {
            "description": "",
            "specifications": {},
            "images": [],
            "fitment": []
        }

        # Extract description
        desc_elem = soup.select_one(".part-description")
        if desc_elem:
            details["description"] = desc_elem.text.strip()

        # Extract specifications
        for spec_row in soup.select(".spec-table tr"):
            key = spec_row.select_one("th")
            value = spec_row.select_one("td")
            if key and value:
                details["specifications"][key.text.strip()] = value.text.strip()

        # Extract all images
        for img in soup.select(".part-images img"):
            img_url = img.get("data-full") or img.get("src")
            if img_url:
                details["images"].append(urljoin(self.BASE_URL, img_url))

        # Extract fitment/compatibility
        for fitment in soup.select(".fitment-list li"):
            details["fitment"].append(fitment.text.strip())

        return details

    def download_image(self, url: str, part_number: str, index: int = 0) -> str:
        """Download and save part image."""
        time.sleep(self.rate_limit)

        response = self.session.get(url)
        response.raise_for_status()

        # Determine file extension
        content_type = response.headers.get("content-type", "")
        ext = ".jpg"
        if "png" in content_type:
            ext = ".png"
        elif "webp" in content_type:
            ext = ".webp"

        # Save image
        filename = f"{part_number}_{index}{ext}"
        filepath = self.image_dir / filename

        with open(filepath, "wb") as f:
            f.write(response.content)

        return str(filepath)

    def _parse_price(self, elem) -> float:
        """Parse price from element."""
        if not elem:
            return 0.0
        text = elem.text.strip()
        match = re.search(r"\$?([\d,]+\.?\d*)", text)
        if match:
            return float(match.group(1).replace(",", ""))
        return 0.0

    def import_to_database(self, parts: list):
        """Import scraped parts to database."""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()

        for part in parts:
            # Insert part
            cursor.execute("""
                INSERT OR REPLACE INTO parts
                (part_number, name, description, manufacturer, unit_price)
                VALUES (?, ?, ?, ?, ?)
            """, (
                part["part_number"],
                part["name"],
                part.get("description", ""),
                part["manufacturer"],
                part.get("price", 0)
            ))

            part_id = cursor.lastrowid

            # Download and store images
            for i, img_url in enumerate(part.get("images", [])):
                try:
                    local_path = self.download_image(
                        img_url,
                        part["part_number"],
                        i
                    )

                    # Read image and store in database
                    with open(local_path, "rb") as f:
                        image_data = f.read()

                    cursor.execute("""
                        INSERT INTO part_images
                        (part_id, image_full, original_filename, is_primary)
                        VALUES (?, ?, ?, ?)
                    """, (part_id, image_data, local_path, i == 0))

                except Exception as e:
                    print(f"Error downloading image: {e}")

        conn.commit()
        conn.close()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Scrape parts from RockAuto")
    parser.add_argument("--make", required=True, help="Vehicle make")
    parser.add_argument("--model", required=True, help="Vehicle model")
    parser.add_argument("--year", type=int, required=True, help="Vehicle year")
    parser.add_argument("--category", required=True, help="Part category")
    parser.add_argument("--db", default="parts.db", help="Database path")
    args = parser.parse_args()

    scraper = RockAutoScraper(args.db)
    parts = scraper.search_parts(args.make, args.model, args.year, args.category)

    print(f"Found {len(parts)} parts")

    # Get details for each part
    for part in parts:
        if part.get("detail_url"):
            details = scraper.get_part_details(part["detail_url"])
            part.update(details)

    scraper.import_to_database(parts)
    print("Import complete!")
EOF

chmod +x tools/scrape-rockauto.py
```

#### eBay Motors Scraper

```bash
cat > tools/scrape-ebay.py << 'EOF'
#!/usr/bin/env python3
"""
eBay Motors Parts Scraper
Uses Playwright for JavaScript-rendered content.
"""

import asyncio
from playwright.async_api import async_playwright
import sqlite3
import json
from pathlib import Path

class EbayMotorsScraper:
    BASE_URL = "https://www.ebay.com/b/Auto-Parts-and-Vehicles/6000/bn_1865334"

    def __init__(self, db_path: str):
        self.db_path = db_path

    async def search_parts(self, query: str, max_pages: int = 5) -> list:
        """Search eBay Motors for parts."""
        parts = []

        async with async_playwright() as p:
            browser = await p.chromium.launch(headless=True)
            page = await browser.new_page()

            # Set realistic viewport and user agent
            await page.set_viewport_size({"width": 1920, "height": 1080})

            for page_num in range(1, max_pages + 1):
                search_url = (
                    f"https://www.ebay.com/sch/i.html?"
                    f"_nkw={query.replace(' ', '+')}&"
                    f"_sacat=6000&"  # Auto Parts category
                    f"_pgn={page_num}"
                )

                await page.goto(search_url, wait_until="networkidle")
                await asyncio.sleep(2)  # Rate limiting

                # Extract listings
                listings = await page.query_selector_all(".s-item")

                for listing in listings:
                    try:
                        title_elem = await listing.query_selector(".s-item__title")
                        price_elem = await listing.query_selector(".s-item__price")
                        img_elem = await listing.query_selector(".s-item__image-img")
                        link_elem = await listing.query_selector(".s-item__link")

                        if title_elem:
                            part = {
                                "name": await title_elem.inner_text(),
                                "price": await price_elem.inner_text() if price_elem else "",
                                "image_url": await img_elem.get_attribute("src") if img_elem else "",
                                "listing_url": await link_elem.get_attribute("href") if link_elem else ""
                            }

                            # Extract part number from title if present
                            part["part_number"] = self._extract_part_number(part["name"])

                            parts.append(part)
                    except Exception as e:
                        print(f"Error parsing listing: {e}")

                print(f"Page {page_num}: Found {len(parts)} parts total")

            await browser.close()

        return parts

    def _extract_part_number(self, title: str) -> str:
        """Extract part number from listing title."""
        import re
        # Common patterns: OEM, PN:, Part#, etc.
        patterns = [
            r"OEM[#:\s]*([A-Z0-9-]+)",
            r"P/?N[#:\s]*([A-Z0-9-]+)",
            r"Part\s*#?\s*([A-Z0-9-]+)",
            r"#\s*([A-Z0-9-]+)"
        ]

        for pattern in patterns:
            match = re.search(pattern, title, re.IGNORECASE)
            if match:
                return match.group(1)

        return ""

    def import_to_database(self, parts: list):
        """Import parts to SQLite database."""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()

        for part in parts:
            cursor.execute("""
                INSERT OR IGNORE INTO parts
                (part_number, name, notes)
                VALUES (?, ?, ?)
            """, (
                part.get("part_number", ""),
                part["name"],
                f"Source: eBay | Price: {part.get('price', 'N/A')}"
            ))

        conn.commit()
        conn.close()

async def main():
    import argparse

    parser = argparse.ArgumentParser(description="Scrape parts from eBay Motors")
    parser.add_argument("--query", required=True, help="Search query")
    parser.add_argument("--pages", type=int, default=3, help="Max pages to scrape")
    parser.add_argument("--db", default="parts.db", help="Database path")
    args = parser.parse_args()

    scraper = EbayMotorsScraper(args.db)
    parts = await scraper.search_parts(args.query, args.pages)

    print(f"Found {len(parts)} parts")
    scraper.import_to_database(parts)
    print("Import complete!")

if __name__ == "__main__":
    asyncio.run(main())
EOF

chmod +x tools/scrape-ebay.py
```

**Usage:**
```bash
# Scrape brake pads from eBay
python3 tools/scrape-ebay.py --query "brake pads F150" --pages 3 --db parts.db
```

---

## Image Management

### Image Import Pipeline

PartsDB uses a multi-resolution image pipeline optimized for different use cases:

```
┌─────────────────┐     ┌──────────────────┐     ┌────────────────────┐
│  Original Image │ ──► │  Image Processor │ ──► │  Database Storage  │
│   (Any format)  │     │                  │     │                    │
└─────────────────┘     │  1. Full: WebP   │     │  image_full (BLOB) │
                        │     800x800 max  │     │  image_thumbnail   │
                        │  2. Thumbnail    │     │  image_embedded    │
                        │     200x200      │     │                    │
                        │  3. Embedded     │     └────────────────────┘
                        │     64x64 4-bit  │
                        └──────────────────┘
```

#### Batch Image Import Tool

```bash
cat > tools/import-images.py << 'EOF'
#!/usr/bin/env python3
"""
Batch Image Import Tool
Processes and imports images into PartsDB.
"""

import os
import sys
import sqlite3
import argparse
from pathlib import Path
from io import BytesIO

try:
    from PIL import Image
except ImportError:
    print("Installing Pillow...")
    os.system(f"{sys.executable} -m pip install Pillow")
    from PIL import Image

class ImageProcessor:
    # Size configurations
    FULL_SIZE = (800, 800)
    THUMBNAIL_SIZE = (200, 200)
    EMBEDDED_SIZE = (64, 64)

    # Quality settings
    WEBP_QUALITY = 85

    def __init__(self, db_path: str):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)

    def process_image(self, image_path: str) -> dict:
        """Process image into multiple resolutions."""
        img = Image.open(image_path)

        # Convert to RGB if necessary
        if img.mode in ("RGBA", "P"):
            img = img.convert("RGB")

        results = {}

        # Full-size WebP
        full = img.copy()
        full.thumbnail(self.FULL_SIZE, Image.Resampling.LANCZOS)
        full_buffer = BytesIO()
        full.save(full_buffer, format="WebP", quality=self.WEBP_QUALITY)
        results["full"] = full_buffer.getvalue()

        # Thumbnail
        thumb = img.copy()
        thumb.thumbnail(self.THUMBNAIL_SIZE, Image.Resampling.LANCZOS)
        thumb_buffer = BytesIO()
        thumb.save(thumb_buffer, format="WebP", quality=80)
        results["thumbnail"] = thumb_buffer.getvalue()

        # Embedded (4-bit grayscale for CLI)
        embedded = img.copy()
        embedded = embedded.convert("L")  # Grayscale
        embedded.thumbnail(self.EMBEDDED_SIZE, Image.Resampling.LANCZOS)
        embedded_buffer = BytesIO()
        # Use PNG for embedded (smaller for small images)
        embedded.save(embedded_buffer, format="PNG", optimize=True)
        results["embedded"] = embedded_buffer.getvalue()

        # Metadata
        results["width"] = full.width
        results["height"] = full.height
        results["file_size"] = len(results["full"])

        return results

    def import_for_part(self, part_id: int, image_path: str, is_primary: bool = False):
        """Import an image for a specific part."""
        processed = self.process_image(image_path)

        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO part_images
            (part_id, image_full, image_thumbnail, image_embedded,
             original_filename, mime_type, width, height, file_size, is_primary)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            part_id,
            processed["full"],
            processed["thumbnail"],
            processed["embedded"],
            Path(image_path).name,
            "image/webp",
            processed["width"],
            processed["height"],
            processed["file_size"],
            1 if is_primary else 0
        ))

        self.conn.commit()
        return cursor.lastrowid

    def batch_import(self, directory: str, match_by: str = "filename"):
        """
        Batch import images from a directory.

        match_by options:
        - "filename": Match image filename to part_number
        - "directory": Each subdirectory is a part_number
        """
        image_dir = Path(directory)
        imported = 0
        errors = []

        cursor = self.conn.cursor()

        if match_by == "filename":
            # Files named like: ABC123.jpg, ABC123_1.jpg, ABC123_2.png
            for img_path in image_dir.glob("*.*"):
                if img_path.suffix.lower() not in (".jpg", ".jpeg", ".png", ".webp", ".gif"):
                    continue

                # Extract part number from filename
                part_number = img_path.stem.split("_")[0]

                # Find part in database
                cursor.execute(
                    "SELECT id FROM parts WHERE part_number = ?",
                    (part_number,)
                )
                row = cursor.fetchone()

                if row:
                    try:
                        # Check if this is the first image (make it primary)
                        cursor.execute(
                            "SELECT COUNT(*) FROM part_images WHERE part_id = ?",
                            (row[0],)
                        )
                        is_primary = cursor.fetchone()[0] == 0

                        self.import_for_part(row[0], str(img_path), is_primary)
                        imported += 1
                        print(f"Imported: {img_path.name} -> Part #{row[0]}")
                    except Exception as e:
                        errors.append((str(img_path), str(e)))
                else:
                    errors.append((str(img_path), f"Part '{part_number}' not found"))

        elif match_by == "directory":
            # Directories named by part number containing images
            for part_dir in image_dir.iterdir():
                if not part_dir.is_dir():
                    continue

                part_number = part_dir.name

                cursor.execute(
                    "SELECT id FROM parts WHERE part_number = ?",
                    (part_number,)
                )
                row = cursor.fetchone()

                if row:
                    for i, img_path in enumerate(sorted(part_dir.glob("*.*"))):
                        if img_path.suffix.lower() not in (".jpg", ".jpeg", ".png", ".webp"):
                            continue

                        try:
                            self.import_for_part(row[0], str(img_path), i == 0)
                            imported += 1
                        except Exception as e:
                            errors.append((str(img_path), str(e)))
                else:
                    errors.append((str(part_dir), f"Part '{part_number}' not found"))

        # Report results
        print(f"\nImport complete: {imported} images imported")
        if errors:
            print(f"Errors ({len(errors)}):")
            for path, error in errors[:10]:
                print(f"  - {path}: {error}")
            if len(errors) > 10:
                print(f"  ... and {len(errors) - 10} more")

    def close(self):
        self.conn.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Import images into PartsDB")
    parser.add_argument("--db", required=True, help="Database path")
    parser.add_argument("--source", required=True, help="Source directory")
    parser.add_argument("--match", choices=["filename", "directory"],
                        default="filename", help="Matching strategy")
    parser.add_argument("--part-id", type=int, help="Import single image to specific part")
    parser.add_argument("--image", help="Single image path (use with --part-id)")

    args = parser.parse_args()

    processor = ImageProcessor(args.db)

    if args.part_id and args.image:
        # Single image import
        processor.import_for_part(args.part_id, args.image, is_primary=True)
        print(f"Imported {args.image} to part {args.part_id}")
    else:
        # Batch import
        processor.batch_import(args.source, args.match)

    processor.close()
EOF

chmod +x tools/import-images.py
```

**Usage:**
```bash
# Single image import
python3 tools/import-images.py --db parts.db --part-id 42 --image ./brake_pad.jpg

# Batch import (filename matching)
# Images named: ABC123.jpg, DEF456.png, etc.
python3 tools/import-images.py --db parts.db --source ./images/ --match filename

# Batch import (directory matching)
# Directory structure: ./images/ABC123/photo1.jpg, ./images/DEF456/photo1.jpg
python3 tools/import-images.py --db parts.db --source ./images/ --match directory
```

---

### ML-Based Image Generation

When physical part images aren't available, you can generate images using machine learning models or CAD blueprints.

#### Option 1: Stable Diffusion for Part Images

```bash
cat > tools/generate-images-sd.py << 'EOF'
#!/usr/bin/env python3
"""
Stable Diffusion Part Image Generator
Generates realistic part images from text descriptions.

Requirements:
- CUDA-capable GPU with 8GB+ VRAM
- pip install torch diffusers transformers accelerate
"""

import torch
from diffusers import StableDiffusionPipeline, DPMSolverMultistepScheduler
from pathlib import Path
import sqlite3
import argparse

class PartImageGenerator:
    def __init__(self, model_id: str = "stabilityai/stable-diffusion-2-1"):
        """Initialize Stable Diffusion pipeline."""
        print("Loading Stable Diffusion model...")

        self.pipe = StableDiffusionPipeline.from_pretrained(
            model_id,
            torch_dtype=torch.float16,
            safety_checker=None  # Disable for non-human content
        )

        # Use faster scheduler
        self.pipe.scheduler = DPMSolverMultistepScheduler.from_config(
            self.pipe.scheduler.config
        )

        # Move to GPU
        self.pipe = self.pipe.to("cuda")

        # Enable memory optimizations
        self.pipe.enable_attention_slicing()

    def generate_part_image(
        self,
        part_name: str,
        part_type: str = "automotive part",
        style: str = "product photography"
    ) -> bytes:
        """Generate an image for a part."""

        # Craft detailed prompt for realistic part images
        prompt = (
            f"Professional {style} of a {part_name}, "
            f"{part_type}, high detail, studio lighting, "
            f"white background, centered composition, "
            f"8k resolution, photorealistic, sharp focus"
        )

        negative_prompt = (
            "blurry, low quality, distorted, text, watermark, "
            "logo, human, person, hand, artistic, painting, "
            "cartoon, anime, sketch, drawing"
        )

        # Generate image
        image = self.pipe(
            prompt=prompt,
            negative_prompt=negative_prompt,
            num_inference_steps=30,
            guidance_scale=7.5,
            width=768,
            height=768
        ).images[0]

        # Convert to bytes
        from io import BytesIO
        buffer = BytesIO()
        image.save(buffer, format="PNG")
        return buffer.getvalue()

    def generate_for_database(self, db_path: str, limit: int = None):
        """Generate images for parts without images."""
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()

        # Find parts without images
        query = """
            SELECT p.id, p.name, p.description, c.name as category
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.id NOT IN (SELECT DISTINCT part_id FROM part_images)
        """
        if limit:
            query += f" LIMIT {limit}"

        cursor.execute(query)
        parts = cursor.fetchall()

        print(f"Found {len(parts)} parts without images")

        for part_id, name, description, category in parts:
            try:
                print(f"Generating image for: {name}")

                # Determine part type from category
                part_type = category or "automotive part"

                image_data = self.generate_part_image(name, part_type)

                # Save to database
                cursor.execute("""
                    INSERT INTO part_images
                    (part_id, image_full, mime_type, is_primary)
                    VALUES (?, ?, 'image/png', 1)
                """, (part_id, image_data))

                conn.commit()
                print(f"  Saved image for part #{part_id}")

            except Exception as e:
                print(f"  Error: {e}")

        conn.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate part images with AI")
    parser.add_argument("--db", required=True, help="Database path")
    parser.add_argument("--limit", type=int, help="Max parts to process")
    parser.add_argument("--single", help="Generate for single part name")
    parser.add_argument("--output", help="Output path for single image")

    args = parser.parse_args()

    generator = PartImageGenerator()

    if args.single:
        image_data = generator.generate_part_image(args.single)
        output_path = args.output or f"{args.single.replace(' ', '_')}.png"
        with open(output_path, "wb") as f:
            f.write(image_data)
        print(f"Saved: {output_path}")
    else:
        generator.generate_for_database(args.db, args.limit)
EOF

chmod +x tools/generate-images-sd.py
```

**Usage:**
```bash
# Generate single test image
python3 tools/generate-images-sd.py --single "brake caliper" --output brake_caliper.png

# Generate for all parts without images
python3 tools/generate-images-sd.py --db parts.db --limit 50
```

#### Option 2: CAD Blueprint Rendering

```bash
cat > tools/render-blueprint.py << 'EOF'
#!/usr/bin/env python3
"""
CAD Blueprint Renderer
Generates technical drawings from parametric descriptions.

Uses matplotlib for 2D technical drawings and blueprints.
For 3D rendering, integrate with FreeCAD or OpenSCAD.
"""

import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.patches import FancyBboxPatch, Circle, Rectangle
import numpy as np
from io import BytesIO
import sqlite3

class BlueprintRenderer:
    """Renders technical blueprint-style drawings."""

    # Blueprint color scheme
    COLORS = {
        "background": "#1a237e",  # Dark blue
        "lines": "#ffffff",       # White
        "dimensions": "#ffeb3b",  # Yellow
        "text": "#ffffff",
        "grid": "#283593"
    }

    def __init__(self, width: int = 800, height: int = 600):
        self.width = width
        self.height = height

    def render_bolt(
        self,
        head_diameter: float,
        head_height: float,
        thread_diameter: float,
        thread_length: float,
        thread_pitch: float = 1.0
    ) -> bytes:
        """Render a bolt technical drawing."""
        fig, ax = plt.subplots(1, 1, figsize=(10, 8), facecolor=self.COLORS["background"])
        ax.set_facecolor(self.COLORS["background"])

        # Remove axes
        ax.set_xlim(-2, thread_length + head_height + 2)
        ax.set_ylim(-head_diameter, head_diameter)
        ax.set_aspect('equal')
        ax.axis('off')

        # Draw bolt head (hexagonal)
        head_center = (head_height / 2, 0)
        hexagon = patches.RegularPolygon(
            head_center, 6, radius=head_diameter/2,
            orientation=np.pi/6,
            fill=False,
            edgecolor=self.COLORS["lines"],
            linewidth=2
        )
        ax.add_patch(hexagon)

        # Draw thread shaft
        shaft_left = head_height
        shaft_right = head_height + thread_length

        # Outer profile
        ax.plot(
            [shaft_left, shaft_right],
            [thread_diameter/2, thread_diameter/2],
            color=self.COLORS["lines"], linewidth=2
        )
        ax.plot(
            [shaft_left, shaft_right],
            [-thread_diameter/2, -thread_diameter/2],
            color=self.COLORS["lines"], linewidth=2
        )

        # Thread lines
        num_threads = int(thread_length / thread_pitch)
        for i in range(num_threads):
            x = shaft_left + i * thread_pitch
            ax.plot(
                [x, x + thread_pitch/2],
                [thread_diameter/2, thread_diameter/2 - thread_pitch/4],
                color=self.COLORS["lines"], linewidth=1
            )
            ax.plot(
                [x, x + thread_pitch/2],
                [-thread_diameter/2, -thread_diameter/2 + thread_pitch/4],
                color=self.COLORS["lines"], linewidth=1
            )

        # Dimension lines
        self._add_dimension(ax, 0, shaft_right, -head_diameter*0.8,
                          f"{thread_length + head_height:.1f}mm")
        self._add_dimension(ax, shaft_left, shaft_right, head_diameter*0.8,
                          f"{thread_length:.1f}mm")

        # Title
        ax.text(
            shaft_left + thread_length/2, -head_diameter*1.2,
            f"BOLT M{thread_diameter:.0f}x{thread_length:.0f}",
            color=self.COLORS["text"],
            fontsize=14,
            fontweight='bold',
            ha='center'
        )

        # Save to bytes
        buffer = BytesIO()
        fig.savefig(buffer, format='png', dpi=100,
                   facecolor=self.COLORS["background"],
                   bbox_inches='tight', pad_inches=0.5)
        plt.close(fig)

        return buffer.getvalue()

    def render_bearing(
        self,
        outer_diameter: float,
        inner_diameter: float,
        width: float
    ) -> bytes:
        """Render a bearing technical drawing (cross-section)."""
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6),
                                        facecolor=self.COLORS["background"])

        for ax in [ax1, ax2]:
            ax.set_facecolor(self.COLORS["background"])
            ax.set_aspect('equal')
            ax.axis('off')

        # Front view (circular)
        ax1.set_xlim(-outer_diameter*0.7, outer_diameter*0.7)
        ax1.set_ylim(-outer_diameter*0.7, outer_diameter*0.7)

        # Outer ring
        outer = Circle((0, 0), outer_diameter/2, fill=False,
                       edgecolor=self.COLORS["lines"], linewidth=2)
        ax1.add_patch(outer)

        # Inner ring
        inner = Circle((0, 0), inner_diameter/2, fill=False,
                       edgecolor=self.COLORS["lines"], linewidth=2)
        ax1.add_patch(inner)

        # Ball bearings
        ball_diameter = (outer_diameter - inner_diameter) / 4
        ball_center_radius = (outer_diameter + inner_diameter) / 4
        num_balls = 8

        for i in range(num_balls):
            angle = 2 * np.pi * i / num_balls
            x = ball_center_radius * np.cos(angle)
            y = ball_center_radius * np.sin(angle)
            ball = Circle((x, y), ball_diameter/2, fill=False,
                         edgecolor=self.COLORS["lines"], linewidth=1)
            ax1.add_patch(ball)

        ax1.set_title("FRONT VIEW", color=self.COLORS["text"], fontsize=12)

        # Cross-section view
        ax2.set_xlim(-outer_diameter*0.7, outer_diameter*0.7)
        ax2.set_ylim(-width*2, width*2)

        # Outer race
        outer_race = Rectangle(
            (-outer_diameter/2, -width/2),
            outer_diameter, width,
            fill=False, edgecolor=self.COLORS["lines"], linewidth=2
        )
        ax2.add_patch(outer_race)

        # Inner race
        inner_race = Rectangle(
            (-inner_diameter/2, -width/2),
            inner_diameter, width,
            fill=False, edgecolor=self.COLORS["lines"], linewidth=2
        )
        ax2.add_patch(inner_race)

        # Dimension
        self._add_dimension(ax2, -outer_diameter/2, outer_diameter/2,
                          -width, f"Ø{outer_diameter:.1f}mm")

        ax2.set_title("CROSS SECTION", color=self.COLORS["text"], fontsize=12)

        # Main title
        fig.suptitle(f"BEARING {outer_diameter:.0f}x{inner_diameter:.0f}x{width:.0f}",
                    color=self.COLORS["text"], fontsize=16, fontweight='bold')

        buffer = BytesIO()
        fig.savefig(buffer, format='png', dpi=100,
                   facecolor=self.COLORS["background"],
                   bbox_inches='tight', pad_inches=0.5)
        plt.close(fig)

        return buffer.getvalue()

    def render_gasket(
        self,
        outer_diameter: float,
        inner_diameter: float,
        bolt_holes: int = 6,
        bolt_hole_diameter: float = 10,
        bolt_circle_diameter: float = None
    ) -> bytes:
        """Render a gasket/flange technical drawing."""
        fig, ax = plt.subplots(1, 1, figsize=(10, 10),
                               facecolor=self.COLORS["background"])
        ax.set_facecolor(self.COLORS["background"])
        ax.set_aspect('equal')
        ax.axis('off')

        ax.set_xlim(-outer_diameter*0.7, outer_diameter*0.7)
        ax.set_ylim(-outer_diameter*0.7, outer_diameter*0.7)

        # Outer circle
        outer = Circle((0, 0), outer_diameter/2, fill=False,
                       edgecolor=self.COLORS["lines"], linewidth=2)
        ax.add_patch(outer)

        # Inner circle (bore)
        inner = Circle((0, 0), inner_diameter/2, fill=False,
                       edgecolor=self.COLORS["lines"], linewidth=2)
        ax.add_patch(inner)

        # Bolt holes
        if bolt_circle_diameter is None:
            bolt_circle_diameter = (outer_diameter + inner_diameter) / 2

        for i in range(bolt_holes):
            angle = 2 * np.pi * i / bolt_holes
            x = (bolt_circle_diameter/2) * np.cos(angle)
            y = (bolt_circle_diameter/2) * np.sin(angle)
            hole = Circle((x, y), bolt_hole_diameter/2, fill=False,
                         edgecolor=self.COLORS["lines"], linewidth=1.5)
            ax.add_patch(hole)

        # Center lines (dashed)
        ax.axhline(y=0, color=self.COLORS["lines"], linewidth=0.5, linestyle='--', alpha=0.5)
        ax.axvline(x=0, color=self.COLORS["lines"], linewidth=0.5, linestyle='--', alpha=0.5)

        # Dimensions
        self._add_dimension(ax, -outer_diameter/2, outer_diameter/2,
                          -outer_diameter*0.6, f"Ø{outer_diameter:.1f}mm")
        self._add_dimension(ax, -inner_diameter/2, inner_diameter/2,
                          outer_diameter*0.4, f"Ø{inner_diameter:.1f}mm (BORE)")

        # Title
        ax.text(0, -outer_diameter*0.75,
               f"GASKET - {bolt_holes}x Ø{bolt_hole_diameter:.0f}mm HOLES",
               color=self.COLORS["text"], fontsize=14, fontweight='bold',
               ha='center')

        buffer = BytesIO()
        fig.savefig(buffer, format='png', dpi=100,
                   facecolor=self.COLORS["background"],
                   bbox_inches='tight', pad_inches=0.5)
        plt.close(fig)

        return buffer.getvalue()

    def _add_dimension(self, ax, x1: float, x2: float, y: float, text: str):
        """Add dimension line with text."""
        ax.annotate(
            '', xy=(x2, y), xytext=(x1, y),
            arrowprops=dict(
                arrowstyle='<->',
                color=self.COLORS["dimensions"],
                lw=1.5
            )
        )
        ax.text(
            (x1 + x2) / 2, y + 0.5,
            text,
            color=self.COLORS["dimensions"],
            fontsize=10,
            ha='center'
        )

def generate_blueprints_for_database(db_path: str):
    """Generate blueprint images for parts based on their properties."""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    renderer = BlueprintRenderer()

    # Find parts with specifications but no images
    cursor.execute("""
        SELECT p.id, p.name, p.description, p.notes
        FROM parts p
        WHERE p.id NOT IN (SELECT DISTINCT part_id FROM part_images)
    """)

    for part_id, name, description, notes in cursor.fetchall():
        name_lower = name.lower()

        try:
            # Determine part type and render appropriate blueprint
            if 'bolt' in name_lower or 'screw' in name_lower:
                # Extract dimensions from name/description
                image_data = renderer.render_bolt(
                    head_diameter=12,
                    head_height=6,
                    thread_diameter=8,
                    thread_length=30
                )
            elif 'bearing' in name_lower:
                image_data = renderer.render_bearing(
                    outer_diameter=50,
                    inner_diameter=25,
                    width=15
                )
            elif 'gasket' in name_lower or 'flange' in name_lower:
                image_data = renderer.render_gasket(
                    outer_diameter=100,
                    inner_diameter=50,
                    bolt_holes=6
                )
            else:
                continue  # Skip parts we can't render

            # Save to database
            cursor.execute("""
                INSERT INTO part_images
                (part_id, image_full, mime_type, is_primary)
                VALUES (?, ?, 'image/png', 1)
            """, (part_id, image_data))

            conn.commit()
            print(f"Generated blueprint for: {name}")

        except Exception as e:
            print(f"Error generating blueprint for {name}: {e}")

    conn.close()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Generate blueprint-style part images")
    parser.add_argument("--db", help="Database path for batch processing")
    parser.add_argument("--type", choices=["bolt", "bearing", "gasket"], help="Part type for single render")
    parser.add_argument("--output", help="Output path for single render")
    parser.add_argument("--params", help="Parameters as JSON")

    args = parser.parse_args()

    renderer = BlueprintRenderer()

    if args.type:
        import json
        params = json.loads(args.params) if args.params else {}

        if args.type == "bolt":
            params.setdefault("head_diameter", 12)
            params.setdefault("head_height", 6)
            params.setdefault("thread_diameter", 8)
            params.setdefault("thread_length", 30)
            image_data = renderer.render_bolt(**params)
        elif args.type == "bearing":
            params.setdefault("outer_diameter", 50)
            params.setdefault("inner_diameter", 25)
            params.setdefault("width", 15)
            image_data = renderer.render_bearing(**params)
        elif args.type == "gasket":
            params.setdefault("outer_diameter", 100)
            params.setdefault("inner_diameter", 50)
            params.setdefault("bolt_holes", 6)
            image_data = renderer.render_gasket(**params)

        output_path = args.output or f"{args.type}_blueprint.png"
        with open(output_path, "wb") as f:
            f.write(image_data)
        print(f"Saved: {output_path}")

    elif args.db:
        generate_blueprints_for_database(args.db)
EOF

chmod +x tools/render-blueprint.py
```

**Usage:**
```bash
# Generate single blueprint
python3 tools/render-blueprint.py --type bolt --output bolt.png \
  --params '{"thread_diameter": 10, "thread_length": 40}'

python3 tools/render-blueprint.py --type bearing --output bearing.png \
  --params '{"outer_diameter": 62, "inner_diameter": 30, "width": 16}'

# Generate blueprints for all parts in database
python3 tools/render-blueprint.py --db parts.db
```

#### Option 3: OpenSCAD 3D Rendering

For true 3D CAD rendering, integrate with OpenSCAD:

```bash
cat > tools/render-3d-openscad.py << 'EOF'
#!/usr/bin/env python3
"""
OpenSCAD 3D Part Renderer
Generates 3D CAD images from parametric models.

Requirements:
- OpenSCAD installed: sudo apt install openscad
"""

import subprocess
import tempfile
from pathlib import Path

class OpenSCADRenderer:
    """Renders 3D parts using OpenSCAD."""

    TEMPLATES = {
        "bolt": '''
// Parametric Bolt
$fn = 50;

head_diameter = {head_diameter};
head_height = {head_height};
thread_diameter = {thread_diameter};
thread_length = {thread_length};

// Head (hexagonal)
cylinder(h=head_height, d=head_diameter, $fn=6);

// Thread shaft
translate([0, 0, head_height])
    cylinder(h=thread_length, d=thread_diameter);
''',
        "bearing": '''
// Ball Bearing
$fn = 100;

outer_d = {outer_diameter};
inner_d = {inner_diameter};
width = {width};

difference() {{
    cylinder(h=width, d=outer_d, center=true);
    cylinder(h=width+1, d=inner_d, center=true);
}}
''',
        "nut": '''
// Parametric Nut
$fn = 50;

outer_diameter = {outer_diameter};
inner_diameter = {inner_diameter};
height = {height};

difference() {{
    cylinder(h=height, d=outer_diameter, $fn=6);
    cylinder(h=height+1, d=inner_diameter, center=true);
}}
'''
    }

    def __init__(self, openscad_path: str = "openscad"):
        self.openscad = openscad_path

    def render(
        self,
        part_type: str,
        params: dict,
        output_path: str,
        camera: str = "0,0,0,45,0,45,200",
        size: tuple = (800, 600)
    ) -> str:
        """Render a 3D part to an image."""

        if part_type not in self.TEMPLATES:
            raise ValueError(f"Unknown part type: {part_type}")

        # Generate OpenSCAD code
        scad_code = self.TEMPLATES[part_type].format(**params)

        # Write to temp file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.scad', delete=False) as f:
            f.write(scad_code)
            scad_path = f.name

        try:
            # Render with OpenSCAD
            cmd = [
                self.openscad,
                "-o", output_path,
                f"--camera={camera}",
                f"--imgsize={size[0]},{size[1]}",
                "--colorscheme=Starnight",
                scad_path
            ]

            result = subprocess.run(cmd, capture_output=True, text=True)

            if result.returncode != 0:
                raise RuntimeError(f"OpenSCAD error: {result.stderr}")

            return output_path

        finally:
            Path(scad_path).unlink()

if __name__ == "__main__":
    import argparse
    import json

    parser = argparse.ArgumentParser(description="Render 3D parts with OpenSCAD")
    parser.add_argument("--type", required=True, choices=["bolt", "bearing", "nut"])
    parser.add_argument("--params", required=True, help="JSON parameters")
    parser.add_argument("--output", required=True, help="Output image path")

    args = parser.parse_args()

    renderer = OpenSCADRenderer()
    params = json.loads(args.params)

    renderer.render(args.type, params, args.output)
    print(f"Rendered: {args.output}")
EOF

chmod +x tools/render-3d-openscad.py
```

**Usage:**
```bash
# Render 3D bolt
python3 tools/render-3d-openscad.py --type bolt --output bolt_3d.png \
  --params '{"head_diameter": 13, "head_height": 5, "thread_diameter": 8, "thread_length": 25}'
```

---

## Configuration

### Database Location

| Platform | Default Location |
|----------|-----------------|
| Windows | `%APPDATA%\PartsDB\parts.db` |
| macOS | `~/Library/Application Support/PartsDB/parts.db` |
| Linux | `~/.local/share/partsdb/parts.db` |
| CLI | `./parts.db` (current directory) |

### Environment Variables

```bash
# Database path
export PARTSDB_DATABASE="/path/to/parts.db"

# API keys for imports
export TECDOC_API_KEY="your-key"
export TECDOC_PROVIDER_ID="12345"

# Image generation
export OPENAI_API_KEY="sk-..."  # For DALL-E integration
export REPLICATE_API_TOKEN="..."  # For cloud Stable Diffusion
```

---

## Documentation

| Document | Description |
|----------|-------------|
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | System design and database schema |
| [BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md) | Detailed build instructions |
| [CSHARP_TUTORIAL.md](docs/CSHARP_TUTORIAL.md) | C# learning guide with code walkthrough |
| [ML_ARCHITECTURE.md](docs/ML_ARCHITECTURE.md) | Machine learning integration plans |
| [DEVELOPMENT_LOG.md](docs/DEVELOPMENT_LOG.md) | Development changelog and debugging guide |

---

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit your changes: `git commit -m 'Add amazing feature'`
4. Push to the branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- [.NET MAUI](https://github.com/dotnet/maui) - Cross-platform app framework
- [SQLite](https://sqlite.org/) - Embedded database
- [Dapper](https://github.com/DapperLib/Dapper) - Micro-ORM
- [ImageSharp](https://github.com/SixLabors/ImageSharp) - Image processing
- [Stable Diffusion](https://stability.ai/) - AI image generation
