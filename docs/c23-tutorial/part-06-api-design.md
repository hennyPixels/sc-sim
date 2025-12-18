# Part 6: API Design & Reusable Libraries

*Source: C Interfaces and Implementations (Hanson)*

> **"An interface specifies what a module does; an implementation specifies how it does it."** — David R. Hanson

Well-designed APIs are the foundation of maintainable C codebases. This part covers principles for creating robust, reusable interfaces, drawing from Hanson's influential work on C library design and the patterns used in PartsDB.

---

## 6.1 Principles of Interface Design

### Separation of Interface and Implementation

```c
/*
 * partsdb.h - PUBLIC INTERFACE
 * Users see only this file
 */
#ifndef PARTSDB_H
#define PARTSDB_H

/* Forward declaration - users don't know internal structure */
typedef struct partsdb partsdb_t;

/* Error type - users can check and report */
typedef enum {
    PARTSDB_OK = 0,
    PARTSDB_ERROR_OPEN = -1,
    /* ... */
} partsdb_error_t;

/* Public API - the only way to interact with PartsDB */
partsdb_error_t partsdb_open(const char *path, partsdb_t **db);
void partsdb_close(partsdb_t *db);
/* ... */

#endif /* PARTSDB_H */
```

```c
/*
 * database.c - PRIVATE IMPLEMENTATION
 * Hidden from users
 */
#include "partsdb.h"
#include <sqlite3.h>

/* The actual structure definition */
struct partsdb {
    sqlite3 *db;      /* Implementation detail */
    char *path;       /* Internal state */
    int ref_count;    /* Could be added without changing API */
};

/* Implementation of public functions */
partsdb_error_t partsdb_open(const char *path, partsdb_t **db) {
    /* ... */
}
```

### Design Principles

**1. Minimize the Interface**
```c
/* BAD: Exposing too much */
struct part {
    int64_t id;
    char *name;              /* Users could free this! */
    struct part *next;       /* Internal linked list exposed */
    sqlite3_stmt *stmt;      /* Implementation leak */
};

/* GOOD: Expose only what's necessary */
typedef struct {
    int64_t id;
    char name[128];          /* Fixed buffer - no ownership issues */
    char part_number[64];
    /* No internal implementation details */
} part_t;
```

**2. Make Interfaces Hard to Misuse**
```c
/* BAD: Easy to swap arguments */
void draw_rectangle(int x1, int y1, int x2, int y2, int color);
draw_rectangle(100, 200, 50, 150, RED);  /* Oops, wrong order */

/* GOOD: Named fields prevent mistakes */
typedef struct {
    int x, y;
} point_t;

typedef struct {
    point_t top_left;
    point_t bottom_right;
} rect_t;

void draw_rectangle(rect_t rect, int color);
draw_rectangle((rect_t){{100, 200}, {150, 250}}, RED);
```

**3. Resource Symmetry**
```c
/* Every resource has matching acquire/release */
partsdb_t *db;
partsdb_open(path, &db);   /* Acquire */
/* ... use db ... */
partsdb_close(db);          /* Release - same module manages lifecycle */

/* For returned arrays, provide matching free function */
part_t *parts;
size_t count;
partsdb_get_all(db, &parts, &count, 100, 0);  /* Allocates */
/* ... use parts ... */
partsdb_free_parts(parts);  /* Free with matching function */
```

---

## 6.2 Opaque Types and Information Hiding

### The Opaque Pointer Pattern

```c
/* In header - opaque type */
typedef struct partsdb partsdb_t;

/* Users cannot access members directly */
partsdb_t *db;
db->db = nullptr;  /* ERROR: incomplete type */

/* Must use accessor functions */
const char *partsdb_get_path(const partsdb_t *db);
```

### When to Use Opaque Types

```c
/*
 * Use opaque types when:
 * 1. Implementation may change
 * 2. Resources need controlled management
 * 3. Invariants must be maintained
 * 4. Multiple implementations possible
 */

/* Opaque: Database handle - complex internal state */
typedef struct partsdb partsdb_t;

/* NOT opaque: Simple data record - no hidden state */
typedef struct {
    int64_t id;
    char name[128];
    int32_t quantity;
} part_t;  /* Full definition in header is fine */
```

### Accessor Functions

