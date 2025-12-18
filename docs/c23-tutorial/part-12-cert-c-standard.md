# Part 12: SEI CERT C Coding Standard

*Drawing from the Software Engineering Institute CERT C Coding Standard*

## Introduction

The SEI CERT C Coding Standard is a comprehensive set of rules and recommendations
for writing secure, reliable C code. Developed by CERT at Carnegie Mellon University,
it addresses common sources of vulnerabilities and undefined behavior.

> "The goal of the CERT C Coding Standard is to produce safe, secure, and reliable
> systems by eliminating undefined behaviors and preventing exploitable vulnerabilities."

The standard organizes rules into categories:
- **Rules**: Must be followed; violations indicate defects
- **Recommendations**: Should be followed; violations indicate code smell

## 12.1 Preprocessor (PRE) Rules

### PRE30-C: Do not create a universal character name through concatenation

```c
/* PRE30-C: Universal character name issues */

/*
 * VIOLATION: Token pasting can create invalid universal character names
 */
#define BAD_PASTE(x) \u00##x  /* Could create invalid \uXXXX */

/*
 * COMPLIANT: Avoid creating UCN through concatenation
 */
/* Use literal characters or complete UCN directly */
#define EURO_SIGN "\u20AC"  /* Complete UCN */
```

### PRE31-C: Avoid side effects in arguments to unsafe macros

```c
/* PRE31-C: Macro argument side effects */

/* UNSAFE MACRO - evaluates argument multiple times */
#define SQUARE_UNSAFE(x) ((x) * (x))

/* VIOLATION: Side effect in unsafe macro */
void
violation_pre31(void)
{
    int i = 5;
    int result = SQUARE_UNSAFE(i++);  /* i++ evaluated twice! */
    /* Expected: 25, Actual: 30 (5 * 6) */
    printf("Result: %d, i: %d\n", result, i);  /* i is now 7, not 6 */
}

/* COMPLIANT: Use function or _Generic (C11+) */
static inline int
square(int x)
{
    return x * x;
}

/* Or use temporary variable */
#define SQUARE_SAFE(x) ({ \
    __typeof__(x) _temp = (x); \
    _temp * _temp; \
})

void
compliant_pre31(void)
{
    int i = 5;
    int result = square(i++);  /* i++ evaluated once */
    printf("Result: %d, i: %d\n", result, i);  /* Correct: 25, 6 */
}
```

### PRE32-C: Do not use preprocessor directives in invocations of function-like macros

```c
/* PRE32-C: Preprocessor directives in macro calls */

#define MULTI_LINE(a, b, c) ((a) + (b) + (c))

/* VIOLATION: Directive inside macro invocation */
/*
int bad = MULTI_LINE(1,
    #ifdef DEBUG
    2,
    #else
    3,
    #endif
    4);
*/

/* COMPLIANT: Use conditional outside macro */
#ifdef DEBUG
    #define DEBUG_VAL 2
#else
    #define DEBUG_VAL 3
#endif

int good = MULTI_LINE(1, DEBUG_VAL, 4);
```

## 12.2 Declarations and Initialization (DCL) Rules

### DCL30-C: Declare objects with appropriate storage durations

```c
/* DCL30-C: Storage duration issues */
#include <stdio.h>

/* VIOLATION: Returning address of automatic variable */
int *
violation_dcl30(void)
{
    int local = 42;
    return &local;  /* BUG: local is destroyed when function returns */
}

/* COMPLIANT: Return pointer to static or heap memory */
int *
compliant_dcl30_static(void)
{
    static int persistent = 42;
    return &persistent;  /* OK: static storage duration */
}

int *
compliant_dcl30_heap(void)
{
    int *heap = malloc(sizeof(int));
    if (heap) {
        *heap = 42;
    }
    return heap;  /* OK: caller owns and must free */
}

/* Also applies to returning pointers to string literals */
const char *
get_message(int code)
{
    /* OK: String literals have static storage duration */
    switch (code) {
    case 0: return "Success";
    case 1: return "Error";
    default: return "Unknown";
    }
}
```

### DCL31-C: Declare identifiers before using them

