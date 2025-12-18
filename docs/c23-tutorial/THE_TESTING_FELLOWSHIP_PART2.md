# The Testing Fellowship

## Part II: The Two Allocators

---

```
    "The world is changed. I feel it in the heap.
     I feel it in the stack. I smell it in the registers.
     Much that once was allocated is lost,
     for none now live who remember to free it."

                                        — Galadriel, Lady of the Memory Pool
```

---

# PART II: THE TWO ALLOCATORS

## Prologue: The Schism of Memory Management

In the ages after Doug Lea created his malloc, the world of memory allocation split into two great kingdoms.

In the East rose **TCMalloc**, forged in the fires of Google's data centers, optimized for the many-threaded workloads of web services.

In the West grew **jemalloc**, born in the halls of FreeBSD, adopted by Facebook, designed for the complex allocation patterns of modern applications.

Both claimed to be the true heir to Doug Lea's legacy. Both promised performance. Both had their champions.

This is the tale of their rivalry, and how the Fellowship learned from both.

---

## Chapter 1: The Architecture of TCMalloc

### The Google Way

TCMalloc was born from necessity. Google's servers handled billions of requests, each spawning threads, each thread allocating memory. The old ways—a single lock protecting the heap—could not scale.

Sanjay Ghemawat and Paul Menage designed a new architecture: give each thread its own cache.

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                     THE REALM OF TCMALLOC                            ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                                                                      ┃
┃    ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                ┃
┃    │  Thread 1   │  │  Thread 2   │  │  Thread 3   │                ┃
┃    │   Cache     │  │   Cache     │  │   Cache     │   TIER 1       ┃
┃    │ ┌─────────┐ │  │ ┌─────────┐ │  │ ┌─────────┐ │   Thread       ┃
┃    │ │ 8 byte  │ │  │ │ 8 byte  │ │  │ │ 8 byte  │ │   Caches       ┃
┃    │ │ 16 byte │ │  │ │ 16 byte │ │  │ │ 16 byte │ │   (No locks!)  ┃
┃    │ │ 32 byte │ │  │ │ 32 byte │ │  │ │ 32 byte │ │                ┃
┃    │ │   ...   │ │  │ │   ...   │ │  │ │   ...   │ │                ┃
┃    │ └─────────┘ │  │ └─────────┘ │  │ └─────────┘ │                ┃
┃    └──────┬──────┘  └──────┬──────┘  └──────┬──────┘                ┃
┃           │                │                │                        ┃
┃           └────────────────┼────────────────┘                        ┃
┃                            ▼                                         ┃
┃    ┌──────────────────────────────────────────────────────┐         ┃
┃    │                  CENTRAL CACHE                        │  TIER 2 ┃
┃    │  ┌────────────────────────────────────────────────┐  │  Central ┃
┃    │  │  Transfer Cache (batch transfers)              │  │  Cache   ┃
┃    │  │  Central Free Lists (size-segregated)          │  │  (Locked)┃
┃    │  └────────────────────────────────────────────────┘  │         ┃
┃    └──────────────────────────┬───────────────────────────┘         ┃
┃                               ▼                                      ┃
┃    ┌──────────────────────────────────────────────────────┐         ┃
┃    │                    PAGE HEAP                          │  TIER 3 ┃
┃    │  ┌────────────────────────────────────────────────┐  │  Page    ┃
┃    │  │  Span Management (contiguous pages)            │  │  Heap    ┃
┃    │  │  Page Map (virtual address → span lookup)      │  │  (Rare)  ┃
┃    │  │  System Allocator Interface (mmap/VirtualAlloc)│  │         ┃
┃    │  └────────────────────────────────────────────────┘  │         ┃
┃    └──────────────────────────────────────────────────────┘         ┃
┃                                                                      ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### The Size Classes

TCMalloc does not allocate exact sizes. It rounds up to predefined "size classes"—a wisdom learned from Doug Lea. This reduces fragmentation and allows objects of the same size to be managed together.

