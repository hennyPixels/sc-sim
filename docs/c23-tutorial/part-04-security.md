# Part 4: Modern C Best Practices & Security

*Sources: Effective C (Seacord), Secure Coding in C and C++ (Seacord)*

> **"Security is not a feature—it's a property of the entire system."** — Robert C. Seacord

Security vulnerabilities in C often stem from the language's trust in the programmer. This part covers modern C security practices, drawing from Robert Seacord's authoritative works on secure C development. Every pattern is illustrated with PartsDB examples.

---

## 4.1 C23 Language Features for Security

### The `nullptr` Keyword

```c
/* Pre-C23: NULL has multiple problems */
#define NULL ((void *)0)  /* or just 0 */

/* Problem 1: Type ambiguity in variadic functions */
void log_message(const char *fmt, ...);
log_message("Value: %p", NULL);  /* NULL might be int 0! */

/* Problem 2: Comparison type */
if (ptr == 0)   /* Compares pointer to integer */
if (ptr == NULL) /* Better, but NULL might be 0 */

/* C23: nullptr is a keyword with type nullptr_t */
#include <stddef.h>

static partsdb_t *db = nullptr;  /* Clear intent */

void process(partsdb_t *db) {
    if (db == nullptr) {  /* Type-safe comparison */
        return;
    }
    /* ... */
}
```

### `[[nodiscard]]` Attribute

```c
/* Force callers to check return values */
[[nodiscard]] partsdb_error_t partsdb_open(const char *path, partsdb_t **db);

/* Usage - compiler warns if return ignored */
partsdb_open(path, &db);  /* Warning: ignoring return value */

/* Correct usage */
partsdb_error_t err = partsdb_open(path, &db);
if (err != PARTSDB_OK) {
    handle_error(err);
}

/* With message (C23) */
[[nodiscard("Memory leak if result ignored")]]
part_t *partsdb_fetch_part(partsdb_t *db, int64_t id);
```

### `constexpr` for Compile-Time Constants

```c
/* Guaranteed compile-time evaluation */
constexpr size_t MAX_PART_NUMBER_LEN = 64;
constexpr size_t MAX_NAME_LEN = 128;
constexpr size_t BUFFER_SIZE = MAX_PART_NUMBER_LEN + MAX_NAME_LEN + 32;

/* Can be used in array declarations */
char buffer[BUFFER_SIZE];  /* Stack allocation with known size */

/* Prevents runtime-dependent buffer sizes that could be exploited */
```

### Safer Enums

```c
/* C23: Enums with explicit underlying type */
enum partsdb_error : int32_t {
    PARTSDB_OK = 0,
    PARTSDB_ERROR_OPEN = -1,
    PARTSDB_ERROR_MEMORY = -2,
    /* Range is bounded to int32_t */
};

/* Prevents integer promotion surprises */
enum small_enum : uint8_t {
    VALUE_A = 0,
    VALUE_B = 1,
    VALUE_C = 255,
    /* VALUE_D = 256  -- Error: doesn't fit in uint8_t */
};
```

---

## 4.2 Integer Security

### Integer Overflow Vulnerabilities

```c
/* VULNERABLE: Integer overflow in allocation */
void *vulnerable_alloc(size_t count, size_t size) {
    size_t total = count * size;  /* Can overflow! */
    return malloc(total);
}

/* Attack: count=0x100000001, size=0x100
 * total = 0x100000001 * 0x100 = 0x100 (after 64-bit wrap)
 * Allocates 256 bytes, caller thinks it's huge
 */

/* SECURE: Check for overflow before multiplication */
void *secure_alloc(size_t count, size_t size) {
    if (count > 0 && size > SIZE_MAX / count) {
        return nullptr;  /* Would overflow */
    }
    return calloc(count, size);  /* calloc also checks internally */
}

/* Best: Use compiler built-ins */
void *secure_alloc_builtin(size_t count, size_t size) {
    size_t total;
    if (__builtin_mul_overflow(count, size, &total)) {
        return nullptr;
    }
    return malloc(total);
}
```

### Integer Truncation

```c
/* VULNERABLE: Truncation on cast */
void vulnerable_copy(int user_size) {
    if (user_size < 0) return;
    size_t size = (size_t)user_size;  /* Truncation on 32-bit size_t? */
    char *buf = malloc(size);
    /* ... */
}

/* SECURE: Validate against target type range */
bool safe_int_to_size(int value, size_t *result) {
    if (value < 0) return false;
    if (sizeof(size_t) < sizeof(int) && value > (int)SIZE_MAX) {
        return false;
    }
    *result = (size_t)value;
    return true;
}
```

