# gperftools Debugging and Profiling Guide

A comprehensive guide to using Google Performance Tools (gperftools) for debugging, profiling, and optimizing C/C++ applications, with specific examples from the PartsDB project.

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [TCMalloc Memory Allocator](#tcmalloc-memory-allocator)
4. [CPU Profiling](#cpu-profiling)
5. [Heap Profiling](#heap-profiling)
6. [Heap Checker (Memory Leak Detection)](#heap-checker)
7. [Integration Examples](#integration-examples)
8. [Platform Notes](#platform-notes)
9. [Troubleshooting](#troubleshooting)

---

## Overview

gperftools is a collection of high-performance tools for C/C++ applications:

| Component | Description | Windows Support |
|-----------|-------------|-----------------|
| **TCMalloc** | Thread-caching malloc replacement | Full (tcmalloc_minimal) |
| **CPU Profiler** | Statistical sampling profiler | Limited |
| **Heap Profiler** | Memory allocation profiler | Limited |
| **Heap Checker** | Memory leak detector | Limited |

### References

- Official Repository: https://github.com/gperftools/gperftools
- CPU Profiler Docs: https://gperftools.github.io/gperftools/cpuprofile.html
- RidgeRun Guide: https://developer.ridgerun.com/wiki/index.php/Profiling_with_GPerfTools

---

## Installation

### Windows (MSYS2/MinGW64)

gperftools is not available as a pre-built MSYS2 package. Build from source:

```bash
# Install prerequisites
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja git

# Clone and build
git clone --depth 1 https://github.com/gperftools/gperftools.git
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ../gperftools

# Fix Windows link issue (add after line 432 in CMakeLists.txt)
# target_link_libraries(common INTERFACE synchronization)

cmake ../gperftools
ninja
```

### Linux (Debian/Ubuntu)

```bash
# From packages
sudo apt-get install google-perftools libgoogle-perftools-dev graphviz

# From source
git clone https://github.com/gperftools/gperftools
cd gperftools
./autogen.sh
./configure
make
sudo make install
```

---

## TCMalloc Memory Allocator

TCMalloc (Thread-Caching Malloc) is a high-performance memory allocator that:
- Reduces lock contention in multi-threaded applications
- Provides faster allocation/deallocation than standard malloc
- Tracks memory statistics

### Linking with TCMalloc

**Static linking:**
```bash
gcc -o myapp myapp.c -L/path/to/lib -ltcmalloc_minimal
```

**Dynamic linking (Linux):**
```bash
LD_PRELOAD=/usr/lib/libtcmalloc_minimal.so ./myapp
```

**Windows linking:**
```bash
gcc -o myapp.exe myapp.c -L./lib -ltcmalloc_minimal
# Ensure libtcmalloc_minimal.dll is in PATH
```

### Memory Statistics API

```c
#include <gperftools/malloc_extension.h>

// Get current allocated bytes
size_t allocated_bytes;
MallocExtension_GetNumericProperty("generic.current_allocated_bytes", &allocated_bytes);

// Get heap size
size_t heap_size;
MallocExtension_GetNumericProperty("generic.heap_size", &heap_size);

// Print statistics
MallocExtension_GetStats(buffer, buffer_size);
```

**Available Properties:**

| Property | Description |
|----------|-------------|
| `generic.current_allocated_bytes` | Currently allocated memory |
| `generic.heap_size` | Total heap size |
| `tcmalloc.pageheap_free_bytes` | Unmapped but free bytes |
| `tcmalloc.max_total_thread_cache_bytes` | Max thread cache size |
| `tcmalloc.current_total_thread_cache_bytes` | Current thread cache |

---

## CPU Profiling

CPU profiling uses statistical sampling to identify performance bottlenecks.

### Enabling CPU Profiling

**Method 1: Environment Variables**
```bash
# Linux
LD_PRELOAD=/usr/lib/libprofiler.so CPUPROFILE=myapp.prof ./myapp

# Set sampling frequency (default: 100 samples/second)
CPUPROFILE_FREQUENCY=200 CPUPROFILE=myapp.prof ./myapp
```

**Method 2: Programmatic Control**
```c
#include <gperftools/profiler.h>

int main() {
    ProfilerStart("output.prof");

    // Code to profile
    expensive_function();

    ProfilerStop();
    return 0;
}
```

Compile with: `gcc -lprofiler myapp.c -o myapp`

**Method 3: Signal-Based Control**
```bash
# Start with profiling disabled
CPUPROFILE=myapp.prof CPUPROFILESIGNAL=12 ./myapp &

# Start profiling
kill -12 $!

# Stop profiling
kill -12 $!
```

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `CPUPROFILE` | — | Output filename |
| `CPUPROFILE_FREQUENCY` | 100 | Samples per second |
| `CPUPROFILE_REALTIME` | unset | Use wall-clock time instead of CPU time |

### Analyzing Profiles with pprof

Install pprof:
```bash
go install github.com/google/pprof@latest
```

Common commands:
```bash
# Interactive mode
pprof /path/to/myapp myapp.prof

# Text report
pprof --text /path/to/myapp myapp.prof

# Web UI (requires graphviz)
pprof --http=:8080 /path/to/myapp myapp.prof

# Generate PDF
pprof --pdf /path/to/myapp myapp.prof > profile.pdf

# Callgrind format (for kcachegrind)
pprof --callgrind /path/to/myapp myapp.prof > callgrind.out
```

### Interpreting Profile Output

```
Total: 1234 samples
     500  40.6%  40.6%      800  64.8% expensive_function
     300  24.3%  64.9%      300  24.3% memory_allocate
     200  16.2%  81.1%      200  16.2% string_parse
```

Columns:
1. **Sample count**: Number of times this function was sampled
2. **Local %**: Percentage of total time in this function only
3. **Cumulative %**: Running total of local percentages
4. **With callees**: Samples including called functions
5. **With callees %**: Percentage including callees
6. **Function name**: The function being profiled

---

## Heap Profiling

Heap profiling tracks memory allocations to identify:
- Memory-intensive functions
- Allocation patterns
- Potential optimization opportunities

### Enabling Heap Profiling

```bash
# Linux
LD_PRELOAD=/usr/lib/libtcmalloc.so HEAPPROFILE=/tmp/heapprof ./myapp
```

**Programmatic control:**
```c
#include <gperftools/heap-profiler.h>

HeapProfilerStart("heap_profile");
// Code to analyze
HeapProfilerDump("checkpoint_1");
// More code
HeapProfilerStop();
```

### Environment Variables

| Variable | Description |
|----------|-------------|
| `HEAPPROFILE` | Output file prefix |
| `HEAP_PROFILE_ALLOCATION_INTERVAL` | Bytes between dumps (default: 1GB) |
| `HEAP_PROFILE_INUSE_INTERVAL` | In-use bytes between dumps |
| `HEAP_PROFILE_TIME_INTERVAL` | Seconds between dumps |

### Analyzing Heap Profiles

```bash
pprof --text /path/to/myapp heap_profile.0001.heap
pprof --inuse_space --text /path/to/myapp heap_profile.0001.heap
pprof --alloc_space --web /path/to/myapp heap_profile.0001.heap
```

---

## Heap Checker

The heap checker detects memory leaks by tracking allocations.

### Enabling Heap Checking

```bash
# Linux
LD_PRELOAD=/usr/lib/libtcmalloc.so HEAPCHECK=normal ./myapp
```

**Check modes:**

| Mode | Description |
|------|-------------|
| `minimal` | Check only on program exit |
| `normal` | Standard checking |
| `strict` | More aggressive checking |
| `draconian` | Report all leaks |

**Programmatic control:**
```c
#include <gperftools/heap-checker.h>

void check_section() {
    HeapLeakChecker checker("my_check");

    // Code to check for leaks

    if (!checker.NoLeaks()) {
        fprintf(stderr, "Memory leak detected!\n");
    }
}
```

---

## Integration Examples

### PartsDB with TCMalloc

**Build command:**
```bash
gcc -std=c17 -Wall -Wextra -g3 -O0 -DDEBUG \
    -fno-omit-frame-pointer \
    -I./lib \
    -o partsdb_debug.exe main.c partsdb.c \
    -L./lib -ltcmalloc_minimal -lm
```

**Directory structure:**
```
partsdb-example/
├── lib/
│   ├── gperftools/          # Headers
│   ├── libtcmalloc_minimal.dll
│   └── libtcmalloc_minimal.dll.a
├── main.c
├── partsdb.c
├── partsdb.h
├── partsdb_internal.h
└── Makefile
```

### Memory Statistics Example

```c
/* tcmalloc_stats.c - Print TCMalloc statistics */
#include <stdio.h>
#include <stdlib.h>
#include <gperftools/malloc_extension.h>

void print_tcmalloc_stats(void) {
    size_t value;

    printf("=== TCMalloc Statistics ===\n");

    if (MallocExtension_GetNumericProperty(
            "generic.current_allocated_bytes", &value)) {
        printf("Current allocated: %zu bytes\n", value);
    }

    if (MallocExtension_GetNumericProperty(
            "generic.heap_size", &value)) {
        printf("Heap size: %zu bytes\n", value);
    }

    if (MallocExtension_GetNumericProperty(
            "tcmalloc.pageheap_free_bytes", &value)) {
        printf("Free in pageheap: %zu bytes\n", value);
    }

    // Full stats
    char stats[4096];
    MallocExtension_GetStats(stats, sizeof(stats));
    printf("\nDetailed stats:\n%s\n", stats);
}

int main(void) {
    // Allocate some memory
    void *p1 = malloc(1024);
    void *p2 = malloc(4096);
    void *p3 = malloc(65536);

    print_tcmalloc_stats();

    free(p1);
    free(p2);
    free(p3);

    return 0;
}
```

Compile:
```bash
gcc -o tcmalloc_stats tcmalloc_stats.c -ltcmalloc_minimal
```

### CPU Profiling Example (Linux)

```c
/* profile_example.c - CPU profiling demonstration */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gperftools/profiler.h>

/* Simulate expensive computation */
void expensive_computation(int iterations) {
    volatile double result = 0.0;
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 1000; j++) {
            result += (double)i / (j + 1);
        }
    }
}

/* Simulate memory-intensive work */
void memory_intensive(int iterations) {
    for (int i = 0; i < iterations; i++) {
        char *buffer = malloc(4096);
        memset(buffer, i & 0xFF, 4096);
        free(buffer);
    }
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    if (argc > 1) iterations = atoi(argv[1]);

    printf("Starting profiled execution with %d iterations...\n", iterations);

    /* Start profiling */
    ProfilerStart("profile_output.prof");

    /* Run workloads */
    expensive_computation(iterations);
    memory_intensive(iterations);

    /* Stop profiling */
    ProfilerStop();

    printf("Profile saved to profile_output.prof\n");
    printf("Analyze with: pprof --text ./profile_example profile_output.prof\n");

    return 0;
}
```

Compile and run:
```bash
gcc -g -O2 -o profile_example profile_example.c -lprofiler
./profile_example 50000
pprof --text ./profile_example profile_output.prof
```

---

## Platform Notes

### Windows Limitations

On Windows, gperftools provides limited functionality:

| Feature | Status | Notes |
|---------|--------|-------|
| TCMalloc | Working | tcmalloc_minimal only |
| Memory Stats API | Working | MallocExtension functions |
| CPU Profiler | Limited | Primarily Linux-based |
| Heap Profiler | Limited | May not work with FPO |
| Heap Checker | Limited | Not fully ported |

**Windows-specific issues:**
- `_recalloc()` may not work correctly with TCMalloc
- Frame Pointer Optimization (FPO) affects profiling accuracy
- Signal-based profiling not available

### Linux Requirements

For full functionality on Linux:
- `libunwind` for stack traces
- `graphviz` for profile visualization
- Debug symbols (`-g` flag) for meaningful output

---

## Troubleshooting

### Common Issues

**1. "Undefined reference to WaitOnAddress"**
```
# Windows/MinGW: Add synchronization library
target_link_libraries(common INTERFACE synchronization)
```

**2. Profile is empty or incomplete**
- Ensure program exits normally (not via signal)
- Check that CPUPROFILE is set correctly
- Verify libprofiler is properly linked

**3. Symbols not resolved in pprof**
- Compile with `-g` for debug symbols
- Don't strip binaries used for profiling
- Use same binary for analysis

**4. TCMalloc DLL not found**
```bash
# Windows: Add to PATH
export PATH="./lib:$PATH"

# Or copy DLL to executable directory
cp lib/libtcmalloc_minimal.dll .
```

**5. Profiling overhead too high**
```bash
# Reduce sampling frequency
CPUPROFILE_FREQUENCY=50 CPUPROFILE=output.prof ./myapp
```

### Debug Build Recommendations

For effective profiling:
```bash
gcc -g3 -O0 -fno-omit-frame-pointer \
    -fno-inline \
    -o myapp_debug myapp.c -ltcmalloc_minimal -lprofiler
```

Flags explained:
- `-g3`: Maximum debug information
- `-O0`: No optimization (clearer profiles)
- `-fno-omit-frame-pointer`: Preserve frame pointers for stack traces
- `-fno-inline`: Don't inline functions (clearer call graphs)

---

## Quick Reference

### Build Commands

```bash
# Debug with TCMalloc
gcc -g3 -O0 -DDEBUG -fno-omit-frame-pointer \
    -o app_debug app.c -ltcmalloc_minimal

# Profile-ready build
gcc -g -O2 -fno-omit-frame-pointer \
    -o app_profile app.c -ltcmalloc_minimal -lprofiler

# Release with TCMalloc
gcc -O2 -DNDEBUG -o app app.c -ltcmalloc_minimal
```

### Environment Variables Cheatsheet

```bash
# CPU Profiling
export CPUPROFILE=/tmp/cpu.prof
export CPUPROFILE_FREQUENCY=100
export CPUPROFILE_REALTIME=1

# Heap Profiling
export HEAPPROFILE=/tmp/heap
export HEAP_PROFILE_ALLOCATION_INTERVAL=1073741824

# Heap Checking
export HEAPCHECK=normal

# TCMalloc tuning
export TCMALLOC_MAX_TOTAL_THREAD_CACHE_BYTES=16777216
```

### pprof Commands

```bash
# Interactive
pprof ./app profile.prof

# Text output
pprof --text ./app profile.prof

# Web UI
pprof --http=:8080 ./app profile.prof

# Flame graph
pprof --web ./app profile.prof

# PDF report
pprof --pdf ./app profile.prof > report.pdf

# Compare profiles
pprof --base=before.prof ./app after.prof
```

---

## References

1. gperftools GitHub: https://github.com/gperftools/gperftools
2. CPU Profiler Documentation: https://gperftools.github.io/gperftools/cpuprofile.html
3. Heap Profiler Documentation: https://gperftools.github.io/gperftools/heapprofile.html
4. RidgeRun Profiling Guide: https://developer.ridgerun.com/wiki/index.php/Profiling_with_GPerfTools
5. pprof Tool: https://github.com/google/pprof
