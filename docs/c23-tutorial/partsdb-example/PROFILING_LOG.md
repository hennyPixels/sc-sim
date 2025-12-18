# PartsDB Profiling and Debug Log

**Date:** 2024-12-18
**Platform:** Windows 10 (MINGW64)
**Toolchain:** GCC 15.2.0 (MSYS2)
**gperftools:** Built from source (tcmalloc_minimal)

---

## 1. Environment Setup

### Tools Installed

```
Component                    Version          Source
---------------------------  ---------------  --------
MSYS2                        20250830         winget
GCC (MinGW-w64)              15.2.0           pacman
CMake                        4.2.1            pacman
Ninja                        1.13.2           pacman
gperftools (tcmalloc)        from source      GitHub
```

### Build Configuration

gperftools was built from source with the following fix for Windows:
```cmake
# Added to CMakeLists.txt line 434
target_link_libraries(common INTERFACE synchronization)
```

This resolves the `WaitOnAddress` undefined reference error on Windows.

---

## 2. PartsDB Build with TCMalloc

### Build Command

```bash
gcc -std=c17 -Wall -Wextra -g3 -O0 -DDEBUG \
    -fno-omit-frame-pointer -I./lib \
    -o partsdb_debug.exe main.c partsdb.c \
    -L./lib -ltcmalloc_minimal -lm
```

### Build Result

```
Executable:  partsdb_debug.exe
Size:        488,941 bytes
Linked:      libtcmalloc_minimal.dll
```

### Functional Test

```
$ ./partsdb_debug.exe -n -f test.db list
Parts in database: 0
[PARTSDB DEBUG] partsdb.c:401 partsdb_ensure_capacity(): Resized to capacity 64

$ ./partsdb_debug.exe -f test.db add 1001 "Hex Bolt M10x50" 0.25 500 "Steel hex bolt"
Added part 1001: Hex Bolt M10x50

$ ./partsdb_debug.exe -f test.db stats
Database Statistics:
  Parts:          3
  Capacity:       64
  Memory used:    22608 bytes
  Total inserts:  3
```

---

## 3. TCMalloc Memory Analysis

### Size Class Analysis

TCMalloc uses size classes to reduce fragmentation:

```
Requested Size    Actual Allocation    Overhead
--------------    -----------------    --------
1 byte            8 bytes              7 bytes (700%)
8 bytes           8 bytes              0 bytes (0%)
15 bytes          16 bytes             1 byte (6.7%)
16 bytes          16 bytes             0 bytes (0%)
17 bytes          32 bytes             15 bytes (88%)
32 bytes          32 bytes             0 bytes (0%)
100 bytes         112 bytes            12 bytes (12%)
1000 bytes        1024 bytes           24 bytes (2.4%)
4096 bytes        4096 bytes           0 bytes (0%)
10000 bytes       10240 bytes          240 bytes (2.4%)
```

**Observations:**
- Power-of-2 sizes have zero overhead
- Small allocations (1-16 bytes) round to 8 or 16 bytes
- Larger allocations have minimal percentage overhead
- Page-sized allocations (4096) have zero overhead

### Memory Release Behavior

Test: Allocate 10 MB, free, then release:

```
State                        Allocated    Heap Size     Page Heap Free
---------------------------  -----------  ------------  ---------------
Initial                      272 bytes    1,048,576     0 bytes
After 10 MB allocation       10,486,032   11,534,336    0 bytes
After free()                 272 bytes    11,534,336    10,485,760 bytes
After ReleaseFreeMemory()    272 bytes    11,534,336    0 bytes
```

**Key Finding:** TCMalloc retains freed memory in the page heap for fast reuse. Call `MallocExtension_ReleaseFreeMemory()` to return memory to OS.

### Allocation Performance

Benchmark: 100,000 random-sized allocations (16-4096 bytes)

```
Operation        Time        Rate
---------------  ----------  ------------------
Allocation       54 ms       1,852 allocs/ms
Deallocation     2 ms        50,000 frees/ms
```

**Memory Efficiency:** 96.6% during active allocations

### Thread Cache Statistics

```
Component                        Size
-----------------------------    ---------------
Bytes in use by application      272 bytes
Page heap freelist               193,658,880 bytes (184.7 MiB)
Central cache freelist           12,122,496 bytes (11.6 MiB)
Transfer cache freelist          16,807,424 bytes (16.0 MiB)
Thread cache freelists           4,951,920 bytes (4.7 MiB)
Malloc metadata                  3,276,864 bytes (3.1 MiB)
Total virtual memory             230,817,856 bytes (220.1 MiB)

Spans in use:                    4,188
Thread heaps:                    1
TCMalloc page size:              8,192 bytes
```

---