```c
/* Provide controlled access to internal state */

/* Getter - returns copy or const reference */
const char *partsdb_get_path(const partsdb_t *db) {
    if (db == nullptr) return nullptr;
    return db->path;  /* Safe: const prevents modification */
}

/* Query state without exposing internals */
bool partsdb_is_open(const partsdb_t *db) {
    return db != nullptr && db->db != nullptr;
}

int64_t partsdb_get_last_insert_id(const partsdb_t *db) {
    if (db == nullptr || db->db == nullptr) return -1;
    return sqlite3_last_insert_rowid(db->db);
}

/* Statistics without exposing data structure */
typedef struct {
    size_t total_parts;
    size_t active_parts;
    size_t low_stock_count;
} partsdb_stats_t;

partsdb_error_t partsdb_get_stats(partsdb_t *db, partsdb_stats_t *stats);
```

---

## 6.3 Resource Management Patterns

### Constructor/Destructor Pattern

```c
/* Constructor: allocate and initialize */
partsdb_error_t partsdb_open(const char *path, partsdb_t **db) {
    if (path == nullptr || db == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    *db = nullptr;  /* Clear output on failure */

    partsdb_t *ctx = calloc(1, sizeof(partsdb_t));
    if (ctx == nullptr) {
        return PARTSDB_ERROR_MEMORY;
    }

    ctx->path = strdup(path);
    if (ctx->path == nullptr) {
        free(ctx);
        return PARTSDB_ERROR_MEMORY;
    }

    int rc = sqlite3_open_v2(path, &ctx->db,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                              nullptr);
    if (rc != SQLITE_OK) {
        free(ctx->path);
        free(ctx);
        return PARTSDB_ERROR_OPEN;
    }

    *db = ctx;
    return PARTSDB_OK;
}

/* Destructor: cleanup in reverse order of construction */
void partsdb_close(partsdb_t *db) {
    if (db == nullptr) return;  /* Safe to call with nullptr */

    if (db->db != nullptr) {
        sqlite3_close(db->db);
        db->db = nullptr;
    }

    free(db->path);
    db->path = nullptr;

    free(db);
}
```

### Output Parameter Pattern

```c
/* Return status code, output through pointer parameter */
partsdb_error_t partsdb_get_by_id(
    partsdb_t *db,       /* Input: database handle */
    int64_t id,          /* Input: ID to lookup */
    part_t *part         /* Output: filled on success */
) {
    if (db == nullptr || part == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Clear output struct on entry */
    memset(part, 0, sizeof(part_t));

    /* ... perform query ... */

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_NOT_FOUND;
    }

    /* Fill output struct */
    part->id = sqlite3_column_int64(stmt, 0);
    safe_strcpy(part->part_number, sqlite_text(stmt, 1),
                sizeof(part->part_number));
    /* ... */

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}

/* Usage: */
part_t part;
partsdb_error_t err = partsdb_get_by_id(db, 123, &part);
if (err == PARTSDB_OK) {
    printf("Found: %s\n", part.name);
}
```

### Array Output Pattern

```c
/* Callee allocates, caller frees */
partsdb_error_t partsdb_get_all(
    partsdb_t *db,
    part_t **parts,      /* Output: array of parts */
    size_t *count,       /* Output: number of parts */
    size_t limit,        /* Input: max results */
    size_t offset        /* Input: skip first N */
) {
    if (db == nullptr || parts == nullptr || count == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    *parts = nullptr;  /* Clear output */
    *count = 0;

    /* Count results first */
    size_t n = count_results(db, limit, offset);
    if (n == 0) {
        return PARTSDB_OK;  /* Empty result is not an error */
    }

    /* Allocate array */
    part_t *result = calloc(n, sizeof(part_t));
    if (result == nullptr) {
        return PARTSDB_ERROR_MEMORY;
    }

    /* Fill array */
    if (!fill_results(db, result, n, limit, offset)) {
        free(result);
        return PARTSDB_ERROR_QUERY;
    }

    /* Transfer ownership to caller */
    *parts = result;
    *count = n;
    return PARTSDB_OK;
}

/* Matching free function */
void partsdb_free_parts(part_t *parts) {
    free(parts);  /* Simple for flat array */
}
```

---

## 6.4 Error Handling Strategies