```c
/* DCL31-C: Implicit declarations */

/* VIOLATION: Using undeclared function (C89 allowed, C99+ error) */
/*
void violation_dcl31(void) {
    int x = undeclared_function();  // Implicit int return assumed
}
*/

/* COMPLIANT: Declare or include header */
#include <stdlib.h>  /* For malloc, etc. */

/* Forward declaration if definition comes later */
int my_function(int x);

void
compliant_dcl31(void)
{
    int x = my_function(5);  /* Properly declared */
    (void)x;
}

int
my_function(int x)
{
    return x * 2;
}
```

### DCL37-C: Do not declare or define a reserved identifier

```c
/* DCL37-C: Reserved identifiers */

/*
 * Reserved identifiers include:
 * - Identifiers starting with underscore + uppercase letter
 * - Identifiers starting with double underscore
 * - Standard library names (if corresponding header included)
 */

/* VIOLATIONS */
/*
#define _MYHEADER_H      // Starts with _ and uppercase
int __my_variable;       // Double underscore
#define errno (-1)       // Redefining standard name (if <errno.h> included)
*/

/* COMPLIANT: Use project-specific prefix */
#define MYPROJECT_HEADER_H
int myproject_variable;
#define MYPROJECT_ERROR_CODE (-1)
```

### DCL40-C: Do not create incompatible declarations of the same function or object

```c
/* DCL40-C: Declaration compatibility */

/* VIOLATION: Incompatible declarations */
/* file1.c */
extern int shared_var;  /* Declared as int */

/* file2.c */
// extern long shared_var;  /* VIOLATION: Incompatible type */

/* VIOLATION: Function signature mismatch */
/* header.h */
int process(int x);

/* implementation.c */
// int process(int x, int y) { ... }  /* VIOLATION: Different signature */

/* COMPLIANT: Use headers to ensure consistency */
/* partsdb.h */
#ifndef PARTSDB_H
#define PARTSDB_H

typedef struct PartRecord {
    uint32_t id;
    char name[64];
    double price;
} PartRecord;

int partsdb_insert(const PartRecord *part);
int partsdb_find(uint32_t id, PartRecord *result);

#endif

/* partsdb.c - must include its own header */
#include "partsdb.h"

int
partsdb_insert(const PartRecord *part)
{
    /* Implementation matches header declaration */
    return 0;
}
```

## 12.3 Expressions (EXP) Rules

### EXP30-C: Do not depend on the order of evaluation for side effects

```c
/* EXP30-C: Order of evaluation */

/* VIOLATION: Multiple side effects with undefined order */
void
violation_exp30(void)
{
    int i = 0;

    /* Order of i++ is undefined */
    int arr[10] = {0};
    arr[i++] = i++;  /* UNDEFINED BEHAVIOR */

    /* Function argument order is unspecified */
    void process(int, int);
    process(i++, i++);  /* UNSPECIFIED BEHAVIOR */
}

/* COMPLIANT: Separate side effects with sequence points */
void
compliant_exp30(void)
{
    int i = 0;
    int arr[10] = {0};

    /* Separate the increments */
    int idx = i++;
    arr[idx] = i++;

    /* Or use intermediate variables */
    int a = i++;
    int b = i++;
    void process(int, int);
    process(a, b);
}
```

### EXP33-C: Do not read uninitialized memory

```c
/* EXP33-C: Uninitialized memory */
#include <stdlib.h>
#include <string.h>

/* VIOLATION: Using uninitialized local */
int
violation_exp33_local(void)
{
    int x;
    return x;  /* UNDEFINED BEHAVIOR: x is uninitialized */
}

/* VIOLATION: Using uninitialized heap memory */
void
violation_exp33_heap(void)
{
    int *arr = malloc(10 * sizeof(int));
    if (arr) {
        printf("%d\n", arr[0]);  /* UNDEFINED: malloc doesn't initialize */
        free(arr);
    }
}

/* COMPLIANT: Initialize variables and memory */
int
compliant_exp33_local(void)
{
    int x = 0;  /* Initialized */
    return x;
}

void
compliant_exp33_heap(void)
{
    /* Use calloc for zero-initialized memory */
    int *arr = calloc(10, sizeof(int));
    if (arr) {
        printf("%d\n", arr[0]);  /* Safe: calloc zeros memory */
        free(arr);
    }

    /* Or explicitly initialize after malloc */
    arr = malloc(10 * sizeof(int));
    if (arr) {
        memset(arr, 0, 10 * sizeof(int));
        printf("%d\n", arr[0]);
        free(arr);
    }
}
```

