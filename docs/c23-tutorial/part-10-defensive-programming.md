# Part 10: Defensive Programming & Assertions

*Drawing from "Writing Solid Code" by Steve Maguire*

## Introduction

"Writing Solid Code" emerged from Microsoft's experiences building large-scale software
in C. The book's central thesis is that bugs are not inevitable—they can be prevented
through disciplined programming practices, defensive coding, and strategic use of
assertions.

> "The key to writing bug-free code is to develop habits that prevent bugs from being
> introduced in the first place." — Steve Maguire

## 10.1 The Philosophy of Defensive Programming

### 10.1.1 Defensive Mindset

```c
/* defensive_mindset.c */

/*
 * Core Principles from Writing Solid Code:
 *
 * 1. "Assume that any code can fail"
 *    - Memory allocations can fail
 *    - Files may not exist
 *    - Network connections drop
 *    - User input is malicious
 *
 * 2. "Make bugs visible, not invisible"
 *    - Don't silently mask errors
 *    - Fail early, fail loudly in debug builds
 *    - Log everything suspicious
 *
 * 3. "Use the compiler as your first line of defense"
 *    - Enable all warnings (-Wall -Wextra)
 *    - Treat warnings as errors (-Werror)
 *    - Use static analyzers
 *
 * 4. "Test the edge cases"
 *    - NULL pointers
 *    - Empty strings
 *    - Zero counts
 *    - Maximum values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * BAD: Optimistic code that ignores potential failures
 */
char *
bad_duplicate_string(const char *str)
{
    char *copy = malloc(strlen(str) + 1);
    strcpy(copy, str);  /* What if malloc failed? What if str is NULL? */
    return copy;
}

/*
 * GOOD: Defensive code that handles all cases
 */
char *
good_duplicate_string(const char *str)
{
    /* Validate input */
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str);

    /* Check for overflow when adding 1 for null terminator */
    if (len == SIZE_MAX) {
        return NULL;
    }

    char *copy = malloc(len + 1);
    if (!copy) {
        return NULL;  /* Allocation failed */
    }

    memcpy(copy, str, len + 1);
    return copy;
}

/*
 * BETTER: Defensive code with debugging support
 */

/* Debug assertion that fires only in debug builds */
#ifdef NDEBUG
    #define ASSERT(cond) ((void)0)
#else
    #define ASSERT(cond) \
        ((cond) ? (void)0 : \
         assert_failed(#cond, __FILE__, __LINE__, __func__))
#endif

/* Runtime check that always executes */
#define VERIFY(cond) \
    ((cond) ? true : \
     (verify_failed(#cond, __FILE__, __LINE__, __func__), false))

static void
assert_failed(const char *condition, const char *file,
              int line, const char *function)
{
    fprintf(stderr,
            "\n*** ASSERTION FAILED ***\n"
            "Condition: %s\n"
            "Location:  %s:%d\n"
            "Function:  %s\n\n",
            condition, file, line, function);

    /* Trigger debugger breakpoint if attached */
    #if defined(_MSC_VER)
        __debugbreak();
    #elif defined(__GNUC__)
        __builtin_trap();
    #else
        abort();
    #endif
}

static void
verify_failed(const char *condition, const char *file,
              int line, const char *function)
{
    fprintf(stderr,
            "Verification failed: %s at %s:%d (%s)\n",
            condition, file, line, function);
}

char *
best_duplicate_string(const char *str)
{
    /* Debug-only assertion catches programmer errors */
    ASSERT(str != NULL && "String to duplicate cannot be NULL");

    /* But still handle NULL gracefully in release builds */
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str);
    ASSERT(len < SIZE_MAX && "String length overflow");

    char *copy = malloc(len + 1);
    if (!VERIFY(copy != NULL)) {
        return NULL;
    }

    memcpy(copy, str, len + 1);

    /* Postcondition: verify the copy is correct */
    ASSERT(strcmp(copy, str) == 0 && "String copy verification failed");

    return copy;
}
```

### 10.1.2 Levels of Checking

```c
/* checking_levels.c */

/*
 * Maguire distinguishes different levels of checking:
 *
 * Level 1: Always check (even in release builds)
 *   - User input validation
 *   - External API boundaries
 *   - Security-critical operations
 *
 * Level 2: Debug-only checks
 *   - Internal API preconditions
 *   - Data structure invariants
 *   - Algorithm correctness verification
 *
 * Level 3: Expensive verification
 *   - Full data structure validation
 *   - Checksums and integrity checks
 *   - Redundant calculations
 */

typedef struct PartRecord {
    uint32_t id;
    char     name[64];
    uint32_t quantity;
    double   price;
    uint32_t checksum;
} PartRecord;

/* Level 1: Always check - validates external input */
static bool
validate_part_input(const PartRecord *part)
{
    if (!part) {
        fprintf(stderr, "Error: NULL part record\n");
        return false;
    }

    if (part->id == 0) {
        fprintf(stderr, "Error: Part ID cannot be zero\n");
        return false;
    }

    if (part->name[0] == '\0') {
        fprintf(stderr, "Error: Part name cannot be empty\n");
        return false;
    }

    /* Check for valid UTF-8 or printable ASCII */
    for (size_t i = 0; i < sizeof(part->name) && part->name[i]; i++) {
        unsigned char c = (unsigned char)part->name[i];
        if (c < 0x20 && c != '\0') {
            fprintf(stderr, "Error: Part name contains control characters\n");
            return false;
        }
    }

    if (part->price < 0.0) {
        fprintf(stderr, "Error: Part price cannot be negative\n");
        return false;
    }

    /* Check for NaN or infinity */
    if (part->price != part->price || part->price > 1e15) {
        fprintf(stderr, "Error: Part price is invalid\n");
        return false;
    }

    return true;
}

/* Level 2: Debug check - internal precondition */
static void
internal_add_to_inventory(PartRecord *inventory, size_t *count,
                          size_t capacity, const PartRecord *part)
{
    /* Debug-only preconditions */
    ASSERT(inventory != NULL);
    ASSERT(count != NULL);
    ASSERT(part != NULL);
    ASSERT(*count < capacity);
    ASSERT(validate_part_input(part));  /* Redundant in release */

    inventory[*count] = *part;
    (*count)++;
}

/* Level 3: Expensive verification */
static uint32_t
compute_part_checksum(const PartRecord *part)
{
    /* Simple checksum for demonstration */
    const uint8_t *bytes = (const uint8_t *)part;
    uint32_t sum = 0;

    /* Skip the checksum field itself */
    size_t checksum_offset = offsetof(PartRecord, checksum);

    for (size_t i = 0; i < sizeof(PartRecord); i++) {
        if (i < checksum_offset || i >= checksum_offset + sizeof(uint32_t)) {
            sum = sum * 31 + bytes[i];
        }
    }

    return sum;
}

#ifdef ENABLE_EXPENSIVE_CHECKS

static bool
verify_inventory_integrity(const PartRecord *inventory, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        uint32_t computed = compute_part_checksum(&inventory[i]);
        if (computed != inventory[i].checksum) {
            fprintf(stderr, "Integrity error: Part %u checksum mismatch "
                    "(expected %u, got %u)\n",
                    inventory[i].id, inventory[i].checksum, computed);
            return false;
        }
    }
    return true;
}

#define VERIFY_INTEGRITY(inv, count) verify_inventory_integrity(inv, count)

#else

#define VERIFY_INTEGRITY(inv, count) true

#endif
```

