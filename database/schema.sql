-- Parts Database Schema
-- SQLite with FTS5 for full-text search

PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;

-- Categories table
CREATE TABLE IF NOT EXISTS categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    description TEXT,
    parent_id INTEGER,
    FOREIGN KEY (parent_id) REFERENCES categories(id) ON DELETE SET NULL
);

-- Parts table
CREATE TABLE IF NOT EXISTS parts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    part_number TEXT UNIQUE NOT NULL,
    name TEXT NOT NULL,
    description TEXT,
    category_id INTEGER,
    manufacturer TEXT,
    quantity INTEGER DEFAULT 0,
    min_quantity INTEGER DEFAULT 0,
    unit_price REAL,
    currency TEXT DEFAULT 'USD',
    location TEXT,
    datasheet_url TEXT,
    notes TEXT,
    is_active INTEGER DEFAULT 1,
    created_at TEXT DEFAULT (datetime('now')),
    updated_at TEXT DEFAULT (datetime('now')),
    FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE SET NULL
);

-- Part images table
CREATE TABLE IF NOT EXISTS part_images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    part_id INTEGER NOT NULL,
    image_full BLOB,
    image_thumbnail BLOB,
    image_embedded BLOB,
    original_filename TEXT,
    mime_type TEXT DEFAULT 'image/webp',
    width INTEGER,
    height INTEGER,
    file_size INTEGER,
    is_primary INTEGER DEFAULT 0,
    created_at TEXT DEFAULT (datetime('now')),
    FOREIGN KEY (part_id) REFERENCES parts(id) ON DELETE CASCADE
);

-- Suppliers table
CREATE TABLE IF NOT EXISTS suppliers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    contact_name TEXT,
    email TEXT,
    phone TEXT,
    website TEXT,
    address TEXT,
    notes TEXT,
    created_at TEXT DEFAULT (datetime('now'))
);

-- Part-supplier relationship with pricing
CREATE TABLE IF NOT EXISTS part_suppliers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    part_id INTEGER NOT NULL,
    supplier_id INTEGER NOT NULL,
    supplier_part_number TEXT,
    unit_price REAL,
    min_order_qty INTEGER DEFAULT 1,
    lead_time_days INTEGER,
    is_preferred INTEGER DEFAULT 0,
    FOREIGN KEY (part_id) REFERENCES parts(id) ON DELETE CASCADE,
    FOREIGN KEY (supplier_id) REFERENCES suppliers(id) ON DELETE CASCADE,
    UNIQUE(part_id, supplier_id)
);

-- Inventory transactions
CREATE TABLE IF NOT EXISTS inventory_transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    part_id INTEGER NOT NULL,
    quantity_change INTEGER NOT NULL,
    transaction_type TEXT NOT NULL CHECK(transaction_type IN ('IN', 'OUT', 'ADJUST', 'COUNT')),
    reference TEXT,
    notes TEXT,
    created_at TEXT DEFAULT (datetime('now')),
    created_by TEXT,
    FOREIGN KEY (part_id) REFERENCES parts(id) ON DELETE CASCADE
);

-- Full-text search virtual table
CREATE VIRTUAL TABLE IF NOT EXISTS parts_fts USING fts5(
    part_number,
    name,
    description,
    manufacturer,
    notes,
    content='parts',
    content_rowid='id'
);

-- Triggers to keep FTS in sync
CREATE TRIGGER IF NOT EXISTS parts_ai AFTER INSERT ON parts BEGIN
    INSERT INTO parts_fts(rowid, part_number, name, description, manufacturer, notes)
    VALUES (new.id, new.part_number, new.name, new.description, new.manufacturer, new.notes);
END;

CREATE TRIGGER IF NOT EXISTS parts_ad AFTER DELETE ON parts BEGIN
    INSERT INTO parts_fts(parts_fts, rowid, part_number, name, description, manufacturer, notes)
    VALUES ('delete', old.id, old.part_number, old.name, old.description, old.manufacturer, old.notes);
END;

CREATE TRIGGER IF NOT EXISTS parts_au AFTER UPDATE ON parts BEGIN
    INSERT INTO parts_fts(parts_fts, rowid, part_number, name, description, manufacturer, notes)
    VALUES ('delete', old.id, old.part_number, old.name, old.description, old.manufacturer, old.notes);
    INSERT INTO parts_fts(rowid, part_number, name, description, manufacturer, notes)
    VALUES (new.id, new.part_number, new.name, new.description, new.manufacturer, new.notes);
END;

-- Trigger to update updated_at
CREATE TRIGGER IF NOT EXISTS parts_update_timestamp AFTER UPDATE ON parts BEGIN
    UPDATE parts SET updated_at = datetime('now') WHERE id = new.id;
END;

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_parts_category ON parts(category_id);
CREATE INDEX IF NOT EXISTS idx_parts_manufacturer ON parts(manufacturer);
CREATE INDEX IF NOT EXISTS idx_parts_location ON parts(location);
CREATE INDEX IF NOT EXISTS idx_parts_active ON parts(is_active);
CREATE INDEX IF NOT EXISTS idx_part_images_part ON part_images(part_id);
CREATE INDEX IF NOT EXISTS idx_inventory_trans_part ON inventory_transactions(part_id);
CREATE INDEX IF NOT EXISTS idx_inventory_trans_date ON inventory_transactions(created_at);

-- Default categories
INSERT OR IGNORE INTO categories (name, description) VALUES
    ('Resistors', 'Passive resistive components'),
    ('Capacitors', 'Passive capacitive components'),
    ('Inductors', 'Passive inductive components'),
    ('Semiconductors', 'Active semiconductor devices'),
    ('Connectors', 'Electrical connectors'),
    ('Mechanical', 'Mechanical parts and hardware'),
    ('ICs', 'Integrated circuits'),
    ('Modules', 'Pre-assembled modules'),
    ('Tools', 'Tools and equipment'),
    ('Misc', 'Miscellaneous parts');
