# Session Log: gperftools Integration Journey

**Session Date:** December 18, 2024
**Duration:** Extended session with multiple troubleshooting phases
**Objective:** Build and test PartsDB with Google Performance Tools (gperftools)
**Platform:** Windows 10 (Build 26100.7462)
**Shell:** Git Bash (MINGW64)

---

## Prologue: The Mission

The user requested integration of Google Performance Tools (gperftools) with the PartsDB example application from the C23 Tutorial. The specific requirements were:

1. Use gperftools instead of Valgrind (which requires Linux/WSL)
2. Build natively on Windows (avoid WSL)
3. Create comprehensive debugging and profiling documentation
4. Reference official gperftools documentation

This log documents the complete journey, including all failures, troubleshooting steps, and eventual successes.

---

## Chapter 1: The Search for a Compiler

### 1.1 Initial Assessment

**Time:** Session start
**Goal:** Find a C compiler on the Windows system

The first challenge was discovering that no C compiler was readily available in the PATH:

```
Attempt 1: Check for MSVC
$ where cl.exe
INFO: Could not find files for the given pattern(s).
Result: FAILED - No MSVC compiler found

Attempt 2: Check for GCC
$ where gcc.exe
INFO: Could not find files for the given pattern(s).
Result: FAILED - No GCC found

Attempt 3: Check for CMake
$ where cmake.exe
INFO: Could not find files for the given pattern(s).
Result: FAILED - No CMake found
```

**Analysis:** Despite Visual Studio 2025 Community being installed, only LLVM formatting tools (clang-format, clang-tidy) were available - not a full compiler toolchain.

### 1.2 Discovery of Package Managers

We searched for available package managers:

```
$ winget --version
v1.9.25200
Result: SUCCESS - winget available!

$ choco --version
Result: FAILED - Chocolatey not installed

$ scoop --version
Result: FAILED - Scoop not installed
```

**Decision:** Use winget to install MSYS2, which provides a complete MinGW-w64 toolchain.

### 1.3 MSYS2 Installation

```bash
$ winget install MSYS2.MSYS2
Found MSYS2 Installer [MSYS2.MSYS2] Version 20250830
Starting package install...
Successfully installed
Result: SUCCESS
```

**Lesson Learned:** Windows development often requires installing a Unix-like environment. MSYS2 provides this along with the pacman package manager.

---

## Chapter 2: Setting Up the Toolchain

### 2.1 Finding MSYS2

After installation, MSYS2 was located at `C:\msys64`. The key was accessing it through the Git Bash shell:

```bash
# Windows paths don't work directly in bash
$ C:\msys64\usr\bin\pacman.exe --version
/usr/bin/bash: line 1: C:msys64usrbinpacman.exe: command not found
Result: FAILED - Backslash path interpretation issue

# Solution: Use Unix-style paths
$ /c/msys64/usr/bin/pacman.exe --version
Pacman v6.1.0 - libalpm v14.0.0
Result: SUCCESS
```

**Key Insight:** In Git Bash, Windows paths like `C:\path` must be written as `/c/path`.

### 2.2 Installing Build Tools

```bash
# Update package database
$ /c/msys64/usr/bin/pacman.exe -Sy --noconfirm
:: Synchronizing package databases...
 clangarm64 downloading...
 mingw32 downloading...
 mingw64 downloading...
Result: SUCCESS

# Install GCC
$ /c/msys64/usr/bin/pacman.exe -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-make
Packages (18): mingw-w64-x86_64-gcc-15.2.0-8 ...
Total Installed Size: 549.50 MiB
Result: SUCCESS

# Verify installation
$ /c/msys64/mingw64/bin/gcc.exe --version
gcc.exe (Rev8, Built by MSYS2 project) 15.2.0
Result: SUCCESS - GCC 15.2.0 installed!
```

### 2.3 Installing Additional Tools

```bash
# CMake and Ninja for building gperftools
$ /c/msys64/usr/bin/pacman.exe -S --noconfirm mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja git
Packages (66): mingw-w64-x86_64-cmake-4.2.1-1 mingw-w64-x86_64-ninja-1.13.2-1 ...
Result: SUCCESS

# Autotools for potential autoconf builds
$ /c/msys64/usr/bin/pacman.exe -S --noconfirm autoconf automake libtool make
Packages (19): autoconf-wrapper-20250528-1 ...
Result: SUCCESS (minor post-install hook error, ignorable)
```