## 10.2 Strategic Use of Assertions

### 10.2.1 What to Assert

```c
/* assertion_strategy.c */

/*
 * Maguire's rules for assertions:
 *
 * DO assert:
 *   - Function preconditions (requirements on inputs)
 *   - Function postconditions (promises about outputs)
 *   - Loop invariants
 *   - Data structure invariants
 *   - Impossible conditions ("this should never happen")
 *
 * DO NOT assert:
 *   - Expected error conditions (use error handling instead)
 *   - Side effects (assertions may be compiled out!)
 *   - External factors (file existence, network connectivity)
 */

#include <limits.h>

/*
 * Example: Binary search with full assertion coverage
 */
static int
binary_search(const int *arr, size_t count, int target)
{
    /* Preconditions */
    ASSERT(arr != NULL && "Array pointer must be valid");
    /* Array must be sorted - expensive check, debug only */
    #ifndef NDEBUG
    for (size_t i = 1; i < count; i++) {
        ASSERT(arr[i-1] <= arr[i] && "Array must be sorted");
    }
    #endif

    size_t low = 0;
    size_t high = count;

    while (low < high) {
        /* Loop invariant: if target exists, it's in arr[low..high) */
        ASSERT(low <= high);
        ASSERT(high <= count);

        size_t mid = low + (high - low) / 2;

        /* Mid calculation invariant */
        ASSERT(mid >= low && mid < high);

        if (arr[mid] < target) {
            low = mid + 1;
        } else if (arr[mid] > target) {
            high = mid;
        } else {
            /* Postcondition: found element matches target */
            ASSERT(arr[mid] == target);
            return (int)mid;
        }
    }

    /* Postcondition: if we return -1, target is not in array */
    #ifndef NDEBUG
    for (size_t i = 0; i < count; i++) {
        ASSERT(arr[i] != target && "Target not found but exists in array");
    }
    #endif

    return -1;
}

/*
 * Example: Memory allocation with assertion patterns
 */

typedef struct DynamicArray {
    int    *data;
    size_t  size;
    size_t  capacity;
} DynamicArray;

/* Invariant checking function */
static bool
dynarray_check_invariants(const DynamicArray *arr)
{
    if (!arr) return false;

    /* Size never exceeds capacity */
    if (arr->size > arr->capacity) return false;

    /* Data pointer consistency */
    if (arr->capacity > 0 && !arr->data) return false;
    if (arr->capacity == 0 && arr->data) return false;

    return true;
}

#define ASSERT_INVARIANTS(arr) \
    ASSERT(dynarray_check_invariants(arr) && "DynamicArray invariants violated")

static DynamicArray *
dynarray_create(size_t initial_capacity)
{
    DynamicArray *arr = calloc(1, sizeof(DynamicArray));
    if (!arr) {
        return NULL;  /* Expected failure - don't assert */
    }

    if (initial_capacity > 0) {
        arr->data = malloc(initial_capacity * sizeof(int));
        if (!arr->data) {
            free(arr);
            return NULL;  /* Expected failure */
        }
        arr->capacity = initial_capacity;
    }

    /* Postcondition */
    ASSERT_INVARIANTS(arr);
    return arr;
}

static bool
dynarray_push(DynamicArray *arr, int value)
{
    /* Precondition */
    ASSERT_INVARIANTS(arr);

    if (arr->size >= arr->capacity) {
        size_t new_cap = arr->capacity == 0 ? 16 : arr->capacity * 2;

        /* Check for overflow */
        if (new_cap < arr->capacity) {
            return false;  /* Expected failure - overflow */
        }

        int *new_data = realloc(arr->data, new_cap * sizeof(int));
        if (!new_data) {
            return false;  /* Expected failure - OOM */
        }

        arr->data = new_data;
        arr->capacity = new_cap;
    }

    size_t old_size = arr->size;  /* For postcondition check */

    arr->data[arr->size] = value;
    arr->size++;

    /* Postconditions */
    ASSERT(arr->size == old_size + 1 && "Size must increment by 1");
    ASSERT(arr->data[old_size] == value && "Value must be stored correctly");
    ASSERT_INVARIANTS(arr);

    return true;
}

static int
dynarray_get(const DynamicArray *arr, size_t index)
{
    /* Preconditions */
    ASSERT_INVARIANTS(arr);
    ASSERT(index < arr->size && "Index out of bounds");

    return arr->data[index];
}
```

### 10.2.2 Custom Assertion Macros