### Error Code Enumeration

```c
/* Comprehensive error type */
typedef enum {
    /* Success */
    PARTSDB_OK = 0,

    /* Resource errors (negative) */
    PARTSDB_ERROR_OPEN = -1,
    PARTSDB_ERROR_MEMORY = -2,
    PARTSDB_ERROR_IO = -3,

    /* Query errors */
    PARTSDB_ERROR_QUERY = -10,
    PARTSDB_ERROR_NOT_FOUND = -11,
    PARTSDB_ERROR_DUPLICATE = -12,

    /* Validation errors */
    PARTSDB_ERROR_INVALID = -20,
    PARTSDB_ERROR_OVERFLOW = -21,

    /* Internal errors */
    PARTSDB_ERROR_INTERNAL = -100,
} partsdb_error_t;

/* Error string lookup */
const char *partsdb_error_string(partsdb_error_t error) {
    switch (error) {
    case PARTSDB_OK:             return "Success";
    case PARTSDB_ERROR_OPEN:     return "Failed to open database";
    case PARTSDB_ERROR_MEMORY:   return "Memory allocation failed";
    case PARTSDB_ERROR_IO:       return "I/O error";
    case PARTSDB_ERROR_QUERY:    return "Query execution failed";
    case PARTSDB_ERROR_NOT_FOUND: return "Record not found";
    case PARTSDB_ERROR_DUPLICATE: return "Duplicate key";
    case PARTSDB_ERROR_INVALID:  return "Invalid parameter";
    case PARTSDB_ERROR_OVERFLOW: return "Integer overflow";
    case PARTSDB_ERROR_INTERNAL: return "Internal error";
    default:                     return "Unknown error";
    }
}
```

### Error Propagation Pattern

```c
/* Consistent error checking style */
partsdb_error_t complex_operation(partsdb_t *db, const part_t *part) {
    partsdb_error_t err;

    err = validate_part(part);
    if (err != PARTSDB_OK) return err;  /* Early return */

    err = begin_transaction(db);
    if (err != PARTSDB_OK) return err;

    err = insert_part(db, part);
    if (err != PARTSDB_OK) {
        rollback_transaction(db);  /* Cleanup on error */
        return err;
    }

    err = update_indexes(db, part);
    if (err != PARTSDB_OK) {
        rollback_transaction(db);
        return err;
    }

    return commit_transaction(db);
}

/* Or with goto for cleanup */
partsdb_error_t complex_operation_goto(partsdb_t *db, const part_t *part) {
    partsdb_error_t err = PARTSDB_ERROR_INTERNAL;
    bool in_transaction = false;

    err = validate_part(part);
    if (err != PARTSDB_OK) goto cleanup;

    err = begin_transaction(db);
    if (err != PARTSDB_OK) goto cleanup;
    in_transaction = true;

    err = insert_part(db, part);
    if (err != PARTSDB_OK) goto cleanup;

    err = update_indexes(db, part);
    if (err != PARTSDB_OK) goto cleanup;

    err = commit_transaction(db);
    in_transaction = false;  /* Commit succeeded */

cleanup:
    if (in_transaction) {
        rollback_transaction(db);
    }
    return err;
}
```

---

## 6.5 Memory Allocator Design

### Custom Allocator Interface

```c
/* Allocator function type */
typedef void *(*partsdb_alloc_fn)(size_t size, void *context);
typedef void *(*partsdb_realloc_fn)(void *ptr, size_t size, void *context);
typedef void (*partsdb_free_fn)(void *ptr, void *context);

typedef struct {
    partsdb_alloc_fn alloc;
    partsdb_realloc_fn realloc;
    partsdb_free_fn free;
    void *context;  /* User data passed to callbacks */
} partsdb_allocator_t;

/* Default allocator using standard functions */
static void *default_alloc(size_t size, void *ctx) {
    (void)ctx;
    return malloc(size);
}

static void *default_realloc(void *ptr, size_t size, void *ctx) {
    (void)ctx;
    return realloc(ptr, size);
}

static void default_free(void *ptr, void *ctx) {
    (void)ctx;
    free(ptr);
}

static const partsdb_allocator_t default_allocator = {
    .alloc = default_alloc,
    .realloc = default_realloc,
    .free = default_free,
    .context = nullptr
};

/* Extended open function with custom allocator */
partsdb_error_t partsdb_open_ex(
    const char *path,
    partsdb_t **db,
    const partsdb_allocator_t *allocator
);
```

