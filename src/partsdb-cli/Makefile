# PartsDB CLI - Makefile
# C23 text-based interface for Linux and embedded systems

CC ?= gcc
CFLAGS = -std=c2x -Wall -Wextra -Wpedantic -O2
LDFLAGS = -lsqlite3

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target
TARGET = $(BIN_DIR)/partsdb

# Include paths
CFLAGS += -I$(INC_DIR)

# Debug build
DEBUG_CFLAGS = -g -DDEBUG -fsanitize=address,undefined
DEBUG_LDFLAGS = -fsanitize=address,undefined

# Embedded build (minimal, no colors)
EMBEDDED_CFLAGS = -Os -DPARTSDB_NO_COLOR -ffunction-sections -fdata-sections
EMBEDDED_LDFLAGS = -Wl,--gc-sections -static

# Default target
.PHONY: all
all: $(TARGET)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Built: $@"

# Debug build
.PHONY: debug
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: clean $(TARGET)
	@echo "Debug build complete"

# Embedded/minimal build
.PHONY: embedded
embedded: CFLAGS = -std=c2x -Wall $(EMBEDDED_CFLAGS) -I$(INC_DIR)
embedded: LDFLAGS = $(EMBEDDED_LDFLAGS) -lsqlite3
embedded: clean $(TARGET)
	@echo "Embedded build complete"
	@ls -lh $(TARGET)

# Install
.PHONY: install
install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/

# Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Help
.PHONY: help
help:
	@echo "PartsDB CLI Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build release version (default)"
	@echo "  debug     - Build with debug symbols and sanitizers"
	@echo "  embedded  - Build minimal version for embedded systems"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  clean     - Remove build artifacts"
	@echo ""
	@echo "Variables:"
	@echo "  CC        - C compiler (default: gcc)"
	@echo "  DESTDIR   - Installation prefix"
	@echo ""
	@echo "Requirements:"
	@echo "  - GCC 13+ or Clang 16+ (C23 support)"
	@echo "  - SQLite3 development libraries"

# Dependencies
$(BUILD_DIR)/main.o: $(INC_DIR)/partsdb.h
$(BUILD_DIR)/database.o: $(INC_DIR)/partsdb.h