### Signed vs Unsigned Comparisons

```c
/* DANGEROUS: Mixing signed and unsigned */
int len = get_user_input();  /* Could be negative */
char buffer[100];

if (len < sizeof(buffer)) {  /* len promoted to size_t! */
    /* -1 becomes SIZE_MAX, passes the check */
    memcpy(buffer, data, len);  /* len cast back to size_t = huge */
}

/* SECURE: Explicit validation */
if (len >= 0 && (size_t)len < sizeof(buffer)) {
    memcpy(buffer, data, (size_t)len);
}

/* PartsDB pattern: validate early, use unsigned internally */
partsdb_error_t partsdb_get_all(partsdb_t *db, part_t **parts,
                                 size_t *count, size_t limit, size_t offset) {
    /* All size parameters are unsigned - no sign confusion */
    if (limit > MAX_QUERY_LIMIT) {
        limit = MAX_QUERY_LIMIT;  /* Cap to safe maximum */
    }
    /* ... */
}
```

---

## 4.3 Buffer Overflow Prevention

### Stack Buffer Overflows

```c
/* VULNERABLE: Classic stack buffer overflow */
void vulnerable_gets(void) {
    char buffer[64];
    gets(buffer);  /* NEVER use gets() - removed in C11 */
}

void vulnerable_strcpy(const char *input) {
    char buffer[64];
    strcpy(buffer, input);  /* No bounds checking */
}

void vulnerable_sprintf(const char *name, int id) {
    char buffer[64];
    sprintf(buffer, "ID: %d, Name: %s", id, name);  /* Can overflow */
}

/* SECURE: Always use bounded functions */
void secure_input(void) {
    char buffer[64];
    if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
        /* Handle error */
    }
    /* Remove trailing newline */
    buffer[strcspn(buffer, "\n")] = '\0';
}

void secure_strcpy(const char *input) {
    char buffer[64];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';  /* Ensure null termination */
}

void secure_sprintf(const char *name, int id) {
    char buffer[64];
    int ret = snprintf(buffer, sizeof(buffer), "ID: %d, Name: %s", id, name);
    if (ret < 0 || (size_t)ret >= sizeof(buffer)) {
        /* Handle truncation or error */
    }
}
```

### PartsDB Safe String Function

```c
/* Safe string copy used throughout PartsDB */
static void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    /* Preconditions */
    if (dest == nullptr || dest_size == 0) {
        return;  /* Cannot write anywhere */
    }

    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }

    size_t src_len = strlen(src);

    /* Truncate if necessary */
    if (src_len >= dest_size) {
        src_len = dest_size - 1;
    }

    /* Use memcpy for efficiency (non-overlapping guaranteed) */
    memcpy(dest, src, src_len);
    dest[src_len] = '\0';  /* Always null-terminate */
}

/* Safe string concatenation */
static bool safe_strcat(char *dest, size_t dest_size,
                        const char *src, size_t *dest_len) {
    if (dest == nullptr || src == nullptr || dest_len == nullptr) {
        return false;
    }

    size_t current_len = *dest_len;
    size_t src_len = strlen(src);
    size_t total = current_len + src_len;

    if (total >= dest_size) {
        return false;  /* Would overflow */
    }

    memcpy(dest + current_len, src, src_len + 1);  /* Include null */
    *dest_len = total;
    return true;
}
```

### Heap Buffer Overflows

```c
/* VULNERABLE: Off-by-one in heap allocation */
char *vulnerable_strdup(const char *s) {
    size_t len = strlen(s);  /* Doesn't include null terminator */
    char *copy = malloc(len);  /* One byte short! */
    strcpy(copy, s);  /* Writes len+1 bytes, overflow! */
    return copy;
}

/* SECURE: Account for null terminator */
char *secure_strdup(const char *s) {
    if (s == nullptr) return nullptr;

    size_t len = strlen(s);

    /* Check for overflow when adding 1 */
    if (len == SIZE_MAX) return nullptr;

    char *copy = malloc(len + 1);
    if (copy == nullptr) return nullptr;

    memcpy(copy, s, len + 1);  /* Copy including null */
    return copy;
}
```

---

## 4.4 Format String Vulnerabilities

### The Danger of User-Controlled Format Strings