### Arena Allocator Pattern

```c
/* Arena for batch allocations */
typedef struct {
    char *base;
    size_t capacity;
    size_t used;
} partsdb_arena_t;

partsdb_arena_t *arena_create(size_t capacity) {
    partsdb_arena_t *arena = malloc(sizeof(partsdb_arena_t));
    if (arena == nullptr) return nullptr;

    arena->base = malloc(capacity);
    if (arena->base == nullptr) {
        free(arena);
        return nullptr;
    }

    arena->capacity = capacity;
    arena->used = 0;
    return arena;
}

void *arena_alloc(partsdb_arena_t *arena, size_t size) {
    /* Align to pointer size */
    size_t aligned_size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

    if (arena->used + aligned_size > arena->capacity) {
        return nullptr;  /* Arena exhausted */
    }

    void *ptr = arena->base + arena->used;
    arena->used += aligned_size;
    return ptr;
}

void arena_reset(partsdb_arena_t *arena) {
    arena->used = 0;  /* "Free" everything at once */
}

void arena_destroy(partsdb_arena_t *arena) {
    if (arena != nullptr) {
        free(arena->base);
        free(arena);
    }
}
```

---

## 6.6 Container Abstractions

### Dynamic Array

```c
typedef struct {
    part_t *data;
    size_t size;      /* Current number of elements */
    size_t capacity;  /* Allocated capacity */
} part_array_t;

part_array_t *part_array_create(size_t initial_capacity) {
    part_array_t *arr = malloc(sizeof(part_array_t));
    if (arr == nullptr) return nullptr;

    arr->data = malloc(initial_capacity * sizeof(part_t));
    if (arr->data == nullptr) {
        free(arr);
        return nullptr;
    }

    arr->size = 0;
    arr->capacity = initial_capacity;
    return arr;
}

bool part_array_push(part_array_t *arr, const part_t *part) {
    if (arr->size >= arr->capacity) {
        /* Grow by 1.5x */
        size_t new_capacity = arr->capacity + arr->capacity / 2;
        if (new_capacity < arr->capacity) return false;  /* Overflow */

        part_t *new_data = realloc(arr->data, new_capacity * sizeof(part_t));
        if (new_data == nullptr) return false;

        arr->data = new_data;
        arr->capacity = new_capacity;
    }

    arr->data[arr->size++] = *part;
    return true;
}

part_t *part_array_get(part_array_t *arr, size_t index) {
    if (index >= arr->size) return nullptr;
    return &arr->data[index];
}

void part_array_destroy(part_array_t *arr) {
    if (arr != nullptr) {
        free(arr->data);
        free(arr);
    }
}
```

### Hash Map

```c
typedef struct hash_entry {
    char *key;
    part_t value;
    struct hash_entry *next;
} hash_entry_t;

typedef struct {
    hash_entry_t **buckets;
    size_t num_buckets;
    size_t size;
} part_map_t;

part_map_t *part_map_create(size_t num_buckets) {
    part_map_t *map = malloc(sizeof(part_map_t));
    if (map == nullptr) return nullptr;

    map->buckets = calloc(num_buckets, sizeof(hash_entry_t*));
    if (map->buckets == nullptr) {
        free(map);
        return nullptr;
    }

    map->num_buckets = num_buckets;
    map->size = 0;
    return map;
}

static size_t hash_string(const char *key, size_t num_buckets) {
    size_t hash = 5381;
    while (*key) {
        hash = ((hash << 5) + hash) + (unsigned char)*key++;
    }
    return hash % num_buckets;
}

bool part_map_put(part_map_t *map, const char *key, const part_t *value) {
    size_t bucket = hash_string(key, map->num_buckets);

    /* Check for existing key */
    for (hash_entry_t *e = map->buckets[bucket]; e != nullptr; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = *value;  /* Update existing */
            return true;
        }
    }

    /* Insert new entry */
    hash_entry_t *entry = malloc(sizeof(hash_entry_t));
    if (entry == nullptr) return false;

    entry->key = strdup(key);
    if (entry->key == nullptr) {
        free(entry);
        return false;
    }

    entry->value = *value;
    entry->next = map->buckets[bucket];
    map->buckets[bucket] = entry;
    map->size++;
    return true;
}

part_t *part_map_get(part_map_t *map, const char *key) {
    size_t bucket = hash_string(key, map->num_buckets);

    for (hash_entry_t *e = map->buckets[bucket]; e != nullptr; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            return &e->value;
        }
    }
    return nullptr;
}
```

