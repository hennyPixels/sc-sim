#!/usr/bin/env python3
"""
PartsDB Image Import Tool
Automated image import with compression pipeline

Features:
- Watch folder for new images
- Auto-detect part number from filename
- Generate all compression variants (WebP for .NET, compressed for embedded)
- Update database automatically
"""

import os
import sys
import re
import io
import sqlite3
import argparse
import time
import struct
from pathlib import Path
from typing import Optional, Tuple
from dataclasses import dataclass

try:
    from PIL import Image
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler
except ImportError:
    print("Required packages not installed. Run:")
    print("  pip install Pillow watchdog")
    sys.exit(1)


# Configuration
MAX_FULL_SIZE = 1024
THUMBNAIL_SIZE = 150
EMBEDDED_SIZE = 64
FULL_QUALITY = 85
THUMBNAIL_QUALITY = 75
EMBEDDED_QUALITY = 50

# Supported image formats
SUPPORTED_FORMATS = {'.jpg', '.jpeg', '.png', '.gif', '.bmp', '.webp', '.tiff'}


@dataclass
class ProcessedImage:
    """Container for processed image data"""
    full_image: bytes
    thumbnail: bytes
    embedded: bytes
    width: int
    height: int


def resize_image(img: Image.Image, max_size: int) -> Image.Image:
    """Resize image maintaining aspect ratio"""
    if img.width <= max_size and img.height <= max_size:
        return img.copy()

    ratio = min(max_size / img.width, max_size / img.height)
    new_size = (int(img.width * ratio), int(img.height * ratio))
    return img.resize(new_size, Image.Resampling.LANCZOS)


def encode_webp(img: Image.Image, quality: int) -> bytes:
    """Encode image as WebP"""
    buffer = io.BytesIO()
    # Ensure RGB mode for WebP
    if img.mode in ('RGBA', 'LA'):
        background = Image.new('RGB', img.size, (255, 255, 255))
        background.paste(img, mask=img.split()[-1])
        img = background
    elif img.mode != 'RGB':
        img = img.convert('RGB')

    img.save(buffer, format='WEBP', quality=quality)
    return buffer.getvalue()


def create_embedded_image(img: Image.Image, max_size: int = 64) -> bytes:
    """
    Create compressed image for embedded systems
    Format: 2 bytes width + 2 bytes height + 4-bit grayscale RLE data
    """
    # Resize
    img = resize_image(img, max_size)

    # Convert to grayscale
    img = img.convert('L')

    # Get dimensions
    width, height = img.size

    # Create header
    header = struct.pack('<HH', width, height)

    # Convert to 4-bit grayscale (2 pixels per byte)
    pixels = list(img.getdata())
    packed_data = []

    for i in range(0, len(pixels), 2):
        high = pixels[i] >> 4
        low = pixels[i + 1] >> 4 if i + 1 < len(pixels) else 0
        packed_data.append((high << 4) | low)

    # Simple RLE compression
    compressed = []
    i = 0
    while i < len(packed_data):
        current = packed_data[i]
        count = 1

        while i + count < len(packed_data) and packed_data[i + count] == current and count < 127:
            count += 1

        if count >= 3:
            # RLE: 0x80 | count, value
            compressed.append(0x80 | count)
            compressed.append(current)
            i += count
        else:
            compressed.append(current)
            i += 1

    return header + bytes(compressed)


def process_image(image_path: Path) -> ProcessedImage:
    """Process an image file and generate all variants"""
    with Image.open(image_path) as img:
        original_width, original_height = img.size

        # Generate full-size WebP
        full = resize_image(img, MAX_FULL_SIZE)
        full_data = encode_webp(full, FULL_QUALITY)

        # Generate thumbnail WebP
        thumb = resize_image(img, THUMBNAIL_SIZE)
        thumb_data = encode_webp(thumb, THUMBNAIL_QUALITY)

        # Generate embedded format
        embedded_data = create_embedded_image(img, EMBEDDED_SIZE)

        return ProcessedImage(
            full_image=full_data,
            thumbnail=thumb_data,
            embedded=embedded_data,
            width=original_width,
            height=original_height
        )


def extract_part_number(filename: str) -> Optional[str]:
    """
    Extract part number from filename
    Supports formats:
    - PN12345.jpg -> PN12345
    - PN12345_front.jpg -> PN12345
    - 12345-ABC.jpg -> 12345-ABC
    """
    stem = Path(filename).stem

    # Try common patterns
    patterns = [
        r'^(PN\d+)',           # PN12345
        r'^(\d{4,}-[A-Z0-9]+)',  # 12345-ABC
        r'^([A-Z]{2,}\d+)',     # ABC123
        r'^(\d+)',              # Just numbers
    ]

    for pattern in patterns:
        match = re.match(pattern, stem, re.IGNORECASE)
        if match:
            return match.group(1).upper()

    # Fallback: use the part before first underscore or dash
    parts = re.split(r'[_-]', stem)
    if parts:
        return parts[0].upper()

    return None


