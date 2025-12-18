# Part 9: Style, Debugging & Portability

*Drawing from "The Practice of Programming" by Kernighan & Pike*

## Introduction

"The Practice of Programming" emphasizes that good programming is more than just writing
code that works—it's about writing code that is readable, maintainable, testable, and
portable. This part covers the practical wisdom that separates amateur code from
professional-grade software.

> "The purpose of style is to make the code easy to read for yourself and others, and
> good style is crucial to good programming." — Kernighan & Pike

## 9.1 Programming Style

### 9.1.1 Naming Conventions

Names are the most important documentation in your code. They should be:
- Descriptive but concise
- Consistent throughout the codebase
- Appropriate to scope (longer names for wider scope)

```c
/* Bad naming - what do these do? */
int f(int x, int y) {
    int t = x;
    x = y;
    y = t;
    return x + y;
}

void p(char *s) {
    while (*s) putchar(*s++);
}

/* Good naming - intent is clear */
int sum_after_swap(int first, int second) {
    int temp = first;
    first = second;
    second = temp;
    return first + second;
}

void print_string(const char *str) {
    while (*str) {
        putchar(*str++);
    }
}
```

#### Naming Guidelines for PartsDB

```c
/* partsdb_naming.h - Consistent naming conventions */
#ifndef PARTSDB_NAMING_H
#define PARTSDB_NAMING_H

/*
 * Naming Convention Rules:
 *
 * 1. Types: PascalCase with _t suffix for typedefs
 *    - PartRecord_t, DatabaseHandle_t, ErrorCode_t
 *
 * 2. Functions: snake_case with module prefix
 *    - partsdb_open(), partsdb_insert_part(), partsdb_close()
 *
 * 3. Constants: SCREAMING_SNAKE_CASE with module prefix
 *    - PARTSDB_MAX_PARTS, PARTSDB_VERSION_MAJOR
 *
 * 4. Variables: snake_case
 *    - part_count, current_index, is_valid
 *
 * 5. Macros: SCREAMING_SNAKE_CASE
 *    - ARRAY_SIZE(), MIN(), MAX()
 *
 * 6. Global variables: g_ prefix (avoid when possible)
 *    - g_database_instance, g_error_handler
 *
 * 7. Static variables: s_ prefix
 *    - s_instance_count, s_initialized
 *
 * 8. Boolean variables/functions: is_, has_, can_, should_ prefix
 *    - is_valid, has_permission, can_modify, should_retry
 */

/* Example: Well-named database operations */
typedef struct PartRecord {
    uint32_t    part_id;
    char        part_name[64];
    char        part_description[256];
    uint32_t    quantity_in_stock;
    uint32_t    reorder_threshold;
    double      unit_price;
    time_t      last_modified;
    bool        is_active;
    bool        requires_certification;
} PartRecord_t;

typedef enum DatabaseResult {
    DB_SUCCESS = 0,
    DB_ERROR_NOT_FOUND,
    DB_ERROR_DUPLICATE,
    DB_ERROR_FULL,
    DB_ERROR_INVALID_INPUT,
    DB_ERROR_IO_FAILURE,
    DB_ERROR_PERMISSION_DENIED
} DatabaseResult_t;

/* Function names clearly indicate their purpose */
DatabaseResult_t partsdb_initialize(const char *database_path);
DatabaseResult_t partsdb_insert_part(const PartRecord_t *part);
DatabaseResult_t partsdb_find_part_by_id(uint32_t part_id, PartRecord_t *result);
DatabaseResult_t partsdb_find_parts_by_name(const char *pattern,
                                            PartRecord_t *results,
                                            size_t max_results,
                                            size_t *actual_count);
DatabaseResult_t partsdb_update_part(const PartRecord_t *part);
DatabaseResult_t partsdb_delete_part(uint32_t part_id);
bool             partsdb_is_initialized(void);
size_t           partsdb_get_part_count(void);
const char      *partsdb_result_to_string(DatabaseResult_t result);

#endif /* PARTSDB_NAMING_H */
```

### 9.1.2 Code Layout and Indentation

Consistent formatting makes code scannable:

```c
/* style_guidelines.c - Layout examples */

/*
 * Indentation: Use consistent indentation (4 spaces or tabs)
 * Braces: Choose a style and stick with it
 */

/* K&R style (recommended by Kernighan & Pike) */
if (condition) {
    do_something();
} else {
    do_other_thing();
}

/* Function definitions - opening brace on new line is acceptable */
static int
process_part(PartRecord_t *part)
{
    if (!part) {
        return -1;
    }

    /* Align related declarations */
    int         result      = 0;
    size_t      name_len    = strlen(part->part_name);
    const char *description = part->part_description;

    /* One statement per line */
    result = validate_part(part);
    if (result != 0) {
        return result;
    }

    result = store_part(part);
    return result;
}

/* Switch statements - clear case alignment */
static const char *
status_to_string(PartStatus status)
{
    switch (status) {
    case PART_ACTIVE:
        return "Active";
    case PART_DISCONTINUED:
        return "Discontinued";
    case PART_PENDING:
        return "Pending Review";
    case PART_RECALLED:
        return "Recalled";
    default:
        return "Unknown";
    }
}

/* Long function calls - break at arguments */
result = partsdb_search_parts(
    database,
    search_pattern,
    SEARCH_FLAG_CASE_INSENSITIVE | SEARCH_FLAG_PARTIAL_MATCH,
    results_buffer,
    MAX_RESULTS,
    &actual_count
);

/* Long conditions - break at logical operators */
if (part->is_active &&
    part->quantity_in_stock > 0 &&
    part->unit_price <= max_price &&
    !part->requires_certification)
{
    add_to_order(part);
}
```

### 9.1.3 Comments and Documentation

Comments should explain *why*, not *what*:

```c
/* commenting_guidelines.c */

/*
 * BAD: Comments that state the obvious
 */
int count = 0;  /* Initialize count to zero */
count++;        /* Increment count */

/*
 * GOOD: Comments that explain reasoning
 */

/*
 * Use power-of-2 table size for fast modulo via bitwise AND.
 * Must be at least 64 to handle worst-case clustering.
 */
#define HASH_TABLE_SIZE 128

/*
 * Double-check locking requires memory barriers on some architectures.
 * The atomic_load with acquire semantics ensures we see the fully
 * constructed object after another thread publishes it.
 */
static PartCache *
get_part_cache(void)
{
    PartCache *cache = atomic_load_explicit(&g_cache, memory_order_acquire);
    if (cache) {
        return cache;
    }

    pthread_mutex_lock(&g_cache_mutex);
    cache = atomic_load_explicit(&g_cache, memory_order_relaxed);
    if (!cache) {
        cache = create_part_cache();
        atomic_store_explicit(&g_cache, cache, memory_order_release);
    }
    pthread_mutex_unlock(&g_cache_mutex);

    return cache;
}

/*
 * Function documentation - explain interface, not implementation
 */

/**
 * Search for parts matching the given criteria.
 *
 * @param db        Initialized database handle (must not be NULL)
 * @param criteria  Search criteria (NULL fields act as wildcards)
 * @param results   Output buffer for matching parts
 * @param max_count Maximum number of results to return
 * @param[out] found_count  Actual number of matches found
 *
 * @return DB_SUCCESS on success (even if no matches found)
 *         DB_ERROR_INVALID_INPUT if db or results is NULL
 *         DB_ERROR_IO_FAILURE on database access error
 *
 * @note Results are sorted by part_id in ascending order.
 * @note This function is thread-safe.
 *
 * @warning The caller must not modify the database while iterating
 *          over results, as this may invalidate internal cursors.
 */
DatabaseResult_t
partsdb_search(
    DatabaseHandle_t *db,
    const SearchCriteria_t *criteria,
    PartRecord_t *results,
    size_t max_count,
    size_t *found_count);
```

## 9.2 Debugging Strategies

### 9.2.1 Scientific Debugging

Debugging is a systematic process, not random guessing:

```c
/* debugging_example.c - Scientific debugging approach */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * The Scientific Method for Debugging:
 *
 * 1. Reproduce the bug reliably
 * 2. Form a hypothesis about the cause
 * 3. Design an experiment to test the hypothesis
 * 4. Analyze results
 * 5. Repeat until bug is found
 */

/* Example: A subtle off-by-one bug */

typedef struct StringBuffer {
    char   *data;
    size_t  length;
    size_t  capacity;
} StringBuffer;

/* BUG: This function has an off-by-one error */
static int
stringbuf_append_buggy(StringBuffer *buf, const char *str)
{
    size_t str_len = strlen(str);
    size_t required = buf->length + str_len;  /* BUG: forgot +1 for null */

    if (required > buf->capacity) {
        size_t new_capacity = required * 2;
        char *new_data = realloc(buf->data, new_capacity);
        if (!new_data) {
            return -1;
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }

    memcpy(buf->data + buf->length, str, str_len + 1);
    buf->length += str_len;
    return 0;
}

/*
 * Debugging Process:
 *
 * 1. Symptom: Occasional crashes when appending strings
 * 2. Hypothesis: Buffer overflow when string fills exactly to capacity
 * 3. Test: Add assertions to catch the condition
 */

static int
stringbuf_append_debugged(StringBuffer *buf, const char *str)
{
    /* Debug: Validate inputs */
    assert(buf != NULL);
    assert(str != NULL);
    assert(buf->data != NULL || buf->capacity == 0);

    size_t str_len = strlen(str);
    size_t required = buf->length + str_len + 1;  /* +1 for null terminator */

    /* Debug: Log state before operation */
    #ifdef DEBUG
    fprintf(stderr, "[DEBUG] append: len=%zu, cap=%zu, adding=%zu, need=%zu\n",
            buf->length, buf->capacity, str_len, required);
    #endif

    if (required > buf->capacity) {
        size_t new_capacity = required * 2;

        /* Debug: Check for integer overflow */
        assert(new_capacity > required);

        char *new_data = realloc(buf->data, new_capacity);
        if (!new_data) {
            return -1;
        }
        buf->data = new_data;
        buf->capacity = new_capacity;

        #ifdef DEBUG
        fprintf(stderr, "[DEBUG] reallocated to capacity=%zu\n", new_capacity);
        #endif
    }

    memcpy(buf->data + buf->length, str, str_len + 1);
    buf->length += str_len;

    /* Debug: Verify invariants after operation */
    assert(buf->length < buf->capacity);  /* Room for null terminator */
    assert(buf->data[buf->length] == '\0');

    return 0;
}

/*
 * Debug Logging Infrastructure for PartsDB
 */

typedef enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

static LogLevel g_log_level = LOG_INFO;
static FILE    *g_log_file = NULL;

#define LOG(level, fmt, ...) \
    do { \
        if ((level) >= g_log_level) { \
            log_message((level), __FILE__, __LINE__, __func__, \
                       (fmt), ##__VA_ARGS__); \
        } \
    } while (0)

#define LOG_TRACE(fmt, ...) LOG(LOG_TRACE, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  LOG(LOG_INFO,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG(LOG_WARN,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOG(LOG_FATAL, fmt, ##__VA_ARGS__)

static const char *
log_level_string(LogLevel level)
{
    static const char *names[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    if (level < 0 || level > LOG_FATAL) {
        return "UNKNOWN";
    }
    return names[level];
}

static void
log_message(LogLevel level, const char *file, int line,
            const char *func, const char *fmt, ...)
{
    FILE *output = g_log_file ? g_log_file : stderr;

    /* Timestamp */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Format: [TIME] LEVEL file:line (func): message */
    fprintf(output, "[%s] %-5s %s:%d (%s): ",
            timestamp, log_level_string(level), file, line, func);

    va_list args;
    va_start(args, fmt);
    vfprintf(output, fmt, args);
    va_end(args);

    fprintf(output, "\n");
    fflush(output);
}
```

### 9.2.2 Defensive Programming with Assertions

