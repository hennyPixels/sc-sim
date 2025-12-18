/**
 * @file tcmalloc_demo.c
 * @brief TCMalloc Memory Allocator Demonstration
 *
 * This program demonstrates TCMalloc features available on Windows:
 * - Memory allocation performance
 * - Memory statistics via MallocExtension API
 * - Comparison with standard malloc
 *
 * Compile:
 *   gcc -O2 -o tcmalloc_demo.exe tcmalloc_demo.c -L./lib -ltcmalloc_minimal
 *
 * Run:
 *   PATH="./lib:$PATH" ./tcmalloc_demo.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* TCMalloc headers */
#include <gperftools/malloc_extension_c.h>
#include <gperftools/tcmalloc.h>

/*============================================================================
 * Configuration
 *============================================================================*/

#define NUM_ALLOCATIONS 100000
#define MIN_ALLOC_SIZE 16
#define MAX_ALLOC_SIZE 4096

/*============================================================================
 * Utility Functions
 *============================================================================*/

/* High-resolution timer */
static double get_time_ms(void)
{
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
}

/* Print memory statistics */
static void print_memory_stats(const char *label)
{
    size_t current_allocated = 0;
    size_t heap_size = 0;
    size_t pageheap_free = 0;
    size_t thread_cache = 0;

    printf("\n=== %s ===\n", label);

    if (MallocExtension_GetNumericProperty(
            "generic.current_allocated_bytes", &current_allocated)) {
        printf("  Current allocated:    %10zu bytes (%zu KB)\n",
               current_allocated, current_allocated / 1024);
    }

    if (MallocExtension_GetNumericProperty(
            "generic.heap_size", &heap_size)) {
        printf("  Total heap size:      %10zu bytes (%zu KB)\n",
               heap_size, heap_size / 1024);
    }

    if (MallocExtension_GetNumericProperty(
            "tcmalloc.pageheap_free_bytes", &pageheap_free)) {
        printf("  Free in pageheap:     %10zu bytes (%zu KB)\n",
               pageheap_free, pageheap_free / 1024);
    }

    if (MallocExtension_GetNumericProperty(
            "tcmalloc.current_total_thread_cache_bytes", &thread_cache)) {
        printf("  Thread cache usage:   %10zu bytes (%zu KB)\n",
               thread_cache, thread_cache / 1024);
    }

    if (heap_size > 0 && current_allocated > 0) {
        double efficiency = (double)current_allocated / heap_size * 100.0;
        printf("  Memory efficiency:    %10.1f%%\n", efficiency);
    }
}

/* Print detailed TCMalloc statistics */
static void print_detailed_stats(void)
{
    char stats_buffer[8192];

    printf("\n=== Detailed TCMalloc Statistics ===\n");

    MallocExtension_GetStats(stats_buffer, sizeof(stats_buffer));
    printf("%s\n", stats_buffer);
}

/*============================================================================
 * Benchmark Functions
 *============================================================================*/

/* Benchmark: Random-sized allocations */
static void benchmark_random_allocations(void)
{
    void **ptrs = malloc(NUM_ALLOCATIONS * sizeof(void *));
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate pointer array\n");
        return;
    }

    printf("\n>>> Benchmark: Random-Sized Allocations\n");
    printf("    Performing %d allocations (sizes %d-%d bytes)...\n",
           NUM_ALLOCATIONS, MIN_ALLOC_SIZE, MAX_ALLOC_SIZE);

    /* Allocation phase */
    double start = get_time_ms();

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = MIN_ALLOC_SIZE +
                      (rand() % (MAX_ALLOC_SIZE - MIN_ALLOC_SIZE));
        ptrs[i] = malloc(size);
        if (ptrs[i]) {
            /* Touch the memory to ensure it's actually allocated */
            memset(ptrs[i], i & 0xFF, size);
        }
    }

    double alloc_time = get_time_ms() - start;
    printf("    Allocation time: %.2f ms (%.0f allocs/ms)\n",
           alloc_time, NUM_ALLOCATIONS / alloc_time);

    print_memory_stats("After Allocations");

    /* Deallocation phase */
    start = get_time_ms();

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        free(ptrs[i]);
    }

    double free_time = get_time_ms() - start;
    printf("\n    Deallocation time: %.2f ms (%.0f frees/ms)\n",
           free_time, NUM_ALLOCATIONS / free_time);

    print_memory_stats("After Deallocations");

    free(ptrs);
}