### EXP34-C: Do not dereference null pointers

```c
/* EXP34-C: Null pointer dereference */

/* VIOLATION: Dereferencing potentially null pointer */
void
violation_exp34(void)
{
    int *p = malloc(sizeof(int));
    *p = 42;  /* UNDEFINED if malloc returned NULL */
    free(p);
}

/* COMPLIANT: Check before dereferencing */
void
compliant_exp34(void)
{
    int *p = malloc(sizeof(int));
    if (p != NULL) {
        *p = 42;
        free(p);
    } else {
        /* Handle allocation failure */
        fprintf(stderr, "Memory allocation failed\n");
    }
}

/* Pattern for functions that may receive null */
int
process_data(const int *data, size_t count)
{
    /* Validate pointer before use */
    if (data == NULL) {
        return -1;  /* Error code */
    }

    int sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += data[i];
    }
    return sum;
}
```

### EXP45-C: Do not perform assignments in selection statements

```c
/* EXP45-C: Assignments in conditions */

/* VIOLATION: Assignment instead of comparison */
void
violation_exp45(int x)
{
    if (x = 5) {  /* BUG: assigns 5 to x, always true */
        printf("x is 5\n");
    }
}

/* COMPLIANT: Use comparison operator */
void
compliant_exp45(int x)
{
    if (x == 5) {  /* Correct: comparison */
        printf("x is 5\n");
    }
}

/* If assignment is intentional, make it clear */
void
intentional_assignment(FILE *fp)
{
    int c;
    while ((c = fgetc(fp)) != EOF) {  /* Intentional - wrapped in parens */
        putchar(c);
    }

    /* Even clearer: separate assignment */
    while (1) {
        c = fgetc(fp);
        if (c == EOF) {
            break;
        }
        putchar(c);
    }
}
```

## 12.4 Integers (INT) Rules

### INT30-C: Ensure that unsigned integer operations do not wrap

```c
/* INT30-C: Unsigned integer wrap */
#include <stdint.h>
#include <limits.h>

/* VIOLATION: Unsigned overflow (wraps silently) */
void
violation_int30(void)
{
    unsigned int a = UINT_MAX;
    unsigned int b = a + 1;  /* Wraps to 0 */
    printf("b = %u\n", b);   /* Prints 0 */
}

/* COMPLIANT: Check before operation */
unsigned int
safe_add_uint(unsigned int a, unsigned int b, bool *overflow)
{
    *overflow = (a > UINT_MAX - b);
    return a + b;  /* May wrap, but caller is informed */
}

/* Using compiler builtins (GCC/Clang) */
unsigned int
builtin_safe_add(unsigned int a, unsigned int b, bool *overflow)
{
    unsigned int result;
    *overflow = __builtin_add_overflow(a, b, &result);
    return result;
}

/* Pattern for array indexing */
void *
safe_array_access(void *base, size_t index, size_t element_size, size_t array_size)
{
    /* Check for multiplication overflow */
    if (element_size > 0 && index > SIZE_MAX / element_size) {
        return NULL;  /* Would overflow */
    }

    size_t offset = index * element_size;

    /* Check bounds */
    if (offset >= array_size) {
        return NULL;  /* Out of bounds */
    }

    return (char *)base + offset;
}
```

### INT31-C: Ensure that integer conversions do not result in lost or misinterpreted data

```c
/* INT31-C: Integer conversion issues */
#include <stdint.h>
#include <limits.h>

/* VIOLATION: Truncation without checking */
void
violation_int31(long long big)
{
    int small = (int)big;  /* May truncate if big > INT_MAX */
    printf("small = %d\n", small);
}

/* VIOLATION: Sign change */
void
violation_int31_sign(int negative)
{
    unsigned int u = (unsigned int)negative;  /* Sign may change meaning */
    /* If negative was -1, u is now UINT_MAX */
}

/* COMPLIANT: Validate before conversion */
int
safe_longlong_to_int(long long value, int *result)
{
    if (value < INT_MIN || value > INT_MAX) {
        return -1;  /* Out of range */
    }
    *result = (int)value;
    return 0;
}

int
safe_int_to_unsigned(int value, unsigned int *result)
{
    if (value < 0) {
        return -1;  /* Negative value */
    }
    *result = (unsigned int)value;
    return 0;
}

size_t
safe_int_to_size(int value)
{
    if (value < 0) {
        return 0;  /* Or SIZE_MAX as error indicator */
    }
    return (size_t)value;
}
```