```c
/* custom_assertions.c */

/*
 * Writing Solid Code recommends domain-specific assertions
 */

#include <math.h>
#include <float.h>

/* General-purpose assertions */
#define ASSERT_NOT_NULL(ptr) \
    ASSERT((ptr) != NULL && #ptr " must not be NULL")

#define ASSERT_IN_RANGE(val, min, max) \
    ASSERT((val) >= (min) && (val) <= (max) && \
           #val " must be in range [" #min ", " #max "]")

#define ASSERT_POSITIVE(val) \
    ASSERT((val) > 0 && #val " must be positive")

#define ASSERT_NON_NEGATIVE(val) \
    ASSERT((val) >= 0 && #val " must be non-negative")

/* String assertions */
#define ASSERT_VALID_STRING(str) \
    ASSERT((str) != NULL && strlen(str) > 0 && #str " must be non-empty")

#define ASSERT_STRING_LENGTH(str, maxlen) \
    ASSERT((str) != NULL && strlen(str) <= (maxlen) && \
           #str " exceeds maximum length")

/* Array assertions */
#define ASSERT_VALID_INDEX(arr, index, count) \
    ASSERT((arr) != NULL && (index) < (count) && \
           "Invalid array index")

/* Floating-point assertions */
#define ASSERT_FINITE(val) \
    ASSERT(isfinite(val) && #val " must be finite (not NaN or Inf)")

#define ASSERT_FLOAT_EQ(a, b, epsilon) \
    ASSERT(fabs((a) - (b)) < (epsilon) && \
           #a " and " #b " must be approximately equal")

/* Pointer alignment assertions */
#define ASSERT_ALIGNED(ptr, alignment) \
    ASSERT(((uintptr_t)(ptr) % (alignment)) == 0 && \
           #ptr " must be aligned to " #alignment " bytes")

/* Bit assertions */
#define ASSERT_POWER_OF_TWO(val) \
    ASSERT((val) > 0 && ((val) & ((val) - 1)) == 0 && \
           #val " must be a power of two")

/*
 * Database-specific assertions for PartsDB
 */

typedef struct Database {
    void   *records;
    size_t  count;
    size_t  capacity;
    int     state;  /* 0=closed, 1=open, 2=in_transaction */
} Database;

#define DB_STATE_CLOSED        0
#define DB_STATE_OPEN          1
#define DB_STATE_IN_TRANSACTION 2

#define ASSERT_DB_OPEN(db) \
    ASSERT((db) != NULL && (db)->state >= DB_STATE_OPEN && \
           "Database must be open")

#define ASSERT_DB_IN_TRANSACTION(db) \
    ASSERT((db) != NULL && (db)->state == DB_STATE_IN_TRANSACTION && \
           "Operation requires active transaction")

#define ASSERT_DB_NOT_IN_TRANSACTION(db) \
    ASSERT((db) != NULL && (db)->state != DB_STATE_IN_TRANSACTION && \
           "Cannot perform operation during transaction")

/*
 * Thread-safety assertions (requires tracking)
 */

#ifdef ENABLE_THREAD_ASSERTIONS

#include <pthread.h>

/* Track which thread owns a resource */
typedef struct ThreadOwner {
    pthread_t owner;
    bool      is_owned;
} ThreadOwner;

#define ASSERT_OWNED_BY_CURRENT_THREAD(owner) \
    ASSERT((owner).is_owned && pthread_equal((owner).owner, pthread_self()) && \
           "Resource must be owned by current thread")

#define ASSERT_NOT_OWNED(owner) \
    ASSERT(!(owner).is_owned && "Resource must not be owned")

#else

#define ASSERT_OWNED_BY_CURRENT_THREAD(owner) ((void)0)
#define ASSERT_NOT_OWNED(owner) ((void)0)

#endif
```

## 10.3 Safe Memory Management

### 10.3.1 Allocation Strategies