```c
/* VULNERABLE: User controls format string */
void vulnerable_log(const char *user_input) {
    printf(user_input);  /* User can inject %s, %n, etc. */
}

/* Attack examples:
 * Input: "%s%s%s%s%s" - crashes by reading invalid pointers
 * Input: "%x%x%x%x"   - leaks stack contents
 * Input: "%n"         - writes to memory!
 */

/* SECURE: Never pass user input as format string */
void secure_log(const char *user_input) {
    printf("%s", user_input);  /* User input is just data */
}

/* Or use puts() for simple strings */
void secure_log_simple(const char *user_input) {
    puts(user_input);  /* No format processing */
}
```

### Format String Best Practices

```c
/* Always use format specifiers appropriate to the type */
void print_part_info(const part_t *part) {
    /* PRId64 for portable int64_t printing */
    printf("ID: %" PRId64 "\n", part->id);

    /* %zu for size_t */
    printf("Name length: %zu\n", strlen(part->name));

    /* Limit string output */
    printf("Part number: %.64s\n", part->part_number);  /* Max 64 chars */
}

/* Validate format strings at compile time */
__attribute__((format(printf, 2, 3)))
void partsdb_log(int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);  /* fmt is trusted, from source code */
    va_end(args);
}
```

---

## 4.5 Memory Management Security

### Use-After-Free Prevention

```c
/* VULNERABLE: Dangling pointer */
void vulnerable_uaf(void) {
    part_t *part = malloc(sizeof(part_t));
    process_part(part);
    free(part);
    /* part still holds the address */
    print_part(part);  /* Use-after-free! */
}

/* SECURE: Null after free */
void secure_free_pattern(void) {
    part_t *part = malloc(sizeof(part_t));
    process_part(part);
    free(part);
    part = nullptr;  /* Prevent use-after-free */

    /* Later access will crash immediately (detectable) */
    /* rather than silently corrupting data */
}

/* PartsDB pattern: dedicated free functions */
void partsdb_free_parts(part_t *parts) {
    /* Safe to call with nullptr */
    free(parts);  /* Don't null - caller should null their pointer */
}

void partsdb_close(partsdb_t *db) {
    if (db == nullptr) return;

    if (db->db != nullptr) {
        sqlite3_close(db->db);
        db->db = nullptr;  /* Prevent double-close of SQLite */
    }

    free(db->path);
    db->path = nullptr;

    /* Zero sensitive data before freeing */
    memset(db, 0, sizeof(partsdb_t));
    free(db);
}
```

### Double-Free Prevention

```c
/* VULNERABLE: Double free */
void vulnerable_double_free(part_t *part) {
    free(part);
    /* ... other code ... */
    free(part);  /* Double free - undefined behavior */
}

/* SECURE: Use ownership semantics */
typedef struct {
    part_t *parts;
    size_t count;
    bool owns_memory;  /* Track ownership */
} part_list_t;

void part_list_free(part_list_t *list) {
    if (list == nullptr) return;

    if (list->owns_memory && list->parts != nullptr) {
        free(list->parts);
        list->parts = nullptr;
        list->owns_memory = false;
    }
}

/* Transfer ownership explicitly */
part_t *part_list_take_ownership(part_list_t *list) {
    if (!list->owns_memory) return nullptr;

    part_t *parts = list->parts;
    list->parts = nullptr;
    list->owns_memory = false;
    return parts;  /* Caller now owns the memory */
}
```

### Memory Leak Prevention

```c
/* Use goto for cleanup (idiomatic C pattern) */
partsdb_error_t partsdb_complex_operation(partsdb_t *db,
                                           const char *query,
                                           part_t **result) {
    partsdb_error_t err = PARTSDB_ERROR_MEMORY;
    sqlite3_stmt *stmt = nullptr;
    char *normalized_query = nullptr;
    part_t *parts = nullptr;

    /* Allocate resources */
    normalized_query = normalize_query(query);
    if (normalized_query == nullptr) goto cleanup;

    parts = calloc(MAX_RESULTS, sizeof(part_t));
    if (parts == nullptr) goto cleanup;

    /* Use resources */
    int rc = sqlite3_prepare_v2(db->db, normalized_query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        err = PARTSDB_ERROR_QUERY;
        goto cleanup;
    }

    /* ... process query ... */

    /* Success - transfer ownership */
    *result = parts;
    parts = nullptr;  /* Don't free - caller owns it now */
    err = PARTSDB_OK;

cleanup:
    /* Free all locally-owned resources */
    if (stmt) sqlite3_finalize(stmt);
    free(normalized_query);
    free(parts);  /* Safe even if nullptr */
    return err;
}
```