/* Benchmark: Fixed-size allocations (simulates object pools) */
static void benchmark_fixed_allocations(size_t object_size)
{
    void **ptrs = malloc(NUM_ALLOCATIONS * sizeof(void *));
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate pointer array\n");
        return;
    }

    printf("\n>>> Benchmark: Fixed-Size Allocations (%zu bytes)\n", object_size);

    /* Allocation phase */
    double start = get_time_ms();

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        ptrs[i] = malloc(object_size);
        if (ptrs[i]) {
            memset(ptrs[i], 0, object_size);
        }
    }

    double alloc_time = get_time_ms() - start;
    printf("    Allocation time: %.2f ms\n", alloc_time);

    /* Free half (simulate object pool churn) */
    for (int i = 0; i < NUM_ALLOCATIONS; i += 2) {
        free(ptrs[i]);
        ptrs[i] = NULL;
    }

    /* Reallocate half */
    start = get_time_ms();
    for (int i = 0; i < NUM_ALLOCATIONS; i += 2) {
        ptrs[i] = malloc(object_size);
    }
    double realloc_time = get_time_ms() - start;
    printf("    Reallocation time: %.2f ms\n", realloc_time);

    /* Final cleanup */
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        free(ptrs[i]);
    }

    free(ptrs);
}

/* Benchmark: Realloc behavior */
static void benchmark_realloc(void)
{
    printf("\n>>> Benchmark: Realloc Behavior\n");

    void *ptr = malloc(64);
    size_t sizes[] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384};

    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        double start = get_time_ms();

        for (int j = 0; j < 10000; j++) {
            ptr = realloc(ptr, sizes[i]);
        }

        double elapsed = get_time_ms() - start;
        printf("    realloc to %5zu bytes: %.2f ms (10000 ops)\n",
               sizes[i], elapsed);
    }

    free(ptr);
}

/*============================================================================
 * TCMalloc Feature Demonstrations
 *============================================================================*/

/* Demonstrate tc_malloc_size - get allocated size */
static void demo_malloc_size(void)
{
    printf("\n>>> Demo: tc_malloc_size (actual allocation size)\n");

    size_t requested_sizes[] = {1, 8, 15, 16, 17, 32, 100, 1000, 4096, 10000};

    printf("    %-12s %-12s %-12s\n",
           "Requested", "Actual", "Overhead");
    printf("    %-12s %-12s %-12s\n",
           "----------", "----------", "----------");

    for (size_t i = 0; i < sizeof(requested_sizes) / sizeof(requested_sizes[0]); i++) {
        void *ptr = malloc(requested_sizes[i]);
        if (ptr) {
            size_t actual = tc_malloc_size(ptr);
            size_t overhead = actual - requested_sizes[i];
            printf("    %-12zu %-12zu %-12zu\n",
                   requested_sizes[i], actual, overhead);
            free(ptr);
        }
    }
}

/* Demonstrate memory release behavior */
static void demo_memory_release(void)
{
    printf("\n>>> Demo: Memory Release Behavior\n");

    print_memory_stats("Initial State");

    /* Allocate large blocks */
    void *large_blocks[10];
    for (int i = 0; i < 10; i++) {
        large_blocks[i] = malloc(1024 * 1024); /* 1 MB each */
        memset(large_blocks[i], 0, 1024 * 1024);
    }

    print_memory_stats("After 10 MB Allocation");

    /* Free the blocks */
    for (int i = 0; i < 10; i++) {
        free(large_blocks[i]);
    }

    print_memory_stats("After Freeing (memory may be cached)");

    /* Request memory release */
    MallocExtension_ReleaseFreeMemory();

    print_memory_stats("After ReleaseFreeMemory()");
}

/*============================================================================
 * Main Program
 *============================================================================*/

int main(int argc, char *argv[])
{
    printf("TCMalloc Demonstration Program\n");
    printf("==============================\n");

    /* Seed random number generator */
    srand((unsigned int)time(NULL));

    /* Show initial state */
    print_memory_stats("Initial Memory State");

    /* Run demonstrations */
    demo_malloc_size();
    demo_memory_release();

    /* Run benchmarks */
    benchmark_random_allocations();
    benchmark_fixed_allocations(64);   /* Small objects */
    benchmark_fixed_allocations(512);  /* Medium objects */
    benchmark_realloc();

    /* Final statistics */
    print_memory_stats("Final Memory State");

    if (argc > 1 && strcmp(argv[1], "--verbose") == 0) {
        print_detailed_stats();
    }

    printf("\n>>> Program completed successfully.\n");
    printf("    Run with --verbose for detailed TCMalloc stats.\n");

    return 0;
}