### INT32-C: Ensure that operations on signed integers do not result in overflow

```c
/* INT32-C: Signed integer overflow (undefined behavior!) */
#include <limits.h>

/* VIOLATION: Signed overflow is UB */
void
violation_int32(void)
{
    int a = INT_MAX;
    int b = a + 1;  /* UNDEFINED BEHAVIOR */
    /* Compiler may assume this never happens and optimize incorrectly */
}

/* COMPLIANT: Check before operation */
int
safe_add_int(int a, int b, bool *overflow)
{
    if ((b > 0 && a > INT_MAX - b) ||
        (b < 0 && a < INT_MIN - b)) {
        *overflow = true;
        return 0;  /* Return value meaningless on overflow */
    }
    *overflow = false;
    return a + b;
}

int
safe_sub_int(int a, int b, bool *overflow)
{
    if ((b > 0 && a < INT_MIN + b) ||
        (b < 0 && a > INT_MAX + b)) {
        *overflow = true;
        return 0;
    }
    *overflow = false;
    return a - b;
}

int
safe_mul_int(int a, int b, bool *overflow)
{
    if (a > 0) {
        if (b > 0) {
            if (a > INT_MAX / b) { *overflow = true; return 0; }
        } else {
            if (b < INT_MIN / a) { *overflow = true; return 0; }
        }
    } else {
        if (b > 0) {
            if (a < INT_MIN / b) { *overflow = true; return 0; }
        } else {
            if (a != 0 && b < INT_MAX / a) { *overflow = true; return 0; }
        }
    }
    *overflow = false;
    return a * b;
}
```

## 12.5 Floating Point (FLP) Rules

### FLP30-C: Do not use floating-point variables as loop counters

```c
/* FLP30-C: Floating-point loop counters */

/* VIOLATION: Float as loop counter */
void
violation_flp30(void)
{
    /* May not execute expected number of times due to precision */
    for (float f = 0.0f; f < 1.0f; f += 0.1f) {
        printf("%f\n", f);
    }
    /* 0.1 cannot be exactly represented in binary floating-point */
}

/* COMPLIANT: Use integer counter */
void
compliant_flp30(void)
{
    /* Integer counter with float calculation */
    for (int i = 0; i < 10; i++) {
        float f = (float)i / 10.0f;
        printf("%f\n", f);
    }

    /* Or count in appropriate units */
    for (int cents = 0; cents < 100; cents += 10) {
        double dollars = cents / 100.0;
        printf("%.2f\n", dollars);
    }
}
```

### FLP32-C: Prevent or detect domain and range errors in math functions

```c
/* FLP32-C: Math function errors */
#include <math.h>
#include <errno.h>
#include <fenv.h>

/* VIOLATION: Unchecked math operation */
double
violation_flp32(double x)
{
    return sqrt(x);  /* UB if x < 0 */
}

/* COMPLIANT: Check domain before operation */
double
safe_sqrt(double x, bool *error)
{
    if (x < 0.0) {
        *error = true;
        return 0.0;
    }
    *error = false;
    return sqrt(x);
}

/* Using errno and floating-point exceptions */
double
safe_log(double x, bool *error)
{
    if (x <= 0.0) {
        *error = true;
        return 0.0;
    }

    errno = 0;
    feclearexcept(FE_ALL_EXCEPT);

    double result = log(x);

    if (errno != 0 || fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW)) {
        *error = true;
        return 0.0;
    }

    *error = false;
    return result;
}

/* Safe division */
double
safe_divide(double a, double b, bool *error)
{
    /* Check for divide by zero */
    if (b == 0.0) {
        *error = true;
        return 0.0;
    }

    /* Check for overflow */
    if (b > -1.0 && b < 1.0 && fabs(a) > fabs(b) * DBL_MAX) {
        *error = true;
        return 0.0;
    }

    *error = false;
    return a / b;
}
```

## 12.6 Arrays (ARR) Rules

### ARR30-C: Do not form or use out-of-bounds pointers or array subscripts

