# Part 5: C Traps and Pitfalls

*Source: C Traps and Pitfalls (Koenig)*

> **"The C language is like a sharp knife: in the hands of a master it is an invaluable tool; in the hands of a novice it can cause severe injury."** — Andrew Koenig

C's flexibility comes with subtle gotchas that can trap even experienced programmers. This part catalogs common pitfalls, drawing from Koenig's classic taxonomy and extending it with modern examples from the PartsDB codebase.

---

## 5.1 Lexical Pitfalls

### The `=` vs `==` Trap

```c
/* WRONG: Assignment instead of comparison */
if (status = PARTSDB_OK) {  /* Always true (unless PARTSDB_OK is 0) */
    /* This code always executes, and status is now PARTSDB_OK */
}

/* CORRECT: Comparison */
if (status == PARTSDB_OK) {
    /* Executes only when status equals PARTSDB_OK */
}

/* Defensive style: Put constant on left (Yoda conditions) */
if (PARTSDB_OK == status) {  /* Compiler error if you write = */
    /* Some find this less readable */
}

/* Best: Enable -Wparentheses warning */
/* gcc -Wparentheses warns: suggest parentheses around assignment */
```

### The Dangling Else

```c
/* AMBIGUOUS: Which if does the else belong to? */
if (db != nullptr)
    if (db->db != nullptr)
        execute_query(db);
else
    return PARTSDB_ERROR_INVALID;  /* Belongs to INNER if! */

/* CORRECT: Use braces to make intent clear */
if (db != nullptr) {
    if (db->db != nullptr) {
        execute_query(db);
    }
} else {
    return PARTSDB_ERROR_INVALID;
}

/* Rule: Always use braces, even for single statements */
```

### String Literal Concatenation

```c
/* Adjacent string literals are concatenated */
const char *sql = "SELECT * FROM parts "
                  "WHERE id = ?;";
/* Becomes: "SELECT * FROM parts WHERE id = ?;" */

/* PITFALL: Missing comma in array */
const char *queries[] = {
    "SELECT * FROM parts",
    "INSERT INTO parts"   /* Missing comma! */
    "UPDATE parts"        /* Concatenated with previous! */
};
/* queries[1] = "INSERT INTO partsUPDATE parts" */

/* CORRECT: Always include trailing comma */
const char *queries[] = {
    "SELECT * FROM parts",
    "INSERT INTO parts",
    "UPDATE parts",  /* Trailing comma prevents this bug */
};
```

### Trigraphs and Digraphs

```c
/* Trigraphs (replaced before preprocessing) - REMOVED in C23 */
/* ??= → #    ??) → ]    ??! → | */
/* ??( → [    ??' → ^    ??> → } */
/* ??/ → \    ??< → {    ??- → ~ */

/* This was a valid printf in C17: */
/* printf("What??!\n");  // Printed "What|" due to ??! → | */

/* C23 removes trigraphs - this is now safe */
printf("What??!\n");  /* Prints "What??!" as expected */

/* Digraphs still exist but are rarely problematic */
/* <: → [    :> → ]    <% → {    %> → }    %: → # */
```

---

## 5.2 Syntactic Traps

### Operator Precedence Surprises

```c
/* PITFALL: Bitwise operators have lower precedence than comparison */
if (flags & MASK == 0) {  /* Parsed as: flags & (MASK == 0) */
    /* Always false if MASK != 0 */
}

/* CORRECT: Use parentheses */
if ((flags & MASK) == 0) {
    /* Correct: checks if masked flags are zero */
}

/* PITFALL: Shift has lower precedence than addition */
int x = 1 << 2 + 3;  /* Parsed as: 1 << (2 + 3) = 32, not (1 << 2) + 3 = 7 */

/* CORRECT: */
int x = (1 << 2) + 3;  /* 7 */

/* Common precedence surprises: */
/* a & b == c  →  a & (b == c)     Use: (a & b) == c */
/* a | b != c  →  a | (b != c)     Use: (a | b) != c */
/* a << b + c  →  a << (b + c)     Use: (a << b) + c */
/* *p++        →  *(p++)           Usually intended, but verify */
/* *p.member   →  *(p.member)      Use: (*p).member or p->member */
```

### The sizeof Pitfall