```c
/*
 * THE SIZE CLASSES OF TCMALLOC
 *
 * When you request memory, TCMalloc rounds up to the next size class.
 * This is not waste—it is wisdom.
 *
 * By grouping similar sizes together:
 *   - Free lists are more efficient
 *   - Fragmentation is reduced
 *   - Cache locality improves
 */

/*
 * Observed size classes from our profiling session:
 *
 * ┌────────────────────────────────────────────────────────────┐
 * │  Requested  │  Allocated  │  Overhead  │  Efficiency       │
 * ├─────────────┼─────────────┼────────────┼───────────────────┤
 * │    1 byte   │    8 bytes  │   7 bytes  │   12.5%           │
 * │    8 bytes  │    8 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │   15 bytes  │   16 bytes  │   1 byte   │   93.8%           │
 * │   16 bytes  │   16 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │   17 bytes  │   32 bytes  │  15 bytes  │   53.1%           │
 * │   32 bytes  │   32 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │  100 bytes  │  112 bytes  │  12 bytes  │   89.3%           │
 * │ 1000 bytes  │ 1024 bytes  │  24 bytes  │   97.7%           │
 * │ 4096 bytes  │ 4096 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │10000 bytes  │10240 bytes  │ 240 bytes  │   97.7%           │
 * └────────────────────────────────────────────────────────────┘
 *
 * THE WISDOM: Align your structures to power-of-2 sizes!
 */

/* Example: Demonstrating size class behavior */
#include <stdio.h>
#include <stdlib.h>
#include <gperftools/tcmalloc.h>

void demonstrate_size_classes(void) {
    size_t test_sizes[] = {1, 8, 15, 16, 17, 32, 100, 1000, 4096, 10000};
    size_t num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    printf("TCMalloc Size Class Demonstration\n");
    printf("%-12s %-12s %-12s %-12s\n",
           "Requested", "Allocated", "Overhead", "Efficiency");
    printf("%-12s %-12s %-12s %-12s\n",
           "----------", "----------", "----------", "----------");

    for (size_t i = 0; i < num_tests; i++) {
        void *ptr = malloc(test_sizes[i]);
        size_t actual = tc_malloc_size(ptr);  /* TCMalloc extension */
        size_t overhead = actual - test_sizes[i];
        double efficiency = (double)test_sizes[i] / actual * 100.0;

        printf("%-12zu %-12zu %-12zu %-11.1f%%\n",
               test_sizes[i], actual, overhead, efficiency);

        free(ptr);
    }
}
```

### The Thread Cache Dance

When a thread requests memory, a beautiful dance occurs:

```c
/*
 * THE ALLOCATION DANCE
 *
 * Step 1: Check Thread Cache (fast path)
 *   - No locks! Thread-local storage.
 *   - If object available in free list → return immediately
 *   - Time: ~10-20 nanoseconds
 *
 * Step 2: Refill from Central Cache (slow path)
 *   - Acquire lock on central cache
 *   - Transfer batch of objects to thread cache
 *   - Release lock
 *   - Time: ~100-200 nanoseconds
 *
 * Step 3: Grow from Page Heap (rare path)
 *   - Acquire page heap lock
 *   - Allocate new span of pages
 *   - Carve into objects, populate central cache
 *   - Time: ~1000+ nanoseconds
 */

/*
 * Pseudocode for TCMalloc's allocation:
 */
void *tcmalloc_allocate(size_t size) {
    /* Round up to size class */
    size_t size_class = SizeToClass(size);

    /* FAST PATH: Thread cache */
    ThreadCache *tc = GetThreadCache();
    if (tc->free_list[size_class] != NULL) {
        void *result = tc->free_list[size_class];
        tc->free_list[size_class] = *(void **)result;
        return result;  /* No locks! */
    }

    /* SLOW PATH: Refill from central cache */
    return RefillThreadCache(tc, size_class);
}

void tcmalloc_free(void *ptr) {
    size_t size_class = GetSizeClass(ptr);
    ThreadCache *tc = GetThreadCache();

    /* Return to thread cache (no locks!) */
    *(void **)ptr = tc->free_list[size_class];
    tc->free_list[size_class] = ptr;

    /* If thread cache is too full, return some to central cache */
    if (tc->size[size_class] > tc->max_size[size_class]) {
        ReleaseToCentralCache(tc, size_class);
    }
}
```

---

## Chapter 2: The Architecture of jemalloc

### The Facebook Way