```c
/* ARR30-C: Array bounds */
#include <stddef.h>

/* VIOLATION: Out-of-bounds access */
void
violation_arr30(void)
{
    int arr[10];
    arr[10] = 0;  /* UNDEFINED: valid indices are 0-9 */

    int *p = arr + 11;  /* UNDEFINED: pointer past one-past-end */
}

/* COMPLIANT: Bounds checking */
int
safe_array_get(const int *arr, size_t size, size_t index, bool *error)
{
    if (index >= size) {
        *error = true;
        return 0;
    }
    *error = false;
    return arr[index];
}

void
safe_array_set(int *arr, size_t size, size_t index, int value, bool *error)
{
    if (index >= size) {
        *error = true;
        return;
    }
    *error = false;
    arr[index] = value;
}

/* Macro for array size */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Safe iteration */
void
safe_iterate(void)
{
    int arr[] = {1, 2, 3, 4, 5};
    size_t size = ARRAY_SIZE(arr);

    for (size_t i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
}
```

### ARR36-C: Do not subtract or compare two pointers that do not refer to the same array

```c
/* ARR36-C: Pointer comparison between arrays */

/* VIOLATION: Comparing pointers to different arrays */
void
violation_arr36(void)
{
    int arr1[10];
    int arr2[10];

    int *p1 = arr1;
    int *p2 = arr2;

    /* UNDEFINED BEHAVIOR */
    if (p1 < p2) {  /* Can't compare pointers to different arrays */
        printf("p1 < p2\n");
    }

    ptrdiff_t diff = p2 - p1;  /* UNDEFINED: different arrays */
}

/* COMPLIANT: Only compare pointers within same array */
void
compliant_arr36(void)
{
    int arr[10];
    int *begin = arr;
    int *end = arr + 10;  /* One past end is valid for comparison */
    int *p = arr + 5;

    /* These are valid */
    if (p >= begin && p < end) {
        printf("p is within bounds\n");
    }

    ptrdiff_t offset = p - begin;  /* Valid: same array */
    printf("Offset: %td\n", offset);
}

/* Pattern for search returning pointer */
int *
find_in_array(int *arr, size_t size, int target)
{
    int *end = arr + size;

    for (int *p = arr; p < end; p++) {  /* Valid comparison */
        if (*p == target) {
            return p;
        }
    }
    return NULL;  /* Not found */
}
```

### ARR38-C: Guarantee that library functions do not form invalid pointers

```c
/* ARR38-C: Library function pointer validity */
#include <string.h>

/* VIOLATION: Potential buffer overflow */
void
violation_arr38(char *dest, size_t dest_size, const char *src)
{
    strcpy(dest, src);  /* No bounds checking */
}

/* COMPLIANT: Use bounded functions */
void
compliant_arr38(char *dest, size_t dest_size, const char *src)
{
    /* strncpy doesn't guarantee null termination */
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';

    /* Or use strlcpy where available */
    /* strlcpy(dest, src, dest_size); */

    /* Or snprintf for formatted strings */
    snprintf(dest, dest_size, "%s", src);
}

/* Safe memcpy */
void *
safe_memcpy(void *dest, size_t dest_size,
            const void *src, size_t count)
{
    if (dest == NULL || src == NULL) {
        return NULL;
    }

    if (count > dest_size) {
        return NULL;  /* Would overflow */
    }

    return memcpy(dest, src, count);
}

/* Safe concatenation */
int
safe_strcat(char *dest, size_t dest_size, const char *src)
{
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    if (dest_len + src_len + 1 > dest_size) {
        return -1;  /* Would overflow */
    }

    memcpy(dest + dest_len, src, src_len + 1);
    return 0;
}
```

## 12.7 Strings (STR) Rules

### STR30-C: Do not attempt to modify string literals

```c
/* STR30-C: String literal modification */

/* VIOLATION: Modifying string literal */
void
violation_str30(void)
{
    char *str = "Hello";
    /* str[0] = 'h';  // UNDEFINED BEHAVIOR - may crash */
}

/* COMPLIANT: Use array for mutable strings */
void
compliant_str30(void)
{
    /* Array copy of literal - mutable */
    char str[] = "Hello";
    str[0] = 'h';  /* OK */

    /* Or use const to prevent accidental modification */
    const char *readonly = "World";
    /* readonly[0] = 'w';  // Compiler error */
}

/* Function taking string parameter */
void
process_string(char *str)  /* Implies modification possible */
{
    /* Should check if caller passed a literal... but can't reliably */
}

void
process_const_string(const char *str)  /* Promises not to modify */
{
    /* Safe to pass literals */
    printf("%s\n", str);
}
```