## 4. Performance Recommendations

### For PartsDB

1. **Use power-of-2 structures:** The Part structure is 352 bytes; consider padding to 512 bytes for optimal TCMalloc sizing.

2. **Batch operations:** TCMalloc's thread cache provides excellent performance for burst allocations. The iterator pattern in PartsDB leverages this.

3. **Periodic memory release:** For long-running instances, call `MallocExtension_ReleaseFreeMemory()` during idle periods.

### General TCMalloc Best Practices

1. **Avoid frequent realloc with growing sizes:** Better to allocate final size upfront

2. **Use tc_malloc_size() to get actual allocation size:** This can avoid unnecessary reallocations

3. **Monitor memory efficiency:** Values below 90% may indicate fragmentation

4. **Configure thread cache:** For high-thread-count applications:
   ```bash
   export TCMALLOC_MAX_TOTAL_THREAD_CACHE_BYTES=33554432  # 32 MB
   ```

---

## 5. Platform-Specific Notes

### Windows Limitations

| Feature | Status | Notes |
|---------|--------|-------|
| TCMalloc (tcmalloc_minimal) | Working | Full functionality |
| MallocExtension API | Working | Memory statistics available |
| CPU Profiler | Not available | Linux-specific (uses ITIMER) |
| Heap Profiler | Limited | May not work with FPO |
| Heap Checker | Limited | Not fully ported |

### Build Requirements

On Windows/MSYS2, link against:
- `-ltcmalloc_minimal`
- `-lsynchronization` (for spinlock support)

### DLL Deployment

```bash
# Option 1: Add to PATH
export PATH="./lib:$PATH"

# Option 2: Copy to application directory
cp lib/libtcmalloc_minimal.dll .
```

---

## 6. Code Examples

### Integrating TCMalloc Statistics

```c
#include <gperftools/malloc_extension_c.h>

void monitor_memory(void) {
    size_t allocated, heap_size;

    MallocExtension_GetNumericProperty(
        "generic.current_allocated_bytes", &allocated);
    MallocExtension_GetNumericProperty(
        "generic.heap_size", &heap_size);

    printf("Memory: %zu / %zu bytes (%.1f%% efficiency)\n",
           allocated, heap_size,
           (double)allocated / heap_size * 100.0);

    // Release memory if efficiency is low
    if (heap_size > 0 && (double)allocated / heap_size < 0.5) {
        MallocExtension_ReleaseFreeMemory();
    }
}
```

### Checking Actual Allocation Size

```c
#include <gperftools/tcmalloc.h>

void *allocate_optimal(size_t requested) {
    void *ptr = malloc(requested);
    if (ptr) {
        size_t actual = tc_malloc_size(ptr);
        // actual >= requested, can use extra space
        printf("Requested %zu, got %zu\n", requested, actual);
    }
    return ptr;
}
```

---

## 7. Troubleshooting Log

### Issue 1: Link Error - WaitOnAddress undefined

**Error:**
```
undefined reference to `WaitOnAddress'
undefined reference to `WakeByAddressAll'
```

**Cause:** Windows synchronization functions not linked

**Solution:** Add to CMakeLists.txt after line 432:
```cmake
target_link_libraries(common INTERFACE synchronization)
```

### Issue 2: Missing stdarg.h

**Error:**
```
error: implicit declaration of function 'va_start'
```

**Solution:** Add `#include <stdarg.h>` to main.c

### Issue 3: DLL not found at runtime

**Error:**
```
The code execution cannot proceed because libtcmalloc_minimal.dll was not found.
```

**Solution:**
```bash
export PATH="./lib:$PATH"
# or copy DLL to current directory
```

---

## 8. Conclusion

TCMalloc integration with PartsDB is fully functional on Windows. Key benefits observed:

1. **High performance:** 1,800+ allocations/ms, 50,000+ frees/ms
2. **Low fragmentation:** 96.6% memory efficiency during workloads
3. **Rich statistics:** Full memory monitoring via MallocExtension API
4. **Size-class optimization:** Zero overhead for power-of-2 allocations

For production use on Windows, consider:
- Using release builds with `-O2 -DNDEBUG`
- Periodic memory release for long-running processes
- Monitoring memory efficiency to detect fragmentation

For full profiling capabilities (CPU profiler, heap profiler), use Linux with the complete gperftools suite.

---

## References

- gperftools Repository: https://github.com/gperftools/gperftools
- CPU Profiler Documentation: https://gperftools.github.io/gperftools/cpuprofile.html
- Heap Profiler Documentation: https://gperftools.github.io/gperftools/heapprofile.html
- RidgeRun Profiling Guide: https://developer.ridgerun.com/wiki/index.php/Profiling_with_GPerfTools