```c
/* defensive_assertions.c */
#include <assert.h>
#include <stddef.h>

/*
 * Use assertions to document and verify assumptions.
 * Assertions should NEVER have side effects.
 */

/* WRONG: Side effect in assertion (stripped in release builds!) */
// assert(read_data(&buffer) == SUCCESS);

/* RIGHT: Separate operation from assertion */
// int result = read_data(&buffer);
// assert(result == SUCCESS);

/*
 * Custom assertion macros for production code
 */

/* Runtime check that stays in release builds */
#define VERIFY(condition) \
    do { \
        if (!(condition)) { \
            handle_verification_failure(#condition, __FILE__, __LINE__); \
        } \
    } while (0)

/* Static assertion (compile-time check) */
#define STATIC_ASSERT(condition, message) \
    _Static_assert((condition), message)

/* Example: PartsDB integrity checks */

typedef struct PartDatabase {
    PartRecord_t *records;
    size_t        count;
    size_t        capacity;
    uint32_t      checksum;
    bool          is_dirty;
} PartDatabase;

/*
 * Invariant checking - verify data structure consistency
 */
static bool
database_check_invariants(const PartDatabase *db)
{
    /* Basic pointer validity */
    if (!db) {
        return false;
    }

    /* Capacity invariants */
    if (db->count > db->capacity) {
        LOG_ERROR("Invariant violation: count > capacity (%zu > %zu)",
                  db->count, db->capacity);
        return false;
    }

    /* Buffer allocation consistency */
    if (db->capacity > 0 && !db->records) {
        LOG_ERROR("Invariant violation: capacity=%zu but records=NULL",
                  db->capacity);
        return false;
    }

    if (db->capacity == 0 && db->records != NULL) {
        LOG_ERROR("Invariant violation: capacity=0 but records!=NULL");
        return false;
    }

    /* Checksum validation (expensive, use sparingly) */
    #ifdef DEBUG
    uint32_t computed = compute_checksum(db->records, db->count);
    if (computed != db->checksum) {
        LOG_ERROR("Invariant violation: checksum mismatch (%u != %u)",
                  computed, db->checksum);
        return false;
    }
    #endif

    return true;
}

/*
 * Function with full defensive checks
 */
DatabaseResult_t
database_insert(PartDatabase *db, const PartRecord_t *part)
{
    /* Preconditions */
    VERIFY(db != NULL);
    VERIFY(part != NULL);
    assert(database_check_invariants(db));

    /* Validate input data */
    if (part->part_id == 0) {
        LOG_WARN("Attempted to insert part with invalid ID 0");
        return DB_ERROR_INVALID_INPUT;
    }

    if (part->part_name[0] == '\0') {
        LOG_WARN("Attempted to insert part with empty name");
        return DB_ERROR_INVALID_INPUT;
    }

    /* Check for duplicates */
    for (size_t i = 0; i < db->count; i++) {
        if (db->records[i].part_id == part->part_id) {
            LOG_DEBUG("Part ID %u already exists at index %zu",
                      part->part_id, i);
            return DB_ERROR_DUPLICATE;
        }
    }

    /* Ensure capacity */
    if (db->count >= db->capacity) {
        size_t new_capacity = (db->capacity == 0) ? 16 : db->capacity * 2;
        PartRecord_t *new_records = realloc(db->records,
                                            new_capacity * sizeof(*new_records));
        if (!new_records) {
            LOG_ERROR("Failed to allocate memory for %zu records", new_capacity);
            return DB_ERROR_IO_FAILURE;
        }
        db->records = new_records;
        db->capacity = new_capacity;
    }

    /* Insert the record */
    db->records[db->count] = *part;
    db->count++;
    db->is_dirty = true;

    /* Update checksum */
    db->checksum = compute_checksum(db->records, db->count);

    /* Postconditions */
    assert(db->count <= db->capacity);
    assert(database_check_invariants(db));

    LOG_DEBUG("Inserted part ID %u at index %zu", part->part_id, db->count - 1);
    return DB_SUCCESS;
}
```

### 9.2.3 Memory Debugging

```c
/* memory_debugging.c */
#include <stdlib.h>
#include <string.h>

/*
 * Memory debugging wrapper for development builds
 */

#ifdef DEBUG_MEMORY

typedef struct AllocationInfo {
    void       *ptr;
    size_t      size;
    const char *file;
    int         line;
    struct AllocationInfo *next;
} AllocationInfo;

static AllocationInfo *g_allocations = NULL;
static size_t g_total_allocated = 0;
static size_t g_allocation_count = 0;

void *
debug_malloc(size_t size, const char *file, int line)
{
    void *ptr = malloc(size);
    if (ptr) {
        AllocationInfo *info = malloc(sizeof(AllocationInfo));
        if (info) {
            info->ptr = ptr;
            info->size = size;
            info->file = file;
            info->line = line;
            info->next = g_allocations;
            g_allocations = info;
            g_total_allocated += size;
            g_allocation_count++;
        }

        /* Fill with recognizable pattern to catch uninitialized use */
        memset(ptr, 0xCD, size);
    }
    return ptr;
}

void *
debug_calloc(size_t count, size_t size, const char *file, int line)
{
    size_t total = count * size;
    void *ptr = debug_malloc(total, file, line);
    if (ptr) {
        memset(ptr, 0, total);  /* Override debug pattern with zeros */
    }
    return ptr;
}

void *
debug_realloc(void *ptr, size_t size, const char *file, int line)
{
    if (!ptr) {
        return debug_malloc(size, file, line);
    }

    /* Find and update allocation record */
    AllocationInfo **pp = &g_allocations;
    while (*pp) {
        if ((*pp)->ptr == ptr) {
            size_t old_size = (*pp)->size;
            void *new_ptr = realloc(ptr, size);
            if (new_ptr) {
                (*pp)->ptr = new_ptr;
                (*pp)->size = size;
                (*pp)->file = file;
                (*pp)->line = line;
                g_total_allocated = g_total_allocated - old_size + size;

                /* Fill new memory with debug pattern */
                if (size > old_size) {
                    memset((char *)new_ptr + old_size, 0xCD, size - old_size);
                }
            }
            return new_ptr;
        }
        pp = &(*pp)->next;
    }

    /* Unknown pointer - potential double-free or corruption */
    fprintf(stderr, "MEMORY ERROR: realloc of unknown pointer %p at %s:%d\n",
            ptr, file, line);
    abort();
}

void
debug_free(void *ptr, const char *file, int line)
{
    if (!ptr) {
        return;
    }

    AllocationInfo **pp = &g_allocations;
    while (*pp) {
        if ((*pp)->ptr == ptr) {
            AllocationInfo *info = *pp;
            *pp = info->next;

            g_total_allocated -= info->size;
            g_allocation_count--;

            /* Fill with recognizable pattern to catch use-after-free */
            memset(ptr, 0xDD, info->size);

            free(info);
            free(ptr);
            return;
        }
        pp = &(*pp)->next;
    }

    /* Unknown pointer */
    fprintf(stderr, "MEMORY ERROR: free of unknown pointer %p at %s:%d\n",
            ptr, file, line);
    abort();
}

void
debug_memory_report(void)
{
    fprintf(stderr, "\n=== Memory Report ===\n");
    fprintf(stderr, "Total allocated: %zu bytes in %zu blocks\n",
            g_total_allocated, g_allocation_count);

    if (g_allocations) {
        fprintf(stderr, "\nLEAKED ALLOCATIONS:\n");
        for (AllocationInfo *info = g_allocations; info; info = info->next) {
            fprintf(stderr, "  %p: %zu bytes allocated at %s:%d\n",
                    info->ptr, info->size, info->file, info->line);
        }
    } else {
        fprintf(stderr, "No memory leaks detected.\n");
    }
}

#define malloc(size)        debug_malloc(size, __FILE__, __LINE__)
#define calloc(count, size) debug_calloc(count, size, __FILE__, __LINE__)
#define realloc(ptr, size)  debug_realloc(ptr, size, __FILE__, __LINE__)
#define free(ptr)           debug_free(ptr, __FILE__, __LINE__)

#endif /* DEBUG_MEMORY */
```

## 9.3 Testing Strategies

### 9.3.1 Unit Testing Framework