### STR31-C: Guarantee that storage for strings has sufficient space for character data and the null terminator

```c
/* STR31-C: String storage size */

/* VIOLATION: Forgetting null terminator */
void
violation_str31(void)
{
    char buf[5];
    strncpy(buf, "Hello", 5);  /* No room for null! */
    /* buf is not null-terminated */
    printf("%s\n", buf);  /* UNDEFINED: reads past buffer */
}

/* COMPLIANT: Always account for null terminator */
void
compliant_str31(void)
{
    /* Include space for null terminator */
    char buf[6];  /* 5 chars + null */
    strncpy(buf, "Hello", sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Or use appropriate size */
    char buf2[32];
    snprintf(buf2, sizeof(buf2), "Value: %d", 42);

    printf("%s\n", buf);
    printf("%s\n", buf2);
}

/* Safe string duplication */
char *
safe_strdup(const char *src, size_t max_len)
{
    if (src == NULL) {
        return NULL;
    }

    size_t len = strlen(src);
    if (len > max_len) {
        len = max_len;
    }

    char *dup = malloc(len + 1);  /* +1 for null */
    if (dup) {
        memcpy(dup, src, len);
        dup[len] = '\0';
    }
    return dup;
}
```

### STR32-C: Do not pass a non-null-terminated character sequence to a library function that expects a string

```c
/* STR32-C: Null termination requirements */

/* VIOLATION: Passing non-null-terminated buffer */
void
violation_str32(void)
{
    char buf[10];
    memset(buf, 'A', sizeof(buf));  /* No null terminator */

    /* These all expect null-terminated strings */
    /* printf("%s\n", buf);     // UNDEFINED */
    /* size_t len = strlen(buf); // UNDEFINED */
    /* strcpy(dest, buf);       // UNDEFINED */
}

/* COMPLIANT: Ensure null termination */
void
compliant_str32(void)
{
    char buf[10];
    memset(buf, 'A', sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';  /* Ensure null termination */

    printf("%s\n", buf);  /* Safe */
    size_t len = strlen(buf);  /* Safe */
    (void)len;
}

/* Reading fixed-width fields */
typedef struct FixedRecord {
    char name[20];  /* May not be null-terminated if full */
    char code[4];
} FixedRecord;

void
process_fixed_record(const FixedRecord *rec)
{
    /* Don't assume fields are null-terminated */
    char name_buf[21];  /* +1 for null */
    memcpy(name_buf, rec->name, sizeof(rec->name));
    name_buf[sizeof(rec->name)] = '\0';

    char code_buf[5];
    memcpy(code_buf, rec->code, sizeof(rec->code));
    code_buf[sizeof(rec->code)] = '\0';

    printf("Name: %s, Code: %s\n", name_buf, code_buf);
}
```

## 12.8 Memory Management (MEM) Rules

### MEM30-C: Do not access freed memory

```c
/* MEM30-C: Use after free */

/* VIOLATION: Using pointer after free */
void
violation_mem30(void)
{
    char *str = malloc(100);
    if (!str) return;

    strcpy(str, "Hello");
    free(str);

    /* UNDEFINED BEHAVIOR - str is dangling */
    /* printf("%s\n", str); */
    /* str[0] = 'X'; */
}

/* COMPLIANT: NULL pointer after free */
void
compliant_mem30(void)
{
    char *str = malloc(100);
    if (!str) return;

    strcpy(str, "Hello");
    printf("%s\n", str);

    free(str);
    str = NULL;  /* Prevents accidental reuse */

    /* Now this would crash immediately (easier to debug) */
    /* printf("%s\n", str); */
}

/* Pattern: Clear-on-free wrapper */
void
secure_free(void **ptr)
{
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

#define SAFE_FREE(ptr) secure_free((void **)&(ptr))
```

### MEM31-C: Free dynamically allocated memory when no longer needed