```c
/* safe_memory.c */

/*
 * Maguire's memory management rules:
 *
 * 1. Always check allocation results
 * 2. Initialize allocated memory immediately
 * 3. Set pointers to NULL after freeing
 * 4. Track allocations in debug builds
 * 5. Use memory pools for fixed-size objects
 */

#include <stdlib.h>
#include <string.h>

/*
 * Safe allocation wrappers
 */

/* Allocate and zero-initialize (like calloc but for any type) */
#define SAFE_ALLOC(type) \
    ((type *)safe_alloc(sizeof(type), 1, __FILE__, __LINE__))

#define SAFE_ALLOC_ARRAY(type, count) \
    ((type *)safe_alloc(sizeof(type), count, __FILE__, __LINE__))

/* Free and NULL the pointer */
#define SAFE_FREE(ptr) \
    do { \
        safe_free(ptr, __FILE__, __LINE__); \
        (ptr) = NULL; \
    } while (0)

static void *
safe_alloc(size_t element_size, size_t count, const char *file, int line)
{
    /* Check for multiplication overflow */
    if (count > 0 && element_size > SIZE_MAX / count) {
        fprintf(stderr, "[%s:%d] Allocation overflow: %zu * %zu\n",
                file, line, element_size, count);
        return NULL;
    }

    size_t total = element_size * count;
    void *ptr = calloc(1, total);  /* Zero-initialized */

    if (!ptr) {
        fprintf(stderr, "[%s:%d] Failed to allocate %zu bytes\n",
                file, line, total);
    }

    #ifdef DEBUG_MEMORY
    track_allocation(ptr, total, file, line);
    #endif

    return ptr;
}

static void
safe_free(void *ptr, const char *file, int line)
{
    if (!ptr) {
        /* Freeing NULL is OK and harmless */
        return;
    }

    #ifdef DEBUG_MEMORY
    untrack_allocation(ptr, file, line);
    #endif

    /* Could zero memory before freeing for security */
    #ifdef SECURE_FREE
    /* Note: This doesn't work if we don't know the size.
     * In practice, track size with the allocation. */
    #endif

    free(ptr);
}

/*
 * Example: PartsDB record allocation
 */

typedef struct Part {
    uint32_t id;
    char    *name;       /* Heap-allocated */
    char    *description; /* Heap-allocated */
    double   price;
} Part;

static Part *
part_create(uint32_t id, const char *name, const char *description, double price)
{
    /* Validate inputs */
    ASSERT_VALID_STRING(name);
    ASSERT_NON_NEGATIVE(price);

    Part *part = SAFE_ALLOC(Part);
    if (!part) {
        return NULL;
    }

    part->id = id;
    part->price = price;

    /* Allocate strings */
    part->name = best_duplicate_string(name);
    if (!part->name) {
        SAFE_FREE(part);
        return NULL;
    }

    if (description) {
        part->description = best_duplicate_string(description);
        if (!part->description) {
            SAFE_FREE(part->name);
            SAFE_FREE(part);
            return NULL;
        }
    }

    return part;
}

static void
part_destroy(Part *part)
{
    if (!part) {
        return;
    }

    SAFE_FREE(part->name);
    SAFE_FREE(part->description);
    SAFE_FREE(part);
}

/*
 * Memory Pool for Fixed-Size Allocations
 */

typedef struct MemoryPool {
    uint8_t  *memory;
    size_t    element_size;
    size_t    capacity;
    size_t   *free_list;
    size_t    free_count;
    size_t    high_water_mark;  /* For profiling */
} MemoryPool;

static MemoryPool *
pool_create(size_t element_size, size_t capacity)
{
    ASSERT_POSITIVE(element_size);
    ASSERT_POSITIVE(capacity);

    MemoryPool *pool = SAFE_ALLOC(MemoryPool);
    if (!pool) {
        return NULL;
    }

    pool->memory = SAFE_ALLOC_ARRAY(uint8_t, element_size * capacity);
    if (!pool->memory) {
        SAFE_FREE(pool);
        return NULL;
    }

    pool->free_list = SAFE_ALLOC_ARRAY(size_t, capacity);
    if (!pool->free_list) {
        SAFE_FREE(pool->memory);
        SAFE_FREE(pool);
        return NULL;
    }

    pool->element_size = element_size;
    pool->capacity = capacity;
    pool->free_count = capacity;
    pool->high_water_mark = 0;

    /* Initialize free list (all slots available) */
    for (size_t i = 0; i < capacity; i++) {
        pool->free_list[i] = i;
    }

    return pool;
}

static void *
pool_alloc(MemoryPool *pool)
{
    ASSERT_NOT_NULL(pool);

    if (pool->free_count == 0) {
        return NULL;  /* Pool exhausted */
    }

    size_t index = pool->free_list[--pool->free_count];
    void *ptr = pool->memory + (index * pool->element_size);

    /* Track high water mark */
    size_t in_use = pool->capacity - pool->free_count;
    if (in_use > pool->high_water_mark) {
        pool->high_water_mark = in_use;
    }

    /* Zero-initialize */
    memset(ptr, 0, pool->element_size);

    return ptr;
}

static void
pool_free(MemoryPool *pool, void *ptr)
{
    ASSERT_NOT_NULL(pool);

    if (!ptr) {
        return;
    }

    /* Validate pointer is from this pool */
    uint8_t *byte_ptr = (uint8_t *)ptr;
    ASSERT(byte_ptr >= pool->memory);
    ASSERT(byte_ptr < pool->memory + (pool->capacity * pool->element_size));

    /* Calculate index */
    size_t offset = (size_t)(byte_ptr - pool->memory);
    ASSERT(offset % pool->element_size == 0 && "Misaligned pool pointer");
    size_t index = offset / pool->element_size;

    /* Clear memory (security) */
    memset(ptr, 0, pool->element_size);

    /* Return to free list */
    ASSERT(pool->free_count < pool->capacity);
    pool->free_list[pool->free_count++] = index;
}

static void
pool_destroy(MemoryPool *pool)
{
    if (!pool) {
        return;
    }

    /* Report usage statistics */
    #ifdef DEBUG
    printf("Pool stats: capacity=%zu, high_water=%zu (%.1f%% used)\n",
           pool->capacity, pool->high_water_mark,
           100.0 * pool->high_water_mark / pool->capacity);
    #endif

    SAFE_FREE(pool->free_list);
    SAFE_FREE(pool->memory);
    SAFE_FREE(pool);
}
```

### 10.3.2 Detecting Memory Errors