```c
/* testing_framework.c - Minimal unit testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/*
 * Lightweight testing framework inspired by Practice of Programming
 */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static jmp_buf g_test_jump;
static const char *g_current_test = NULL;

#define TEST(name) \
    static void test_##name(void); \
    static void run_test_##name(void) { \
        g_current_test = #name; \
        g_tests_run++; \
        if (setjmp(g_test_jump) == 0) { \
            test_##name(); \
            g_tests_passed++; \
            printf("  PASS: %s\n", #name); \
        } else { \
            g_tests_failed++; \
        } \
    } \
    static void test_##name(void)

#define RUN_TEST(name) run_test_##name()

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", g_current_test); \
            printf("        Assertion failed: %s\n", #condition); \
            printf("        at %s:%d\n", __FILE__, __LINE__); \
            longjmp(g_test_jump, 1); \
        } \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  FAIL: %s\n", g_current_test); \
            printf("        Expected: %d, Actual: %d\n", \
                   (int)(expected), (int)(actual)); \
            printf("        at %s:%d\n", __FILE__, __LINE__); \
            longjmp(g_test_jump, 1); \
        } \
    } while (0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf("  FAIL: %s\n", g_current_test); \
            printf("        Expected: \"%s\"\n", (expected)); \
            printf("        Actual:   \"%s\"\n", (actual)); \
            printf("        at %s:%d\n", __FILE__, __LINE__); \
            longjmp(g_test_jump, 1); \
        } \
    } while (0)

#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_NULL(ptr)     ASSERT_TRUE((ptr) == NULL)

static void
print_test_summary(void)
{
    printf("\n=== Test Summary ===\n");
    printf("Tests run:    %d\n", g_tests_run);
    printf("Tests passed: %d\n", g_tests_passed);
    printf("Tests failed: %d\n", g_tests_failed);

    if (g_tests_failed == 0) {
        printf("\nAll tests PASSED!\n");
    } else {
        printf("\n%d test(s) FAILED!\n", g_tests_failed);
    }
}

/*
 * Example: PartsDB Unit Tests
 */

/* Forward declarations (normally in header) */
PartDatabase *database_create(void);
void          database_destroy(PartDatabase *db);
DatabaseResult_t database_insert(PartDatabase *db, const PartRecord_t *part);
DatabaseResult_t database_find(PartDatabase *db, uint32_t id, PartRecord_t *out);
size_t        database_count(const PartDatabase *db);

TEST(database_create_returns_valid_handle)
{
    PartDatabase *db = database_create();
    ASSERT_NOT_NULL(db);
    ASSERT_EQ(0, database_count(db));
    database_destroy(db);
}

TEST(database_insert_single_part)
{
    PartDatabase *db = database_create();
    ASSERT_NOT_NULL(db);

    PartRecord_t part = {
        .part_id = 1001,
        .part_name = "Test Part",
        .quantity_in_stock = 50,
        .unit_price = 19.99
    };

    DatabaseResult_t result = database_insert(db, &part);
    ASSERT_EQ(DB_SUCCESS, result);
    ASSERT_EQ(1, database_count(db));

    database_destroy(db);
}

TEST(database_insert_rejects_duplicate)
{
    PartDatabase *db = database_create();

    PartRecord_t part = { .part_id = 1001, .part_name = "Part A" };

    ASSERT_EQ(DB_SUCCESS, database_insert(db, &part));
    ASSERT_EQ(DB_ERROR_DUPLICATE, database_insert(db, &part));
    ASSERT_EQ(1, database_count(db));

    database_destroy(db);
}

TEST(database_find_existing_part)
{
    PartDatabase *db = database_create();

    PartRecord_t original = {
        .part_id = 2001,
        .part_name = "Widget",
        .quantity_in_stock = 100,
        .unit_price = 5.50
    };

    database_insert(db, &original);

    PartRecord_t found;
    DatabaseResult_t result = database_find(db, 2001, &found);

    ASSERT_EQ(DB_SUCCESS, result);
    ASSERT_EQ(original.part_id, found.part_id);
    ASSERT_STR_EQ(original.part_name, found.part_name);
    ASSERT_EQ(original.quantity_in_stock, found.quantity_in_stock);

    database_destroy(db);
}

TEST(database_find_nonexistent_part)
{
    PartDatabase *db = database_create();

    PartRecord_t found;
    DatabaseResult_t result = database_find(db, 9999, &found);

    ASSERT_EQ(DB_ERROR_NOT_FOUND, result);

    database_destroy(db);
}

TEST(database_handles_many_inserts)
{
    PartDatabase *db = database_create();

    /* Insert 1000 parts */
    for (uint32_t i = 1; i <= 1000; i++) {
        PartRecord_t part = { .part_id = i };
        snprintf(part.part_name, sizeof(part.part_name), "Part %u", i);
        ASSERT_EQ(DB_SUCCESS, database_insert(db, &part));
    }

    ASSERT_EQ(1000, database_count(db));

    /* Verify a few random ones */
    PartRecord_t found;
    ASSERT_EQ(DB_SUCCESS, database_find(db, 1, &found));
    ASSERT_EQ(DB_SUCCESS, database_find(db, 500, &found));
    ASSERT_EQ(DB_SUCCESS, database_find(db, 1000, &found));

    database_destroy(db);
}

int
main(int argc, char *argv[])
{
    printf("Running PartsDB Unit Tests\n\n");

    RUN_TEST(database_create_returns_valid_handle);
    RUN_TEST(database_insert_single_part);
    RUN_TEST(database_insert_rejects_duplicate);
    RUN_TEST(database_find_existing_part);
    RUN_TEST(database_find_nonexistent_part);
    RUN_TEST(database_handles_many_inserts);

    print_test_summary();

    return g_tests_failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
```

## 9.4 Portability

### 9.4.1 Writing Portable Code

