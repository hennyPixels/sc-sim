# Part 8: Memory Hierarchy, Caching & Linking

*Source: Computer Systems: A Programmer's Perspective (Bryant & O'Hallaron)*

> **"The memory system is a major bottleneck."** — Bryant & O'Hallaron

Understanding the memory hierarchy is crucial for writing high-performance C code. This part explores how memory works from CPU caches to virtual memory, with practical optimization techniques for PartsDB.

---

## 8.1 Memory Hierarchy Overview

### The Memory Pyramid

```
                     ┌─────────┐
                     │ Regs    │ < 1 ns, ~KB
                     ├─────────┤
                   ┌─┴─────────┴─┐
                   │   L1 Cache   │ ~1-4 ns, 32-64KB
                   ├─────────────┤
                 ┌─┴─────────────┴─┐
                 │    L2 Cache      │ ~10-20 ns, 256KB-1MB
                 ├─────────────────┤
               ┌─┴─────────────────┴─┐
               │      L3 Cache        │ ~40-75 ns, 2-64MB
               ├─────────────────────┤
             ┌─┴─────────────────────┴─┐
             │       Main Memory        │ ~100-300 ns, GBs
             ├─────────────────────────┤
           ┌─┴─────────────────────────┴─┐
           │          SSD/NVMe            │ ~10-100 μs, TBs
           ├─────────────────────────────┤
         ┌─┴─────────────────────────────┴─┐
         │             HDD                   │ ~5-10 ms, TBs
         └───────────────────────────────────┘
```

### Key Concepts

```c
/*
 * Locality Principles:
 *
 * Temporal Locality: Recently accessed data likely to be accessed again
 * Spatial Locality: Data near recently accessed data likely to be accessed
 *
 * Cache organization:
 * - Cache line: Unit of transfer (typically 64 bytes)
 * - Set: Group of cache lines that can hold an address
 * - Way: Number of lines per set (associativity)
 *
 * For a 32KB, 8-way set associative cache with 64B lines:
 * - 32KB / 64B = 512 lines total
 * - 512 / 8 = 64 sets
 * - Set index: 6 bits (log2(64))
 * - Line offset: 6 bits (log2(64))
 */

/* Typical cache line size */
#define CACHE_LINE_SIZE 64

/* Check if system matches our assumption */
#include <unistd.h>
void check_cache_line(void) {
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 data cache line size: %ld bytes\n", line_size);
}
```

---

## 8.2 Cache Organization and Behavior

### Cache Miss Types

```c
/*
 * Three Cs of Cache Misses:
 *
 * 1. Compulsory (Cold) - First access to a block
 * 2. Capacity - Cache can't hold all needed data
 * 3. Conflict - Multiple addresses map to same set
 */

/* Cold miss - unavoidable on first access */
int first_access(int *array) {
    return array[0];  /* Must fetch from memory */
}

/* Capacity miss - working set too large */
void large_working_set(int *array, size_t size) {
    /* If size > cache size, some data will be evicted */
    for (size_t i = 0; i < size; i++) {
        array[i]++;  /* May cause capacity misses */
    }
}

/* Conflict miss - bad access pattern */
void conflict_misses(int *a, int *b, size_t stride) {
    /* If stride is power of 2 matching cache size,
     * a[i] and b[i] may map to same cache set */
    for (size_t i = 0; i < 1000; i++) {
        a[i * stride] += b[i * stride];  /* Potential conflicts */
    }
}
```

### Cache-Friendly Access Patterns

```c
/* GOOD: Sequential access - spatial locality */
void sum_array_good(const int *arr, size_t n) {
    int sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += arr[i];  /* Sequential access */
    }
}

/* BAD: Strided access - poor spatial locality */
void sum_array_bad(const int *arr, size_t n, size_t stride) {
    int sum = 0;
    for (size_t i = 0; i < n; i += stride) {
        sum += arr[i];  /* Skips elements, wastes cache lines */
    }
}

/* Matrix traversal: row-major vs column-major */
#define N 1000
int matrix[N][N];

/* GOOD: Row-major order (matches C memory layout) */
void sum_rows(void) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum += matrix[i][j];  /* Sequential in memory */
        }
    }
}

/* BAD: Column-major order */
void sum_cols(void) {
    int sum = 0;
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            sum += matrix[i][j];  /* Jumps N*4 bytes each access */
        }
    }
}
/* sum_rows can be 10-100x faster than sum_cols! */
```