```c
/* memory_debugging.c */

/*
 * Debug memory system that detects:
 * - Buffer overruns (sentinel values)
 * - Use after free (poison patterns)
 * - Double free (tracking)
 * - Memory leaks (allocation tracking)
 */

#ifdef DEBUG_MEMORY

#include <stdint.h>

#define SENTINEL_HEAD  0xDEADBEEF
#define SENTINEL_TAIL  0xCAFEBABE
#define FREED_PATTERN  0xDD
#define UNINITIALIZED_PATTERN 0xCD

typedef struct DebugHeader {
    uint32_t    sentinel_head;
    size_t      size;
    const char *file;
    int         line;
    struct DebugHeader *next;
    struct DebugHeader *prev;
    uint32_t    freed;  /* Non-zero if already freed */
} DebugHeader;

static DebugHeader *g_alloc_list = NULL;
static size_t g_total_allocated = 0;
static size_t g_allocation_count = 0;
static pthread_mutex_t g_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *
debug_alloc(size_t size, const char *file, int line)
{
    if (size == 0) {
        return NULL;
    }

    /* Allocate header + user data + tail sentinel */
    size_t total = sizeof(DebugHeader) + size + sizeof(uint32_t);
    uint8_t *raw = malloc(total);

    if (!raw) {
        fprintf(stderr, "[%s:%d] Allocation of %zu bytes failed\n",
                file, line, size);
        return NULL;
    }

    /* Set up header */
    DebugHeader *header = (DebugHeader *)raw;
    header->sentinel_head = SENTINEL_HEAD;
    header->size = size;
    header->file = file;
    header->line = line;
    header->freed = 0;

    /* Set up tail sentinel */
    uint8_t *user_data = raw + sizeof(DebugHeader);
    uint32_t *tail = (uint32_t *)(user_data + size);
    *tail = SENTINEL_TAIL;

    /* Fill user data with uninitialized pattern */
    memset(user_data, UNINITIALIZED_PATTERN, size);

    /* Add to tracking list */
    pthread_mutex_lock(&g_alloc_mutex);
    header->next = g_alloc_list;
    header->prev = NULL;
    if (g_alloc_list) {
        g_alloc_list->prev = header;
    }
    g_alloc_list = header;
    g_total_allocated += size;
    g_allocation_count++;
    pthread_mutex_unlock(&g_alloc_mutex);

    return user_data;
}

static void
debug_free(void *ptr, const char *file, int line)
{
    if (!ptr) {
        return;
    }

    /* Get header */
    DebugHeader *header = (DebugHeader *)((uint8_t *)ptr - sizeof(DebugHeader));

    /* Check head sentinel */
    if (header->sentinel_head != SENTINEL_HEAD) {
        fprintf(stderr, "[%s:%d] MEMORY ERROR: Head sentinel corrupted at %p "
                "(heap underrun?)\n", file, line, ptr);
        abort();
    }

    /* Check tail sentinel */
    uint32_t *tail = (uint32_t *)((uint8_t *)ptr + header->size);
    if (*tail != SENTINEL_TAIL) {
        fprintf(stderr, "[%s:%d] MEMORY ERROR: Tail sentinel corrupted at %p "
                "(heap overrun?)\n"
                "  Originally allocated at %s:%d (%zu bytes)\n",
                file, line, ptr, header->file, header->line, header->size);
        abort();
    }

    /* Check for double-free */
    if (header->freed) {
        fprintf(stderr, "[%s:%d] MEMORY ERROR: Double free at %p\n"
                "  Originally allocated at %s:%d\n",
                file, line, ptr, header->file, header->line);
        abort();
    }

    /* Mark as freed */
    header->freed = 1;

    /* Remove from tracking list */
    pthread_mutex_lock(&g_alloc_mutex);
    if (header->prev) {
        header->prev->next = header->next;
    } else {
        g_alloc_list = header->next;
    }
    if (header->next) {
        header->next->prev = header->prev;
    }
    g_total_allocated -= header->size;
    g_allocation_count--;
    pthread_mutex_unlock(&g_alloc_mutex);

    /* Poison the memory */
    memset(ptr, FREED_PATTERN, header->size);

    free(header);
}

static void
debug_check_leaks(void)
{
    pthread_mutex_lock(&g_alloc_mutex);

    if (!g_alloc_list) {
        printf("No memory leaks detected.\n");
        pthread_mutex_unlock(&g_alloc_mutex);
        return;
    }

    printf("\n*** MEMORY LEAKS DETECTED ***\n");
    printf("Total: %zu bytes in %zu allocations\n\n",
           g_total_allocated, g_allocation_count);

    for (DebugHeader *h = g_alloc_list; h; h = h->next) {
        printf("  Leak: %zu bytes at %s:%d\n", h->size, h->file, h->line);

        /* Show first few bytes of leaked data */
        uint8_t *data = (uint8_t *)(h + 1);
        printf("    Data: ");
        for (size_t i = 0; i < h->size && i < 16; i++) {
            printf("%02X ", data[i]);
        }
        if (h->size > 16) {
            printf("...");
        }
        printf("\n");
    }

    pthread_mutex_unlock(&g_alloc_mutex);
}

/* Override standard functions */
#define malloc(size) debug_alloc(size, __FILE__, __LINE__)
#define free(ptr) debug_free(ptr, __FILE__, __LINE__)

#endif /* DEBUG_MEMORY */
```

## 10.4 Input Validation

### 10.4.1 Defensive Input Handling