---

## 4.6 File I/O Security

### Path Traversal Prevention

```c
/* VULNERABLE: Path traversal attack */
void vulnerable_read_file(const char *filename) {
    char path[256];
    snprintf(path, sizeof(path), "/data/parts/%s", filename);
    /* filename = "../../../etc/passwd" would escape /data/parts */
    FILE *f = fopen(path, "r");
    /* ... */
}

/* SECURE: Validate and sanitize paths */
#include <stdlib.h>  /* realpath */

bool is_path_within_directory(const char *path, const char *directory) {
    char *real_path = realpath(path, nullptr);
    char *real_dir = realpath(directory, nullptr);

    if (real_path == nullptr || real_dir == nullptr) {
        free(real_path);
        free(real_dir);
        return false;
    }

    size_t dir_len = strlen(real_dir);
    bool within = (strncmp(real_path, real_dir, dir_len) == 0 &&
                   (real_path[dir_len] == '/' || real_path[dir_len] == '\0'));

    free(real_path);
    free(real_dir);
    return within;
}

partsdb_error_t secure_read_file(const char *filename, const char *base_dir) {
    /* Reject obviously dangerous patterns */
    if (strstr(filename, "..") != nullptr ||
        filename[0] == '/' ||
        filename[0] == '\\') {
        return PARTSDB_ERROR_INVALID;
    }

    char path[PATH_MAX];
    int ret = snprintf(path, sizeof(path), "%s/%s", base_dir, filename);
    if (ret < 0 || (size_t)ret >= sizeof(path)) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Verify path is within allowed directory */
    if (!is_path_within_directory(path, base_dir)) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Now safe to open */
    FILE *f = fopen(path, "r");
    /* ... */
}
```

### Secure File Permissions

```c
#include <sys/stat.h>
#include <fcntl.h>

/* Create file with restricted permissions */
int create_secure_file(const char *path) {
    /* Set umask to restrict permissions */
    mode_t old_umask = umask(077);  /* Only owner can access */

    int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, 0600);

    umask(old_umask);  /* Restore umask */

    if (fd < 0) {
        return -1;
    }

    return fd;
}

/* Verify file ownership and permissions before use */
bool verify_file_security(const char *path, uid_t expected_uid) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }

    /* Check owner */
    if (st.st_uid != expected_uid) {
        return false;
    }

    /* Check not world-readable/writable */
    if (st.st_mode & (S_IROTH | S_IWOTH)) {
        return false;
    }

    /* Check not group-writable */
    if (st.st_mode & S_IWGRP) {
        return false;
    }

    return true;
}
```

### TOCTOU (Time-of-Check to Time-of-Use) Prevention

```c
/* VULNERABLE: TOCTOU race condition */
void vulnerable_toctou(const char *path) {
    if (access(path, R_OK) == 0) {  /* Check */
        /* Attacker swaps file here! */
        FILE *f = fopen(path, "r");  /* Use - different file! */
        /* ... */
    }
}

/* SECURE: Open first, then check */
int secure_open_and_verify(const char *path, uid_t expected_uid) {
    /* Open the file first */
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    /* Verify using the file descriptor (not path) */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return -1;
    }

    if (st.st_uid != expected_uid) {
        close(fd);
        return -1;
    }

    return fd;  /* Return the already-opened, verified fd */
}
```

---

## 4.7 Concurrency Security

### Data Race Prevention

```c
#include <stdatomic.h>
#include <threads.h>

/* VULNERABLE: Data race on shared counter */
int vulnerable_counter = 0;

void vulnerable_increment(void) {
    vulnerable_counter++;  /* Read-modify-write is not atomic! */
}

/* SECURE: Use atomic operations */
atomic_int secure_counter = 0;

void secure_increment(void) {
    atomic_fetch_add(&secure_counter, 1);
}

/* SECURE: Use mutex for complex operations */
typedef struct {
    part_t *cache;
    size_t count;
    mtx_t mutex;
} part_cache_t;

partsdb_error_t cache_get(part_cache_t *cache, int64_t id, part_t *result) {
    mtx_lock(&cache->mutex);

    /* Critical section - only one thread at a time */
    for (size_t i = 0; i < cache->count; i++) {
        if (cache->cache[i].id == id) {
            *result = cache->cache[i];
            mtx_unlock(&cache->mutex);
            return PARTSDB_OK;
        }
    }

    mtx_unlock(&cache->mutex);
    return PARTSDB_ERROR_NOT_FOUND;
}
```