```c
/* sizeof is an operator, not a function */
int arr[10];
size_t s1 = sizeof arr;      /* OK: 40 (10 * 4) */
size_t s2 = sizeof(arr);     /* OK: 40 */
size_t s3 = sizeof(int);     /* OK: 4 */
size_t s4 = sizeof int;      /* ERROR: needs parentheses for types */

/* PITFALL: sizeof pointer vs array */
void process(int arr[10]) {  /* arr is actually int* */
    size_t s = sizeof(arr);  /* Returns sizeof(int*), not 40! */
}

/* PITFALL: sizeof with VLA */
int n = get_size();
int vla[n];
size_t s = sizeof(vla);  /* Evaluated at runtime! Not a constant */
```

### Declaration Syntax Confusion

```c
/* Pointer declaration: * binds to the variable, not the type */
int* a, b;    /* a is int*, b is int (not int*!) */
int *a, *b;   /* Both are int* - clearer */
int* a;       /* Preferred: one declaration per line */
int* b;

/* Function pointer syntax */
int (*fp)(int, int);           /* Pointer to function returning int */
int *fp(int, int);             /* Function returning int* */
int (*fp[10])(int, int);       /* Array of 10 function pointers */

/* Use typedefs for clarity */
typedef int (*binary_op_t)(int, int);
binary_op_t operations[10];  /* Much clearer */
```

### The Missing Semicolon

```c
/* PITFALL: Missing semicolon after struct definition */
struct part {
    int64_t id;
    char name[64];
}                              /* Missing semicolon! */

partsdb_error_t partsdb_open(const char *path, partsdb_t **db)
{
    /* Compiler thinks struct definition continues here */
    /* Very confusing error messages result */
}

/* CORRECT: */
struct part {
    int64_t id;
    char name[64];
};  /* Semicolon required */
```

---

## 5.3 Semantic Traps

### Integer Division Truncation

```c
/* Division truncates toward zero */
int result = 7 / 3;    /* 2, not 2.333... */
int neg = -7 / 3;      /* -2 (toward zero), not -3 (toward -∞) */

/* PITFALL: Computing percentage */
int percent = (count / total) * 100;  /* Always 0 if count < total! */

/* CORRECT: */
int percent = (count * 100) / total;  /* Works, but may overflow */
int percent = (int)((double)count / total * 100);  /* Safe */

/* PITFALL: Averaging two numbers */
int avg = (a + b) / 2;  /* Can overflow if a + b > INT_MAX */

/* CORRECT: Avoid overflow */
int avg = a + (b - a) / 2;  /* Safe if a <= b */
int avg = (a / 2) + (b / 2) + ((a % 2 + b % 2) / 2);  /* General */
```

### Unsigned Arithmetic Wrapping

```c
/* Unsigned integers wrap around (modulo 2^n) */
unsigned int u = 0;
u--;  /* u = UINT_MAX (4294967295 on 32-bit) */

/* PITFALL: Loop with unsigned counter going down to 0 */
for (unsigned i = n; i >= 0; i--) {  /* Infinite loop! */
    /* i is always >= 0 because it's unsigned */
}

/* CORRECT: */
for (unsigned i = n; i > 0; i--) {
    process(i - 1);  /* Process 0 to n-1 */
}
/* Or: */
for (unsigned i = n; i-- > 0; ) {
    process(i);  /* Post-decrement in condition */
}

/* PITFALL: Subtracting unsigned values */
size_t a = 5, b = 10;
size_t diff = a - b;  /* Huge positive number, not -5! */

/* CORRECT: Check before subtracting */
if (a >= b) {
    size_t diff = a - b;
}
```

### Array Decay

```c
/* Arrays decay to pointers in most contexts */
void func(int arr[10]) {  /* arr is actually int*, the 10 is ignored */
    sizeof(arr);  /* Returns sizeof(int*), not 40 */
}

/* PITFALL: Returning local array */
int* get_array(void) {
    int arr[10];       /* Stack allocation */
    return arr;        /* Returns dangling pointer! */
}

/* PITFALL: Array as struct member vs pointer */
struct with_array {
    int data[100];  /* 400 bytes embedded in struct */
};

struct with_pointer {
    int *data;      /* 8 bytes, points elsewhere */
};

/* Copying behavior differs! */
struct with_array a1, a2;
a2 = a1;  /* Copies all 400 bytes of data */

struct with_pointer p1, p2;
p2 = p1;  /* Copies only the pointer, data is shared! */
```