---

## 6.7 Exception-Like Mechanisms in C

### setjmp/longjmp for Error Recovery

```c
#include <setjmp.h>

/* Thread-local jump buffer */
static _Thread_local jmp_buf error_jump;
static _Thread_local partsdb_error_t error_code;

/* "Throw" an error */
static void throw_error(partsdb_error_t err) {
    error_code = err;
    longjmp(error_jump, 1);
}

/* Use with caution - cleanup is tricky */
partsdb_error_t partsdb_batch_operation(partsdb_t *db) {
    partsdb_error_t result = PARTSDB_ERROR_INTERNAL;

    if (setjmp(error_jump) != 0) {
        /* Jumped here from throw_error */
        return error_code;
    }

    /* Code that might "throw" */
    perform_risky_operation(db);

    result = PARTSDB_OK;
    return result;
}

static void perform_risky_operation(partsdb_t *db) {
    if (db == nullptr) {
        throw_error(PARTSDB_ERROR_INVALID);  /* "Throws" */
    }
    /* ... */
}
```

### Cleanup Handler Pattern (GNU C Extension)

```c
#ifdef __GNUC__
/* Automatic cleanup on scope exit */
#define AUTO_CLEANUP(func) __attribute__((cleanup(func)))

static void cleanup_file(FILE **fp) {
    if (*fp != nullptr) {
        fclose(*fp);
    }
}

static void cleanup_ptr(void **ptr) {
    free(*ptr);
}

partsdb_error_t process_file(const char *path) {
    AUTO_CLEANUP(cleanup_file) FILE *f = fopen(path, "r");
    AUTO_CLEANUP(cleanup_ptr) char *buffer = malloc(1024);

    if (f == nullptr || buffer == nullptr) {
        return PARTSDB_ERROR_IO;
        /* f and buffer automatically cleaned up on return */
    }

    /* ... process file ... */

    return PARTSDB_OK;
    /* Cleanup still happens on normal return */
}
#endif
```

---

## 6.8 Versioning and ABI Stability

### Version Macros

```c
/* Version information */
#define PARTSDB_VERSION_MAJOR 1
#define PARTSDB_VERSION_MINOR 2
#define PARTSDB_VERSION_PATCH 3

#define PARTSDB_VERSION \
    ((PARTSDB_VERSION_MAJOR << 16) | \
     (PARTSDB_VERSION_MINOR << 8) | \
     PARTSDB_VERSION_PATCH)

#define PARTSDB_VERSION_STRING "1.2.3"

/* Runtime version check */
uint32_t partsdb_version(void) {
    return PARTSDB_VERSION;
}

const char *partsdb_version_string(void) {
    return PARTSDB_VERSION_STRING;
}

/* Compile-time version check */
#if PARTSDB_VERSION < 0x010200  /* < 1.2.0 */
#error "PartsDB 1.2.0 or later required"
#endif
```

### ABI Compatibility

```c
/* Reserve space for future expansion */
typedef struct {
    int64_t id;
    char name[128];
    char part_number[64];
    /* ... current fields ... */

    char _reserved[256];  /* Future fields without ABI break */
} part_t;

/* Or use size field */
typedef struct {
    uint32_t struct_size;  /* sizeof(partsdb_config_t) */
    uint32_t flags;
    const char *custom_path;
    /* New fields added here */
} partsdb_config_t;

partsdb_error_t partsdb_open_with_config(
    const char *path,
    partsdb_t **db,
    const partsdb_config_t *config
) {
    /* Check struct size for compatibility */
    if (config != nullptr && config->struct_size < sizeof(partsdb_config_t)) {
        /* Old client - use defaults for new fields */
    }
    /* ... */
}
```

---

## 6.9 PartsDB Case Study: API Redesign

### API Audit