### Lock Ordering to Prevent Deadlock

```c
/* VULNERABLE: Potential deadlock with inconsistent lock ordering */
mtx_t lock_a, lock_b;

void thread1(void) {
    mtx_lock(&lock_a);
    mtx_lock(&lock_b);  /* Waits for lock_b */
    /* ... */
    mtx_unlock(&lock_b);
    mtx_unlock(&lock_a);
}

void thread2(void) {
    mtx_lock(&lock_b);  /* Thread 2 has lock_b */
    mtx_lock(&lock_a);  /* Waits for lock_a - DEADLOCK! */
    /* ... */
    mtx_unlock(&lock_a);
    mtx_unlock(&lock_b);
}

/* SECURE: Consistent lock ordering */
/* Rule: Always acquire lock_a before lock_b */

void thread1_fixed(void) {
    mtx_lock(&lock_a);  /* Always a first */
    mtx_lock(&lock_b);
    /* ... */
    mtx_unlock(&lock_b);
    mtx_unlock(&lock_a);
}

void thread2_fixed(void) {
    mtx_lock(&lock_a);  /* Always a first */
    mtx_lock(&lock_b);
    /* ... */
    mtx_unlock(&lock_b);
    mtx_unlock(&lock_a);
}
```

---

## 4.8 Input Validation Strategies

### Defense in Depth

```c
/* PartsDB: Multi-layer input validation */

/* Layer 1: Type validation at API boundary */
partsdb_error_t partsdb_create(partsdb_t *db, const part_t *part, int64_t *id) {
    /* Null checks */
    if (db == nullptr || part == nullptr || id == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Layer 2: Content validation */
    if (!validate_part(part)) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Layer 3: SQL parameterization (never trust validated input either) */
    sqlite3_bind_text(stmt, 1, part->part_number, -1, SQLITE_STATIC);
    /* ... */
}

/* Comprehensive part validation */
static bool validate_part(const part_t *part) {
    /* Required fields */
    if (part->part_number[0] == '\0') return false;
    if (part->name[0] == '\0') return false;

    /* String length limits (defense against unterminated strings) */
    if (strnlen(part->part_number, sizeof(part->part_number))
        >= sizeof(part->part_number)) return false;
    if (strnlen(part->name, sizeof(part->name))
        >= sizeof(part->name)) return false;

    /* Character validation - only printable ASCII */
    if (!is_printable_string(part->part_number)) return false;

    /* Numeric range validation */
    if (part->quantity < 0) return false;
    if (part->min_quantity < 0) return false;
    if (part->unit_price < 0.0) return false;

    return true;
}

static bool is_printable_string(const char *s) {
    while (*s) {
        if (*s < 0x20 || *s > 0x7E) {
            return false;  /* Non-printable or non-ASCII */
        }
        s++;
    }
    return true;
}
```

### SQL Injection Prevention

```c
/* VULNERABLE: String concatenation */
void vulnerable_search(partsdb_t *db, const char *query) {
    char sql[1024];
    sprintf(sql, "SELECT * FROM parts WHERE name LIKE '%%%s%%'", query);
    /* query = "'; DROP TABLE parts; --" destroys database! */
    sqlite3_exec(db->db, sql, callback, nullptr, nullptr);
}

/* SECURE: Parameterized queries (PartsDB pattern) */
partsdb_error_t partsdb_search(partsdb_t *db, const char *query,
                                part_t **parts, size_t *count) {
    const char *sql =
        "SELECT * FROM parts_fts WHERE parts_fts MATCH ?;";

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    /* Bind user input as parameter - automatically escaped */
    char fts_query[256];
    snprintf(fts_query, sizeof(fts_query), "\"%s\"*", query);
    sqlite3_bind_text(stmt, 1, fts_query, -1, SQLITE_TRANSIENT);

    /* ... execute and process results ... */

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}
```

---

## 4.9 PartsDB Case Study: Security Audit

### Security Audit Checklist