While Google built TCMalloc, Jason Evans at FreeBSD created jemalloc. Later adopted by Facebook (now Meta), jemalloc took a different approach to the same problem.

Where TCMalloc uses thread caches, jemalloc uses **arenas**—independent heaps that threads can access.

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                      THE REALM OF JEMALLOC                           ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                                                                      ┃
┃    ┌─────────────────────────────────────────────────────────┐      ┃
┃    │                      ARENAS                              │      ┃
┃    │  ┌───────────┐  ┌───────────┐  ┌───────────┐            │      ┃
┃    │  │  Arena 0  │  │  Arena 1  │  │  Arena 2  │  ...       │      ┃
┃    │  │  ┌─────┐  │  │  ┌─────┐  │  │  ┌─────┐  │            │      ┃
┃    │  │  │Bins │  │  │  │Bins │  │  │  │Bins │  │            │      ┃
┃    │  │  │ 8B  │  │  │  │ 8B  │  │  │  │ 8B  │  │            │      ┃
┃    │  │  │16B  │  │  │  │16B  │  │  │  │16B  │  │            │      ┃
┃    │  │  │32B  │  │  │  │32B  │  │  │  │32B  │  │            │      ┃
┃    │  │  │...  │  │  │  │...  │  │  │  │...  │  │            │      ┃
┃    │  │  └─────┘  │  │  └─────┘  │  │  └─────┘  │            │      ┃
┃    │  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘            │      ┃
┃    └────────┼──────────────┼──────────────┼──────────────────┘      ┃
┃             │              │              │                          ┃
┃    Thread 1─┘    Thread 2──┴──Thread 3    └─Thread 4                 ┃
┃    (bound to     (can use any arena,      (bound to                  ┃
┃     Arena 0)      picks least contended)   Arena 2)                  ┃
┃                                                                      ┃
┃    ┌─────────────────────────────────────────────────────────┐      ┃
┃    │                    THREAD CACHES                         │      ┃
┃    │   (Optional layer, similar to TCMalloc's thread caches)  │      ┃
┃    │   Introduced in later versions for even faster allocs    │      ┃
┃    └─────────────────────────────────────────────────────────┘      ┃
┃                                                                      ┃
┃    ┌─────────────────────────────────────────────────────────┐      ┃
┃    │                       EXTENTS                            │      ┃
┃    │   Large allocations managed separately                   │      ┃
┃    │   Uses red-black trees for coalescing                    │      ┃
┃    └─────────────────────────────────────────────────────────┘      ┃
┃                                                                      ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### The Philosophy Difference

```c
/*
 * TCMALLOC vs JEMALLOC: A PHILOSOPHICAL DIFFERENCE
 *
 * TCMalloc says: "Give each thread its own cache."
 *   - Thread-local storage for maximum speed
 *   - Threads are isolated, no sharing
 *   - Simple mental model
 *
 * jemalloc says: "Create multiple arenas, let threads choose."
 *   - Arenas can be shared or dedicated
 *   - More flexible for diverse workloads
 *   - Better for producer-consumer patterns
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    ALLOCATION PATTERNS                        │
 * ├──────────────────────────────────────────────────────────────┤
 * │                                                               │
 * │  TCMALLOC excels when:                                       │
 * │    - Threads allocate and free their own memory              │
 * │    - Workload is homogeneous (similar allocation sizes)      │
 * │    - High allocation rate, many small objects                │
 * │                                                               │
 * │  jemalloc excels when:                                       │
 * │    - Memory is passed between threads                        │
 * │    - Workload is heterogeneous (varied allocation sizes)     │
 * │    - Fragmentation is a concern                              │
 * │    - Memory needs to be returned to OS predictably           │
 * │                                                               │
 * └──────────────────────────────────────────────────────────────┘
 */
```

---

## Chapter 3: The Battle of Benchmarks

### The Fellowship's Test

The Fellowship decided to put both allocators to the test. They created a benchmark that would reveal the strengths and weaknesses of each.

```c
/*
 * THE GREAT ALLOCATOR BENCHMARK
 *
 * This code was run during the Fellowship's gperftools session.
 * Results below are from actual measurements.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ALLOCATIONS 100000
#define MIN_SIZE 16
#define MAX_SIZE 4096

typedef struct {
    const char *name;
    double alloc_time_ms;
    double free_time_ms;
    double allocs_per_ms;
    double frees_per_ms;
    double memory_efficiency;
} BenchmarkResult;

void run_allocation_benchmark(BenchmarkResult *result) {
    void **ptrs = malloc(NUM_ALLOCATIONS * sizeof(void *));
    clock_t start, end;

    /* Allocation phase */
    start = clock();
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = MIN_SIZE + (rand() % (MAX_SIZE - MIN_SIZE));
        ptrs[i] = malloc(size);
        memset(ptrs[i], i & 0xFF, size);  /* Touch memory */
    }
    end = clock();
    result->alloc_time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    result->allocs_per_ms = NUM_ALLOCATIONS / result->alloc_time_ms;

    /* Deallocation phase */
    start = clock();
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        free(ptrs[i]);
    }
    end = clock();
    result->free_time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    result->frees_per_ms = NUM_ALLOCATIONS / result->free_time_ms;

    free(ptrs);
}

/*
 * ACTUAL RESULTS FROM THE FELLOWSHIP'S SESSION
 * Platform: Windows 10, MSYS2/MinGW64, GCC 15.2.0
 * Allocator: TCMalloc (tcmalloc_minimal)
 *
 * ┌────────────────────────────────────────────────────────────────┐
 * │                    TCMALLOC RESULTS                             │
 * ├────────────────────────────────────────────────────────────────┤
 * │  Metric                    │  Value                            │
 * ├────────────────────────────┼───────────────────────────────────┤
 * │  Allocation Time           │  54 ms                            │
 * │  Allocations per ms        │  1,852                            │
 * │  Deallocation Time         │  2 ms                             │
 * │  Deallocations per ms      │  50,000                           │
 * │  Memory Efficiency         │  96.6%                            │
 * │  Peak Heap Size            │  227 MB                           │
 * │  Thread Cache Usage        │  4.8 MB                           │
 * └────────────────────────────┴───────────────────────────────────┘
 *
 * INTERPRETATION:
 *   - Deallocation is 27x faster than allocation
 *   - This is because freed memory goes to thread cache (no syscall)
 *   - 96.6% efficiency means minimal fragmentation
 *   - Thread cache holds recently freed objects for instant reuse
 */
```

### The Memory Efficiency Mystery

During the benchmark, the Fellowship observed something curious:

```c
/*
 * THE MEMORY EFFICIENCY MYSTERY
 *
 * After allocating 100,000 objects totaling ~210 MB of requested memory,
 * the heap size was 227 MB. That's 96.6% efficiency.
 *
 * But after freeing everything, the heap remained at 227 MB!
 * Only 272 bytes were "in use," yet 227 MB was still claimed.
 *
 * This is not a bug. This is TCMalloc's CACHING STRATEGY.
 */

/*
 * MEMORY STATES OBSERVED:
 *
 * ┌─────────────────────────────────────────────────────────────────┐
 * │  State                    │ Allocated │ Heap Size │ Efficiency  │
 * ├───────────────────────────┼───────────┼───────────┼─────────────┤
 * │  Initial                  │    272 B  │    1 MB   │    0.0%     │
 * │  After 100K allocations   │   219 MB  │  227 MB   │   96.6%     │
 * │  After freeing all        │    272 B  │  227 MB   │    0.0%     │
 * │  After ReleaseFreeMemory  │    272 B  │  227 MB   │    0.0%     │
 * └─────────────────────────────────────────────────────────────────┘
 *
 * WHERE DID THE MEMORY GO?
 *
 * After freeing:
 *   - Page heap freelist:     193 MB  (ready for reuse)
 *   - Central cache freelist:  12 MB  (ready for reuse)
 *   - Transfer cache:          16 MB  (ready for reuse)
 *   - Thread cache:             5 MB  (ready for reuse)
 *   - Metadata overhead:        3 MB  (bookkeeping)
 *
 * The memory is not lost—it's CACHED for future allocations.
 * This makes subsequent allocations blazingly fast.
 */

#include <gperftools/malloc_extension_c.h>

void explain_memory_caching(void) {
    size_t allocated, heap_size, pageheap_free, thread_cache;

    MallocExtension_GetNumericProperty(
        "generic.current_allocated_bytes", &allocated);
    MallocExtension_GetNumericProperty(
        "generic.heap_size", &heap_size);
    MallocExtension_GetNumericProperty(
        "tcmalloc.pageheap_free_bytes", &pageheap_free);
    MallocExtension_GetNumericProperty(
        "tcmalloc.current_total_thread_cache_bytes", &thread_cache);

    printf("\n=== TCMalloc Memory Analysis ===\n");
    printf("Currently allocated:  %12zu bytes\n", allocated);
    printf("Total heap size:      %12zu bytes\n", heap_size);
    printf("Cached in page heap:  %12zu bytes\n", pageheap_free);
    printf("Cached in thread:     %12zu bytes\n", thread_cache);
    printf("Cached total:         %12zu bytes\n",
           heap_size - allocated);

    if (heap_size > allocated) {
        printf("\n[INFO] %zu bytes are cached for fast reuse.\n",
               heap_size - allocated);
        printf("[INFO] Call MallocExtension_ReleaseFreeMemory() to ");
        printf("return memory to OS.\n");
    }
}
```

### Releasing Memory to the OS

```c
/*
 * THE RELEASE RITUAL
 *
 * TCMalloc holds onto freed memory for performance.
 * But sometimes you need to return it to the operating system:
 *   - Long-running services during idle periods
 *   - Before spawning memory-intensive child processes
 *   - When system is under memory pressure
 */

void demonstrate_memory_release(void) {
    printf("=== Memory Release Demonstration ===\n\n");

    /* Allocate significant memory */
    void *large_blocks[10];
    for (int i = 0; i < 10; i++) {
        large_blocks[i] = malloc(1024 * 1024);  /* 1 MB each */
        memset(large_blocks[i], 0, 1024 * 1024);
    }
    printf("After allocating 10 MB:\n");
    explain_memory_caching();

    /* Free all blocks */
    for (int i = 0; i < 10; i++) {
        free(large_blocks[i]);
    }
    printf("\nAfter freeing (memory still cached):\n");
    explain_memory_caching();

    /* Release to OS */
    MallocExtension_ReleaseFreeMemory();
    printf("\nAfter ReleaseFreeMemory (returned to OS):\n");
    explain_memory_caching();
}

/*
 * ACTUAL OUTPUT FROM THE FELLOWSHIP'S SESSION:
 *
 * After allocating 10 MB:
 *   Currently allocated:     10,486,032 bytes
 *   Total heap size:         11,534,336 bytes
 *   Cached in page heap:              0 bytes
 *
 * After freeing (memory still cached):
 *   Currently allocated:            272 bytes
 *   Total heap size:         11,534,336 bytes
 *   Cached in page heap:     10,485,760 bytes  ← All cached!
 *
 * After ReleaseFreeMemory:
 *   Currently allocated:            272 bytes
 *   Total heap size:         11,534,336 bytes
 *   Cached in page heap:              0 bytes  ← Returned to OS
 */
```

---

## Chapter 4: The Detailed Statistics

### Reading the TCMalloc Oracle

TCMalloc provides an oracle—a way to see inside the memory palace and understand its structure.

```c
/*
 * THE ORACLE'S REVELATION
 *
 * When the Fellowship asked TCMalloc for its secrets,
 * this is what it revealed:
 */

void consult_the_oracle(void) {
    char stats_buffer[8192];
    MallocExtension_GetStats(stats_buffer, sizeof(stats_buffer));
    printf("%s\n", stats_buffer);
}

/*
 * ACTUAL OUTPUT FROM THE FELLOWSHIP'S SESSION:
 *
 * ────────────────────────────────────────────────────────────────
 * MALLOC:            272 (    0.0 MiB) Bytes in use by application
 * MALLOC: +    193,658,880 (  184.7 MiB) Bytes in page heap freelist
 * MALLOC: +     12,122,496 (   11.6 MiB) Bytes in central cache freelist
 * MALLOC: +     16,807,424 (   16.0 MiB) Bytes in transfer cache freelist
 * MALLOC: +      4,951,920 (    4.7 MiB) Bytes in thread cache freelists
 * MALLOC: +      3,276,864 (    3.1 MiB) Bytes in malloc metadata
 * MALLOC:   ────────────
 * MALLOC: =    230,817,856 (  220.1 MiB) Actual memory used (physical + swap)
 * MALLOC: +            0 (    0.0 MiB) Bytes released to OS (aka unmapped)
 * MALLOC:   ────────────
 * MALLOC: =    230,817,856 (  220.1 MiB) Virtual address space used
 * MALLOC:
 * MALLOC:           4,188              Spans in use
 * MALLOC:               1              Thread heaps in use
 * MALLOC:           8,192              Tcmalloc page size
 * ────────────────────────────────────────────────────────────────
 *
 * INTERPRETING THE ORACLE:
 *
 * "Bytes in use by application" (272)
 *   → The memory YOU allocated and haven't freed
 *   → This should match your expectations
 *
 * "Bytes in page heap freelist" (184.7 MiB)
 *   → Free pages ready for large allocations
 *   → This is cached, not leaked
 *
 * "Bytes in central cache freelist" (11.6 MiB)
 *   → Free objects in the central cache
 *   → Shared between all threads
 *
 * "Bytes in transfer cache freelist" (16.0 MiB)
 *   → Objects being transferred between caches
 *   → Optimization for batch operations
 *
 * "Bytes in thread cache freelists" (4.7 MiB)
 *   → Free objects in per-thread caches
 *   → This is YOUR thread's stash
 *
 * "Bytes in malloc metadata" (3.1 MiB)
 *   → Bookkeeping overhead
 *   → Page maps, span descriptors, etc.
 *
 * "Spans in use" (4,188)
 *   → Number of contiguous page ranges
 *   → More spans = more fragmentation potential
 *
 * "Thread heaps in use" (1)
 *   → Number of active thread caches
 *   → Should match your thread count
 *
 * "Tcmalloc page size" (8,192)
 *   → Internal page size (not OS page size)
 *   → Allocations >= this go to page heap directly
 */
```

---

## Chapter 5: Choosing Your Allocator

### The Decision Tree

```
                    ┌─────────────────────────────────┐
                    │   What is your primary concern?  │
                    └─────────────────┬───────────────┘
                                      │
              ┌───────────────────────┼───────────────────────┐
              │                       │                       │
              ▼                       ▼                       ▼
     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
     │  Raw Speed     │     │ Memory Return  │     │ Fragmentation  │
     │  (throughput)  │     │   to OS        │     │   Control      │
     └───────┬────────┘     └───────┬────────┘     └───────┬────────┘
             │                      │                      │
             ▼                      ▼                      ▼
     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
     │   TCMalloc     │     │   jemalloc     │     │   jemalloc     │
     │                │     │   (with decay) │     │   (arenas)     │
     │ - Thread cache │     │                │     │                │
     │ - Size classes │     │ - muzzy pages  │     │ - Dedicated    │
     │ - Batch xfer   │     │ - dirty decay  │     │   arenas       │
     └────────────────┘     └────────────────┘     └────────────────┘


                    ┌─────────────────────────────────┐
                    │   What is your workload pattern? │
                    └─────────────────┬───────────────┘
                                      │
              ┌───────────────────────┼───────────────────────┐
              │                       │                       │
              ▼                       ▼                       ▼
     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
     │ Thread-local   │     │  Producer-     │     │   Mixed/       │
     │ (alloc & free  │     │  Consumer      │     │   Unknown      │
     │  same thread)  │     │  (cross-thread)│     │                │
     └───────┬────────┘     └───────┬────────┘     └───────┬────────┘
             │                      │                      │
             ▼                      ▼                      ▼
     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
     │   TCMalloc     │     │   jemalloc     │     │  Benchmark     │
     │   (optimal)    │     │   (arenas)     │     │   Both!        │
     └────────────────┘     └────────────────┘     └────────────────┘
```

### The Comparison Table

```c
/*
 * THE GREAT COMPARISON
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  Feature              │  TCMalloc          │  jemalloc           │
 * ├───────────────────────┼────────────────────┼─────────────────────┤
 * │  Thread-local cache   │  ✓ Built-in        │  ✓ Optional (tcache)│
 * │  Arena support        │  ✗ Single heap     │  ✓ Multiple arenas  │
 * │  Size classes         │  ~85 classes       │  ~100+ classes      │
 * │  Page size            │  8 KB              │  4 KB (default)     │
 * │  Memory return to OS  │  Manual            │  Automatic (decay)  │
 * │  Profiling built-in   │  ✓ Heap profiler   │  ✓ Stats + profiling│
 * │  Windows support      │  Minimal (no prof) │  Better             │
 * │  Origin               │  Google            │  FreeBSD/Facebook   │
 * │  License              │  BSD-3             │  BSD-2              │
 * └───────────────────────┴────────────────────┴─────────────────────┘
 *
 * PERFORMANCE (typical, varies by workload):
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  Metric                │  TCMalloc          │  jemalloc           │
 * ├────────────────────────┼────────────────────┼─────────────────────┤
 * │  Small alloc (< 256B)  │  ★★★★★             │  ★★★★☆              │
 * │  Medium alloc          │  ★★★★☆             │  ★★★★★              │
 * │  Large alloc (> 32KB)  │  ★★★☆☆             │  ★★★★☆              │
 * │  Multi-threaded        │  ★★★★★             │  ★★★★☆              │
 * │  Fragmentation         │  ★★★☆☆             │  ★★★★★              │
 * │  Memory overhead       │  ★★★★☆             │  ★★★★☆              │
 * └────────────────────────┴────────────────────┴─────────────────────┘
 */
```

---

## Chapter 6: Practical Integration

### Using TCMalloc in Your Project

```c
/*
 * INTEGRATING TCMALLOC: A PRACTICAL GUIDE
 *
 * The Fellowship learned these integration patterns
 * during their PartsDB optimization quest.
 */

/* === METHOD 1: Link-time replacement === */

/*
 * Makefile approach (simplest):
 *
 *   # Link against tcmalloc
 *   LDFLAGS += -ltcmalloc_minimal
 *
 *   # Or with full profiling support (Linux only)
 *   LDFLAGS += -ltcmalloc
 *
 * Your code doesn't change—TCMalloc replaces malloc/free automatically.
 */

/* === METHOD 2: LD_PRELOAD (Linux) === */

/*
 * No recompilation needed:
 *
 *   $ LD_PRELOAD=/usr/lib/libtcmalloc_minimal.so ./your_program
 *
 * Useful for testing without rebuilding.
 */

/* === METHOD 3: Explicit API usage === */

#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension_c.h>

/*
 * Use TCMalloc-specific functions for advanced features:
 */

void *optimized_allocate(size_t requested_size) {
    void *ptr = tc_malloc(requested_size);

    if (ptr) {
        /* Get actual allocation size (may be larger) */
        size_t actual_size = tc_malloc_size(ptr);

        /* You can use the extra space! */
        if (actual_size > requested_size) {
            /* actual_size - requested_size bytes are bonus */
        }
    }

    return ptr;
}

void periodic_maintenance(void) {
    /* Check memory efficiency */
    size_t allocated, heap_size;
    MallocExtension_GetNumericProperty(
        "generic.current_allocated_bytes", &allocated);
    MallocExtension_GetNumericProperty(
        "generic.heap_size", &heap_size);

    double efficiency = (double)allocated / heap_size;

    /* If efficiency drops below 50%, release memory */
    if (efficiency < 0.5) {
        MallocExtension_ReleaseFreeMemory();
        printf("[MAINTENANCE] Released cached memory to OS\n");
    }

    /* Optionally, set aggressive memory release */
    MallocExtension_SetNumericProperty(
        "tcmalloc.aggressive_memory_decommit", 1);
}

/* === METHOD 4: Custom memory pools (advanced) === */

/*
 * For specialized use cases, create custom allocators
 * that use TCMalloc as the underlying provider:
 */

typedef struct {
    void *base;
    size_t size;
    size_t used;
} ArenaAllocator;

ArenaAllocator *arena_create(size_t size) {
    ArenaAllocator *arena = tc_malloc(sizeof(ArenaAllocator));
    arena->base = tc_malloc(size);
    arena->size = size;
    arena->used = 0;
    return arena;
}

void *arena_alloc(ArenaAllocator *arena, size_t size) {
    /* Align to 8 bytes */
    size = (size + 7) & ~7;

    if (arena->used + size > arena->size) {
        return NULL;  /* Arena full */
    }

    void *ptr = (char *)arena->base + arena->used;
    arena->used += size;
    return ptr;
}

void arena_reset(ArenaAllocator *arena) {
    arena->used = 0;  /* Reset without freeing—instant! */
}

void arena_destroy(ArenaAllocator *arena) {
    tc_free(arena->base);
    tc_free(arena);
}
```

### The PartsDB Makefile Integration

```makefile
#============================================================================
# TCMalloc Integration for PartsDB
# From the Fellowship's quest, December 2024
#============================================================================

# TCMalloc library location
TCMALLOC_INC := ./lib
TCMALLOC_LIB := ./lib

# Standard build (uses system malloc)
standard: CFLAGS := -std=c17 $(WARNINGS) $(DEBUG_FLAGS)
standard: LDFLAGS := -lm
standard: all

# TCMalloc build (uses Google's allocator)
tcmalloc: CFLAGS := -std=c17 $(WARNINGS) $(DEBUG_FLAGS) -I$(TCMALLOC_INC)
tcmalloc: LDFLAGS := -L$(TCMALLOC_LIB) -ltcmalloc_minimal -lm
tcmalloc: all
	@echo "Built with TCMalloc support"
	@echo "Run with: PATH=$(TCMALLOC_LIB):$$PATH ./partsdb"

# Profiling build (Linux only, full TCMalloc)
profile: CFLAGS := -std=c17 $(WARNINGS) -g -O2 -I$(TCMALLOC_INC)
profile: LDFLAGS := -L$(TCMALLOC_LIB) -ltcmalloc -lm
profile: all
	@echo "Built with TCMalloc profiling support"
	@echo "CPU profile: CPUPROFILE=cpu.prof ./partsdb"
	@echo "Heap profile: HEAPPROFILE=heap ./partsdb"
```

---

## Epilogue: The Wisdom of the Two Allocators

The Fellowship emerged from their study of the Two Allocators with hard-won wisdom:

```c
/*
 * THE TEN COMMANDMENTS OF MEMORY ALLOCATION
 *
 * I.    Thou shalt align thy structures to size classes.
 * II.   Thou shalt not assume allocation is free.
 * III.  Thou shalt measure before thou optimize.
 * IV.   Thou shalt understand thy workload pattern.
 * V.    Thou shalt release memory in long-running services.
 * VI.   Thou shalt not fight the allocator's design.
 * VII.  Thou shalt use thread-local allocation when possible.
 * VIII. Thou shalt batch allocations when feasible.
 * IX.   Thou shalt consult the oracle (statistics) often.
 * X.    Thou shalt test with multiple allocators.
 */

/*
 * FINAL BENCHMARK SUMMARY
 *
 * The Fellowship achieved these results with TCMalloc on PartsDB:
 *
 * ┌────────────────────────────────────────────────────────────┐
 * │  Achievement              │  Value                         │
 * ├───────────────────────────┼────────────────────────────────┤
 * │  Allocation rate          │  1,852 allocs/ms               │
 * │  Deallocation rate        │  50,000 frees/ms               │
 * │  Memory efficiency        │  96.6%                         │
 * │  Thread cache hit rate    │  >99% (estimated)              │
 * │  Lock contention          │  Near zero (single-threaded)   │
 * └───────────────────────────┴────────────────────────────────┘
 *
 * These numbers represent the power of understanding your allocator.
 * The same code with a naive malloc might achieve 10x less throughput.
 */
```

---

```
    "The Two Allocators are not enemies, but different paths
     up the same mountain. TCMalloc races up the rocky face,
     swift and direct. jemalloc takes the winding trail,
     careful of the terrain, leaving no trace behind.
     Both reach the summit. Choose the path that suits your journey."

                                        — Gandalf the Memory-Grey
```

---

# END OF PART II

**Previously:**
- **Part I: The Forming of the Fellowship** — *Meet the Ancients, Keepers, and Young Upstarts*

**Continue to:**
- **Part III: The Return of the Profiler** — *The Complete gperftools Integration Story*

---

*This document is part of the C23 Tutorial series.*
*Created during the gperftools integration session, December 2024.*
*All benchmark results are from actual measurements on Windows 10, MSYS2/MinGW64, GCC 15.2.0.*