### Short-Circuit Evaluation

```c
/* && and || short-circuit - right side may not execute */
if (ptr != nullptr && ptr->value > 0) {
    /* ptr->value only evaluated if ptr != nullptr - SAFE */
}

/* PITFALL: Side effects in short-circuited expression */
if (check_valid(db) || log_and_fail()) {
    /* log_and_fail() never called if check_valid() returns true */
}

/* PITFALL: Bitwise vs logical operators */
if (a & b) { }   /* Bitwise AND - both evaluated */
if (a && b) { }  /* Logical AND - short-circuits */

int x = 0;
if (0 & (x = 5)) { }  /* x is now 5 */
if (0 && (x = 5)) { } /* x is still 0 - assignment skipped */
```

---

## 5.4 Preprocessor Pitfalls

### Macro Argument Evaluation

```c
/* PITFALL: Arguments evaluated multiple times */
#define SQUARE(x) x * x
int a = 5;
int b = SQUARE(a++);  /* Expands to: a++ * a++ - undefined behavior! */

/* PITFALL: Operator precedence in macros */
#define DOUBLE(x) x + x
int c = DOUBLE(3) * 2;  /* Expands to: 3 + 3 * 2 = 9, not 12! */

/* CORRECT: Parenthesize everything */
#define SQUARE(x) ((x) * (x))  /* Still has double-evaluation issue */
#define DOUBLE(x) ((x) + (x))

/* BEST: Use inline functions for type safety */
static inline int square(int x) {
    return x * x;  /* x evaluated once, type-checked */
}

/* Or use statement expressions (GCC/Clang extension) */
#define SQUARE_SAFE(x) ({ \
    __typeof__(x) _x = (x); \
    _x * _x; \
})
```

### Macro Semicolon Issues

```c
/* PITFALL: Macro with trailing semicolon */
#define LOG(msg) printf("%s\n", msg);

if (debug)
    LOG("debugging");  /* Extra semicolon causes empty statement */
else
    do_something();    /* Syntax error: else without if */

/* CORRECT: No trailing semicolon, user adds it */
#define LOG(msg) printf("%s\n", msg)

/* For multi-statement macros, use do-while(0) */
#define SWAP(a, b) do { \
    __typeof__(a) _tmp = (a); \
    (a) = (b); \
    (b) = _tmp; \
} while (0)

/* Now works correctly: */
if (condition)
    SWAP(x, y);  /* User's semicolon completes do-while */
else
    something_else();
```

### Include Guard Issues

```c
/* PITFALL: Missing include guards */
/* header.h */
struct thing { int x; };  /* Error if included twice */

/* CORRECT: Include guards */
#ifndef HEADER_H
#define HEADER_H

struct thing { int x; };

#endif /* HEADER_H */

/* Alternative: #pragma once (widely supported but not standard) */
#pragma once

struct thing { int x; };

/* PITFALL: Include guard name collision */
/* Both files use UTILS_H - only first one's content is included */
/* Use unique names: PROJECT_MODULE_HEADER_H */
#ifndef PARTSDB_DATABASE_H
#define PARTSDB_DATABASE_H
/* ... */
#endif
```

### Stringification and Token Pasting

```c
/* Stringification: # turns argument into string literal */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)  /* Double-expansion needed */

#define VERSION 1.0
printf("Version: " STRINGIFY(VERSION));  /* "VERSION" - wrong! */
printf("Version: " TOSTRING(VERSION));   /* "1.0" - correct */

/* Token pasting: ## concatenates tokens */
#define MAKE_FUNC(name) void name##_init(void)
MAKE_FUNC(partsdb);  /* Creates: void partsdb_init(void) */

/* PITFALL: ## with empty argument */
#define PASTE(a, b) a ## b
PASTE(, suffix);  /* Undefined behavior - empty token */
```

---

## 5.5 Library Function Gotchas

### String Functions