def import_image(db_path: str, image_path: Path, part_id: Optional[int] = None,
                 part_number: Optional[str] = None, is_primary: bool = True) -> bool:
    """Import an image into the database"""

    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    try:
        # Find part ID if not provided
        if part_id is None:
            if part_number is None:
                part_number = extract_part_number(image_path.name)

            if part_number is None:
                print(f"Could not extract part number from: {image_path.name}")
                return False

            cursor.execute("SELECT id FROM parts WHERE part_number = ?", (part_number,))
            row = cursor.fetchone()

            if row is None:
                print(f"Part not found: {part_number}")
                return False

            part_id = row[0]

        # Process image
        print(f"Processing: {image_path.name}")
        processed = process_image(image_path)

        # If this is primary, clear existing primary flag
        if is_primary:
            cursor.execute(
                "UPDATE part_images SET is_primary = 0 WHERE part_id = ?",
                (part_id,)
            )

        # Insert image
        cursor.execute("""
            INSERT INTO part_images
            (part_id, image_full, image_thumbnail, image_embedded,
             original_filename, mime_type, width, height, file_size, is_primary)
            VALUES (?, ?, ?, ?, ?, 'image/webp', ?, ?, ?, ?)
        """, (
            part_id,
            processed.full_image,
            processed.thumbnail,
            processed.embedded,
            image_path.name,
            processed.width,
            processed.height,
            len(processed.full_image),
            1 if is_primary else 0
        ))

        conn.commit()

        print(f"  Imported for part ID {part_id}")
        print(f"  Full: {len(processed.full_image):,} bytes")
        print(f"  Thumbnail: {len(processed.thumbnail):,} bytes")
        print(f"  Embedded: {len(processed.embedded):,} bytes")

        return True

    except Exception as e:
        print(f"Error importing {image_path}: {e}")
        conn.rollback()
        return False

    finally:
        conn.close()


class ImageWatcher(FileSystemEventHandler):
    """Watch for new images and import them"""

    def __init__(self, db_path: str):
        self.db_path = db_path

    def on_created(self, event):
        if event.is_directory:
            return

        path = Path(event.src_path)
        if path.suffix.lower() not in SUPPORTED_FORMATS:
            return

        # Wait a moment for file to be fully written
        time.sleep(0.5)

        import_image(self.db_path, path)


def watch_folder(db_path: str, folder: Path):
    """Watch a folder for new images"""
    print(f"Watching folder: {folder}")
    print("Press Ctrl+C to stop...")

    handler = ImageWatcher(db_path)
    observer = Observer()
    observer.schedule(handler, str(folder), recursive=False)
    observer.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
        print("\nStopped watching.")

    observer.join()


def batch_import(db_path: str, folder: Path):
    """Import all images from a folder"""
    images = [f for f in folder.iterdir()
              if f.is_file() and f.suffix.lower() in SUPPORTED_FORMATS]

    if not images:
        print("No images found in folder.")
        return

    print(f"Found {len(images)} image(s)")

    success = 0
    for image_path in images:
        if import_image(db_path, image_path):
            success += 1

    print(f"\nImported {success}/{len(images)} images")


def main():
    parser = argparse.ArgumentParser(
        description='PartsDB Image Import Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  Import single image:
    %(prog)s --db parts.db --file image.jpg --part-number PN12345

  Import all images from folder:
    %(prog)s --db parts.db --import ./images/

  Watch folder for new images:
    %(prog)s --db parts.db --watch ./incoming/

Filename conventions:
  Images are matched to parts by filename:
  - PN12345.jpg      -> matches part PN12345
  - PN12345_front.jpg -> matches part PN12345
  - 12345-ABC.png    -> matches part 12345-ABC
        """
    )

    parser.add_argument('--db', '-d', required=True,
                        help='Path to SQLite database')
    parser.add_argument('--file', '-f',
                        help='Single image file to import')
    parser.add_argument('--part-number', '-p',
                        help='Part number (overrides filename detection)')
    parser.add_argument('--part-id', '-i', type=int,
                        help='Part ID (overrides part number lookup)')
    parser.add_argument('--import', dest='import_folder',
                        help='Folder to batch import from')
    parser.add_argument('--watch', '-w',
                        help='Folder to watch for new images')
    parser.add_argument('--no-primary', action='store_true',
                        help='Do not set as primary image')

    args = parser.parse_args()

    # Validate database exists
    if not Path(args.db).exists():
        print(f"Database not found: {args.db}")
        sys.exit(1)

    if args.file:
        # Single file import
        image_path = Path(args.file)
        if not image_path.exists():
            print(f"File not found: {args.file}")
            sys.exit(1)

        success = import_image(
            args.db,
            image_path,
            part_id=args.part_id,
            part_number=args.part_number,
            is_primary=not args.no_primary
        )
        sys.exit(0 if success else 1)

    elif args.import_folder:
        # Batch import
        folder = Path(args.import_folder)
        if not folder.is_dir():
            print(f"Folder not found: {args.import_folder}")
            sys.exit(1)

        batch_import(args.db, folder)

    elif args.watch:
        # Watch mode
        folder = Path(args.watch)
        if not folder.is_dir():
            print(f"Folder not found: {args.watch}")
            sys.exit(1)

        watch_folder(args.db, folder)

    else:
        parser.print_help()
        sys.exit(1)


if __name__ == '__main__':
    main()