---

## Chapter 3: The gperftools Build Saga

### 3.1 Checking for Pre-built Packages

```bash
$ /c/msys64/usr/bin/pacman.exe -Ss gperftools
Result: No results (exit code 1)

$ /c/msys64/usr/bin/pacman.exe -Ss tcmalloc
Result: No results

$ /c/msys64/usr/bin/pacman.exe -Ss profiler
Found: samply (a different profiler, not gperftools)
```

**Conclusion:** gperftools is not available as a pre-built MSYS2 package. We must build from source.

### 3.2 Cloning gperftools

```bash
# First attempt with MSYS2 git (failed)
$ /c/msys64/usr/bin/git.exe clone https://github.com/gperftools/gperftools.git
fatal: remote helper 'https' aborted session
Result: FAILED - MSYS2 git has HTTPS issues

# Solution: Use Windows git (from Git for Windows)
$ git clone --depth 1 https://github.com/gperftools/gperftools.git
Cloning into 'gperftools'...
Result: SUCCESS
```

**Lesson:** MSYS2's git may have SSL/HTTPS configuration issues. The system's Git for Windows works better for cloning.

### 3.3 CMake Configuration

```bash
$ mkdir build && cd build
$ PATH="/c/msys64/mingw64/bin:$PATH" cmake -G "Ninja" \
    -DCMAKE_C_COMPILER=/c/msys64/mingw64/bin/gcc.exe \
    -DCMAKE_CXX_COMPILER=/c/msys64/mingw64/bin/g++.exe \
    ../gperftools

-- The C compiler identification is GNU 15.2.0
-- The CXX compiler identification is GNU 15.2.0
-- Performing Test x86_64 - Success
-- Looking for sbrk - not found (Windows doesn't have sbrk)
-- Looking for libunwind.h - not found (Linux-specific)
CMake Warning: gperftools' cmake support is incomplete and is best-effort only
-- Configuring done (13.8s)
Result: SUCCESS (with expected warnings)
```

### 3.4 First Build Attempt - FAILURE

```bash
$ ninja
[78/123] Linking CXX shared library libtcmalloc_minimal.dll
FAILED: libtcmalloc_minimal.dll

Error message:
undefined reference to `WaitOnAddress'
undefined reference to `WakeByAddressAll'
undefined reference to `WakeByAddressSingle'

Result: FAILED - Missing Windows synchronization library
```

**Root Cause Analysis:**

The functions `WaitOnAddress`, `WakeByAddressAll`, and `WakeByAddressSingle` are Windows 8+ synchronization primitives. They're declared in `<synchapi.h>` and implemented in `synchronization.dll`.

The gperftools CMakeLists.txt has:
```cmake
link_libraries(psapi synchronization shlwapi)
```

But this applies the library globally, and due to static library link order rules, `libsynchronization.a` was being linked BEFORE `libcommon.a` (which needs the symbols).

### 3.5 The Fix

```bash
# Check if library exists
$ ls /c/msys64/mingw64/lib | grep sync
libsynchronization.a  # EXISTS!

# Verify symbols are in the library
$ /c/msys64/mingw64/bin/nm.exe /c/msys64/mingw64/lib/libsynchronization.a | grep Wait
0000000000000000 T WaitOnAddress  # Symbol IS there!

# The fix: Add target_link_libraries to common library
# Added after line 432 in CMakeLists.txt:
target_link_libraries(common INTERFACE synchronization)
```

**Why INTERFACE?** Using `INTERFACE` means any target that links against `common` will automatically also link against `synchronization`, and in the correct order.

### 3.6 Successful Build

```bash
# Reconfigure with fix
$ rm -rf * && cmake -G "Ninja" ../gperftools

# Build
$ ninja
[1/123] Building CXX object CMakeFiles/common.dir/src/base/spinlock_internal.cc.obj
...
[121/123] Linking CXX executable for_each_line_test.exe
[122/123] Linking CXX executable min_per_thread_cache_size_test.exe
[123/123] Linking CXX executable stack_trace_table_test.exe

Result: SUCCESS! All 123 targets built.
```

**Build Artifacts:**
```
libtcmalloc_minimal.dll      3,178,943 bytes  (Main library)
libtcmalloc_minimal.dll.a      131,010 bytes  (Import library)
+ 24 test executables
```

---

## Chapter 4: Building PartsDB with TCMalloc

### 4.1 Setting Up Libraries

```bash
# Create lib directory in PartsDB example
$ mkdir -p docs/c23-tutorial/partsdb-example/lib