---

## 8.3 Writing Cache-Friendly Code

### Loop Blocking (Tiling)

```c
/* Without blocking: poor cache utilization for large matrices */
void matmul_naive(double *A, double *B, double *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                sum += A[i*n + k] * B[k*n + j];
            }
            C[i*n + j] = sum;
        }
    }
}

/* With blocking: keeps blocks in cache */
#define BLOCK_SIZE 64  /* Tune for your cache */

void matmul_blocked(double *A, double *B, double *C, int n) {
    for (int i0 = 0; i0 < n; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < n; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < n; k0 += BLOCK_SIZE) {
                /* Process block */
                for (int i = i0; i < i0 + BLOCK_SIZE && i < n; i++) {
                    for (int j = j0; j < j0 + BLOCK_SIZE && j < n; j++) {
                        double sum = C[i*n + j];
                        for (int k = k0; k < k0 + BLOCK_SIZE && k < n; k++) {
                            sum += A[i*n + k] * B[k*n + j];
                        }
                        C[i*n + j] = sum;
                    }
                }
            }
        }
    }
}
```

### Data Structure Layout

```c
/* Structure of Arrays (SoA) vs Array of Structures (AoS) */

/* AoS: Good when accessing all fields of one element */
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float mass;
} particle_aos_t;

particle_aos_t particles_aos[1000];

/* Update all fields of each particle - AoS is good */
void update_particle_aos(particle_aos_t *p) {
    p->x += p->vx;
    p->y += p->vy;
    p->z += p->vz;
}

/* SoA: Good when accessing one field of many elements */
typedef struct {
    float x[1000], y[1000], z[1000];
    float vx[1000], vy[1000], vz[1000];
    float mass[1000];
} particles_soa_t;

/* Update only positions - SoA is better */
void update_positions_soa(particles_soa_t *p, int n) {
    /* Vectorizable, sequential memory access */
    for (int i = 0; i < n; i++) {
        p->x[i] += p->vx[i];  /* x array sequential */
    }
    for (int i = 0; i < n; i++) {
        p->y[i] += p->vy[i];
    }
    for (int i = 0; i < n; i++) {
        p->z[i] += p->vz[i];
    }
}
```

### Prefetching

```c
#ifdef __GNUC__
#define PREFETCH(addr) __builtin_prefetch(addr)
#define PREFETCH_WRITE(addr) __builtin_prefetch(addr, 1)
#else
#define PREFETCH(addr) ((void)0)
#define PREFETCH_WRITE(addr) ((void)0)
#endif

/* Manual prefetching for predictable access patterns */
void process_parts_prefetch(part_t *parts, size_t n) {
    for (size_t i = 0; i < n; i++) {
        /* Prefetch 4 elements ahead */
        if (i + 4 < n) {
            PREFETCH(&parts[i + 4]);
        }
        process_single_part(&parts[i]);
    }
}

/* Software pipelining with prefetch */
void copy_with_prefetch(void *dst, const void *src, size_t n) {
    char *d = dst;
    const char *s = src;

    /* Prefetch first blocks */
    for (size_t i = 0; i < 4 && i * CACHE_LINE_SIZE < n; i++) {
        PREFETCH(s + i * CACHE_LINE_SIZE);
    }

    for (size_t i = 0; i < n; i += CACHE_LINE_SIZE) {
        /* Prefetch 4 cache lines ahead */
        if (i + 4 * CACHE_LINE_SIZE < n) {
            PREFETCH(s + i + 4 * CACHE_LINE_SIZE);
        }
        /* Copy one cache line */
        size_t chunk = (n - i < CACHE_LINE_SIZE) ? n - i : CACHE_LINE_SIZE;
        memcpy(d + i, s + i, chunk);
    }
}
```

---

## 8.4 Data Alignment and Padding

### Alignment Requirements