```c
/* PITFALL: strncpy doesn't null-terminate */
char dest[10];
strncpy(dest, "hello world", sizeof(dest));  /* No null terminator! */
dest[sizeof(dest) - 1] = '\0';  /* Must manually terminate */

/* PITFALL: strncat size parameter */
char buf[20] = "Hello";
strncat(buf, " World", sizeof(buf));  /* WRONG: sizeof is buffer size */
strncat(buf, " World", sizeof(buf) - strlen(buf) - 1);  /* Correct */

/* PITFALL: strlen on unterminated string */
char data[10];
memcpy(data, "hello", 5);  /* No null terminator */
size_t len = strlen(data);  /* Undefined behavior! */

/* Use strnlen for potentially unterminated strings */
size_t len = strnlen(data, sizeof(data));  /* Safe */
```

### Memory Functions

```c
/* PITFALL: memcpy with overlapping regions */
char buf[] = "hello world";
memcpy(buf + 2, buf, 5);  /* Undefined behavior - overlapping! */
memmove(buf + 2, buf, 5); /* Correct - handles overlap */

/* PITFALL: memset with sizeof pointer */
int *arr = malloc(100 * sizeof(int));
memset(arr, 0, sizeof(arr));  /* Only clears 8 bytes (pointer size)! */
memset(arr, 0, 100 * sizeof(int));  /* Correct */

/* PITFALL: memset to non-zero for non-char types */
int arr[10];
memset(arr, 1, sizeof(arr));  /* arr[i] = 0x01010101, not 1! */
/* Only memset to 0 or -1 (0xFF) works as expected for non-char */
```

### printf Family

```c
/* PITFALL: Wrong format specifier */
int64_t big = 1234567890123LL;
printf("%d\n", big);   /* Undefined behavior - wrong size */
printf("%" PRId64 "\n", big);  /* Correct - from <inttypes.h> */

size_t sz = sizeof(int);
printf("%d\n", sz);    /* Wrong on 64-bit (size_t is 64-bit) */
printf("%zu\n", sz);   /* Correct */

/* PITFALL: Missing argument */
printf("%s %s\n", "hello");  /* Undefined behavior - missing arg */

/* Enable compiler warnings: gcc -Wformat */
```

### File Operations

```c
/* PITFALL: Forgetting to check fopen return */
FILE *f = fopen("data.txt", "r");
fscanf(f, "%d", &x);  /* Crash if fopen failed! */

/* CORRECT: */
FILE *f = fopen("data.txt", "r");
if (f == nullptr) {
    perror("fopen");
    return -1;
}

/* PITFALL: Using feof() incorrectly */
while (!feof(f)) {  /* WRONG: feof true only AFTER failed read */
    fgets(buffer, sizeof(buffer), f);
    process(buffer);  /* Processes last line twice! */
}

/* CORRECT: Check return value */
while (fgets(buffer, sizeof(buffer), f) != nullptr) {
    process(buffer);
}
```

---

## 5.6 Portability Pitfalls

### Data Type Sizes

```c
/* Sizes vary by platform */
/* int: 16 or 32 bits (usually 32 on modern systems) */
/* long: 32 or 64 bits (32 on Windows 64-bit, 64 on Linux 64-bit) */
/* pointer: 32 or 64 bits */

/* PITFALL: Assuming int is 32 bits */
int mask = 0xFFFFFFFF;  /* May not work on 16-bit int */

/* CORRECT: Use fixed-width types from <stdint.h> */
uint32_t mask = 0xFFFFFFFF;  /* Always 32 bits */
int32_t id = part->id;  /* Explicit size */

/* PITFALL: Assuming pointer fits in int */
void *ptr = malloc(100);
int addr = (int)ptr;  /* Truncation on 64-bit! */
intptr_t addr = (intptr_t)ptr;  /* Correct */
```

### Byte Order (Endianness)

```c
/* PITFALL: Assuming byte order */
uint32_t value = 0x01020304;
uint8_t *bytes = (uint8_t *)&value;
/* bytes[0] = 0x04 on little-endian (x86) */
/* bytes[0] = 0x01 on big-endian (some ARM, network) */

/* CORRECT: Use portable serialization */
void write_uint32_be(uint8_t *buf, uint32_t value) {
    buf[0] = (value >> 24) & 0xFF;
    buf[1] = (value >> 16) & 0xFF;
    buf[2] = (value >> 8) & 0xFF;
    buf[3] = value & 0xFF;
}

uint32_t read_uint32_be(const uint8_t *buf) {
    return ((uint32_t)buf[0] << 24) |
           ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8) |
           (uint32_t)buf[3];
}
```

