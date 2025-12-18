#!/usr/bin/env python3
"""
NHTSA vPIC API Importer
=======================
Imports vehicle and parts data from the NHTSA (National Highway Traffic Safety
Administration) Vehicle Product Information Catalog.

This is a FREE API that requires no authentication.

API Documentation: https://vpic.nhtsa.dot.gov/api/

Usage:
    # List all vehicle makes
    python import-nhtsa.py --list-makes

    # List models for a make and year
    python import-nhtsa.py --make Ford --year 2023 --list-models

    # Import parts for a specific vehicle
    python import-nhtsa.py --db parts.db --make Ford --model F-150 --year 2023

    # Decode a VIN
    python import-nhtsa.py --vin 1FTFW1E50MFA12345

Requirements:
    pip install requests
"""

import argparse
import sqlite3
import time
from typing import Optional, Dict, List
from datetime import datetime

try:
    import requests
except ImportError:
    import os
    import sys
    print("Installing requests...")
    os.system(f"{sys.executable} -m pip install requests")
    import requests


class NHTSAClient:
    """
    Client for the NHTSA vPIC (Vehicle Product Information Catalog) API.

    This API provides:
    - Vehicle make/model/year information
    - VIN decoding
    - Parts and equipment data
    - Safety recall information
    """

    BASE_URL = "https://vpic.nhtsa.dot.gov/api/vehicles"

    def __init__(self, rate_limit: float = 0.2):
        """
        Initialize the NHTSA API client.

        Args:
            rate_limit: Minimum seconds between requests (default: 0.2 = 5 req/sec)
        """
        self.session = requests.Session()
        self.session.headers.update({
            "Accept": "application/json",
            "User-Agent": "PartsDB/1.0 (Parts Inventory System)"
        })
        self.rate_limit = rate_limit
        self.last_request = 0

    def _request(self, endpoint: str, params: Optional[Dict] = None) -> Dict:
        """
        Make a rate-limited request to the API.

        Args:
            endpoint: API endpoint path
            params: Query parameters

        Returns:
            JSON response as dictionary
        """
        # Respect rate limit
        elapsed = time.time() - self.last_request
        if elapsed < self.rate_limit:
            time.sleep(self.rate_limit - elapsed)

        url = f"{self.BASE_URL}/{endpoint}"
        params = params or {}
        params["format"] = "json"

        response = self.session.get(url, params=params)
        self.last_request = time.time()

        response.raise_for_status()
        return response.json()

    def get_all_makes(self) -> List[Dict]:
        """
        Get all vehicle makes.

        Returns:
            List of make dictionaries with 'Make_ID' and 'Make_Name'
        """
        data = self._request("GetAllMakes")
        return data.get("Results", [])

    def get_models_for_make_year(self, make: str, year: int) -> List[Dict]:
        """
        Get all models for a specific make and year.

        Args:
            make: Vehicle make (e.g., "Ford", "Toyota")
            year: Model year

        Returns:
            List of model dictionaries
        """
        data = self._request(f"GetModelsForMakeYear/make/{make}/modelyear/{year}")
        return data.get("Results", [])

    def get_models_for_make_id(self, make_id: int) -> List[Dict]:
        """
        Get all models for a make by ID.

        Args:
            make_id: NHTSA make ID

        Returns:
            List of model dictionaries
        """
        data = self._request(f"GetModelsForMakeId/{make_id}")
        return data.get("Results", [])

    def decode_vin(self, vin: str) -> Dict:
        """
        Decode a VIN (Vehicle Identification Number).

        Args:
            vin: 17-character VIN

        Returns:
            Dictionary of decoded vehicle information
        """
        data = self._request(f"DecodeVinValues/{vin}")
        results = data.get("Results", [{}])
        return results[0] if results else {}

    def decode_vin_extended(self, vin: str) -> Dict:
        """
        Decode a VIN with extended information.

        Args:
            vin: 17-character VIN

        Returns:
            Dictionary with extended vehicle details
        """
        data = self._request(f"DecodeVinExtended/{vin}")
        results = data.get("Results", [{}])
        return results[0] if results else {}

    def get_parts(self, part_type: int, from_date: str, to_date: str) -> List[Dict]:
        """
        Get parts/equipment data.

        Args:
            part_type: Type of part (575 = Tires, 565 = Equipment)
            from_date: Start date (MM/DD/YYYY)
            to_date: End date (MM/DD/YYYY)

        Returns:
            List of parts data
        """
        data = self._request("GetParts", {
            "type": part_type,
            "fromDate": from_date,
            "toDate": to_date
        })
        return data.get("Results", [])

    def get_equipment_plant_codes(self, year: int) -> List[Dict]:
        """
        Get equipment plant codes for a year.

        Args:
            year: Model year

        Returns:
            List of plant code data
        """
        data = self._request(f"GetEquipmentPlantCodes/{year}")
        return data.get("Results", [])

    def get_vehicle_types_for_make(self, make: str) -> List[Dict]:
        """
        Get vehicle types for a make.

        Args:
            make: Vehicle make name

        Returns:
            List of vehicle types
        """
        data = self._request(f"GetVehicleTypesForMake/{make}")
        return data.get("Results", [])