```c
#include <stdalign.h>

/* Check alignment of types */
printf("alignof(char) = %zu\n", alignof(char));      /* 1 */
printf("alignof(int) = %zu\n", alignof(int));        /* 4 */
printf("alignof(double) = %zu\n", alignof(double));  /* 8 */
printf("alignof(part_t) = %zu\n", alignof(part_t));  /* 8 (max member) */

/* Cache-line aligned data */
alignas(64) char cache_aligned_buffer[1024];

/* Verify alignment at runtime */
#define IS_ALIGNED(ptr, align) (((uintptr_t)(ptr) & ((align) - 1)) == 0)

void *get_aligned_memory(size_t size, size_t align) {
    void *ptr = aligned_alloc(align, size);
    assert(IS_ALIGNED(ptr, align));
    return ptr;
}
```

### Structure Padding Optimization

```c
/* Inefficient layout - 32 bytes with padding */
struct inefficient {
    char a;         /* 1 byte */
    /* 7 bytes padding */
    double b;       /* 8 bytes */
    char c;         /* 1 byte */
    /* 3 bytes padding */
    int d;          /* 4 bytes */
    char e;         /* 1 byte */
    /* 7 bytes padding for struct alignment */
};

/* Efficient layout - 24 bytes */
struct efficient {
    double b;       /* 8 bytes (largest first) */
    int d;          /* 4 bytes */
    char a, c, e;   /* 3 bytes (pack small together) */
    /* 1 byte padding */
};

/* Tool to visualize padding */
void print_struct_layout(void) {
    printf("inefficient: size=%zu, align=%zu\n",
           sizeof(struct inefficient), alignof(struct inefficient));
    printf("  a: offset=%zu\n", offsetof(struct inefficient, a));
    printf("  b: offset=%zu\n", offsetof(struct inefficient, b));
    printf("  c: offset=%zu\n", offsetof(struct inefficient, c));
    printf("  d: offset=%zu\n", offsetof(struct inefficient, d));
    printf("  e: offset=%zu\n", offsetof(struct inefficient, e));
}

/* Use pahole tool: pahole -C struct_name binary */
```

### False Sharing Prevention

```c
/* BAD: Counters on same cache line - false sharing */
struct counters_bad {
    atomic_uint counter1;  /* Thread 1 uses */
    atomic_uint counter2;  /* Thread 2 uses */
    /* Both on same cache line - contention! */
};

/* GOOD: Padded to separate cache lines */
struct counter_padded {
    atomic_uint value;
    char padding[CACHE_LINE_SIZE - sizeof(atomic_uint)];
};

struct counters_good {
    struct counter_padded counter1;  /* Own cache line */
    struct counter_padded counter2;  /* Own cache line */
};

/* Alternative: use alignas */
struct alignas(64) counter_aligned {
    atomic_uint value;
};
```

---

## 8.5 Static and Dynamic Linking

### Static Linking

```bash
# Create static library
gcc -c database.c -o database.o
ar rcs libpartsdb.a database.o

# Link statically
gcc main.c -L. -lpartsdb -o partsdb-static

# Or link specific .a file
gcc main.c libpartsdb.a -o partsdb-static

# Fully static (including libc)
gcc -static main.c libpartsdb.a -lsqlite3 -o partsdb-static
```

### Dynamic Linking

```bash
# Create shared library
gcc -fPIC -shared database.c -o libpartsdb.so

# Or with proper versioning
gcc -fPIC -shared -Wl,-soname,libpartsdb.so.1 \
    database.c -o libpartsdb.so.1.0.0
ln -s libpartsdb.so.1.0.0 libpartsdb.so.1
ln -s libpartsdb.so.1 libpartsdb.so

# Link dynamically
gcc main.c -L. -lpartsdb -Wl,-rpath,'$ORIGIN' -o partsdb-dynamic

# Check dependencies
ldd partsdb-dynamic
```

### Symbol Resolution

```c
/* Control symbol visibility */
#if defined(__GNUC__) && __GNUC__ >= 4
#define PUBLIC __attribute__((visibility("default")))
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define PUBLIC
#define HIDDEN
#endif

/* Public API */
PUBLIC partsdb_error_t partsdb_open(const char *path, partsdb_t **db);

/* Internal helper - not exported */
HIDDEN static void internal_cleanup(partsdb_t *db);
```