### Character Set Issues

```c
/* PITFALL: Assuming ASCII */
char c = 'A';
int is_upper = (c >= 'A' && c <= 'Z');  /* ASCII-dependent */

/* CORRECT: Use standard functions */
#include <ctype.h>
int is_upper = isupper((unsigned char)c);  /* Portable */

/* PITFALL: Signed char with high-bit characters */
char c = '\xFF';  /* -1 if char is signed */
if (c == 0xFF) {  /* May fail: -1 != 255 */
    /* ... */
}

/* CORRECT: Use unsigned char for binary data */
unsigned char c = '\xFF';  /* Always 255 */
```

### Structure Padding

```c
/* PITFALL: Assuming struct size equals sum of member sizes */
struct example {
    char a;     /* 1 byte */
    int b;      /* 4 bytes, but starts at offset 4 (padding!) */
    char c;     /* 1 byte, at offset 8 */
};
/* sizeof(struct example) = 12, not 6 */

/* For binary file/network formats, use explicit padding or pack */
struct __attribute__((packed)) wire_format {
    char a;
    int b;
    char c;
};
/* sizeof = 6, but may cause alignment issues */

/* BEST: Define wire format explicitly */
struct wire_format {
    uint8_t a;
    uint8_t padding[3];
    uint32_t b;
    uint8_t c;
    uint8_t padding2[3];
};
```

---

## 5.7 Undefined Behavior Catalog

### Null Pointer Dereference

```c
/* Dereferencing nullptr is UB */
int *p = nullptr;
int x = *p;  /* Undefined behavior - may crash or not */

/* PITFALL: Compiler may optimize assuming no UB */
int *p = get_pointer();
int x = *p;  /* Compiler assumes p != nullptr after this */
if (p == nullptr) {  /* Compiler may remove this check! */
    /* This code may never execute even if p was nullptr */
}

/* CORRECT: Check before use */
int *p = get_pointer();
if (p == nullptr) {
    return error;
}
int x = *p;  /* Now safe */
```

### Signed Overflow

```c
/* Signed integer overflow is UB (unlike unsigned wrap) */
int a = INT_MAX;
int b = a + 1;  /* Undefined behavior! */

/* Compiler may optimize assuming no overflow */
bool check(int x) {
    return x + 1 > x;  /* Compiler may optimize to: return true; */
}

/* CORRECT: Check before operation */
bool safe_add(int a, int b, int *result) {
    if (b > 0 && a > INT_MAX - b) return false;  /* Would overflow */
    if (b < 0 && a < INT_MIN - b) return false;  /* Would underflow */
    *result = a + b;
    return true;
}
```

### Use After Free

```c
/* Accessing freed memory is UB */
int *p = malloc(sizeof(int));
*p = 42;
free(p);
int x = *p;  /* UB - may return 42, garbage, or crash */
*p = 10;     /* UB - may corrupt heap metadata */
free(p);     /* UB - double free */

/* DEFENSIVE: Null after free */
free(p);
p = nullptr;
/* Now: *p crashes immediately (better than silent corruption) */
```

### Uninitialized Variables

```c
/* Reading uninitialized variable is UB */
int x;
printf("%d\n", x);  /* UB - may print anything */

/* PITFALL: Partially initialized struct */
struct part p;
p.id = 1;
printf("%s\n", p.name);  /* UB - name not initialized */

/* CORRECT: Zero-initialize */
struct part p = {0};  /* All members zeroed */
p.id = 1;

/* Or use designated initializers (C99+) */
struct part p = { .id = 1 };  /* Other members zeroed */
```

### Shift Undefined Behavior

```c
/* Shifting by negative or >= width is UB */
int x = 1 << -1;   /* UB */
int y = 1 << 32;   /* UB on 32-bit int */
int z = 1 << 31;   /* UB: shifts into sign bit */

/* Shifting negative value is implementation-defined (left) or UB (right) */
int neg = -1;
int a = neg << 1;   /* Implementation-defined (C99), UB (C89) */
int b = neg >> 1;   /* Implementation-defined (arithmetic or logical) */

/* CORRECT: Use unsigned for bit manipulation */
uint32_t x = 1U << 31;  /* Well-defined */
uint32_t mask = ~0U >> (32 - n);  /* Well-defined */
```

