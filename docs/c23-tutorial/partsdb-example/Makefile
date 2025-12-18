# PartsDB Example Makefile
#
# Demonstrates secure compilation flags from the C23 Tutorial:
# - Part 2:  Modern build practices (21st Century C)
# - Part 4:  Security hardening flags (Effective C, Secure Coding)
# - Appendix A: Security checklist

# Compiler settings
CC      ?= gcc
CSTD    ?= c17
TARGET  := partsdb

# Source files
SRCS    := main.c partsdb.c
OBJS    := $(SRCS:.c=.o)
DEPS    := partsdb.h partsdb_internal.h

#============================================================================
# Warning Flags (Part 2: Enable all warnings)
#============================================================================
WARNINGS := -Wall \
            -Wextra \
            -Wpedantic \
            -Wshadow \
            -Wconversion \
            -Wsign-conversion \
            -Wstrict-prototypes \
            -Wmissing-prototypes \
            -Wformat=2 \
            -Wformat-security \
            -Wnull-dereference \
            -Wdouble-promotion \
            -Wstrict-overflow=2

#============================================================================
# Security Hardening Flags (Part 4 & Appendix A)
#============================================================================
SECURITY := -D_FORTIFY_SOURCE=2 \
            -fstack-protector-strong \
            -fPIE

# Linker security flags (Linux/Unix)
LDFLAGS_SECURITY := -pie \
                    -Wl,-z,relro \
                    -Wl,-z,now \
                    -Wl,-z,noexecstack

#============================================================================
# Build Configurations
#============================================================================

# Debug build
DEBUG_FLAGS := -g3 -O0 -DDEBUG \
               -fno-omit-frame-pointer

# Release build
RELEASE_FLAGS := -O2 -DNDEBUG

# Sanitizer builds (Part 10: Dynamic analysis)
ASAN_FLAGS := -fsanitize=address,undefined \
              -fno-omit-frame-pointer
TSAN_FLAGS := -fsanitize=thread

#============================================================================
# Default target: debug build
#============================================================================
CFLAGS := -std=$(CSTD) $(WARNINGS) $(SECURITY) $(DEBUG_FLAGS)
LDFLAGS := $(LDFLAGS_SECURITY) -lm

# Detect OS for platform-specific flags
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME_S),Darwin)
    # macOS: some flags not supported
    LDFLAGS_SECURITY :=
    LDFLAGS := -lm
endif
ifeq ($(UNAME_S),Windows)
    # Windows/MinGW
    TARGET := partsdb.exe
    LDFLAGS_SECURITY :=
    LDFLAGS := -lm
endif

#============================================================================
# Build Targets
#============================================================================

.PHONY: all clean debug release sanitize tsan help test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $@"

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Debug build (default)
debug: CFLAGS := -std=$(CSTD) $(WARNINGS) $(SECURITY) $(DEBUG_FLAGS)
debug: clean all
	@echo "Debug build complete"

# Release build
release: CFLAGS := -std=$(CSTD) $(WARNINGS) $(SECURITY) $(RELEASE_FLAGS)
release: clean all
	strip $(TARGET)
	@echo "Release build complete"

# AddressSanitizer build
sanitize: CFLAGS := -std=$(CSTD) $(WARNINGS) $(DEBUG_FLAGS) $(ASAN_FLAGS)
sanitize: LDFLAGS := $(ASAN_FLAGS) -lm
sanitize: clean all
	@echo "Sanitizer build complete"
	@echo "Run with: ASAN_OPTIONS=abort_on_error=1 ./$(TARGET)"

# ThreadSanitizer build
tsan: CFLAGS := -std=$(CSTD) $(WARNINGS) $(DEBUG_FLAGS) $(TSAN_FLAGS)
tsan: LDFLAGS := $(TSAN_FLAGS) -lm
tsan: clean all
	@echo "ThreadSanitizer build complete"

# Static analysis (requires clang)
analyze:
	@echo "Running static analysis..."
	clang --analyze -Xanalyzer -analyzer-output=text $(SRCS)
	@echo "Static analysis complete"

# Clean
clean:
	rm -f $(OBJS) $(TARGET) *.plist
	@echo "Cleaned"

#============================================================================
# Test Targets
#============================================================================

test: $(TARGET)
	@echo "Running basic tests..."
	@echo ""
	@echo "=== Create new database ==="
	./$(TARGET) -n -f test.db list
	@echo ""
	@echo "=== Add parts ==="
	./$(TARGET) -f test.db add 1001 "Hex Bolt M10x50" 0.25 500 "Steel hex bolt"
	./$(TARGET) -f test.db add 1002 "Hex Nut M10" 0.10 1000 "Steel hex nut"
	./$(TARGET) -f test.db add 1003 "Washer M10" 0.05 2000 "Flat washer"
	./$(TARGET) -f test.db add 2001 "Socket Bolt M8x30" 0.35 50 "Low stock item"
	@echo ""
	@echo "=== List all parts ==="
	./$(TARGET) -f test.db list
	@echo ""
	@echo "=== Find by ID ==="
	./$(TARGET) -f test.db find 1001
	@echo ""
	@echo "=== Search by name ==="
	./$(TARGET) -f test.db search bolt
	@echo ""
	@echo "=== Update part ==="
	./$(TARGET) -f test.db update 1001 quantity=450 price=0.30
	./$(TARGET) -f test.db find 1001
	@echo ""
	@echo "=== Low stock report ==="
	./$(TARGET) -f test.db low-stock 100
	@echo ""
	@echo "=== Statistics ==="
	./$(TARGET) -f test.db stats
	@echo ""
	@echo "=== Delete part ==="
	./$(TARGET) -f test.db delete 1003
	./$(TARGET) -f test.db list
	@echo ""
	@echo "=== Cleanup ==="
	rm -f test.db
	@echo "Tests complete!"

#============================================================================
# Help
#============================================================================

help:
	@echo "PartsDB Example Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       Build with debug settings (default)"
	@echo "  debug     Build with debug settings"
	@echo "  release   Build optimized release"
	@echo "  sanitize  Build with AddressSanitizer"
	@echo "  tsan      Build with ThreadSanitizer"
	@echo "  analyze   Run static analysis (requires clang)"
	@echo "  test      Run basic functionality tests"
	@echo "  clean     Remove build artifacts"
	@echo "  help      Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  CC=$(CC)"
	@echo "  CSTD=$(CSTD)"
	@echo ""
	@echo "Example:"
	@echo "  make release CC=clang"
