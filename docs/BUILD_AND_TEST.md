# Build and Test Guide

## Prerequisites

### .NET (Blazor Hybrid MAUI)
```bash
# Install .NET 10 SDK
# Windows: Download from https://dotnet.microsoft.com/download
# Linux:
wget https://dot.net/v1/dotnet-install.sh
chmod +x dotnet-install.sh
./dotnet-install.sh --channel 10.0

# Install MAUI workload
dotnet workload install maui
```

### C23 CLI
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install gcc-13 sqlite3 libsqlite3-dev

# Fedora
sudo dnf install gcc sqlite sqlite-devel

# macOS
brew install gcc sqlite3
```

---

## Running Tests

### .NET Tests

```bash
# Run all tests
dotnet test

# Run with verbose output
dotnet test --logger "console;verbosity=detailed"

# Run specific test class
dotnet test --filter "FullyQualifiedName~PartsServiceTests"

# Run with coverage
dotnet test --collect:"XPlat Code Coverage"

# Generate coverage report (requires reportgenerator)
dotnet tool install -g dotnet-reportgenerator-globaltool
reportgenerator -reports:"**/coverage.cobertura.xml" -targetdir:"coverage-report" -reporttypes:Html
```

### C23 CLI Tests

```bash
cd src/partsdb-cli

# Build with debug symbols
make debug

# Run basic functionality test
./bin/partsdb -d test.db -c "list"

# Run with valgrind (memory check)
valgrind --leak-check=full ./bin/partsdb -d test.db -c "list"

# Run with AddressSanitizer (built into debug target)
./bin/partsdb -d test.db
```

---

## Building Portable Executables

### Option 1: .NET Single-File Executable (Recommended)

Creates a single self-contained executable with all dependencies bundled.

```bash
# Windows x64
dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
    -c Release \
    -r win-x64 \
    --self-contained true \
    -p:PublishSingleFile=true \
    -p:IncludeNativeLibrariesForSelfExtract=true \
    -o publish/win-x64

# Linux x64
dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
    -c Release \
    -r linux-x64 \
    --self-contained true \
    -p:PublishSingleFile=true \
    -o publish/linux-x64

# macOS x64
dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
    -c Release \
    -r osx-x64 \
    --self-contained true \
    -p:PublishSingleFile=true \
    -o publish/osx-x64

# macOS ARM (Apple Silicon)
dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
    -c Release \
    -r osx-arm64 \
    --self-contained true \
    -p:PublishSingleFile=true \
    -o publish/osx-arm64
```

### Option 2: .NET Trimmed Executable (Smaller Size)

```bash
# Trimmed + AOT for smallest size (experimental)
dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
    -c Release \
    -r win-x64 \
    --self-contained true \
    -p:PublishSingleFile=true \
    -p:PublishTrimmed=true \
    -p:TrimMode=link \
    -o publish/win-x64-trimmed
```

### Option 3: C23 CLI Static Binary

```bash
cd src/partsdb-cli

# Standard build
make clean && make

# Static build for maximum portability (Linux)
make embedded

# The binary will be at:
# ./bin/partsdb
```

### Option 4: Cross-Platform Script

Create a build script for all platforms:

```bash
#!/bin/bash
# build-all.sh

set -e

echo "Building PartsDB for all platforms..."

# Create output directory
mkdir -p dist

# .NET MAUI builds
for runtime in win-x64 linux-x64 osx-x64 osx-arm64; do
    echo "Building .NET for $runtime..."
    dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
        -c Release \
        -r $runtime \
        --self-contained true \
        -p:PublishSingleFile=true \
        -o dist/$runtime
done

# C23 CLI build
echo "Building C23 CLI..."
cd src/partsdb-cli
make clean && make
cp bin/partsdb ../../dist/linux-x64/partsdb-cli
cd ../..

# Create archives
echo "Creating archives..."
cd dist
for dir in */; do
    platform="${dir%/}"
    tar -czvf "partsdb-$platform.tar.gz" "$platform"
done
cd ..

echo "Done! Builds are in ./dist/"
ls -lh dist/*.tar.gz
```

---

## Quick Start Commands

### Run Tests Only
```bash
# From project root
dotnet test tests/PartsDb.Tests/PartsDb.Tests.csproj
```

### Build and Run (Development)
```bash
# .NET MAUI (requires display)
dotnet run --project src/PartsDb.Maui/PartsDb.Maui.csproj

# C23 CLI
cd src/partsdb-cli && make && ./bin/partsdb -d ../../test.db
```

### Build Portable Executable (Quick)
```bash
# Detect current platform and build
dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
    -c Release \
    --self-contained true \
    -p:PublishSingleFile=true \
    -o publish/current
```

---

## Platform-Specific Notes

### Windows
- Output: `PartsDb.Maui.exe` (single file, ~80-150MB)
- Requires: Windows 10 version 1809+ or Windows 11

### Linux
- Output: `PartsDb.Maui` (single file, ~80-120MB)
- Requires: glibc 2.17+ (Ubuntu 18.04+, Debian 10+)
- For GUI: X11 or Wayland display server

### macOS
- Output: `PartsDb.Maui` (single file, ~80-120MB)
- Requires: macOS 10.15 Catalina+
- May need to allow in Security settings

### Embedded (C23 CLI)
- Output: `partsdb` (~200KB static, ~50KB dynamic)
- Requires: SQLite3 runtime (if dynamic build)
- Works on: Any Linux with glibc, including embedded systems

---

## CI/CD Integration

### GitHub Actions Example

```yaml
# .github/workflows/build.yml
name: Build and Test

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup .NET
        uses: actions/setup-dotnet@v4
        with:
          dotnet-version: '10.0.x'

      - name: Restore dependencies
        run: dotnet restore

      - name: Build
        run: dotnet build --no-restore

      - name: Test
        run: dotnet test --no-build --verbosity normal

  build-executables:
    needs: test
    runs-on: ubuntu-latest
    strategy:
      matrix:
        runtime: [win-x64, linux-x64, osx-x64]

    steps:
      - uses: actions/checkout@v4

      - name: Setup .NET
        uses: actions/setup-dotnet@v4
        with:
          dotnet-version: '10.0.x'

      - name: Publish
        run: |
          dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
            -c Release \
            -r ${{ matrix.runtime }} \
            --self-contained true \
            -p:PublishSingleFile=true \
            -o publish/${{ matrix.runtime }}

      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: partsdb-${{ matrix.runtime }}
          path: publish/${{ matrix.runtime }}
```

---

## Troubleshooting

### "MAUI workload not installed"
```bash
dotnet workload install maui --skip-manifest-update
```

### "Platform not supported" on Linux
```bash
# MAUI requires specific Linux dependencies
sudo apt install libgtk-3-dev libwebkit2gtk-4.0-dev
```

### C23 compiler errors
```bash
# Ensure GCC 13+ or Clang 16+
gcc --version

# If using older compiler, modify Makefile:
# Change: CFLAGS = -std=c2x
# To:     CFLAGS = -std=c17
```

### SQLite not found (C23)
```bash
# Install development headers
sudo apt install libsqlite3-dev
```