---

## 5.8 PartsDB Case Study: Trap Hunting

### Audit Checklist

```c
/*
 * PartsDB Trap Audit
 *
 * [✓] = versus == checked
 * [✓] All switch cases have break or fallthrough comment
 * [✓] Unsigned loop variables don't go below 0
 * [✓] Division checked for zero divisor
 * [✓] Pointer checked before dereference
 * [✓] Array bounds validated
 * [✓] Format strings match argument types
 * [✓] strncpy results null-terminated
 * [✓] malloc returns checked
 * [✓] No signed overflow in size calculations
 */
```

### Fixed Patterns from PartsDB

```c
/* Pattern: Safe string copy */
static void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (dest == nullptr || dest_size == 0) return;
    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= dest_size) {
        len = dest_size - 1;
    }
    memcpy(dest, src, len);
    dest[len] = '\0';  /* Always null-terminate */
}

/* Pattern: Safe SQLite text extraction */
static const char *sqlite_text(sqlite3_stmt *stmt, int col) {
    const char *text = (const char *)sqlite3_column_text(stmt, col);
    return text ? text : "";  /* Never return nullptr */
}

/* Pattern: Defensive loop bounds */
partsdb_error_t partsdb_get_all(partsdb_t *db, part_t **parts,
                                 size_t *count, size_t limit, size_t offset) {
    /* Limit to prevent excessive memory allocation */
    if (limit > MAX_QUERY_LIMIT) {
        limit = MAX_QUERY_LIMIT;
    }

    /* Use size_t for counts to avoid signed issues */
    size_t n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < limit) {
        n++;
    }
    /* ... */
}
```

---

## 5.9 Review Questions - Part 5

> **Instructions**: Research these questions using *C Traps and Pitfalls*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[C Traps and Pitfalls, Ch. 1]** Explain why `x = y/*p` is parsed as `x = y` followed by a comment. How do you write this correctly?

2. **[C Traps and Pitfalls, Ch. 2]** What is "the biggest pebble" in C operator precedence? (Hint: it involves assignment.)

3. **[C Traps and Pitfalls, Ch. 3]** Why can the expression `i = i++` produce different results on different compilers? What sequence point rule does it violate?

4. **[C Traps and Pitfalls, Ch. 4]** Explain why `#define TWICE(x) (x) + (x)` is safer than `#define TWICE(x) x + x` but still has a subtle bug.

5. **[C Traps and Pitfalls, Ch. 6]** What is the "most vexing parse"? Give an example in C.

### Practical Exercises

6. **Precedence Audit**: Review all expressions in PartsDB using `&`, `|`, `<<`, `>>`. Ensure all have explicit parentheses where precedence is non-obvious.

7. **Macro Replacement**: Identify all function-like macros in PartsDB. Replace them with `static inline` functions where appropriate.

8. **Unsigned Audit**: Find all loops with unsigned counters in PartsDB. Verify none can underflow. Add comments explaining why each is safe.

9. **Format String Audit**: Check all `printf`/`sprintf` calls. Ensure format specifiers match argument types. Add `__attribute__((format))` to wrapper functions.

10. **Portability Test**: Compile PartsDB with both GCC and Clang, with `-Wpedantic`. Fix all warnings related to non-standard constructs.

### Research Topics

11. **[C Traps and Pitfalls, Ch. 5]** Research "sequence points" in C. What operations create sequence points? Why does the concept matter for undefined behavior?

12. **[C Traps and Pitfalls, Ch. 7]** Research the "strict aliasing rule" and type punning. What is the safe way to convert between `float` and `uint32_t` representations?

13. **[C Traps and Pitfalls, Appendix]** Research compiler-specific behavior: what happens when you access uninitialized memory, dereference nullptr, or cause signed overflow on GCC vs Clang vs MSVC?

---

[← Part 4: Security](part-04-security.md) | [Back to Index](README.md) | [Part 6: API Design →](part-06-api-design.md)