```c
/* input_validation.c */

/*
 * Writing Solid Code emphasizes: "Never trust input"
 *
 * All external input must be validated:
 * - User input (command line, interactive)
 * - File contents
 * - Network data
 * - Inter-process communication
 * - Environment variables
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>

/*
 * String input validation
 */

typedef enum ValidationResult {
    VALID = 0,
    INVALID_NULL,
    INVALID_EMPTY,
    INVALID_TOO_LONG,
    INVALID_CHARACTERS,
    INVALID_FORMAT
} ValidationResult;

static const char *
validation_message(ValidationResult result)
{
    switch (result) {
    case VALID:              return "Valid";
    case INVALID_NULL:       return "Input is NULL";
    case INVALID_EMPTY:      return "Input is empty";
    case INVALID_TOO_LONG:   return "Input exceeds maximum length";
    case INVALID_CHARACTERS: return "Input contains invalid characters";
    case INVALID_FORMAT:     return "Input format is invalid";
    default:                 return "Unknown validation error";
    }
}

/* Validate a part name: alphanumeric, spaces, hyphens only */
static ValidationResult
validate_part_name(const char *name, size_t max_length)
{
    if (!name) {
        return INVALID_NULL;
    }

    if (name[0] == '\0') {
        return INVALID_EMPTY;
    }

    size_t len = 0;
    for (const char *p = name; *p; p++) {
        len++;

        if (len > max_length) {
            return INVALID_TOO_LONG;
        }

        /* Allow alphanumeric, space, hyphen, underscore */
        if (!isalnum((unsigned char)*p) &&
            *p != ' ' && *p != '-' && *p != '_') {
            return INVALID_CHARACTERS;
        }
    }

    return VALID;
}

/* Validate a part ID string (must be positive integer) */
static ValidationResult
validate_part_id_string(const char *str, uint32_t *out_id)
{
    if (!str) {
        return INVALID_NULL;
    }

    if (str[0] == '\0') {
        return INVALID_EMPTY;
    }

    /* Must start with digit */
    if (!isdigit((unsigned char)str[0])) {
        return INVALID_FORMAT;
    }

    /* Parse with overflow checking */
    char *end;
    errno = 0;
    unsigned long value = strtoul(str, &end, 10);

    if (errno == ERANGE || value > UINT32_MAX || value == 0) {
        return INVALID_FORMAT;
    }

    /* Must consume entire string */
    while (*end) {
        if (!isspace((unsigned char)*end)) {
            return INVALID_FORMAT;
        }
        end++;
    }

    if (out_id) {
        *out_id = (uint32_t)value;
    }

    return VALID;
}

/* Validate a price string */
static ValidationResult
validate_price_string(const char *str, double *out_price)
{
    if (!str) {
        return INVALID_NULL;
    }

    /* Skip leading whitespace */
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return INVALID_EMPTY;
    }

    /* Optional currency symbol */
    if (*str == '$') {
        str++;
    }

    /* Parse with error checking */
    char *end;
    errno = 0;
    double value = strtod(str, &end);

    if (errno == ERANGE) {
        return INVALID_FORMAT;
    }

    /* Check for valid number */
    if (end == str) {
        return INVALID_FORMAT;
    }

    /* Must be non-negative */
    if (value < 0.0) {
        return INVALID_FORMAT;
    }

    /* Check for NaN or infinity */
    if (value != value || value > 1e15) {
        return INVALID_FORMAT;
    }

    /* Rest must be whitespace */
    while (*end) {
        if (!isspace((unsigned char)*end)) {
            return INVALID_FORMAT;
        }
        end++;
    }

    if (out_price) {
        *out_price = value;
    }

    return VALID;
}

/*
 * Input sanitization
 */

/* Sanitize a string by removing/replacing dangerous characters */
static void
sanitize_string(char *str, size_t max_len)
{
    if (!str) {
        return;
    }

    size_t write = 0;

    for (size_t read = 0; str[read] && write < max_len - 1; read++) {
        unsigned char c = (unsigned char)str[read];

        /* Skip control characters except newline and tab */
        if (c < 0x20 && c != '\n' && c != '\t') {
            continue;
        }

        /* Skip DEL */
        if (c == 0x7F) {
            continue;
        }

        str[write++] = (char)c;
    }

    str[write] = '\0';
}

/* Escape string for safe display (replace control chars with ^X notation) */
static char *
escape_for_display(const char *str, char *buffer, size_t buffer_size)
{
    if (!str || !buffer || buffer_size == 0) {
        return NULL;
    }

    size_t write = 0;

    for (const char *p = str; *p && write < buffer_size - 1; p++) {
        unsigned char c = (unsigned char)*p;

        if (c < 0x20) {
            /* Control character - show as ^X */
            if (write + 2 >= buffer_size) break;
            buffer[write++] = '^';
            buffer[write++] = (char)(c + '@');
        } else if (c == 0x7F) {
            /* DEL - show as ^? */
            if (write + 2 >= buffer_size) break;
            buffer[write++] = '^';
            buffer[write++] = '?';
        } else {
            buffer[write++] = (char)c;
        }
    }

    buffer[write] = '\0';
    return buffer;
}

/*
 * Command-line argument parsing with validation
 */

typedef struct CommandLineArgs {
    const char *database_path;
    const char *operation;
    uint32_t    part_id;
    const char *part_name;
    double      price;
    int         quantity;
    bool        verbose;
    bool        dry_run;
} CommandLineArgs;

static bool
parse_command_line(int argc, char *argv[], CommandLineArgs *args)
{
    ASSERT_NOT_NULL(args);

    /* Initialize with defaults */
    memset(args, 0, sizeof(*args));
    args->database_path = "partsdb.dat";

    for (int i = 1; i < argc; i++) {
        if (!argv[i]) {
            fprintf(stderr, "Error: NULL argument at position %d\n", i);
            return false;
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--database") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: --database requires an argument\n");
                return false;
            }
            args->database_path = argv[i];
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            args->verbose = true;
        }
        else if (strcmp(argv[i], "--dry-run") == 0) {
            args->dry_run = true;
        }
        else if (strcmp(argv[i], "--id") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: --id requires an argument\n");
                return false;
            }
            ValidationResult vr = validate_part_id_string(argv[i], &args->part_id);
            if (vr != VALID) {
                fprintf(stderr, "Error: Invalid part ID: %s\n",
                        validation_message(vr));
                return false;
            }
        }
        else if (strcmp(argv[i], "--name") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: --name requires an argument\n");
                return false;
            }
            ValidationResult vr = validate_part_name(argv[i], 64);
            if (vr != VALID) {
                fprintf(stderr, "Error: Invalid part name: %s\n",
                        validation_message(vr));
                return false;
            }
            args->part_name = argv[i];
        }
        else if (strcmp(argv[i], "--price") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: --price requires an argument\n");
                return false;
            }
            ValidationResult vr = validate_price_string(argv[i], &args->price);
            if (vr != VALID) {
                fprintf(stderr, "Error: Invalid price: %s\n",
                        validation_message(vr));
                return false;
            }
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            return false;
        }
        else {
            /* Positional argument - operation */
            if (!args->operation) {
                args->operation = argv[i];
            } else {
                fprintf(stderr, "Error: Unexpected argument: %s\n", argv[i]);
                return false;
            }
        }
    }

    return true;
}
```

## 10.5 Error Handling Patterns

### 10.5.1 Comprehensive Error Handling

