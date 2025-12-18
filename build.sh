#!/bin/bash
#
# PartsDB Build Script
# Usage: ./build.sh [command]
#
# Commands:
#   test      - Run all tests
#   build     - Build debug version
#   release   - Build release version
#   publish   - Create portable executables
#   cli       - Build C23 CLI only
#   clean     - Clean all build artifacts
#   all       - Run tests, then build release
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

# Print colored message
print_status() {
    echo -e "${BLUE}==>${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}!${NC} $1"
}

# Check prerequisites
check_dotnet() {
    if ! command -v dotnet &> /dev/null; then
        print_error ".NET SDK not found. Install from https://dot.net"
        exit 1
    fi
    print_success ".NET SDK found: $(dotnet --version)"
}

check_gcc() {
    if command -v gcc-13 &> /dev/null; then
        CC=gcc-13
    elif command -v gcc &> /dev/null; then
        CC=gcc
    else
        print_warning "GCC not found. C23 CLI will not be built."
        return 1
    fi
    print_success "C compiler found: $($CC --version | head -1)"
    return 0
}

# Run tests
run_tests() {
    print_status "Running .NET tests..."

    dotnet test tests/PartsDb.Tests/PartsDb.Tests.csproj \
        --logger "console;verbosity=normal" \
        --results-directory TestResults

    if [ $? -eq 0 ]; then
        print_success "All tests passed!"
    else
        print_error "Some tests failed"
        exit 1
    fi
}

# Build debug
build_debug() {
    print_status "Building .NET (Debug)..."
    dotnet build -c Debug

    if check_gcc; then
        print_status "Building C23 CLI (Debug)..."
        cd src/partsdb-cli
        make debug CC=$CC
        cd "$PROJECT_ROOT"
    fi

    print_success "Debug build complete"
}

# Build release
build_release() {
    print_status "Building .NET (Release)..."
    dotnet build -c Release

    if check_gcc; then
        print_status "Building C23 CLI (Release)..."
        cd src/partsdb-cli
        make clean && make CC=$CC
        cd "$PROJECT_ROOT"
    fi

    print_success "Release build complete"
}

# Create portable executables
publish_all() {
    print_status "Creating portable executables..."

    mkdir -p dist

    # Detect current OS
    case "$(uname -s)" in
        Linux*)     CURRENT_OS=linux;;
        Darwin*)    CURRENT_OS=osx;;
        MINGW*|CYGWIN*|MSYS*) CURRENT_OS=win;;
        *)          CURRENT_OS=unknown;;
    esac

    # Detect architecture
    case "$(uname -m)" in
        x86_64)     CURRENT_ARCH=x64;;
        arm64|aarch64) CURRENT_ARCH=arm64;;
        *)          CURRENT_ARCH=x64;;
    esac

    CURRENT_RID="${CURRENT_OS}-${CURRENT_ARCH}"
    print_status "Current platform: $CURRENT_RID"

    # Build for current platform (cross-compilation requires additional setup)
    print_status "Publishing for $CURRENT_RID..."

    dotnet publish src/PartsDb.Maui/PartsDb.Maui.csproj \
        -c Release \
        -r "$CURRENT_RID" \
        --self-contained true \
        -p:PublishSingleFile=true \
        -p:IncludeNativeLibrariesForSelfExtract=true \
        -o "dist/$CURRENT_RID" || {
            print_warning "MAUI publish failed (may need workload). Trying shared library..."
            dotnet publish src/PartsDb.Shared/PartsDb.Shared.csproj \
                -c Release \
                -r "$CURRENT_RID" \
                --self-contained true \
                -o "dist/$CURRENT_RID"
        }

    # Build C23 CLI
    if check_gcc; then
        print_status "Building C23 CLI..."
        cd src/partsdb-cli
        make clean && make CC=$CC
        cp bin/partsdb "$PROJECT_ROOT/dist/$CURRENT_RID/partsdb-cli" 2>/dev/null || true
        cd "$PROJECT_ROOT"
    fi

    # Copy database schema
    cp database/schema.sql "dist/$CURRENT_RID/"

    # Show results
    print_success "Portable executables created in dist/$CURRENT_RID/"
    ls -lh "dist/$CURRENT_RID/"
}

# Build C23 CLI only
build_cli() {
    if ! check_gcc; then
        print_error "GCC not found"
        exit 1
    fi

    print_status "Building C23 CLI..."
    cd src/partsdb-cli
    make clean && make CC=$CC
    cd "$PROJECT_ROOT"

    print_success "C23 CLI built: src/partsdb-cli/bin/partsdb"
}

# Clean all build artifacts
clean_all() {
    print_status "Cleaning build artifacts..."

    # .NET
    dotnet clean 2>/dev/null || true
    rm -rf */bin */obj
    rm -rf src/*/bin src/*/obj
    rm -rf tests/*/bin tests/*/obj
    rm -rf TestResults
    rm -rf dist
    rm -rf publish

    # C23
    cd src/partsdb-cli
    make clean 2>/dev/null || true
    cd "$PROJECT_ROOT"

    # Test databases
    rm -f *.db *.db-journal *.db-wal *.db-shm
    rm -f test.db

    print_success "Clean complete"
}

# Show help
show_help() {
    echo "PartsDB Build Script"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  test      Run all tests"
    echo "  build     Build debug version"
    echo "  release   Build release version"
    echo "  publish   Create portable executables for current platform"
    echo "  cli       Build C23 CLI only"
    echo "  clean     Clean all build artifacts"
    echo "  all       Run tests, then build release"
    echo "  help      Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 test           # Run tests only"
    echo "  $0 publish        # Create portable exe"
    echo "  $0 all            # Full build with tests"
}

# Main
case "${1:-help}" in
    test)
        check_dotnet
        run_tests
        ;;
    build)
        check_dotnet
        build_debug
        ;;
    release)
        check_dotnet
        build_release
        ;;
    publish)
        check_dotnet
        publish_all
        ;;
    cli)
        build_cli
        ;;
    clean)
        clean_all
        ;;
    all)
        check_dotnet
        run_tests
        build_release
        publish_all
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