```c
/* portability.c - Cross-platform considerations */
#include <stdint.h>
#include <limits.h>

/*
 * Key portability issues from Practice of Programming:
 *
 * 1. Data sizes vary across platforms
 * 2. Byte order (endianness) differs
 * 3. Alignment requirements vary
 * 4. System interfaces differ
 * 5. Compiler extensions are non-portable
 */

/*
 * Issue 1: Use fixed-width types for binary data
 */

/* BAD: Size varies by platform */
struct BadFileHeader {
    int     magic;      /* 2 or 4 bytes? */
    long    file_size;  /* 4 or 8 bytes? */
    short   version;    /* 2 bytes usually, but... */
};

/* GOOD: Explicit sizes for binary formats */
typedef struct FileHeader {
    uint32_t magic;
    uint64_t file_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t flags;
    uint32_t record_count;
} FileHeader;

/* Compile-time size verification */
_Static_assert(sizeof(FileHeader) == 24, "FileHeader size mismatch");

/*
 * Issue 2: Endianness - network byte order for portability
 */

static inline uint16_t
host_to_be16(uint16_t value)
{
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
    #else
    return value;
    #endif
}

static inline uint32_t
host_to_be32(uint32_t value)
{
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((value & 0xFF) << 24) |
           ((value & 0xFF00) << 8) |
           ((value >> 8) & 0xFF00) |
           ((value >> 24) & 0xFF);
    #else
    return value;
    #endif
}

static inline uint64_t
host_to_be64(uint64_t value)
{
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((uint64_t)host_to_be32(value & 0xFFFFFFFF) << 32) |
           host_to_be32(value >> 32);
    #else
    return value;
    #endif
}

#define be16_to_host(x) host_to_be16(x)
#define be32_to_host(x) host_to_be32(x)
#define be64_to_host(x) host_to_be64(x)

/* Portable file I/O */
static int
write_header(FILE *file, const FileHeader *header)
{
    FileHeader be_header = {
        .magic         = host_to_be32(header->magic),
        .file_size     = host_to_be64(header->file_size),
        .version_major = host_to_be16(header->version_major),
        .version_minor = host_to_be16(header->version_minor),
        .flags         = host_to_be32(header->flags),
        .record_count  = host_to_be32(header->record_count)
    };

    return fwrite(&be_header, sizeof(be_header), 1, file) == 1 ? 0 : -1;
}

static int
read_header(FILE *file, FileHeader *header)
{
    FileHeader be_header;
    if (fread(&be_header, sizeof(be_header), 1, file) != 1) {
        return -1;
    }

    header->magic         = be32_to_host(be_header.magic);
    header->file_size     = be64_to_host(be_header.file_size);
    header->version_major = be16_to_host(be_header.version_major);
    header->version_minor = be16_to_host(be_header.version_minor);
    header->flags         = be32_to_host(be_header.flags);
    header->record_count  = be32_to_host(be_header.record_count);

    return 0;
}

/*
 * Issue 3: Alignment - use packed structures carefully
 */

/* May have different alignment/padding on different systems */
struct UnpackedRecord {
    uint8_t  type;       /* 1 byte + 3 padding */
    uint32_t id;         /* 4 bytes */
    uint8_t  flags;      /* 1 byte + 7 padding */
    uint64_t timestamp;  /* 8 bytes */
};  /* Total: 24 bytes with typical alignment */

/* Force specific layout (be careful - may hurt performance) */
#if defined(__GNUC__) || defined(__clang__)
#define PACKED __attribute__((packed))
#elif defined(_MSC_VER)
#define PACKED
#pragma pack(push, 1)
#endif

struct PACKED PackedRecord {
    uint8_t  type;
    uint32_t id;
    uint8_t  flags;
    uint64_t timestamp;
};  /* Total: 14 bytes, no padding */

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

/*
 * Issue 4: Platform abstraction layer
 */

/* platform.h - Abstract platform differences */
#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"
    #define LINE_ENDING "\r\n"
#else
    #define PLATFORM_UNIX 1
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
    #define LINE_ENDING "\n"
#endif

/* File system operations */
int platform_file_exists(const char *path);
int platform_mkdir(const char *path, int mode);
int platform_remove(const char *path);
char *platform_get_temp_dir(void);

/* Time operations */
uint64_t platform_get_time_ms(void);
void platform_sleep_ms(unsigned int ms);

/* Thread operations (wrapper for pthread/Windows threads) */
typedef struct PlatformThread PlatformThread;
typedef struct PlatformMutex PlatformMutex;
typedef void (*ThreadFunc)(void *arg);

PlatformThread *platform_thread_create(ThreadFunc func, void *arg);
int platform_thread_join(PlatformThread *thread);
void platform_thread_destroy(PlatformThread *thread);

PlatformMutex *platform_mutex_create(void);
void platform_mutex_lock(PlatformMutex *mutex);
void platform_mutex_unlock(PlatformMutex *mutex);
void platform_mutex_destroy(PlatformMutex *mutex);

#endif /* PLATFORM_H */

/* platform_unix.c */
#ifdef PLATFORM_UNIX

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

int
platform_file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

int
platform_mkdir(const char *path, int mode)
{
    return mkdir(path, (mode_t)mode);
}

uint64_t
platform_get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void
platform_sleep_ms(unsigned int ms)
{
    usleep(ms * 1000);
}

struct PlatformThread {
    pthread_t thread;
};

struct PlatformMutex {
    pthread_mutex_t mutex;
};

PlatformThread *
platform_thread_create(ThreadFunc func, void *arg)
{
    PlatformThread *thread = malloc(sizeof(PlatformThread));
    if (!thread) return NULL;

    if (pthread_create(&thread->thread, NULL, (void *(*)(void *))func, arg) != 0) {
        free(thread);
        return NULL;
    }
    return thread;
}

int
platform_thread_join(PlatformThread *thread)
{
    return pthread_join(thread->thread, NULL);
}

PlatformMutex *
platform_mutex_create(void)
{
    PlatformMutex *mutex = malloc(sizeof(PlatformMutex));
    if (!mutex) return NULL;

    if (pthread_mutex_init(&mutex->mutex, NULL) != 0) {
        free(mutex);
        return NULL;
    }
    return mutex;
}

void
platform_mutex_lock(PlatformMutex *mutex)
{
    pthread_mutex_lock(&mutex->mutex);
}

void
platform_mutex_unlock(PlatformMutex *mutex)
{
    pthread_mutex_unlock(&mutex->mutex);
}

#endif /* PLATFORM_UNIX */

/* platform_windows.c */
#ifdef PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int
platform_file_exists(const char *path)
{
    DWORD attrs = GetFileAttributesA(path);
    return attrs != INVALID_FILE_ATTRIBUTES;
}

int
platform_mkdir(const char *path, int mode)
{
    (void)mode;  /* Windows doesn't use Unix permissions */
    return CreateDirectoryA(path, NULL) ? 0 : -1;
}

uint64_t
platform_get_time_ms(void)
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (uint64_t)(counter.QuadPart * 1000 / freq.QuadPart);
}

void
platform_sleep_ms(unsigned int ms)
{
    Sleep(ms);
}

struct PlatformThread {
    HANDLE handle;
};

struct PlatformMutex {
    CRITICAL_SECTION cs;
};

PlatformThread *
platform_thread_create(ThreadFunc func, void *arg)
{
    PlatformThread *thread = malloc(sizeof(PlatformThread));
    if (!thread) return NULL;

    thread->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func,
                                  arg, 0, NULL);
    if (!thread->handle) {
        free(thread);
        return NULL;
    }
    return thread;
}

int
platform_thread_join(PlatformThread *thread)
{
    WaitForSingleObject(thread->handle, INFINITE);
    return 0;
}

PlatformMutex *
platform_mutex_create(void)
{
    PlatformMutex *mutex = malloc(sizeof(PlatformMutex));
    if (!mutex) return NULL;

    InitializeCriticalSection(&mutex->cs);
    return mutex;
}

void
platform_mutex_lock(PlatformMutex *mutex)
{
    EnterCriticalSection(&mutex->cs);
}

void
platform_mutex_unlock(PlatformMutex *mutex)
{
    LeaveCriticalSection(&mutex->cs);
}

#endif /* PLATFORM_WINDOWS */
```