```bash
# Compile with default hidden visibility
gcc -fvisibility=hidden -c database.c

# Check exported symbols
nm -D libpartsdb.so | grep ' T '  # Exported functions
```

---

## 8.6 Position-Independent Code

### PIC for Shared Libraries

```bash
# -fPIC generates position-independent code
gcc -fPIC -c database.c -o database.o

# Required for shared libraries on x86-64
gcc -fPIC -shared database.o -o libpartsdb.so
```

### PIE for Security

```bash
# Position-Independent Executable
gcc -fPIE -pie main.c -o partsdb

# Verify
readelf -h partsdb | grep Type
# Output: Type: DYN (Position-Independent Executable file)

# Check ASLR effectiveness
for i in 1 2 3; do
    ./partsdb --print-addresses
done
# Different addresses each run
```

### Global Offset Table (GOT)

```c
/*
 * GOT: Table of addresses for global data and functions
 * PLT: Procedure Linkage Table for lazy binding
 *
 * Security implications:
 * - GOT is writable by default (for lazy binding)
 * - Attackers can overwrite GOT entries
 * - RELRO makes GOT read-only after startup
 */
```

```bash
# Full RELRO: resolve all symbols at load time, make GOT read-only
gcc -Wl,-z,relro,-z,now main.c -o partsdb

# Verify
readelf -d partsdb | grep FLAGS
# Output includes BIND_NOW
```

---

## 8.7 Library Interposition

### Compile-Time Interposition

```c
/* my_malloc.c: Custom malloc wrapper */
#include <stdio.h>
#include <stdlib.h>

/* Interpose malloc */
void *malloc(size_t size) {
    /* Get original malloc */
    static void *(*real_malloc)(size_t) = nullptr;
    if (real_malloc == nullptr) {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    void *ptr = real_malloc(size);
    fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
    return ptr;
}
```

### Link-Time Interposition

```bash
# Wrap function at link time
gcc -Wl,--wrap=malloc -c main.c
gcc -Wl,--wrap=malloc main.o wrapper.o -o program
```

```c
/* wrapper.c */
#include <stdlib.h>

/* Original function */
extern void *__real_malloc(size_t size);

/* Wrapper function */
void *__wrap_malloc(size_t size) {
    printf("malloc(%zu)\n", size);
    return __real_malloc(size);
}
```

### Runtime Interposition (LD_PRELOAD)

```c
/* malloc_tracker.c */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

static size_t total_allocated = 0;

void *malloc(size_t size) {
    static void *(*real_malloc)(size_t) = nullptr;
    if (real_malloc == nullptr) {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    void *ptr = real_malloc(size);
    total_allocated += size;
    return ptr;
}

/* Called when library unloads */
__attribute__((destructor))
void report_memory(void) {
    fprintf(stderr, "Total allocated: %zu bytes\n", total_allocated);
}
```

```bash
# Build interposition library
gcc -fPIC -shared malloc_tracker.c -o libmalloc_tracker.so -ldl

# Use with any program
LD_PRELOAD=./libmalloc_tracker.so ./partsdb
```

---

## 8.8 PartsDB Case Study: Cache Optimization

### Query Result Caching