```c
/* MEM31-C: Memory leaks */

/* VIOLATION: Memory leak */
void
violation_mem31(void)
{
    char *str = malloc(100);
    if (!str) return;

    strcpy(str, "Hello");
    printf("%s\n", str);
    /* Missing: free(str) - memory leaked */
}

/* COMPLIANT: Free all allocated memory */
void
compliant_mem31(void)
{
    char *str = malloc(100);
    if (!str) return;

    strcpy(str, "Hello");
    printf("%s\n", str);
    free(str);
}

/* Pattern: Single point of exit for cleanup */
int
process_file(const char *filename)
{
    int result = -1;
    FILE *file = NULL;
    char *buffer = NULL;

    file = fopen(filename, "r");
    if (!file) goto cleanup;

    buffer = malloc(4096);
    if (!buffer) goto cleanup;

    /* ... processing ... */

    result = 0;  /* Success */

cleanup:
    free(buffer);  /* free(NULL) is safe */
    if (file) fclose(file);
    return result;
}
```

### MEM33-C: Allocate and copy structures containing a flexible array member dynamically

```c
/* MEM33-C: Flexible array member allocation */

typedef struct FlexibleBuffer {
    size_t size;
    char data[];  /* Flexible array member */
} FlexibleBuffer;

/* VIOLATION: Stack allocation with flexible array member */
void
violation_mem33(void)
{
    /* FlexibleBuffer buf;  // WRONG: data[] has no space */
    /* buf.data[0] = 'X';   // UNDEFINED: writing past struct */
}

/* COMPLIANT: Dynamic allocation */
FlexibleBuffer *
create_flexible_buffer(size_t size)
{
    /* Allocate struct + array space */
    FlexibleBuffer *buf = malloc(sizeof(FlexibleBuffer) + size);
    if (buf) {
        buf->size = size;
        memset(buf->data, 0, size);
    }
    return buf;
}

FlexibleBuffer *
copy_flexible_buffer(const FlexibleBuffer *src)
{
    if (!src) return NULL;

    size_t total_size = sizeof(FlexibleBuffer) + src->size;
    FlexibleBuffer *copy = malloc(total_size);
    if (copy) {
        memcpy(copy, src, total_size);
    }
    return copy;
}
```

### MEM35-C: Allocate sufficient memory for an object

```c
/* MEM35-C: Sufficient allocation */

typedef struct Part {
    uint32_t id;
    char *name;
    double price;
} Part;

/* VIOLATION: Wrong size calculation */
Part *
violation_mem35(void)
{
    /* Wrong: sizeof pointer, not struct */
    Part *part = malloc(sizeof(part));  /* Only 8 bytes! */
    return part;
}

/* COMPLIANT: Correct size */
Part *
compliant_mem35(void)
{
    /* Correct: sizeof(*pointer) gives the struct size */
    Part *part = malloc(sizeof(*part));  /* sizeof(Part) bytes */
    return part;
}

/* Array allocation */
Part *
allocate_parts(size_t count)
{
    /* Check for overflow */
    if (count > SIZE_MAX / sizeof(Part)) {
        return NULL;
    }

    Part *parts = malloc(count * sizeof(*parts));
    /* Or use calloc which checks internally: */
    /* Part *parts = calloc(count, sizeof(*parts)); */

    return parts;
}
```

## 12.9 Input/Output (FIO) Rules

### FIO30-C: Exclude user input from format strings

```c
/* FIO30-C: Format string vulnerabilities */

/* VIOLATION: User input as format string */
void
violation_fio30(const char *user_input)
{
    printf(user_input);  /* SECURITY VULNERABILITY! */
    /* If user_input contains %s, %x, %n, etc., bad things happen */
}

/* COMPLIANT: Use format specifier */
void
compliant_fio30(const char *user_input)
{
    printf("%s", user_input);  /* Safe: user input is argument, not format */

    /* Or use puts for simple strings */
    puts(user_input);

    /* Or fputs to not add newline */
    fputs(user_input, stdout);
}

/* For logging */
void
safe_log(const char *message)
{
    /* Never let external input be the format string */
    fprintf(stderr, "%s\n", message);
}

void
safe_log_formatted(const char *fmt, ...)
{
    /* Format string is from code, not user input */
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}
```

### FIO37-C: Do not assume that fgets() or fgetws() returns a nonempty string when successful