### 9.4.2 Avoiding Implementation-Defined Behavior

```c
/* avoid_implementation_defined.c */

/*
 * Implementation-defined behaviors to avoid or handle explicitly
 */

#include <stdint.h>
#include <limits.h>

/*
 * 1. Right shift of negative numbers
 *    Could be arithmetic (sign-extended) or logical (zero-filled)
 */

/* BAD: Implementation-defined */
int bad_divide_by_8(int x) {
    return x >> 3;  /* May not preserve sign on all compilers */
}

/* GOOD: Use division for signed integers */
int good_divide_by_8(int x) {
    return x / 8;  /* Compiler will optimize to shift if appropriate */
}

/* Or use unsigned explicitly */
unsigned safe_shift(unsigned x) {
    return x >> 3;  /* Always logical shift for unsigned */
}

/*
 * 2. Integer overflow
 *    Signed overflow is undefined; unsigned wraps around
 */

/* BAD: Signed overflow is UB */
int bad_add(int a, int b) {
    return a + b;  /* UB if overflows */
}

/* GOOD: Check before operation */
int safe_add(int a, int b, int *result) {
    if ((b > 0 && a > INT_MAX - b) ||
        (b < 0 && a < INT_MIN - b)) {
        return -1;  /* Would overflow */
    }
    *result = a + b;
    return 0;
}

/* Alternative: Use unsigned arithmetic */
uint32_t safe_add_u32(uint32_t a, uint32_t b, bool *overflow) {
    uint32_t result = a + b;
    *overflow = (result < a);  /* Wrapped around */
    return result;
}

/*
 * 3. Order of evaluation
 *    Argument evaluation order is unspecified
 */

/* BAD: Undefined order of operations */
int bad_sequence(int *p) {
    return (*p++) + (*p++);  /* Which increment happens first? */
}

/* GOOD: Separate side effects */
int good_sequence(int *p) {
    int first = *p++;
    int second = *p++;
    return first + second;
}

/*
 * 4. Struct padding and alignment
 */

/* May have different sizes on different compilers/platforms */
struct VariableSize {
    char  c;
    int   i;
    char  c2;
};

/* Use this instead for portable binary formats */
struct PortableFormat {
    uint8_t  field1;
    uint8_t  padding1[3];
    uint32_t field2;
    uint8_t  field3;
    uint8_t  padding2[3];
};

_Static_assert(sizeof(struct PortableFormat) == 12, "Unexpected struct size");

/*
 * 5. Bit-field ordering
 *    Order of bits within a byte is implementation-defined
 */

/* BAD: Non-portable bit-field layout */
struct BadFlags {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int reserved : 29;
};

/* GOOD: Use explicit bit masks */
typedef uint32_t Flags;

#define FLAG_ONE   (1U << 0)
#define FLAG_TWO   (1U << 1)
#define FLAG_THREE (1U << 2)

static inline bool flags_has(Flags f, uint32_t mask) {
    return (f & mask) == mask;
}

static inline Flags flags_set(Flags f, uint32_t mask) {
    return f | mask;
}

static inline Flags flags_clear(Flags f, uint32_t mask) {
    return f & ~mask;
}
```

## 9.5 Performance Analysis

### 9.5.1 Profiling and Optimization

```c
/* profiling.c - Performance measurement */
#include <stdio.h>
#include <time.h>

/*
 * "Premature optimization is the root of all evil"
 * - Donald Knuth (quoted in Practice of Programming)
 *
 * But when you DO need to optimize:
 * 1. Measure first
 * 2. Find the bottleneck
 * 3. Optimize that specific code
 * 4. Measure again to verify improvement
 */

/*
 * Simple timing infrastructure
 */

typedef struct Timer {
    struct timespec start;
    struct timespec end;
    const char *name;
} Timer;

static inline void
timer_start(Timer *timer, const char *name)
{
    timer->name = name;
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

static inline double
timer_stop(Timer *timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->end);

    double start_sec = timer->start.tv_sec + timer->start.tv_nsec / 1e9;
    double end_sec = timer->end.tv_sec + timer->end.tv_nsec / 1e9;

    return end_sec - start_sec;
}

static inline void
timer_report(Timer *timer)
{
    double elapsed = timer_stop(timer);
    printf("%s: %.6f seconds\n", timer->name, elapsed);
}

/*
 * Macro for easy timing
 */
#define TIME_BLOCK(name, code) \
    do { \
        Timer _timer; \
        timer_start(&_timer, name); \
        { code; } \
        timer_report(&_timer); \
    } while (0)

/*
 * Example: Comparing search algorithms
 */

/* Linear search - O(n) */
static int
linear_search(const int *arr, size_t n, int target)
{
    for (size_t i = 0; i < n; i++) {
        if (arr[i] == target) {
            return (int)i;
        }
    }
    return -1;
}

/* Binary search - O(log n), requires sorted array */
static int
binary_search(const int *arr, size_t n, int target)
{
    size_t low = 0;
    size_t high = n;

    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (arr[mid] < target) {
            low = mid + 1;
        } else if (arr[mid] > target) {
            high = mid;
        } else {
            return (int)mid;
        }
    }
    return -1;
}

void
benchmark_search(void)
{
    /* Generate test data */
    #define TEST_SIZE 1000000
    int *data = malloc(TEST_SIZE * sizeof(int));
    for (int i = 0; i < TEST_SIZE; i++) {
        data[i] = i * 2;  /* Sorted, even numbers */
    }

    int target = TEST_SIZE;  /* Middle of range */

    printf("Searching for %d in %d elements:\n", target, TEST_SIZE);

    TIME_BLOCK("Linear search", {
        for (int i = 0; i < 100; i++) {
            linear_search(data, TEST_SIZE, target);
        }
    });

    TIME_BLOCK("Binary search", {
        for (int i = 0; i < 100; i++) {
            binary_search(data, TEST_SIZE, target);
        }
    });

    free(data);
}

/*
 * Profiling-friendly code organization
 */

/*
 * Group related operations together for cache efficiency.
 * Make hot paths obvious and easy to profile.
 */

typedef struct DatabaseStats {
    uint64_t queries_executed;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t bytes_read;
    uint64_t bytes_written;
    double   total_query_time;
    double   max_query_time;
} DatabaseStats;

static DatabaseStats g_stats = {0};

/* Hot path - inline for performance */
static inline DatabaseResult_t
cache_lookup(DatabaseCache *cache, uint32_t key, CacheEntry **entry)
{
    uint32_t bucket = key % cache->bucket_count;
    CacheEntry *e = cache->buckets[bucket];

    while (e) {
        if (e->key == key) {
            *entry = e;
            g_stats.cache_hits++;
            return DB_SUCCESS;
        }
        e = e->next;
    }

    g_stats.cache_misses++;
    return DB_ERROR_NOT_FOUND;
}

/* Cold path - separate function to keep hot path tight */
static DatabaseResult_t
disk_lookup(Database *db, uint32_t key, PartRecord_t *record)
{
    Timer timer;
    timer_start(&timer, "disk_lookup");

    /* ... disk I/O ... */

    double elapsed = timer_stop(&timer);
    g_stats.total_query_time += elapsed;
    if (elapsed > g_stats.max_query_time) {
        g_stats.max_query_time = elapsed;
    }

    return DB_SUCCESS;
}

void
print_database_stats(void)
{
    double hit_rate = (double)g_stats.cache_hits /
                      (g_stats.cache_hits + g_stats.cache_misses) * 100;

    printf("=== Database Statistics ===\n");
    printf("Queries executed: %llu\n", g_stats.queries_executed);
    printf("Cache hit rate:   %.1f%%\n", hit_rate);
    printf("Total query time: %.3f seconds\n", g_stats.total_query_time);
    printf("Max query time:   %.6f seconds\n", g_stats.max_query_time);
    printf("Data transferred: %llu bytes read, %llu bytes written\n",
           g_stats.bytes_read, g_stats.bytes_written);
}
```

