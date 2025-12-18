#!/usr/bin/env python3
"""
PartsDB Image Import Tool
=========================
Processes and imports images into the PartsDB database.

Features:
- Multi-resolution image processing (full, thumbnail, embedded)
- WebP compression for optimal storage
- Batch import with filename/directory matching
- Support for JPG, PNG, WebP, GIF formats

Usage:
    # Single image import
    python import-images.py --db parts.db --part-id 42 --image ./brake_pad.jpg

    # Batch import (filename matching)
    python import-images.py --db parts.db --source ./images/ --match filename

    # Batch import (directory matching)
    python import-images.py --db parts.db --source ./images/ --match directory

Requirements:
    pip install Pillow
"""

import os
import sys
import sqlite3
import argparse
from pathlib import Path
from io import BytesIO
from typing import Optional, Dict, Tuple

# Attempt to import Pillow, install if missing
try:
    from PIL import Image
except ImportError:
    print("Pillow not found. Installing...")
    os.system(f"{sys.executable} -m pip install Pillow")
    from PIL import Image


class ImageProcessor:
    """
    Processes images into multiple resolutions for the PartsDB application.

    Resolution Pipeline:
    - Full: 800x800 max, WebP format, 85% quality
    - Thumbnail: 200x200, WebP format, 80% quality
    - Embedded: 64x64, grayscale PNG (for CLI display)
    """

    # Size configurations
    FULL_SIZE: Tuple[int, int] = (800, 800)
    THUMBNAIL_SIZE: Tuple[int, int] = (200, 200)
    EMBEDDED_SIZE: Tuple[int, int] = (64, 64)

    # Quality settings
    WEBP_QUALITY_FULL: int = 85
    WEBP_QUALITY_THUMB: int = 80

    # Supported formats
    SUPPORTED_FORMATS: set = {".jpg", ".jpeg", ".png", ".webp", ".gif", ".bmp", ".tiff"}

    def __init__(self, db_path: str):
        """
        Initialize the image processor.

        Args:
            db_path: Path to the SQLite database file
        """
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        self._ensure_schema()

    def _ensure_schema(self) -> None:
        """Ensure the part_images table exists."""
        cursor = self.conn.cursor()
        cursor.execute("""
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
            )
        """)
        self.conn.commit()

    def process_image(self, image_path: str) -> Dict[str, bytes]:
        """
        Process an image into multiple resolutions.

        Args:
            image_path: Path to the source image

        Returns:
            Dictionary containing processed image data:
            - 'full': Full-size WebP bytes
            - 'thumbnail': Thumbnail WebP bytes
            - 'embedded': Embedded grayscale PNG bytes
            - 'width': Final width of full image
            - 'height': Final height of full image
            - 'file_size': Size of full image in bytes
        """
        # Open and validate image
        img = Image.open(image_path)

        # Convert to RGB if necessary (handle RGBA, palette, etc.)
        if img.mode in ("RGBA", "P", "LA"):
            # Create white background for transparency
            background = Image.new("RGB", img.size, (255, 255, 255))
            if img.mode == "P":
                img = img.convert("RGBA")
            background.paste(img, mask=img.split()[-1] if img.mode in ("RGBA", "LA") else None)
            img = background
        elif img.mode != "RGB":
            img = img.convert("RGB")

        results = {}

        # Process full-size image
        full = img.copy()
        full.thumbnail(self.FULL_SIZE, Image.Resampling.LANCZOS)
        full_buffer = BytesIO()
        full.save(full_buffer, format="WebP", quality=self.WEBP_QUALITY_FULL)
        results["full"] = full_buffer.getvalue()

        # Process thumbnail
        thumb = img.copy()
        thumb.thumbnail(self.THUMBNAIL_SIZE, Image.Resampling.LANCZOS)
        thumb_buffer = BytesIO()
        thumb.save(thumb_buffer, format="WebP", quality=self.WEBP_QUALITY_THUMB)
        results["thumbnail"] = thumb_buffer.getvalue()

        # Process embedded (grayscale for CLI)
        embedded = img.copy()
        embedded = embedded.convert("L")  # Convert to grayscale
        embedded.thumbnail(self.EMBEDDED_SIZE, Image.Resampling.LANCZOS)
        embedded_buffer = BytesIO()
        embedded.save(embedded_buffer, format="PNG", optimize=True)
        results["embedded"] = embedded_buffer.getvalue()

        # Store metadata
        results["width"] = full.width
        results["height"] = full.height
        results["file_size"] = len(results["full"])

        return results

    def import_for_part(
        self,
        part_id: int,
        image_path: str,
        is_primary: bool = False
    ) -> int:
        """
        Import an image for a specific part.

        Args:
            part_id: ID of the part to attach the image to
            image_path: Path to the source image
            is_primary: Whether this is the primary image for the part

        Returns:
            ID of the newly created image record
        """
        # Verify part exists
        cursor = self.conn.cursor()
        cursor.execute("SELECT id FROM parts WHERE id = ?", (part_id,))
        if not cursor.fetchone():
            raise ValueError(f"Part with ID {part_id} not found")

        # Process the image
        processed = self.process_image(image_path)

        # If this should be primary, un-primary existing images
        if is_primary:
            cursor.execute(
                "UPDATE part_images SET is_primary = 0 WHERE part_id = ?",
                (part_id,)
            )

        # Insert the image record
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

    def batch_import(
        self,
        directory: str,
        match_by: str = "filename"
    ) -> Tuple[int, list]:
        """
        Batch import images from a directory.

        Args:
            directory: Source directory containing images
            match_by: Matching strategy
                - "filename": Match image filename to part_number
                  (e.g., ABC123.jpg matches part ABC123)
                - "directory": Each subdirectory is a part_number
                  (e.g., images/ABC123/photo1.jpg)

        Returns:
            Tuple of (imported_count, list of errors)
        """
        image_dir = Path(directory)
        if not image_dir.exists():
            raise ValueError(f"Directory not found: {directory}")

        imported = 0
        errors = []
        cursor = self.conn.cursor()

        if match_by == "filename":
            # Files named like: ABC123.jpg, ABC123_1.jpg, ABC123_2.png
            for img_path in image_dir.iterdir():
                if img_path.suffix.lower() not in self.SUPPORTED_FORMATS:
                    continue

                # Extract part number from filename (before first underscore or extension)
                part_number = img_path.stem.split("_")[0]

                # Find part in database
                cursor.execute(
                    "SELECT id FROM parts WHERE part_number = ?",
                    (part_number,)
                )
                row = cursor.fetchone()

                if row:
                    try:
                        # Check if this is the first image for the part
                        cursor.execute(
                            "SELECT COUNT(*) FROM part_images WHERE part_id = ?",
                            (row[0],)
                        )
                        is_primary = cursor.fetchone()[0] == 0

                        self.import_for_part(row[0], str(img_path), is_primary)
                        imported += 1
                        print(f"✓ Imported: {img_path.name} -> Part #{row[0]} ({part_number})")
                    except Exception as e:
                        errors.append((str(img_path), str(e)))
                        print(f"✗ Error: {img_path.name} - {e}")
                else:
                    errors.append((str(img_path), f"Part '{part_number}' not found"))
                    print(f"? Skipped: {img_path.name} - Part '{part_number}' not in database")

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
                        if img_path.suffix.lower() not in self.SUPPORTED_FORMATS:
                            continue

                        try:
                            self.import_for_part(row[0], str(img_path), i == 0)
                            imported += 1
                            print(f"✓ Imported: {part_number}/{img_path.name}")
                        except Exception as e:
                            errors.append((str(img_path), str(e)))
                            print(f"✗ Error: {img_path.name} - {e}")
                else:
                    errors.append((str(part_dir), f"Part '{part_number}' not found"))
                    print(f"? Skipped directory: {part_number} - Not in database")

        else:
            raise ValueError(f"Unknown match_by value: {match_by}")

        return imported, errors

    def list_parts_without_images(self, limit: int = 50) -> list:
        """
        List parts that don't have any images.

        Args:
            limit: Maximum number of parts to return

        Returns:
            List of (id, part_number, name) tuples
        """
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT p.id, p.part_number, p.name
            FROM parts p
            WHERE p.id NOT IN (SELECT DISTINCT part_id FROM part_images)
            LIMIT ?
        """, (limit,))
        return cursor.fetchall()

    def close(self) -> None:
        """Close the database connection."""
        self.conn.close()


def main():
    """Main entry point for the image import tool."""
    parser = argparse.ArgumentParser(
        description="Import images into PartsDB",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Import single image
  %(prog)s --db parts.db --part-id 42 --image ./brake_pad.jpg

  # Batch import with filename matching
  %(prog)s --db parts.db --source ./images/ --match filename

  # Batch import with directory structure
  %(prog)s --db parts.db --source ./images/ --match directory

  # List parts without images
  %(prog)s --db parts.db --list-missing
        """
    )

    parser.add_argument(
        "--db",
        required=True,
        help="Path to SQLite database"
    )
    parser.add_argument(
        "--source",
        help="Source directory for batch import"
    )
    parser.add_argument(
        "--match",
        choices=["filename", "directory"],
        default="filename",
        help="Matching strategy for batch import (default: filename)"
    )
    parser.add_argument(
        "--part-id",
        type=int,
        help="Part ID for single image import"
    )
    parser.add_argument(
        "--image",
        help="Image path for single import (use with --part-id)"
    )
    parser.add_argument(
        "--list-missing",
        action="store_true",
        help="List parts without images"
    )

    args = parser.parse_args()

    # Validate arguments
    if not args.list_missing and not args.source and not (args.part_id and args.image):
        parser.error("Must specify --source for batch import, --part-id and --image for single import, or --list-missing")

    if args.part_id and not args.image:
        parser.error("--image required when using --part-id")

    if args.image and not args.part_id:
        parser.error("--part-id required when using --image")

    # Create processor
    processor = ImageProcessor(args.db)

    try:
        if args.list_missing:
            # List parts without images
            parts = processor.list_parts_without_images()
            if parts:
                print(f"\nParts without images ({len(parts)} found):\n")
                print(f"{'ID':<8} {'Part Number':<20} {'Name':<40}")
                print("-" * 70)
                for part_id, part_number, name in parts:
                    print(f"{part_id:<8} {part_number:<20} {name[:40]:<40}")
            else:
                print("All parts have images!")

        elif args.part_id and args.image:
            # Single image import
            image_id = processor.import_for_part(args.part_id, args.image, is_primary=True)
            print(f"✓ Imported image as ID {image_id} for part {args.part_id}")

        else:
            # Batch import
            print(f"Starting batch import from: {args.source}")
            print(f"Matching strategy: {args.match}")
            print("-" * 50)

            imported, errors = processor.batch_import(args.source, args.match)

            print("-" * 50)
            print(f"\n✓ Import complete: {imported} images imported")

            if errors:
                print(f"✗ Errors: {len(errors)}")
                if len(errors) <= 10:
                    for path, error in errors:
                        print(f"  - {path}: {error}")
                else:
                    for path, error in errors[:10]:
                        print(f"  - {path}: {error}")
                    print(f"  ... and {len(errors) - 10} more errors")

    finally:
        processor.close()


if __name__ == "__main__":
    main()