```c
/*
 * PartsDB Security Audit Results
 *
 * [✓] Input Validation
 *     - All public functions validate pointer arguments
 *     - String lengths validated before copy
 *     - Numeric ranges checked
 *
 * [✓] Memory Safety
 *     - All allocations checked for nullptr
 *     - Consistent ownership patterns
 *     - No buffer overflows (bounded string operations)
 *
 * [✓] SQL Injection
 *     - All queries use parameterized statements
 *     - No string concatenation for SQL
 *
 * [✓] Integer Safety
 *     - Size calculations checked for overflow
 *     - Signed/unsigned conversions explicit
 *
 * [!] Potential Improvements
 *     - Add rate limiting for search queries
 *     - Implement connection timeout
 *     - Add audit logging
 *     - Consider memory encryption for sensitive data
 */
```

### Security-Enhanced partsdb_open

```c
partsdb_error_t partsdb_open_secure(const char *path, partsdb_t **db,
                                     const partsdb_security_options_t *opts) {
    /* Input validation */
    if (path == nullptr || db == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    *db = nullptr;

    /* Path validation */
    if (strlen(path) >= PATH_MAX) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Optional: Verify path is in allowed directory */
    if (opts && opts->allowed_directory) {
        if (!is_path_within_directory(path, opts->allowed_directory)) {
            return PARTSDB_ERROR_INVALID;
        }
    }

    /* Allocate context */
    partsdb_t *ctx = calloc(1, sizeof(partsdb_t));
    if (ctx == nullptr) {
        return PARTSDB_ERROR_MEMORY;
    }

    /* Store path (for reconnection) */
    ctx->path = strdup(path);
    if (ctx->path == nullptr) {
        free(ctx);
        return PARTSDB_ERROR_MEMORY;
    }

    /* Open with secure flags */
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    if (opts && opts->read_only) {
        flags = SQLITE_OPEN_READONLY;
    }

    int rc = sqlite3_open_v2(path, &ctx->db, flags, nullptr);
    if (rc != SQLITE_OK) {
        free(ctx->path);
        free(ctx);
        return PARTSDB_ERROR_OPEN;
    }

    /* Security pragmas */
    sqlite3_exec(ctx->db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    /* Optional: Timeout to prevent DoS */
    if (opts && opts->busy_timeout_ms > 0) {
        sqlite3_busy_timeout(ctx->db, opts->busy_timeout_ms);
    }

    /* Success */
    *db = ctx;
    return PARTSDB_OK;
}
```

---

## 4.10 Review Questions - Part 4

> **Instructions**: Research these questions using *Effective C* and *Secure Coding in C and C++*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[Effective C, Ch. 5]** Explain the difference between undefined behavior and implementation-defined behavior. Give three examples of each in C.

2. **[Secure Coding, Ch. 4]** What is the "billion laughs" attack? How could it affect a C program processing XML or JSON? How would you defend against it?

3. **[Effective C, Ch. 6]** Why is checking `malloc()` return value not sufficient for security? What other validations are needed?

4. **[Secure Coding, Ch. 5]** Explain the "double-fetch" vulnerability. How does it relate to TOCTOU? Give an example in a multi-threaded context.

5. **[Effective C, Ch. 8]** What is the strict aliasing rule? How can violating it lead to security vulnerabilities?

### Practical Exercises

6. **Integer Overflow Audit**: Review all arithmetic operations in PartsDB. Add overflow checks using `__builtin_*_overflow` functions where appropriate.

7. **Input Validation Framework**: Implement a comprehensive input validation module for PartsDB that can be configured with validation rules (length limits, character sets, numeric ranges).

8. **Fuzzing Harness**: Create a fuzzing harness for PartsDB using AFL++ or libFuzzer. Document any bugs found.

9. **Static Analysis**: Run the Clang Static Analyzer with `security.*` checkers enabled. Fix all reported issues.

10. **Memory Safety Hardening**: Add compile-time options to PartsDB for:
    - FORTIFY_SOURCE
    - Stack canaries
    - Address sanitizer
    - Control-flow integrity

### Research Topics

11. **[Secure Coding, Ch. 7]** Research "format string exploits" in depth. Write a demonstration (in a controlled environment) showing how `%n` can be used to write to arbitrary memory.

12. **[Effective C, Ch. 10]** Research constant-time programming. Implement a constant-time string comparison function for PartsDB to prevent timing attacks on part number lookups.

13. **[Secure Coding, Ch. 8]** Research return-oriented programming (ROP). How do modern mitigations (ASLR, stack canaries, CFI) protect against ROP? What are their limitations?

---

[← Part 3: Bit Manipulation](part-03-bit-manipulation.md) | [Back to Index](README.md) | [Part 5: Traps and Pitfalls →](part-05-traps-pitfalls.md)