```c
/*
 * PartsDB API Quality Checklist
 *
 * Naming:
 * [✓] Consistent prefix (partsdb_)
 * [✓] Verb-noun convention (partsdb_get_by_id, partsdb_create)
 * [✓] Clear parameter order (context first, output last)
 *
 * Safety:
 * [✓] All pointers validated
 * [✓] All functions documented
 * [✓] Error codes for all failure modes
 * [✓] Resource cleanup functions provided
 *
 * Usability:
 * [✓] Opaque handle hides implementation
 * [✓] Const correctness throughout
 * [✓] Thread-safety documented
 * [!] Could add: builder pattern for complex queries
 */
```

### Proposed API Extension

```c
/* Query builder for complex searches */
typedef struct partsdb_query partsdb_query_t;

partsdb_query_t *partsdb_query_create(partsdb_t *db);

partsdb_query_t *partsdb_query_where_category(
    partsdb_query_t *q, const char *category);

partsdb_query_t *partsdb_query_where_quantity_below(
    partsdb_query_t *q, int32_t threshold);

partsdb_query_t *partsdb_query_order_by(
    partsdb_query_t *q, const char *field, bool ascending);

partsdb_query_t *partsdb_query_limit(
    partsdb_query_t *q, size_t limit);

partsdb_error_t partsdb_query_execute(
    partsdb_query_t *q, part_t **parts, size_t *count);

void partsdb_query_destroy(partsdb_query_t *q);

/* Usage: */
partsdb_query_t *q = partsdb_query_create(db);
partsdb_query_where_category(q, "Capacitors");
partsdb_query_where_quantity_below(q, 10);
partsdb_query_order_by(q, "quantity", true);
partsdb_query_limit(q, 50);

part_t *parts;
size_t count;
partsdb_error_t err = partsdb_query_execute(q, &parts, &count);
partsdb_query_destroy(q);
```

---

## 6.10 Review Questions - Part 6

> **Instructions**: Research these questions using *C Interfaces and Implementations*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[C Interfaces, Ch. 1]** What is the difference between a "checked" and "unchecked" runtime error in Hanson's terminology? Give examples of each in PartsDB.

2. **[C Interfaces, Ch. 2]** Explain the "first-class ADT" pattern. How does it differ from the opaque pointer pattern?

3. **[C Interfaces, Ch. 3]** What are the trade-offs between caller-allocates vs callee-allocates patterns? When would you choose each?

4. **[C Interfaces, Ch. 4]** Explain "resource acquisition is initialization" (RAII) and why C makes it difficult. What workarounds exist?

5. **[C Interfaces, Ch. 5]** What is the "fragile base class problem"? How can it affect C libraries with opaque types?

### Practical Exercises

6. **API Consistency Audit**: Review all PartsDB function signatures. Ensure consistent parameter ordering, naming conventions, and const correctness.

7. **Error Handling Improvement**: Add a `partsdb_error_t partsdb_get_last_error_detail(partsdb_t *db, char *buffer, size_t size)` function that provides detailed error messages including SQLite error text.

8. **Query Builder**: Implement the query builder API proposed in section 6.9. Include proper memory management and error handling.

9. **Version API**: Add version checking API to PartsDB. Include compile-time and runtime version information, and compatibility checking.

10. **Iterator Pattern**: Implement a cursor/iterator API for streaming large result sets without loading all into memory:
    ```c
    partsdb_cursor_t *partsdb_search_cursor(partsdb_t *db, const char *query);
    bool partsdb_cursor_next(partsdb_cursor_t *cursor, part_t *part);
    void partsdb_cursor_close(partsdb_cursor_t *cursor);
    ```

### Research Topics

11. **[C Interfaces, Ch. 6]** Research the "facade" pattern. How would you design a simplified API for PartsDB that wraps the full API for common use cases?

12. **[C Interfaces, Ch. 8]** Research "callback" patterns in C APIs. Design a callback-based API for PartsDB that notifies callers of database changes.

13. **[C Interfaces, Ch. 10]** Research "plugin" architectures in C. How would you design PartsDB to support loadable storage backends (SQLite, PostgreSQL, file-based)?

---

[← Part 5: Traps and Pitfalls](part-05-traps-pitfalls.md) | [Back to Index](README.md) | [Part 7: Concurrency →](part-07-concurrency.md)