class PartsDBImporter:
    """Imports NHTSA data into the PartsDB database."""

    def __init__(self, db_path: str):
        """
        Initialize the importer.

        Args:
            db_path: Path to SQLite database
        """
        self.conn = sqlite3.connect(db_path)
        self.client = NHTSAClient()
        self._ensure_schema()

    def _ensure_schema(self) -> None:
        """Ensure required tables exist."""
        cursor = self.conn.cursor()

        # Parts table
        cursor.execute("""
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
                updated_at TEXT DEFAULT (datetime('now'))
            )
        """)

        # Categories table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS categories (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT UNIQUE NOT NULL,
                description TEXT,
                parent_id INTEGER
            )
        """)

        # Vehicle compatibility table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS vehicle_compatibility (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                part_id INTEGER NOT NULL,
                make TEXT,
                model TEXT,
                year_start INTEGER,
                year_end INTEGER,
                vin_pattern TEXT,
                notes TEXT,
                FOREIGN KEY (part_id) REFERENCES parts(id) ON DELETE CASCADE
            )
        """)

        self.conn.commit()

    def import_vehicle_info(self, vin: str) -> Dict:
        """
        Import vehicle information from a VIN.

        Args:
            vin: Vehicle Identification Number

        Returns:
            Decoded vehicle information
        """
        info = self.client.decode_vin_extended(vin)

        print(f"\nVehicle Information for VIN: {vin}")
        print("-" * 50)

        # Display key information
        fields = [
            ("Make", "Make"),
            ("Model", "Model"),
            ("Year", "ModelYear"),
            ("Body Class", "BodyClass"),
            ("Engine", "EngineModel"),
            ("Displacement", "DisplacementL"),
            ("Cylinders", "EngineCylinders"),
            ("Fuel Type", "FuelTypePrimary"),
            ("Drive Type", "DriveType"),
            ("Transmission", "TransmissionStyle"),
            ("Plant Country", "PlantCountry"),
            ("Plant City", "PlantCity"),
        ]

        for label, key in fields:
            value = info.get(key, "N/A")
            if value and value != "N/A":
                print(f"{label}: {value}")

        return info

    def import_models_as_categories(self, make: str, year: int) -> int:
        """
        Import vehicle models as part categories.

        Args:
            make: Vehicle make
            year: Model year

        Returns:
            Number of categories created
        """
        cursor = self.conn.cursor()
        models = self.client.get_models_for_make_year(make, year)

        # Create parent category for make
        cursor.execute("""
            INSERT OR IGNORE INTO categories (name, description)
            VALUES (?, ?)
        """, (make, f"Parts for {make} vehicles"))

        cursor.execute("SELECT id FROM categories WHERE name = ?", (make,))
        make_id = cursor.fetchone()[0]

        created = 0
        for model in models:
            model_name = model.get("Model_Name", "")
            if model_name:
                cursor.execute("""
                    INSERT OR IGNORE INTO categories (name, description, parent_id)
                    VALUES (?, ?, ?)
                """, (
                    f"{make} {model_name}",
                    f"Parts for {year} {make} {model_name}",
                    make_id
                ))
                created += 1

        self.conn.commit()
        print(f"Created {created} model categories for {year} {make}")
        return created

    def create_sample_parts(self, make: str, model: str, year: int) -> int:
        """
        Create sample parts entries for a vehicle.

        This creates common part categories with placeholder entries
        that can be filled in with actual inventory.

        Args:
            make: Vehicle make
            model: Vehicle model
            year: Model year

        Returns:
            Number of parts created
        """
        cursor = self.conn.cursor()

        # Common vehicle part categories
        common_parts = [
            ("Brake Pads - Front", "Brakes", f"Front brake pads for {year} {make} {model}"),
            ("Brake Pads - Rear", "Brakes", f"Rear brake pads for {year} {make} {model}"),
            ("Brake Rotor - Front", "Brakes", f"Front brake rotor for {year} {make} {model}"),
            ("Brake Rotor - Rear", "Brakes", f"Rear brake rotor for {year} {make} {model}"),
            ("Oil Filter", "Filters", f"Engine oil filter for {year} {make} {model}"),
            ("Air Filter", "Filters", f"Engine air filter for {year} {make} {model}"),
            ("Cabin Air Filter", "Filters", f"Cabin air filter for {year} {make} {model}"),
            ("Fuel Filter", "Filters", f"Fuel filter for {year} {make} {model}"),
            ("Spark Plugs", "Ignition", f"Spark plugs for {year} {make} {model}"),
            ("Ignition Coil", "Ignition", f"Ignition coil for {year} {make} {model}"),
            ("Alternator", "Electrical", f"Alternator for {year} {make} {model}"),
            ("Starter Motor", "Electrical", f"Starter motor for {year} {make} {model}"),
            ("Battery", "Electrical", f"Battery for {year} {make} {model}"),
            ("Headlight Bulb", "Lighting", f"Headlight bulb for {year} {make} {model}"),
            ("Tail Light Bulb", "Lighting", f"Tail light bulb for {year} {make} {model}"),
            ("Wiper Blades", "Body", f"Windshield wiper blades for {year} {make} {model}"),
            ("Serpentine Belt", "Engine", f"Serpentine belt for {year} {make} {model}"),
            ("Timing Belt", "Engine", f"Timing belt for {year} {make} {model}"),
            ("Water Pump", "Cooling", f"Water pump for {year} {make} {model}"),
            ("Thermostat", "Cooling", f"Thermostat for {year} {make} {model}"),
            ("Radiator", "Cooling", f"Radiator for {year} {make} {model}"),
            ("CV Axle - Left", "Drivetrain", f"Left CV axle for {year} {make} {model}"),
            ("CV Axle - Right", "Drivetrain", f"Right CV axle for {year} {make} {model}"),
            ("Tie Rod End", "Steering", f"Tie rod end for {year} {make} {model}"),
            ("Ball Joint", "Suspension", f"Ball joint for {year} {make} {model}"),
            ("Shock Absorber - Front", "Suspension", f"Front shock absorber for {year} {make} {model}"),
            ("Shock Absorber - Rear", "Suspension", f"Rear shock absorber for {year} {make} {model}"),
            ("Strut Assembly - Front", "Suspension", f"Front strut assembly for {year} {make} {model}"),
        ]

        created = 0
        vehicle_code = f"{make[:3].upper()}{model[:3].upper()}{str(year)[-2:]}"

        for i, (name, category, description) in enumerate(common_parts, 1):
            part_number = f"{vehicle_code}-{i:04d}"

            try:
                # Create category if not exists
                cursor.execute("""
                    INSERT OR IGNORE INTO categories (name, description)
                    VALUES (?, ?)
                """, (category, f"{category} parts"))

                cursor.execute("SELECT id FROM categories WHERE name = ?", (category,))
                category_id = cursor.fetchone()[0]

                # Create part
                cursor.execute("""
                    INSERT OR IGNORE INTO parts
                    (part_number, name, description, category_id, manufacturer, notes)
                    VALUES (?, ?, ?, ?, ?, ?)
                """, (
                    part_number,
                    f"{name} - {year} {make} {model}",
                    description,
                    category_id,
                    "OEM",
                    f"Vehicle: {year} {make} {model}"
                ))

                if cursor.rowcount > 0:
                    part_id = cursor.lastrowid

                    # Add vehicle compatibility
                    cursor.execute("""
                        INSERT INTO vehicle_compatibility
                        (part_id, make, model, year_start, year_end)
                        VALUES (?, ?, ?, ?, ?)
                    """, (part_id, make, model, year, year))

                    created += 1

            except Exception as e:
                print(f"Error creating part {name}: {e}")

        self.conn.commit()
        print(f"Created {created} sample parts for {year} {make} {model}")
        return created

    def close(self) -> None:
        """Close the database connection."""
        self.conn.close()


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Import vehicle/parts data from NHTSA API",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # List all vehicle makes
  %(prog)s --list-makes

  # List models for Ford in 2023
  %(prog)s --make Ford --year 2023 --list-models

  # Decode a VIN
  %(prog)s --vin 1FTFW1E50MFA12345

  # Create sample parts for a vehicle
  %(prog)s --db parts.db --make Ford --model F-150 --year 2023 --create-parts

  # Import model categories
  %(prog)s --db parts.db --make Toyota --year 2023 --import-categories
        """
    )

    parser.add_argument("--db", help="Path to SQLite database")
    parser.add_argument("--make", help="Vehicle make (e.g., Ford)")
    parser.add_argument("--model", help="Vehicle model (e.g., F-150)")
    parser.add_argument("--year", type=int, help="Model year")
    parser.add_argument("--vin", help="Decode a VIN")
    parser.add_argument("--list-makes", action="store_true", help="List all makes")
    parser.add_argument("--list-models", action="store_true", help="List models (requires --make and --year)")
    parser.add_argument("--import-categories", action="store_true", help="Import models as categories")
    parser.add_argument("--create-parts", action="store_true", help="Create sample parts for vehicle")

    args = parser.parse_args()

    client = NHTSAClient()

    # Handle simple queries that don't need database
    if args.list_makes:
        makes = client.get_all_makes()
        print(f"\nVehicle Makes ({len(makes)} found):\n")
        for make in sorted(makes, key=lambda x: x.get("Make_Name", ""))[:100]:
            print(f"  {make.get('Make_ID', 'N/A'):>5}  {make.get('Make_Name', 'N/A')}")
        if len(makes) > 100:
            print(f"\n  ... and {len(makes) - 100} more")
        return

    if args.list_models:
        if not args.make or not args.year:
            parser.error("--list-models requires --make and --year")

        models = client.get_models_for_make_year(args.make, args.year)
        print(f"\n{args.year} {args.make} Models ({len(models)} found):\n")
        for model in sorted(models, key=lambda x: x.get("Model_Name", "")):
            print(f"  {model.get('Model_ID', 'N/A'):>6}  {model.get('Model_Name', 'N/A')}")
        return

    if args.vin:
        if args.db:
            importer = PartsDBImporter(args.db)
            importer.import_vehicle_info(args.vin)
            importer.close()
        else:
            info = client.decode_vin_extended(args.vin)
            print(f"\nVIN Decode: {args.vin}\n" + "-" * 50)
            for key, value in sorted(info.items()):
                if value and value != "Not Applicable":
                    print(f"{key}: {value}")
        return

    # Database operations
    if not args.db:
        parser.error("--db required for import operations")

    importer = PartsDBImporter(args.db)

    try:
        if args.import_categories:
            if not args.make or not args.year:
                parser.error("--import-categories requires --make and --year")
            importer.import_models_as_categories(args.make, args.year)

        if args.create_parts:
            if not args.make or not args.model or not args.year:
                parser.error("--create-parts requires --make, --model, and --year")
            importer.create_sample_parts(args.make, args.model, args.year)

    finally:
        importer.close()


if __name__ == "__main__":
    main()