```c
/* Simple LRU cache for query results */
#define CACHE_SIZE 64

typedef struct cache_entry {
    uint64_t hash;
    part_t *results;
    size_t count;
    struct cache_entry *prev, *next;
} cache_entry_t;

typedef struct {
    cache_entry_t *head;  /* Most recently used */
    cache_entry_t *tail;  /* Least recently used */
    cache_entry_t entries[CACHE_SIZE];
    cache_entry_t *hash_table[CACHE_SIZE];  /* Quick lookup */
} query_cache_t;

/* Move entry to front (most recently used) */
static void cache_move_to_front(query_cache_t *cache, cache_entry_t *entry) {
    if (entry == cache->head) return;

    /* Remove from current position */
    if (entry->prev) entry->prev->next = entry->next;
    if (entry->next) entry->next->prev = entry->prev;
    if (entry == cache->tail) cache->tail = entry->prev;

    /* Insert at front */
    entry->prev = nullptr;
    entry->next = cache->head;
    if (cache->head) cache->head->prev = entry;
    cache->head = entry;
    if (cache->tail == nullptr) cache->tail = entry;
}

/* Lookup with cache-line-friendly hash table */
part_t *cache_lookup(query_cache_t *cache, const char *query, size_t *count) {
    uint64_t hash = fnv1a_hash(query);
    size_t bucket = hash % CACHE_SIZE;

    cache_entry_t *entry = cache->hash_table[bucket];
    if (entry && entry->hash == hash) {
        cache_move_to_front(cache, entry);
        *count = entry->count;
        return entry->results;
    }
    return nullptr;  /* Cache miss */
}
```

### Memory-Mapped Database Access

```c
/* Use mmap for read-heavy workloads */
typedef struct {
    int fd;
    void *mapped;
    size_t size;
} mmap_db_t;

mmap_db_t *mmap_db_open(const char *path) {
    mmap_db_t *db = malloc(sizeof(mmap_db_t));

    db->fd = open(path, O_RDONLY);
    if (db->fd < 0) {
        free(db);
        return nullptr;
    }

    struct stat st;
    fstat(db->fd, &st);
    db->size = st.st_size;

    db->mapped = mmap(nullptr, db->size, PROT_READ, MAP_PRIVATE, db->fd, 0);
    if (db->mapped == MAP_FAILED) {
        close(db->fd);
        free(db);
        return nullptr;
    }

    /* Advise kernel about access pattern */
    madvise(db->mapped, db->size, MADV_SEQUENTIAL);

    return db;
}

void mmap_db_close(mmap_db_t *db) {
    if (db) {
        munmap(db->mapped, db->size);
        close(db->fd);
        free(db);
    }
}
```

---

## 8.9 Review Questions - Part 8

> **Instructions**: Research these questions using *CS:APP*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[CS:APP, Ch. 6]** Explain the three Cs of cache misses. For each type, give an example from PartsDB and suggest how to reduce it.

2. **[CS:APP, Ch. 6]** What is cache thrashing? Design a scenario where PartsDB could experience cache thrashing and explain how to fix it.

3. **[CS:APP, Ch. 7]** Explain the difference between load-time relocation and run-time dynamic linking. What are the trade-offs?

4. **[CS:APP, Ch. 9]** How does the TLB (Translation Lookaside Buffer) affect performance? What programming patterns cause TLB misses?

5. **[CS:APP, Ch. 6]** Why is loop interchange sometimes necessary for cache optimization? Give a matrix operation example.

### Practical Exercises

6. **Cache Profiling**: Use `perf` or `cachegrind` to measure cache miss rates for PartsDB search operations. Identify the hot spots.

7. **Structure Optimization**: Analyze PartsDB's `part_t` structure with `pahole`. Propose a more cache-friendly layout.

8. **Blocked Algorithm**: Implement a blocked version of PartsDB's bulk insert that processes parts in cache-friendly chunks.

9. **ASLR Verification**: Write a program that prints the addresses of different memory segments (code, data, heap, stack, libraries) and verify ASLR randomization.

10. **Memory Tracker**: Implement a memory tracking library using LD_PRELOAD that reports PartsDB's allocation patterns (sizes, frequencies, lifetimes).

### Research Topics

11. **[CS:APP, Ch. 6]** Research cache-oblivious algorithms. How do they differ from cache-aware algorithms? Implement a cache-oblivious matrix transpose.

12. **[CS:APP, Ch. 9]** Research huge pages (2MB, 1GB). When would PartsDB benefit from huge pages? Implement support using `mmap` with `MAP_HUGETLB` or `madvise(MADV_HUGEPAGE)`.

13. **[CS:APP, Ch. 7]** Research symbol versioning in shared libraries. How would you maintain backward compatibility when changing PartsDB's ABI?

---

[← Part 7: Concurrency](part-07-concurrency.md) | [Back to Index](README.md) | [Part 9: Style and Debugging →](part-09-style-debugging.md)