```c
/* FIO37-C: fgets return value */

/* VIOLATION: Assuming non-empty string */
void
violation_fio37(FILE *fp)
{
    char buf[100];
    if (fgets(buf, sizeof(buf), fp)) {
        buf[strlen(buf) - 1] = '\0';  /* BUG if line is empty! */
    }
}

/* COMPLIANT: Check for empty string */
void
compliant_fio37(FILE *fp)
{
    char buf[100];
    if (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }
    }
}

/* Better: Use a helper function */
char *
trim_newline(char *str)
{
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }

    return str;
}
```

## 12.10 Environment (ENV) Rules

### ENV32-C: All exit handlers must return normally

```c
/* ENV32-C: Exit handlers */
#include <stdlib.h>

static int g_cleanup_done = 0;

/* VIOLATION: Exit handler that doesn't return */
void
violation_env32_handler(void)
{
    if (!g_cleanup_done) {
        /* exit(1);  // UNDEFINED BEHAVIOR in exit handler */
        /* abort();  // Also problematic */
        /* longjmp(...);  // Also undefined */
    }
}

/* COMPLIANT: Exit handler returns normally */
void
compliant_env32_handler(void)
{
    if (!g_cleanup_done) {
        /* Perform cleanup */
        printf("Cleaning up...\n");
        g_cleanup_done = 1;
    }
    /* Return normally */
}

void
setup_handlers(void)
{
    if (atexit(compliant_env32_handler) != 0) {
        fprintf(stderr, "Failed to register exit handler\n");
    }
}
```

### ENV33-C: Do not call system()

```c
/* ENV33-C: Avoiding system() */

/* VIOLATION: Using system() with user input */
void
violation_env33(const char *filename)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ls -l %s", filename);
    system(cmd);  /* SECURITY VULNERABILITY if filename is untrusted */
    /* User could input: "; rm -rf /" */
}

/* COMPLIANT: Use exec family instead */
#include <unistd.h>
#include <sys/wait.h>

void
compliant_env33(const char *filename)
{
    pid_t pid = fork();

    if (pid == 0) {
        /* Child process */
        execlp("ls", "ls", "-l", filename, NULL);
        _exit(127);  /* exec failed */
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
    } else {
        /* Fork failed */
        perror("fork");
    }
}

/* For Windows, use CreateProcess instead of system() */
```

## 12.11 Review Questions

1. **PRE30-C and PRE31-C**: What are the dangers of macro side effects? Design a type-safe
   MAX macro using _Generic that evaluates each argument exactly once.

2. **DCL30-C**: Explain the storage duration categories in C (automatic, static, thread,
   allocated). For each, give an example of when returning a pointer to such an object
   is safe vs unsafe.

3. **EXP30-C**: The C standard defines "sequence points." What are they, and why do they
   matter for expressions like `a[i] = i++`? How does C11/C17 change the sequencing rules?

4. **INT32-C**: Signed integer overflow is undefined behavior, but unsigned overflow is
   defined (wraps). Why this asymmetry? What optimizations does the compiler enable
   by assuming signed overflow never happens?

5. **ARR30-C**: The standard allows forming a pointer to "one past the end" of an array.
   Why is this allowed, and what operations are legal with such a pointer?

6. **STR31-C**: Many security vulnerabilities involve null terminator issues. Design a
   "counted string" type that doesn't rely on null termination. What are the trade-offs?

7. **MEM30-C through MEM35-C**: Design a memory allocator wrapper that prevents:
   - Use after free
   - Double free
   - Memory leaks
   - Insufficient allocation
   What runtime overhead does this add?

8. **FIO30-C**: Format string vulnerabilities have been exploited in real systems. Beyond
   information disclosure, what can an attacker do with %n? How do modern compilers
   help detect these issues?

9. **ENV33-C**: Why is system() dangerous? The CERT standard recommends using exec family
   instead. What precautions are still needed when using exec?

10. **Practical application**: Audit the following function for CERT C violations:
    ```c
    void process(char *input) {
        char buf[10];
        sprintf(buf, "%s", input);
        int *p = malloc(sizeof(int));
        *p = strlen(buf);
        printf(input);
    }
    ```
    List all violations and fix them.

---

*Next: Part 13 - K&R Design Philosophy & C's Origins*