```c
/* error_handling.c */

/*
 * Writing Solid Code error handling principles:
 *
 * 1. Every function should have a clear error contract
 * 2. Errors should propagate, not be silently swallowed
 * 3. Cleanup on error paths must be complete
 * 4. Error messages should be informative
 */

#include <stdarg.h>

/*
 * Error context structure - captures error details
 */

typedef enum ErrorCode {
    ERR_SUCCESS = 0,
    ERR_NULL_ARGUMENT,
    ERR_INVALID_ARGUMENT,
    ERR_OUT_OF_MEMORY,
    ERR_NOT_FOUND,
    ERR_ALREADY_EXISTS,
    ERR_IO_ERROR,
    ERR_PERMISSION_DENIED,
    ERR_CORRUPT_DATA,
    ERR_INTERNAL_ERROR
} ErrorCode;

typedef struct ErrorContext {
    ErrorCode   code;
    const char *file;
    int         line;
    const char *function;
    char        message[256];
} ErrorContext;

/* Thread-local error context */
static _Thread_local ErrorContext g_error = {0};

static void
set_error(ErrorCode code, const char *file, int line,
          const char *function, const char *fmt, ...)
{
    g_error.code = code;
    g_error.file = file;
    g_error.line = line;
    g_error.function = function;

    va_list args;
    va_start(args, fmt);
    vsnprintf(g_error.message, sizeof(g_error.message), fmt, args);
    va_end(args);
}

#define SET_ERROR(code, fmt, ...) \
    set_error(code, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

static const ErrorContext *
get_last_error(void)
{
    return &g_error;
}

static const char *
error_code_string(ErrorCode code)
{
    switch (code) {
    case ERR_SUCCESS:           return "Success";
    case ERR_NULL_ARGUMENT:     return "Null argument";
    case ERR_INVALID_ARGUMENT:  return "Invalid argument";
    case ERR_OUT_OF_MEMORY:     return "Out of memory";
    case ERR_NOT_FOUND:         return "Not found";
    case ERR_ALREADY_EXISTS:    return "Already exists";
    case ERR_IO_ERROR:          return "I/O error";
    case ERR_PERMISSION_DENIED: return "Permission denied";
    case ERR_CORRUPT_DATA:      return "Corrupt data";
    case ERR_INTERNAL_ERROR:    return "Internal error";
    default:                    return "Unknown error";
    }
}

static void
print_error(void)
{
    const ErrorContext *err = get_last_error();
    if (err->code == ERR_SUCCESS) {
        return;
    }

    fprintf(stderr, "Error: %s\n", err->message);
    fprintf(stderr, "  Code: %s (%d)\n", error_code_string(err->code), err->code);
    fprintf(stderr, "  Location: %s:%d in %s()\n",
            err->file, err->line, err->function);
}

/*
 * GOTO-based cleanup pattern
 * (Maguire's recommended pattern for C error handling)
 */

static ErrorCode
complex_operation(const char *input_file, const char *output_file)
{
    ErrorCode result = ERR_SUCCESS;
    FILE *input = NULL;
    FILE *output = NULL;
    char *buffer = NULL;

    /* Validate arguments */
    if (!input_file || !output_file) {
        SET_ERROR(ERR_NULL_ARGUMENT, "File paths cannot be NULL");
        result = ERR_NULL_ARGUMENT;
        goto cleanup;
    }

    /* Open input file */
    input = fopen(input_file, "r");
    if (!input) {
        SET_ERROR(ERR_IO_ERROR, "Cannot open input file: %s", input_file);
        result = ERR_IO_ERROR;
        goto cleanup;
    }

    /* Open output file */
    output = fopen(output_file, "w");
    if (!output) {
        SET_ERROR(ERR_IO_ERROR, "Cannot open output file: %s", output_file);
        result = ERR_IO_ERROR;
        goto cleanup;
    }

    /* Allocate buffer */
    buffer = malloc(4096);
    if (!buffer) {
        SET_ERROR(ERR_OUT_OF_MEMORY, "Cannot allocate read buffer");
        result = ERR_OUT_OF_MEMORY;
        goto cleanup;
    }

    /* Process file */
    while (fgets(buffer, 4096, input)) {
        if (fputs(buffer, output) == EOF) {
            SET_ERROR(ERR_IO_ERROR, "Write error to %s", output_file);
            result = ERR_IO_ERROR;
            goto cleanup;
        }
    }

    if (ferror(input)) {
        SET_ERROR(ERR_IO_ERROR, "Read error from %s", input_file);
        result = ERR_IO_ERROR;
        goto cleanup;
    }

cleanup:
    /* Cleanup in reverse order of allocation */
    free(buffer);

    if (output) {
        fclose(output);
        /* Delete output file on error */
        if (result != ERR_SUCCESS) {
            remove(output_file);
        }
    }

    if (input) {
        fclose(input);
    }

    return result;
}

/*
 * Wrapper macro for consistent error propagation
 */

#define TRY(expr) \
    do { \
        ErrorCode _err = (expr); \
        if (_err != ERR_SUCCESS) { \
            return _err; \
        } \
    } while (0)

#define TRY_GOTO(expr, label) \
    do { \
        result = (expr); \
        if (result != ERR_SUCCESS) { \
            goto label; \
        } \
    } while (0)

/* Example using TRY macro */
static ErrorCode
process_parts_file(const char *filename)
{
    TRY(validate_file_path(filename));
    TRY(open_database(filename));
    TRY(load_parts());
    TRY(validate_parts());
    return ERR_SUCCESS;
}
```

### 10.5.2 Resource Acquisition Is Initialization (RAII) in C

```c
/* raii_patterns.c */

/*
 * While C doesn't have destructors, we can simulate RAII
 * using cleanup attributes (GCC/Clang extension) or
 * structured cleanup patterns.
 */

#ifdef __GNUC__

/* GCC/Clang cleanup attribute */
#define AUTO_CLEANUP(func) __attribute__((cleanup(func)))

static void
cleanup_file(FILE **fp)
{
    if (*fp) {
        fclose(*fp);
        *fp = NULL;
    }
}

static void
cleanup_free(void **ptr)
{
    if (*ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

#define AUTO_FILE AUTO_CLEANUP(cleanup_file) FILE *
#define AUTO_FREE(type) AUTO_CLEANUP(cleanup_free) type *

/* Usage example */
static ErrorCode
process_with_auto_cleanup(const char *filename)
{
    AUTO_FILE file = fopen(filename, "r");
    if (!file) {
        SET_ERROR(ERR_IO_ERROR, "Cannot open %s", filename);
        return ERR_IO_ERROR;
    }

    AUTO_FREE(char) buffer = malloc(4096);
    if (!buffer) {
        SET_ERROR(ERR_OUT_OF_MEMORY, "Cannot allocate buffer");
        return ERR_OUT_OF_MEMORY;
    }

    /* file and buffer automatically cleaned up on any return */

    while (fgets(buffer, 4096, file)) {
        /* Process line */
        if (some_error_condition()) {
            return ERR_INTERNAL_ERROR;  /* Cleanup happens automatically */
        }
    }

    return ERR_SUCCESS;
    /* Cleanup happens here too */
}

#endif /* __GNUC__ */

/*
 * Portable RAII-like pattern using explicit scope guards
 */

typedef struct ScopeGuard {
    void (*cleanup)(void *);
    void *data;
} ScopeGuard;

#define SCOPE_GUARD_MAX 8

typedef struct ScopeGuardStack {
    ScopeGuard guards[SCOPE_GUARD_MAX];
    size_t count;
} ScopeGuardStack;

static void
scope_init(ScopeGuardStack *stack)
{
    stack->count = 0;
}

static void
scope_push(ScopeGuardStack *stack, void (*cleanup)(void *), void *data)
{
    ASSERT(stack->count < SCOPE_GUARD_MAX);
    stack->guards[stack->count].cleanup = cleanup;
    stack->guards[stack->count].data = data;
    stack->count++;
}

static void
scope_cleanup(ScopeGuardStack *stack)
{
    /* Cleanup in reverse order */
    while (stack->count > 0) {
        stack->count--;
        ScopeGuard *g = &stack->guards[stack->count];
        if (g->cleanup && g->data) {
            g->cleanup(g->data);
        }
    }
}

/* Cleanup functions */
static void cleanup_fclose(void *fp) { fclose(*(FILE **)fp); }
static void cleanup_ptr(void *ptr) { free(*(void **)ptr); }

/* Usage example */
static ErrorCode
process_with_scope_guards(const char *filename)
{
    ScopeGuardStack scope;
    scope_init(&scope);
    ErrorCode result = ERR_SUCCESS;

    FILE *file = fopen(filename, "r");
    if (!file) {
        result = ERR_IO_ERROR;
        goto done;
    }
    scope_push(&scope, cleanup_fclose, &file);

    char *buffer = malloc(4096);
    if (!buffer) {
        result = ERR_OUT_OF_MEMORY;
        goto done;
    }
    scope_push(&scope, cleanup_ptr, &buffer);

    /* Process... */

done:
    scope_cleanup(&scope);
    return result;
}
```