# Copy gperftools artifacts
$ cp /tmp/gperftools-build/build/libtcmalloc_minimal.dll* lib/
$ cp -r /tmp/gperftools-build/gperftools/src/gperftools lib/
```

### 4.2 First Compile Attempt - FAILURE

```bash
$ gcc -std=c17 -Wall -Wextra -g3 -O0 -DDEBUG \
    -I./lib -o partsdb_debug.exe main.c partsdb.c \
    -L./lib -ltcmalloc_minimal -lm

main.c: In function 'fatal':
main.c:57:5: error: implicit declaration of function 'va_start'
main.c:59:5: error: implicit declaration of function 'va_end'

Result: FAILED - Missing header include
```

**Root Cause:** The `main.c` file uses variadic functions (`va_start`, `va_end`) but was missing `#include <stdarg.h>`.

### 4.3 The Fix

```c
// Added to main.c after other includes:
#include <stdarg.h>
```

### 4.4 Successful Build

```bash
$ gcc -std=c17 -Wall -Wextra -g3 -O0 -DDEBUG \
    -fno-omit-frame-pointer -I./lib \
    -o partsdb_debug.exe main.c partsdb.c \
    -L./lib -ltcmalloc_minimal -lm

Result: SUCCESS

$ ls -la partsdb_debug.exe
-rwxr-xr-x 1 gd 197121 488941 Dec 18 17:15 partsdb_debug.exe
```

### 4.5 Functional Testing

```bash
$ PATH="./lib:$PATH" ./partsdb_debug.exe --help
Usage: partsdb_debug.exe [options] <command> [args...]
Result: SUCCESS - Program runs!

$ ./partsdb_debug.exe -n -f test.db list
Parts in database: 0
[PARTSDB DEBUG] partsdb.c:401 partsdb_ensure_capacity(): Resized to capacity 64
Result: SUCCESS - Database operations work!

$ ./partsdb_debug.exe -f test.db add 1001 "Hex Bolt" 0.25 500
Added part 1001: Hex Bolt
Result: SUCCESS - TCMalloc handling allocations!
```

---

## Chapter 5: TCMalloc Performance Analysis

### 5.1 Creating the Demo Program

A comprehensive demo program (`tcmalloc_demo.c`) was created to exercise TCMalloc's features:

- Memory allocation benchmarks
- Size class analysis
- Memory release behavior
- Statistics API usage

### 5.2 Build and Run

```bash
$ gcc -O2 -I./lib -o tcmalloc_demo.exe tcmalloc_demo.c -L./lib -ltcmalloc_minimal
Result: SUCCESS

$ PATH="./lib:$PATH" ./tcmalloc_demo.exe --verbose
```

### 5.3 Key Findings

**Size Class Overhead:**
```
Requested    Actual    Overhead
1 byte       8 bytes   700% (minimum allocation unit)
17 bytes     32 bytes  88%  (rounds to next power of 2)
4096 bytes   4096      0%   (page-aligned, no overhead)
```

**Memory Release Behavior:**
```
State                    Allocated     Heap Size      Page Heap Free
Initial                  272 bytes     1,048,576      0
After 10 MB alloc        10,486,032    11,534,336     0
After free()             272 bytes     11,534,336     10,485,760 (cached!)
After ReleaseFreeMemory  272 bytes     11,534,336     0 (returned to OS)
```

**Key Insight:** TCMalloc retains freed memory in a cache for fast reuse. Call `MallocExtension_ReleaseFreeMemory()` to return memory to the OS.

**Performance Benchmarks:**
```
Operation          Time        Rate
Allocation         54 ms       1,852 allocs/ms
Deallocation       2 ms        50,000 frees/ms
Memory Efficiency: 96.6% during active use
```

**Detailed Statistics:**
```
MALLOC:            272 (    0.0 MiB) Bytes in use by application
MALLOC: +    193,658,880 (  184.7 MiB) Bytes in page heap freelist
MALLOC: +     12,122,496 (   11.6 MiB) Bytes in central cache freelist
MALLOC: +     16,807,424 (   16.0 MiB) Bytes in transfer cache freelist
MALLOC: +      4,951,920 (    4.7 MiB) Bytes in thread cache freelists
MALLOC: +      3,276,864 (    3.1 MiB) Bytes in malloc metadata
MALLOC:   ------------
MALLOC: =    230,817,856 (  220.1 MiB) Actual memory used

Spans in use:         4,188
Thread heaps:         1
TCMalloc page size:   8,192 bytes
```