## 9.6 Configuration File Parsing

A practical example combining style, testing, and portability:

```c
/* config_parser.c - INI-style configuration file parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define CONFIG_MAX_LINE     1024
#define CONFIG_MAX_KEY      64
#define CONFIG_MAX_VALUE    256
#define CONFIG_MAX_SECTION  64
#define CONFIG_MAX_ENTRIES  256

typedef struct ConfigEntry {
    char section[CONFIG_MAX_SECTION];
    char key[CONFIG_MAX_KEY];
    char value[CONFIG_MAX_VALUE];
} ConfigEntry;

typedef struct Config {
    ConfigEntry entries[CONFIG_MAX_ENTRIES];
    size_t      count;
    bool        modified;
} Config;

/*
 * Helper functions - clear naming, single responsibility
 */

/* Trim whitespace from both ends of a string in-place */
static char *
trim_whitespace(char *str)
{
    /* Trim leading */
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    /* Trim trailing */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

/* Remove inline comments (starting with ; or #) */
static void
strip_comment(char *line)
{
    bool in_quotes = false;

    for (char *p = line; *p; p++) {
        if (*p == '"') {
            in_quotes = !in_quotes;
        } else if (!in_quotes && (*p == ';' || *p == '#')) {
            *p = '\0';
            return;
        }
    }
}

/* Check if line is a section header [section] */
static bool
parse_section(const char *line, char *section, size_t section_size)
{
    if (line[0] != '[') {
        return false;
    }

    const char *end = strchr(line, ']');
    if (!end) {
        return false;
    }

    size_t len = (size_t)(end - line - 1);
    if (len >= section_size) {
        return false;
    }

    memcpy(section, line + 1, len);
    section[len] = '\0';

    return true;
}

/* Parse key=value line */
static bool
parse_key_value(const char *line, char *key, size_t key_size,
                char *value, size_t value_size)
{
    const char *equals = strchr(line, '=');
    if (!equals) {
        return false;
    }

    /* Extract key */
    size_t key_len = (size_t)(equals - line);
    if (key_len >= key_size) {
        return false;
    }
    memcpy(key, line, key_len);
    key[key_len] = '\0';

    /* Trim key */
    char *trimmed_key = trim_whitespace(key);
    if (trimmed_key != key) {
        memmove(key, trimmed_key, strlen(trimmed_key) + 1);
    }

    /* Extract value */
    const char *val_start = equals + 1;
    while (isspace((unsigned char)*val_start)) {
        val_start++;
    }

    size_t val_len = strlen(val_start);
    if (val_len >= value_size) {
        val_len = value_size - 1;
    }
    memcpy(value, val_start, val_len);
    value[val_len] = '\0';

    /* Trim value */
    char *trimmed_val = trim_whitespace(value);
    if (trimmed_val != value) {
        memmove(value, trimmed_val, strlen(trimmed_val) + 1);
    }

    /* Remove quotes if present */
    val_len = strlen(value);
    if (val_len >= 2 && value[0] == '"' && value[val_len-1] == '"') {
        memmove(value, value + 1, val_len - 2);
        value[val_len - 2] = '\0';
    }

    return true;
}

/*
 * Public API
 */

Config *
config_create(void)
{
    Config *config = calloc(1, sizeof(Config));
    return config;
}

void
config_destroy(Config *config)
{
    free(config);
}

int
config_load(Config *config, const char *filename)
{
    if (!config || !filename) {
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    char line[CONFIG_MAX_LINE];
    char current_section[CONFIG_MAX_SECTION] = "";
    int line_number = 0;

    config->count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;

        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len > 0 && line[len-1] == '\r') {
            line[len-1] = '\0';
        }

        strip_comment(line);
        char *trimmed = trim_whitespace(line);

        /* Skip empty lines */
        if (*trimmed == '\0') {
            continue;
        }

        /* Check for section header */
        char section[CONFIG_MAX_SECTION];
        if (parse_section(trimmed, section, sizeof(section))) {
            strncpy(current_section, section, sizeof(current_section) - 1);
            current_section[sizeof(current_section) - 1] = '\0';
            continue;
        }

        /* Parse key=value */
        char key[CONFIG_MAX_KEY];
        char value[CONFIG_MAX_VALUE];

        if (!parse_key_value(trimmed, key, sizeof(key), value, sizeof(value))) {
            fprintf(stderr, "Warning: Invalid syntax at line %d: %s\n",
                    line_number, trimmed);
            continue;
        }

        /* Add entry */
        if (config->count >= CONFIG_MAX_ENTRIES) {
            fprintf(stderr, "Warning: Maximum entries reached, ignoring rest\n");
            break;
        }

        ConfigEntry *entry = &config->entries[config->count];
        strncpy(entry->section, current_section, sizeof(entry->section) - 1);
        strncpy(entry->key, key, sizeof(entry->key) - 1);
        strncpy(entry->value, value, sizeof(entry->value) - 1);
        config->count++;
    }

    fclose(file);
    config->modified = false;

    return 0;
}

int
config_save(const Config *config, const char *filename)
{
    if (!config || !filename) {
        return -1;
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1;
    }

    const char *current_section = NULL;

    for (size_t i = 0; i < config->count; i++) {
        const ConfigEntry *entry = &config->entries[i];

        /* Write section header if changed */
        if (!current_section || strcmp(current_section, entry->section) != 0) {
            if (entry->section[0] != '\0') {
                if (current_section) {
                    fprintf(file, "\n");  /* Blank line between sections */
                }
                fprintf(file, "[%s]\n", entry->section);
            }
            current_section = entry->section;
        }

        /* Write key=value */
        fprintf(file, "%s = %s\n", entry->key, entry->value);
    }

    fclose(file);
    return 0;
}

const char *
config_get(const Config *config, const char *section, const char *key)
{
    if (!config || !key) {
        return NULL;
    }

    const char *section_match = section ? section : "";

    for (size_t i = 0; i < config->count; i++) {
        const ConfigEntry *entry = &config->entries[i];

        if (strcmp(entry->section, section_match) == 0 &&
            strcmp(entry->key, key) == 0) {
            return entry->value;
        }
    }

    return NULL;
}

int
config_get_int(const Config *config, const char *section,
               const char *key, int default_value)
{
    const char *value = config_get(config, section, key);
    if (!value) {
        return default_value;
    }

    char *end;
    long result = strtol(value, &end, 0);

    if (*end != '\0') {
        return default_value;  /* Invalid integer */
    }

    return (int)result;
}

bool
config_get_bool(const Config *config, const char *section,
                const char *key, bool default_value)
{
    const char *value = config_get(config, section, key);
    if (!value) {
        return default_value;
    }

    /* Accept various boolean representations */
    if (strcasecmp(value, "true") == 0 ||
        strcasecmp(value, "yes") == 0 ||
        strcasecmp(value, "on") == 0 ||
        strcmp(value, "1") == 0) {
        return true;
    }

    if (strcasecmp(value, "false") == 0 ||
        strcasecmp(value, "no") == 0 ||
        strcasecmp(value, "off") == 0 ||
        strcmp(value, "0") == 0) {
        return false;
    }

    return default_value;
}

int
config_set(Config *config, const char *section,
           const char *key, const char *value)
{
    if (!config || !key || !value) {
        return -1;
    }

    const char *section_match = section ? section : "";

    /* Look for existing entry */
    for (size_t i = 0; i < config->count; i++) {
        ConfigEntry *entry = &config->entries[i];

        if (strcmp(entry->section, section_match) == 0 &&
            strcmp(entry->key, key) == 0) {
            strncpy(entry->value, value, sizeof(entry->value) - 1);
            entry->value[sizeof(entry->value) - 1] = '\0';
            config->modified = true;
            return 0;
        }
    }

    /* Add new entry */
    if (config->count >= CONFIG_MAX_ENTRIES) {
        return -1;  /* Full */
    }

    ConfigEntry *entry = &config->entries[config->count];
    strncpy(entry->section, section_match, sizeof(entry->section) - 1);
    strncpy(entry->key, key, sizeof(entry->key) - 1);
    strncpy(entry->value, value, sizeof(entry->value) - 1);
    config->count++;
    config->modified = true;

    return 0;
}

/*
 * PartsDB Configuration Example
 */

typedef struct PartsDBConfig {
    char     database_path[256];
    int      max_connections;
    int      cache_size_mb;
    bool     enable_logging;
    char     log_file[256];
    int      log_level;
    bool     auto_backup;
    int      backup_interval_hours;
} PartsDBConfig;

int
partsdb_config_load(PartsDBConfig *pconfig, const char *filename)
{
    Config *config = config_create();
    if (!config) {
        return -1;
    }

    if (config_load(config, filename) != 0) {
        config_destroy(config);
        return -1;
    }

    /* Database settings */
    const char *db_path = config_get(config, "database", "path");
    if (db_path) {
        strncpy(pconfig->database_path, db_path,
                sizeof(pconfig->database_path) - 1);
    } else {
        strcpy(pconfig->database_path, "partsdb.dat");  /* Default */
    }

    pconfig->max_connections = config_get_int(config, "database",
                                               "max_connections", 10);
    pconfig->cache_size_mb = config_get_int(config, "database",
                                            "cache_size_mb", 64);

    /* Logging settings */
    pconfig->enable_logging = config_get_bool(config, "logging",
                                              "enabled", true);

    const char *log_path = config_get(config, "logging", "file");
    if (log_path) {
        strncpy(pconfig->log_file, log_path, sizeof(pconfig->log_file) - 1);
    } else {
        strcpy(pconfig->log_file, "partsdb.log");
    }

    pconfig->log_level = config_get_int(config, "logging", "level", 2);

    /* Backup settings */
    pconfig->auto_backup = config_get_bool(config, "backup",
                                           "auto_enabled", true);
    pconfig->backup_interval_hours = config_get_int(config, "backup",
                                                    "interval_hours", 24);

    config_destroy(config);
    return 0;
}
```