## 10.6 Compiler as a Tool

### 10.6.1 Maximizing Compiler Warnings

```c
/* compiler_warnings.c */

/*
 * Maguire: "The compiler is your first line of defense"
 *
 * Recommended warning flags for GCC/Clang:
 *
 * Essential:
 *   -Wall            Enable common warnings
 *   -Wextra          Enable extra warnings
 *   -Werror          Treat warnings as errors
 *
 * Highly recommended:
 *   -Wpedantic       Strict ISO C compliance
 *   -Wshadow         Variable shadowing
 *   -Wconversion     Implicit type conversions
 *   -Wstrict-prototypes  Require function prototypes
 *
 * Security focused:
 *   -Wformat=2       Format string issues
 *   -Wstack-protector  Stack protection
 *   -D_FORTIFY_SOURCE=2  Buffer overflow detection
 *
 * Full recommended set:
 *   CFLAGS = -Wall -Wextra -Werror -Wpedantic \
 *            -Wshadow -Wconversion -Wstrict-prototypes \
 *            -Wformat=2 -Wformat-security \
 *            -Wnull-dereference -Wdouble-promotion \
 *            -Wimplicit-fallthrough -Wstrict-overflow=2 \
 *            -fstack-protector-strong \
 *            -D_FORTIFY_SOURCE=2
 */

/*
 * Common warnings and how to fix them
 */

/* Warning: unused parameter */
void
handle_event_unused(int event_type, void *data)
{
    /* Fix: use (void) cast to acknowledge intentionally unused */
    (void)data;

    printf("Event: %d\n", event_type);
}

/* Warning: implicit conversion loses precision */
void
implicit_conversion_warning(void)
{
    size_t big = 1000;
    int small = big;  /* Warning! */

    /* Fix: explicit cast with range check */
    ASSERT(big <= INT_MAX);
    int safe = (int)big;

    (void)small;
    (void)safe;
}

/* Warning: comparing signed and unsigned */
void
signed_unsigned_comparison(int count)
{
    size_t size = 100;

    if (count < size) { }  /* Warning! */

    /* Fix: explicit conversion */
    if (count < 0 || (size_t)count < size) { }
}

/* Warning: variable shadowing */
int g_value = 10;

void
shadow_warning(void)
{
    int g_value = 20;  /* Warning: shadows global */

    /* Fix: use different name */
    int local_value = 20;

    (void)local_value;
}

/* Warning: implicit fallthrough in switch */
void
switch_fallthrough(int x)
{
    switch (x) {
    case 1:
        printf("One\n");
        /* Warning: falls through to case 2 */
    case 2:
        printf("Two\n");
        break;
    }

    /* Fix: use explicit fallthrough attribute (C23) or comment */
    switch (x) {
    case 1:
        printf("One\n");
        [[fallthrough]];  /* C23 */
    case 2:
        printf("Two\n");
        break;
    }
}

/*
 * Static analysis annotations (Clang)
 */

#if defined(__clang__)

/* Mark function as not returning */
void fatal_error(const char *msg) __attribute__((noreturn));

/* Mark parameter as non-null */
void process_data(void *data) __attribute__((nonnull(1)));

/* Mark return value as must-use */
__attribute__((warn_unused_result))
int important_function(void);

/* Mark function as deprecated */
__attribute__((deprecated("Use new_function instead")))
void old_function(void);

/* Mark as printf-like for format checking */
__attribute__((format(printf, 1, 2)))
void log_message(const char *fmt, ...);

#endif
```

## 10.7 Review Questions

1. **Philosophy of defense**: Maguire states "bugs are not inevitable." What development
   practices does he recommend to prevent bugs before they're written? How do these
   practices apply to safety-critical systems like medical devices or aviation?

2. **Assertion strategy**: What is the difference between assertions (which may be
   compiled out) and verification checks (which always run)? When should you use each?
   Give examples from a database application.

3. **Memory debugging**: The chapter shows sentinel values for detecting buffer overruns.
   What other techniques does "Writing Solid Code" recommend for catching memory errors?
   How do these compare to modern tools like AddressSanitizer?

4. **Input validation layers**: At what points in a program should input validation occur?
   Why is it important to validate both at boundaries and internally? What's the
   trade-off between security and performance?

5. **Error propagation**: Compare the GOTO-based cleanup pattern with the TRY macro
   approach. What are the advantages and disadvantages of each? How would you choose
   between them for a particular codebase?

6. **Compiler warnings**: List five compiler warnings that commonly catch real bugs.
   For each, explain what the warning detects and how to properly fix the code
   (not just suppress the warning).

7. **Invariant checking**: What are class invariants and loop invariants? Write
   invariant-checking functions for a hash table implementation. When should
   these checks run in production vs. debug builds?

8. **Memory pool benefits**: Why does "Writing Solid Code" recommend memory pools
   for fixed-size allocations? What bugs do they help prevent? What are the
   limitations of the pool approach?

9. **RAII in C**: The chapter shows cleanup attributes and scope guards for resource
   management. Compare these approaches with traditional GOTO-based cleanup. What
   are the portability considerations?

10. **Testing defensive code**: How do you test defensive checks that should "never"
    trigger in normal operation? Design tests for the error handling code shown
    in this chapter.

---

*Next: Part 11 - Pointer Mechanics & Dynamic Allocation (Pointers on C)*