---

## Chapter 6: Documentation Created

### 6.1 Files Generated

| File | Purpose | Lines |
|------|---------|-------|
| `GPERFTOOLS_GUIDE.md` | Comprehensive usage guide | ~450 |
| `PROFILING_LOG.md` | Detailed profiling results | ~200 |
| `SESSION_LOG.md` | This narrative log | ~600+ |
| `tcmalloc_demo.c` | Demo/benchmark program | ~250 |

### 6.2 Makefile Updates

Added new build targets:
```makefile
tcmalloc:      # Build PartsDB with TCMalloc
tcmalloc-demo: # Build TCMalloc demo program
```

Updated help text and clean targets accordingly.

---

## Chapter 7: Lessons Learned

### 7.1 Windows Native Development Challenges

1. **No built-in C compiler:** Unlike Linux/macOS, Windows requires explicit toolchain installation
2. **Path format issues:** Windows paths (`C:\`) don't work in bash; use Unix paths (`/c/`)
3. **Link order matters:** Static libraries must be ordered so that libraries providing symbols come AFTER objects that need them
4. **DLL deployment:** Windows executables need DLLs in PATH or current directory

### 7.2 gperftools on Windows

1. **Limited feature set:** Only `tcmalloc_minimal` is fully supported
2. **No CPU profiler:** The CPU profiler uses Linux-specific `ITIMER` signals
3. **No heap checker:** Memory leak detection not fully ported
4. **Build from source required:** No pre-built packages available

### 7.3 Debugging Strategy

When encountering link errors:
1. Check if the library exists (`ls`, `find`)
2. Check if symbols are in the library (`nm`)
3. Analyze link order in the build command
4. Consider INTERFACE vs PRIVATE vs PUBLIC link scope

---

## Chapter 8: Platform Comparison

### What Works Where

| Feature | Linux | macOS | Windows |
|---------|-------|-------|---------|
| TCMalloc | Full | Full | tcmalloc_minimal |
| CPU Profiler | Full | Full | Not available |
| Heap Profiler | Full | Limited | Limited |
| Heap Checker | Full | Limited | Not available |
| Build complexity | Low | Medium | High |

### Recommended Approach by Platform

**Linux:** Full gperftools via package manager + pprof
**macOS:** Instruments.app + limited gperftools
**Windows:** TCMalloc for performance + Visual Studio profiler for CPU

---

## Epilogue: What We Achieved

Starting from a Windows system with no C compiler, we:

1. ✅ Installed a complete MinGW-w64 toolchain via MSYS2
2. ✅ Built gperftools from source (fixing Windows-specific issues)
3. ✅ Integrated TCMalloc with the PartsDB application
4. ✅ Created comprehensive benchmarks showing 1,800+ allocs/ms
5. ✅ Documented everything for future reference

The PartsDB example now demonstrates:
- Memory-efficient allocation patterns
- Performance monitoring via MallocExtension API
- Best practices for TCMalloc integration

**Total time investment:** Multiple hours of troubleshooting, but resulting in a complete, working solution with comprehensive documentation.

---

## Appendix A: Quick Reference Commands

```bash
# Build everything
cd docs/c23-tutorial/partsdb-example
make tcmalloc
make tcmalloc-demo

# Run with TCMalloc
export PATH="./lib:$PATH"
./partsdb_tcmalloc.exe -f test.db list
./tcmalloc_demo.exe --verbose

# Check TCMalloc is being used
nm partsdb_tcmalloc.exe | grep tc_malloc
```

## Appendix B: Error Solutions Quick Reference

| Error | Cause | Solution |
|-------|-------|----------|
| `undefined reference to WaitOnAddress` | Missing sync lib | Add `target_link_libraries(common INTERFACE synchronization)` |
| `implicit declaration of va_start` | Missing header | Add `#include <stdarg.h>` |
| `DLL not found` | Not in PATH | `export PATH="./lib:$PATH"` |
| `command not found` (Windows path) | Path format | Use `/c/path` not `C:\path` |

---

*End of Session Log*