## 9.7 Review Questions

1. **From "The Practice of Programming"**: "Good programmers write code that humans can
   understand." What specific techniques does this chapter recommend for improving code
   readability? How do these techniques apply to safety-critical systems?

2. **Naming philosophy**: Kernighan and Pike discuss appropriate name lengths for different
   scopes. What are their guidelines, and why do scope and name length correlate? Give
   examples from a database application.

3. **Debugging as science**: How does "The Practice of Programming" describe the systematic
   approach to debugging? What are the key steps, and how do assertions help document and
   verify hypotheses during debugging?

4. **Portability layers**: The chapter shows Windows and Unix platform abstraction. What
   other operating system differences must portable C code handle? How would you extend
   the platform layer for a real-time operating system?

5. **Testing philosophy**: Kernighan and Pike emphasize testing throughout development.
   What is the difference between testing for correctness and testing for robustness?
   How would you design tests for the configuration parser shown above?

6. **Implementation-defined behavior**: List five implementation-defined behaviors in C
   that could cause portability problems. For each, explain both the potential problem
   and the portable solution.

7. **Profiling strategy**: The chapter quotes Knuth on premature optimization. What
   measurement and profiling steps should precede optimization? How do you identify
   the "hot path" in a database query engine?

8. **Comment quality**: What distinguishes good comments from bad comments according
   to Kernighan and Pike? Write appropriate comments for a binary search function that
   explain the invariants but don't just paraphrase the code.

9. **Error handling philosophy**: How does "The Practice of Programming" approach error
   handling differently from languages with exceptions? What are the trade-offs of
   error codes versus other approaches in C?

10. **Defensive programming balance**: When is defensive checking appropriate versus
    unnecessary overhead? How do debug-only assertions help find the right balance?

---

*Next: Part 10 - Defensive Programming & Assertions (Writing Solid Code)*
