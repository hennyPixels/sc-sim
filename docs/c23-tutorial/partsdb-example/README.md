# PartsDB Example Code

This directory contains a complete implementation of PartsDB, demonstrating concepts from the C23 Tutorial.

## Files

| File | Description | Tutorial Parts |
|------|-------------|----------------|
| `partsdb.h` | Public API header with opaque types | Part 6: API Design |
| `partsdb_internal.h` | Internal implementation details | Part 10: Defensive Programming |
| `partsdb.c` | Core implementation | Parts 4, 5, 9, 10, 11, 12 |
| `main.c` | Command-line interface | Parts 4, 9, 12, 13 |
| `Makefile` | Build system with security flags | Part 2, Appendix A |

## Building

```bash
# Debug build (with assertions and debug info)
make debug

# Release build (optimized, stripped)
make release

# Build with AddressSanitizer (memory error detection)
make sanitize

# Run static analysis
make analyze

# Run tests
make test
```

## Usage

```bash
# Add parts
./partsdb add 1001 "Hex Bolt M10x50" 0.25 500 "Steel hex bolt"
./partsdb add 1002 "Hex Nut M10" 0.10 1000

# List all parts
./partsdb list

# Find by ID
./partsdb find 1001

# Search by name (case-insensitive)
./partsdb search bolt

# Update a part
./partsdb update 1001 quantity=450 price=0.30

# Find low stock items
./partsdb low-stock 100

# Delete a part
./partsdb delete 1002

# Show statistics
./partsdb stats

# Use different database file
./partsdb -f inventory.db list

# Create new (empty) database
./partsdb -n -f new.db list
```

## Demonstrated Concepts

### Part 4: Security Best Practices
- Input validation for all user input
- Safe string handling (bounded copies)
- Integer overflow checking
- Floating-point validation (NaN, infinity checks)

### Part 5: Avoiding Traps and Pitfalls
- Null pointer checks before dereference
- Proper initialization of all variables
- Avoiding undefined behavior

### Part 6: API Design (C Interfaces and Implementations)
- Opaque types (`PartsDB *`, `PartsDBIterator *`)
- Clear ownership semantics
- Iterator pattern for traversal
- Comprehensive error codes

### Part 9: Style and Debugging (Practice of Programming)
- Consistent K&R-style formatting
- Clear error messages with context
- Debug logging infrastructure

### Part 10: Defensive Programming (Writing Solid Code)
- Assertions for invariant checking
- Magic number validation
- Memory integrity with CRC32 checksums
- Debug vs. release build separation

### Part 11: Memory Management (Pointers on C)
- Safe allocation wrappers
- No memory leaks (all paths free resources)
- Null-after-free pattern
- Overflow checks for sizes

### Part 12: CERT C Compliance
- No format string vulnerabilities (FIO30-C)
- Proper null termination (STR31-C)
- Checked integer operations (INT30-C, INT32-C)
- Valid array access (ARR30-C)

### Part 13: K&R Philosophy
- Simple, direct code
- Minimal dependencies
- Error handling via return codes
- Program name in error messages

## Security Features

The Makefile enables comprehensive security hardening:

```makefile
# Warnings
-Wall -Wextra -Wpedantic -Wformat=2 -Wformat-security

# Stack protection
-fstack-protector-strong

# Position Independent Executable
-fPIE -pie

# Fortify source (buffer overflow detection)
-D_FORTIFY_SOURCE=2

# Linker hardening
-Wl,-z,relro,-z,now  # Full RELRO
-Wl,-z,noexecstack   # Non-executable stack
```

## Testing

Run the built-in test suite:

```bash
make test
```

For memory error detection:

```bash
make sanitize
ASAN_OPTIONS=abort_on_error=1 ./partsdb add 1001 "Test" 1.00
```

## API Documentation

See `partsdb.h` for complete API documentation with:
- Function signatures and parameters
- Return values and error conditions
- Thread-safety guarantees
- Usage examples
