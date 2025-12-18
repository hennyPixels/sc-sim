# Part 11: Pointer Mechanics & Dynamic Allocation

*Drawing from "Pointers on C" by Kenneth Reek and "The C Puzzle Book" by Alan Feuer*

## Introduction

Pointers are both C's greatest power and its greatest source of bugs. This part provides
a deep understanding of pointer mechanics—how they work at the machine level, common
patterns and pitfalls, and advanced techniques for dynamic memory management.

> "In C, pointers are first-class citizens... Understanding pointers is the key to
> understanding C." — Kenneth Reek

## 11.1 Pointer Fundamentals

### 11.1.1 What Pointers Really Are

```c
/* pointer_basics.c */

/*
 * A pointer is simply a variable that holds a memory address.
 * Understanding this fundamental concept is key to mastering C.
 *
 * On a 64-bit system:
 * - All pointer types are 8 bytes
 * - The type of pointer determines what operations are legal
 *   and how arithmetic works
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

void
demonstrate_pointer_basics(void)
{
    int x = 42;
    int *p = &x;  /* p holds the address of x */

    printf("Value of x:        %d\n", x);
    printf("Address of x:      %p\n", (void *)&x);
    printf("Value of p:        %p\n", (void *)p);
    printf("Value pointed to:  %d\n", *p);

    /*
     * Memory layout (assuming 64-bit, little-endian):
     *
     * Address        Value
     * 0x7fff1000     42 00 00 00  <- x (4 bytes)
     * 0x7fff1008     00 10 ff 7f  <- p (8 bytes, holds 0x7fff1000)
     *                00 00 00 00
     */

    /* Pointers have a type for a reason */
    char *cp = (char *)&x;
    printf("\nReading x byte-by-byte:\n");
    for (size_t i = 0; i < sizeof(int); i++) {
        printf("  Byte %zu: 0x%02x\n", i, (unsigned char)cp[i]);
    }

    /* The type affects arithmetic */
    int arr[4] = {10, 20, 30, 40};
    int *ip = arr;
    char *cp2 = (char *)arr;

    printf("\nPointer arithmetic:\n");
    printf("  ip:     %p (points to %d)\n", (void *)ip, *ip);
    printf("  ip + 1: %p (points to %d)\n", (void *)(ip + 1), *(ip + 1));
    printf("  cp2:     %p\n", (void *)cp2);
    printf("  cp2 + 1: %p\n", (void *)(cp2 + 1));
    /* Notice: ip+1 advances by sizeof(int)=4 bytes,
       cp2+1 advances by sizeof(char)=1 byte */
}

/*
 * Pointer sizes and alignment
 */
void
demonstrate_pointer_sizes(void)
{
    printf("\nPointer sizes:\n");
    printf("  char*:     %zu bytes\n", sizeof(char *));
    printf("  int*:      %zu bytes\n", sizeof(int *));
    printf("  double*:   %zu bytes\n", sizeof(double *));
    printf("  void*:     %zu bytes\n", sizeof(void *));
    printf("  func ptr:  %zu bytes\n", sizeof(void (*)(void)));

    /* All pointer types have the same size (on most systems) */
    /* But the TYPE matters for:
     * 1. Dereferencing (how many bytes to read/write)
     * 2. Arithmetic (how much to add per unit)
     * 3. Type safety (what operations are allowed)
     */
}
```

### 11.1.2 Pointer Indirection Levels

```c
/* indirection_levels.c */

/*
 * Pointers can point to pointers, creating multiple levels
 * of indirection. Each level adds flexibility but also complexity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
demonstrate_indirection(void)
{
    int value = 100;
    int *ptr = &value;      /* Single indirection */
    int **pptr = &ptr;      /* Double indirection */
    int ***ppptr = &pptr;   /* Triple indirection (rare, but legal) */

    printf("Value: %d\n", value);
    printf("*ptr:  %d\n", *ptr);
    printf("**pptr: %d\n", **pptr);
    printf("***ppptr: %d\n", ***ppptr);

    /* Modifying through different levels */
    ***ppptr = 200;
    printf("\nAfter ***ppptr = 200:\n");
    printf("Value: %d\n", value);
}

/*
 * Practical use: Array of strings
 * (char** is very common)
 */
void
demonstrate_string_array(void)
{
    /* Array of string pointers */
    const char *names[] = {
        "Alice",
        "Bob",
        "Charlie",
        "Diana"
    };
    size_t count = sizeof(names) / sizeof(names[0]);

    printf("\nArray of strings:\n");
    for (size_t i = 0; i < count; i++) {
        printf("  names[%zu] = \"%s\" at %p\n",
               i, names[i], (void *)names[i]);
    }

    /* Using pointer to array of pointers */
    const char **pp = names;
    printf("\nUsing char**:\n");
    printf("  *pp = \"%s\"\n", *pp);
    printf("  *(pp+1) = \"%s\"\n", *(pp + 1));
}

/*
 * Practical use: Function that modifies a pointer
 */
static void
allocate_buffer(char **buffer, size_t size)
{
    /* Need char** to modify the caller's pointer */
    *buffer = malloc(size);
    if (*buffer) {
        memset(*buffer, 0, size);
    }
}

static void
free_buffer(char **buffer)
{
    if (buffer && *buffer) {
        free(*buffer);
        *buffer = NULL;  /* Prevent use-after-free */
    }
}

void
demonstrate_output_parameters(void)
{
    char *buf = NULL;

    printf("\nOutput parameter pattern:\n");
    printf("Before: buf = %p\n", (void *)buf);

    allocate_buffer(&buf, 100);
    printf("After allocate: buf = %p\n", (void *)buf);

    if (buf) {
        strcpy(buf, "Hello, World!");
        printf("Content: %s\n", buf);
    }

    free_buffer(&buf);
    printf("After free: buf = %p\n", (void *)buf);
}
```

### 11.1.3 Pointer Arithmetic Deep Dive

```c
/* pointer_arithmetic.c */

/*
 * Pointer arithmetic is scaled by the size of the pointed-to type.
 * This is what makes array indexing work naturally.
 */

#include <stdio.h>
#include <stdint.h>

void
demonstrate_arithmetic(void)
{
    int arr[] = {10, 20, 30, 40, 50};
    int *p = arr;

    printf("Array base: %p\n", (void *)arr);
    printf("sizeof(int): %zu\n\n", sizeof(int));

    /* Incrementing pointers */
    printf("Pointer arithmetic:\n");
    for (int i = 0; i < 5; i++) {
        printf("  p + %d = %p, value = %d\n",
               i, (void *)(p + i), *(p + i));
    }

    /* Equivalence of array notation and pointer arithmetic */
    printf("\nEquivalent expressions:\n");
    printf("  arr[2] = %d\n", arr[2]);
    printf("  *(arr + 2) = %d\n", *(arr + 2));
    printf("  *(p + 2) = %d\n", *(p + 2));
    printf("  2[arr] = %d\n", 2[arr]);  /* Weird but valid! */

    /* Pointer subtraction */
    int *p1 = &arr[1];
    int *p2 = &arr[4];
    ptrdiff_t diff = p2 - p1;  /* Number of elements, not bytes */

    printf("\nPointer subtraction:\n");
    printf("  p1 points to arr[1]\n");
    printf("  p2 points to arr[4]\n");
    printf("  p2 - p1 = %td elements\n", diff);
    printf("  Byte difference: %td\n",
           (char *)p2 - (char *)p1);
}

/*
 * Walking through memory with different pointer types
 */
void
demonstrate_different_views(void)
{
    /* A buffer of bytes */
    uint8_t buffer[16] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };

    printf("\nViewing same memory with different pointer types:\n");

    /* View as bytes */
    uint8_t *bp = buffer;
    printf("As uint8_t: ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", bp[i]);
    }
    printf("\n");

    /* View as 16-bit words */
    uint16_t *wp = (uint16_t *)buffer;
    printf("As uint16_t: ");
    for (int i = 0; i < 8; i++) {
        printf("%04X ", wp[i]);
    }
    printf("\n");

    /* View as 32-bit words */
    uint32_t *dp = (uint32_t *)buffer;
    printf("As uint32_t: ");
    for (int i = 0; i < 4; i++) {
        printf("%08X ", dp[i]);
    }
    printf("\n");

    /* Note: Results depend on endianness! */
}

/*
 * Pointer comparison
 */
void
demonstrate_comparison(void)
{
    int arr[10] = {0};

    int *p1 = &arr[3];
    int *p2 = &arr[7];
    int *p3 = &arr[3];  /* Same as p1 */

    printf("\nPointer comparison:\n");
    printf("  p1 < p2:  %d (p1 points to earlier element)\n", p1 < p2);
    printf("  p1 == p3: %d (same address)\n", p1 == p3);
    printf("  p2 - p1:  %td elements apart\n", p2 - p1);

    /* WARNING: Comparing pointers to different arrays is undefined! */
    /* int other[10];
     * int *q = other;
     * if (p1 < q) { ... }  // UNDEFINED BEHAVIOR
     */
}
```

## 11.2 Arrays and Pointers

### 11.2.1 The Array-Pointer Relationship

```c
/* arrays_and_pointers.c */

/*
 * Arrays and pointers are NOT the same thing, but they are
 * closely related. Understanding the difference is crucial.
 */

#include <stdio.h>
#include <string.h>

/*
 * Key differences:
 *
 * Array:
 * - Has a fixed base address (cannot be reassigned)
 * - sizeof gives total array size
 * - Decays to pointer in most expressions
 *
 * Pointer:
 * - Can point anywhere
 * - sizeof gives pointer size (usually 8 bytes)
 * - Must be explicitly initialized
 */

void
demonstrate_array_pointer_difference(void)
{
    int arr[5] = {1, 2, 3, 4, 5};
    int *ptr = arr;

    printf("Array vs Pointer:\n");
    printf("  sizeof(arr): %zu (total array size)\n", sizeof(arr));
    printf("  sizeof(ptr): %zu (pointer size)\n", sizeof(ptr));

    /* Array name decays to pointer */
    printf("\nArray decay:\n");
    printf("  arr:  %p\n", (void *)arr);
    printf("  &arr: %p\n", (void *)&arr);
    printf("  ptr:  %p\n", (void *)ptr);

    /* But they have different types! */
    /* arr is int[5], &arr is int(*)[5], ptr is int* */

    /* This is why arr++ doesn't work but ptr++ does */
    /* arr++;  // ERROR: array name is not an lvalue */
    ptr++;     /* OK: ptr now points to arr[1] */

    printf("\nAfter ptr++:\n");
    printf("  *ptr = %d (was arr[1])\n", *ptr);
}

/*
 * Multi-dimensional arrays
 */
void
demonstrate_2d_arrays(void)
{
    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    printf("\n2D array layout (row-major):\n");
    printf("Matrix address: %p\n", (void *)matrix);

    /* Memory is contiguous */
    int *flat = (int *)matrix;
    printf("Flat view: ");
    for (int i = 0; i < 12; i++) {
        printf("%d ", flat[i]);
    }
    printf("\n");

    /* Row access */
    printf("\nRow pointers:\n");
    for (int i = 0; i < 3; i++) {
        printf("  matrix[%d] = %p\n", i, (void *)matrix[i]);
    }

    /* Pointer to array of 4 ints */
    int (*row_ptr)[4] = matrix;
    printf("\nUsing int(*)[4] pointer:\n");
    printf("  row_ptr[1][2] = %d\n", row_ptr[1][2]);

    /* Pointer to first element */
    int *elem_ptr = &matrix[0][0];
    printf("  elem_ptr[6] = %d (row 1, col 2)\n", elem_ptr[6]);
}

/*
 * Dynamic 2D arrays - two approaches
 */
static int **
create_jagged_array(size_t rows, size_t cols)
{
    /* Array of pointers to rows */
    int **arr = malloc(rows * sizeof(int *));
    if (!arr) return NULL;

    for (size_t i = 0; i < rows; i++) {
        arr[i] = malloc(cols * sizeof(int));
        if (!arr[i]) {
            /* Cleanup on failure */
            for (size_t j = 0; j < i; j++) {
                free(arr[j]);
            }
            free(arr);
            return NULL;
        }
    }

    return arr;
}

static void
free_jagged_array(int **arr, size_t rows)
{
    if (!arr) return;
    for (size_t i = 0; i < rows; i++) {
        free(arr[i]);
    }
    free(arr);
}

static int *
create_flat_2d_array(size_t rows, size_t cols)
{
    /* Single contiguous allocation */
    return malloc(rows * cols * sizeof(int));
}

/* Access element in flat 2D array */
#define FLAT_2D(arr, row, col, cols) ((arr)[(row) * (cols) + (col)])

void
demonstrate_dynamic_2d(void)
{
    const size_t rows = 3, cols = 4;

    printf("\nDynamic 2D arrays:\n");

    /* Jagged array (array of pointers) */
    int **jagged = create_jagged_array(rows, cols);
    if (jagged) {
        jagged[1][2] = 42;
        printf("  Jagged[1][2] = %d\n", jagged[1][2]);
        free_jagged_array(jagged, rows);
    }

    /* Flat array (single allocation) */
    int *flat = create_flat_2d_array(rows, cols);
    if (flat) {
        FLAT_2D(flat, 1, 2, cols) = 42;
        printf("  Flat[1][2] = %d\n", FLAT_2D(flat, 1, 2, cols));
        free(flat);
    }

    /*
     * Trade-offs:
     * Jagged: Each row can have different size, but more allocations
     * Flat:   Single allocation, cache-friendly, but fixed column count
     */
}
```

### 11.2.2 Array Parameters

```c
/* array_parameters.c */

/*
 * When you pass an array to a function, it decays to a pointer.
 * This is a common source of confusion.
 */

#include <stdio.h>

/* These three declarations are IDENTICAL for parameters */
void func1(int arr[10]);   /* Looks like array of 10, but it's a pointer */
void func2(int arr[]);     /* Explicitly unspecified size */
void func3(int *arr);      /* Pointer (most honest declaration) */

/* The array size in parameters is ignored by the compiler! */
void
show_parameter_behavior(int arr[1000])  /* Size is ignored! */
{
    printf("sizeof(arr) in function: %zu (pointer size)\n", sizeof(arr));
    /* This is always sizeof(int*), not sizeof(int[1000]) */
}

/* To preserve size information, pass it explicitly */
void
process_array(int *arr, size_t count)
{
    printf("Processing %zu elements\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("  arr[%zu] = %d\n", i, arr[i]);
    }
}

/* Or use a pointer to array (preserves size) */
void
process_fixed_array(int (*arr)[5])
{
    printf("Fixed array of 5 elements:\n");
    for (int i = 0; i < 5; i++) {
        printf("  (*arr)[%d] = %d\n", i, (*arr)[i]);
    }
}

/* For 2D arrays, second dimension must be specified */
void
process_2d_array(int arr[][4], size_t rows)
{
    /* Compiler needs column count for address calculation */
    printf("2D array with %zu rows of 4 columns:\n", rows);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < 4; j++) {
            printf("%4d", arr[i][j]);
        }
        printf("\n");
    }
}

/* VLA syntax for variable-size arrays (C99+) */
void
process_variable_2d(size_t rows, size_t cols, int arr[rows][cols])
{
    printf("Variable 2D array (%zux%zu):\n", rows, cols);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            printf("%4d", arr[i][j]);
        }
        printf("\n");
    }
}

void
demonstrate_array_parameters(void)
{
    int arr[] = {10, 20, 30, 40, 50};
    int arr2d[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    printf("Array parameter demonstration:\n\n");

    printf("sizeof(arr) in caller: %zu\n", sizeof(arr));
    show_parameter_behavior(arr);

    printf("\n");
    process_array(arr, 5);

    printf("\n");
    process_fixed_array(&arr);

    printf("\n");
    process_2d_array(arr2d, 3);
}
```

## 11.3 Function Pointers

### 11.3.1 Function Pointer Syntax

```c
/* function_pointers.c */

/*
 * Functions have addresses too. A function pointer stores
 * the address of executable code.
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple functions to point to */
static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }
static int divide(int a, int b) { return b != 0 ? a / b : 0; }

void
demonstrate_function_pointers(void)
{
    /* Function pointer declaration */
    int (*operation)(int, int);

    /* Function name IS its address (no & needed, but allowed) */
    operation = add;
    printf("add(3, 4) via pointer: %d\n", operation(3, 4));

    operation = &sub;  /* & is optional but explicit */
    printf("sub(7, 2) via pointer: %d\n", operation(7, 2));

    /* Explicit dereference (optional but shows what's happening) */
    operation = mul;
    printf("mul(5, 6) via pointer: %d\n", (*operation)(5, 6));
}

/*
 * Array of function pointers
 */
void
demonstrate_function_table(void)
{
    /* Array of function pointers */
    int (*ops[])(int, int) = {add, sub, mul, divide};
    const char *names[] = {"add", "sub", "mul", "div"};

    printf("\nFunction table:\n");
    int a = 10, b = 3;
    for (int i = 0; i < 4; i++) {
        printf("  %s(%d, %d) = %d\n", names[i], a, b, ops[i](a, b));
    }
}

/*
 * Typedef makes function pointer syntax cleaner
 */
typedef int (*BinaryOp)(int, int);

/* Calculator with pluggable operations */
typedef struct Calculator {
    BinaryOp add;
    BinaryOp sub;
    BinaryOp mul;
    BinaryOp div;
} Calculator;

static Calculator
make_calculator(void)
{
    return (Calculator){
        .add = add,
        .sub = sub,
        .mul = mul,
        .div = divide
    };
}

void
demonstrate_calculator(void)
{
    Calculator calc = make_calculator();

    printf("\nCalculator object:\n");
    printf("  10 + 5 = %d\n", calc.add(10, 5));
    printf("  10 - 5 = %d\n", calc.sub(10, 5));
    printf("  10 * 5 = %d\n", calc.mul(10, 5));
    printf("  10 / 5 = %d\n", calc.div(10, 5));
}

/*
 * Callback pattern
 */
typedef void (*EventCallback)(void *data, int event_id);

typedef struct EventHandler {
    EventCallback callback;
    void         *user_data;
} EventHandler;

static void
trigger_event(EventHandler *handler, int event_id)
{
    if (handler && handler->callback) {
        handler->callback(handler->user_data, event_id);
    }
}

static void
my_callback(void *data, int event_id)
{
    const char *name = (const char *)data;
    printf("Event %d received by '%s'\n", event_id, name);
}

void
demonstrate_callbacks(void)
{
    printf("\nCallback pattern:\n");

    EventHandler handler = {
        .callback = my_callback,
        .user_data = "MyHandler"
    };

    trigger_event(&handler, 42);
}

/*
 * qsort comparison function
 */
static int
compare_ints(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);  /* Safe subtraction */
}

static int
compare_ints_desc(const void *a, const void *b)
{
    return compare_ints(b, a);  /* Reverse order */
}

void
demonstrate_qsort(void)
{
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    size_t count = sizeof(arr) / sizeof(arr[0]);

    printf("\nUsing qsort with function pointer:\n");

    printf("Original: ");
    for (size_t i = 0; i < count; i++) printf("%d ", arr[i]);
    printf("\n");

    qsort(arr, count, sizeof(int), compare_ints);
    printf("Ascending: ");
    for (size_t i = 0; i < count; i++) printf("%d ", arr[i]);
    printf("\n");

    qsort(arr, count, sizeof(int), compare_ints_desc);
    printf("Descending: ");
    for (size_t i = 0; i < count; i++) printf("%d ", arr[i]);
    printf("\n");
}
```

### 11.3.2 Complex Declarations

```c
/* complex_declarations.c */

/*
 * C's declaration syntax can be confusing. The "spiral rule"
 * and "clockwise spiral" help decode complex declarations.
 */

#include <stdio.h>

/*
 * Reading declarations:
 * 1. Start at the identifier
 * 2. Go right until you hit a ) or end
 * 3. Go left until you hit a ( or end
 * 4. Repeat
 */

/* Simple examples */
int *p;                    /* p is pointer to int */
int arr[10];              /* arr is array of 10 ints */
int *arr2[10];            /* arr2 is array of 10 pointers to int */
int (*arr3)[10];          /* arr3 is pointer to array of 10 ints */
int (*fp)(int);           /* fp is pointer to function taking int, returning int */

/* More complex */
int *(*fparr[5])(int);    /* fparr is array of 5 pointers to functions
                             taking int and returning pointer to int */

int (*(*fp2)(int, int))[10];  /* fp2 is pointer to function taking (int, int)
                                  and returning pointer to array of 10 ints */

/* Function returning pointer to function */
int (*get_operation(char op))(int, int)
{
    /* Returns a function pointer based on op */
    extern int add(int, int);
    extern int sub(int, int);

    switch (op) {
    case '+': return add;
    case '-': return sub;
    default:  return NULL;
    }
}

/*
 * Using typedef to simplify
 */

/* Instead of: int (*fp)(int, int) */
typedef int (*BinaryFunc)(int, int);

/* Instead of: int (*arr[5])(int, int) */
typedef BinaryFunc BinaryFuncArray[5];

/* Instead of: int (*(*fp)(int))(int, int) */
typedef BinaryFunc (*FuncReturningFunc)(int);

/* Signal handler (famous example) */
/* void (*signal(int sig, void (*func)(int)))(int); */

/* Much clearer with typedef: */
typedef void (*SignalHandler)(int);
/* SignalHandler signal(int sig, SignalHandler func); */

/*
 * Const with pointers
 */
void
const_pointer_examples(void)
{
    int x = 10;
    int y = 20;

    /* Read right to left */
    const int *p1 = &x;       /* Pointer to const int (can't modify *p1) */
    int const *p2 = &x;       /* Same as above */
    int *const p3 = &x;       /* Const pointer to int (can't modify p3) */
    const int *const p4 = &x; /* Const pointer to const int */

    /* What's allowed? */
    /* *p1 = 5;  // Error: can't modify through p1 */
    p1 = &y;     /* OK: p1 can point elsewhere */

    /* *p3 = 5;  // OK: can modify through p3 */
    /* p3 = &y;  // Error: can't change p3 */

    /* *p4 = 5;  // Error */
    /* p4 = &y;  // Error */

    (void)p2;
    (void)p4;
}
```

## 11.4 Dynamic Memory Management

### 11.4.1 Allocation Patterns

```c
/* dynamic_memory.c */

/*
 * Dynamic memory management in C requires careful attention
 * to avoid leaks, double-frees, and use-after-free bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * Basic allocation patterns
 */

/* Pattern 1: Simple allocation with error checking */
static int *
allocate_int_array(size_t count)
{
    if (count == 0) {
        return NULL;
    }

    /* Check for overflow */
    if (count > SIZE_MAX / sizeof(int)) {
        return NULL;
    }

    int *arr = malloc(count * sizeof(int));
    /* Note: malloc may return NULL on failure */
    return arr;
}

/* Pattern 2: Allocation with initialization */
static int *
allocate_zeroed_array(size_t count)
{
    if (count == 0 || count > SIZE_MAX / sizeof(int)) {
        return NULL;
    }

    /* calloc zeros memory AND checks for overflow internally */
    return calloc(count, sizeof(int));
}

/* Pattern 3: Flexible array member (C99) */
typedef struct VariableSizeData {
    size_t  length;
    uint8_t data[];  /* Flexible array member - must be last */
} VariableSizeData;

static VariableSizeData *
create_variable_data(size_t length)
{
    /* Allocate struct + array in one block */
    VariableSizeData *vsd = malloc(sizeof(VariableSizeData) + length);
    if (vsd) {
        vsd->length = length;
        memset(vsd->data, 0, length);
    }
    return vsd;
}

/* Pattern 4: Reallocation with exponential growth */
typedef struct DynamicBuffer {
    char   *data;
    size_t  length;
    size_t  capacity;
} DynamicBuffer;

static bool
buffer_ensure_capacity(DynamicBuffer *buf, size_t required)
{
    if (required <= buf->capacity) {
        return true;
    }

    /* Grow by doubling, with minimum size */
    size_t new_capacity = buf->capacity ? buf->capacity * 2 : 16;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2) {
            return false;  /* Would overflow */
        }
        new_capacity *= 2;
    }

    char *new_data = realloc(buf->data, new_capacity);
    if (!new_data) {
        return false;
    }

    buf->data = new_data;
    buf->capacity = new_capacity;
    return true;
}

static bool
buffer_append(DynamicBuffer *buf, const char *str)
{
    size_t str_len = strlen(str);
    size_t required = buf->length + str_len + 1;

    if (!buffer_ensure_capacity(buf, required)) {
        return false;
    }

    memcpy(buf->data + buf->length, str, str_len + 1);
    buf->length += str_len;
    return true;
}

/*
 * Memory ownership patterns
 */

typedef struct Part {
    uint32_t id;
    char    *name;        /* Owned: must be freed */
    char    *description; /* Owned: must be freed */
} Part;

/* Factory function - transfers ownership to caller */
static Part *
part_create(uint32_t id, const char *name, const char *desc)
{
    Part *part = malloc(sizeof(Part));
    if (!part) {
        return NULL;
    }

    part->id = id;
    part->name = strdup(name);
    part->description = desc ? strdup(desc) : NULL;

    if (!part->name || (desc && !part->description)) {
        free(part->name);
        free(part->description);
        free(part);
        return NULL;
    }

    return part;
}

/* Destructor - caller must call this */
static void
part_destroy(Part *part)
{
    if (!part) {
        return;
    }
    free(part->name);
    free(part->description);
    free(part);
}

/* Deep copy - creates independent copy */
static Part *
part_clone(const Part *original)
{
    if (!original) {
        return NULL;
    }
    return part_create(original->id, original->name, original->description);
}

/* Move semantics - transfers ownership */
static Part *
part_move(Part **src)
{
    if (!src || !*src) {
        return NULL;
    }
    Part *moved = *src;
    *src = NULL;  /* Source no longer owns */
    return moved;
}
```

### 11.4.2 Memory Debugging Techniques

```c
/* memory_debug.c */

/*
 * Techniques from Pointers on C for finding memory bugs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Technique 1: Poison patterns
 */
#define UNINIT_PATTERN   0xCD  /* Uninitialized memory */
#define FREED_PATTERN    0xDD  /* Freed memory */
#define GUARD_PATTERN    0xFD  /* Guard bytes */

/* Debug allocator with poison patterns */
static void *
debug_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr) {
        memset(ptr, UNINIT_PATTERN, size);
    }
    return ptr;
}

static void
debug_free(void *ptr, size_t size)
{
    if (ptr) {
        memset(ptr, FREED_PATTERN, size);  /* Poison before free */
        free(ptr);
    }
}

/*
 * Technique 2: Guard bytes (canaries)
 */
#define GUARD_SIZE 4

typedef struct GuardedAllocation {
    size_t size;
    uint8_t front_guard[GUARD_SIZE];
    /* User data follows */
    /* uint8_t back_guard[GUARD_SIZE] at end */
} GuardedHeader;

static void *
guarded_malloc(size_t size)
{
    size_t total = sizeof(GuardedHeader) + size + GUARD_SIZE;
    GuardedHeader *header = malloc(total);

    if (!header) {
        return NULL;
    }

    header->size = size;
    memset(header->front_guard, GUARD_PATTERN, GUARD_SIZE);

    uint8_t *user_data = (uint8_t *)(header + 1);
    uint8_t *back_guard = user_data + size;
    memset(back_guard, GUARD_PATTERN, GUARD_SIZE);

    return user_data;
}

static bool
guarded_check(void *ptr)
{
    if (!ptr) {
        return true;
    }

    GuardedHeader *header = (GuardedHeader *)ptr - 1;

    /* Check front guard */
    for (size_t i = 0; i < GUARD_SIZE; i++) {
        if (header->front_guard[i] != GUARD_PATTERN) {
            fprintf(stderr, "MEMORY CORRUPTION: Front guard violated at %p\n", ptr);
            return false;
        }
    }

    /* Check back guard */
    uint8_t *back_guard = (uint8_t *)ptr + header->size;
    for (size_t i = 0; i < GUARD_SIZE; i++) {
        if (back_guard[i] != GUARD_PATTERN) {
            fprintf(stderr, "MEMORY CORRUPTION: Back guard violated at %p\n", ptr);
            return false;
        }
    }

    return true;
}

static void
guarded_free(void *ptr)
{
    if (!ptr) {
        return;
    }

    if (!guarded_check(ptr)) {
        fprintf(stderr, "Freeing corrupted block!\n");
    }

    GuardedHeader *header = (GuardedHeader *)ptr - 1;
    size_t total = sizeof(GuardedHeader) + header->size + GUARD_SIZE;
    memset(header, FREED_PATTERN, total);
    free(header);
}

/*
 * Technique 3: Leak detection via tracking
 */

typedef struct AllocNode {
    void             *ptr;
    size_t            size;
    const char       *file;
    int               line;
    struct AllocNode *next;
} AllocNode;

static AllocNode *g_alloc_list = NULL;

static void *
tracked_malloc(size_t size, const char *file, int line)
{
    void *ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }

    AllocNode *node = malloc(sizeof(AllocNode));
    if (!node) {
        free(ptr);
        return NULL;
    }

    node->ptr = ptr;
    node->size = size;
    node->file = file;
    node->line = line;
    node->next = g_alloc_list;
    g_alloc_list = node;

    return ptr;
}

static void
tracked_free(void *ptr)
{
    if (!ptr) {
        return;
    }

    AllocNode **pp = &g_alloc_list;
    while (*pp) {
        if ((*pp)->ptr == ptr) {
            AllocNode *node = *pp;
            *pp = node->next;
            free(node);
            free(ptr);
            return;
        }
        pp = &(*pp)->next;
    }

    fprintf(stderr, "WARNING: Freeing untracked pointer %p\n", ptr);
    free(ptr);  /* Free anyway, might be from external code */
}

static void
tracked_report_leaks(void)
{
    if (!g_alloc_list) {
        printf("No memory leaks detected.\n");
        return;
    }

    printf("\n*** MEMORY LEAKS DETECTED ***\n");
    size_t total = 0;
    int count = 0;

    for (AllocNode *node = g_alloc_list; node; node = node->next) {
        printf("  Leak: %zu bytes at %p (%s:%d)\n",
               node->size, node->ptr, node->file, node->line);
        total += node->size;
        count++;
    }

    printf("Total: %d allocations, %zu bytes\n", count, total);
}

#ifdef DEBUG_MEMORY
    #define malloc(size) tracked_malloc(size, __FILE__, __LINE__)
    #define free(ptr) tracked_free(ptr)
#endif
```

## 11.5 Common Pointer Bugs

### 11.5.1 Classic Mistakes

```c
/* pointer_bugs.c */

/*
 * The C Puzzle Book catalogs many subtle pointer bugs.
 * Here are the most common ones with explanations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Bug 1: Uninitialized pointer
 */
void
bug_uninitialized_pointer(void)
{
    int *p;  /* Uninitialized - contains garbage */
    /* *p = 42;  // UNDEFINED BEHAVIOR - could crash, corrupt memory, or "work" */

    /* Fix: Initialize pointers */
    int *q = NULL;
    int value = 42;
    int *r = &value;
    (void)q;
    (void)r;
}

/*
 * Bug 2: Dangling pointer (use after free)
 */
void
bug_dangling_pointer(void)
{
    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    /* *p = 100;  // UNDEFINED BEHAVIOR - p is dangling */

    /* Fix: NULL after free */
    p = NULL;
    /* Now accessing *p would crash immediately (easier to debug) */
    (void)p;
}

/*
 * Bug 3: Returning address of local variable
 */
int *
bug_return_local_address(void)
{
    int local = 42;
    return &local;  /* BUG: local is gone after function returns */
}

int *
fix_return_heap_address(void)
{
    int *heap = malloc(sizeof(int));
    if (heap) {
        *heap = 42;
    }
    return heap;  /* OK: caller owns and must free */
}

/*
 * Bug 4: Buffer overrun
 */
void
bug_buffer_overrun(void)
{
    char buf[10];
    /* strcpy(buf, "This string is way too long!");  // BUFFER OVERRUN */

    /* Fix: Use bounded copy */
    strncpy(buf, "This string is way too long!", sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
}

/*
 * Bug 5: Off-by-one in array access
 */
void
bug_off_by_one(void)
{
    int arr[10];

    /* Bug: Accessing arr[10] (11th element) */
    /* for (int i = 0; i <= 10; i++) arr[i] = i;  // OFF BY ONE */

    /* Fix: Use < not <= */
    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }
}

/*
 * Bug 6: sizeof applied to pointer instead of array
 */
void
bug_sizeof_pointer(int arr[])  /* arr is really int* */
{
    /* Bug: This gives pointer size, not array size */
    /* size_t count = sizeof(arr) / sizeof(arr[0]);  // WRONG */

    /* There's no fix here - must pass size separately */
}

void
fix_pass_size(int *arr, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        printf("%d ", arr[i]);
    }
}

/*
 * Bug 7: Mismatched allocation/deallocation
 */
void
bug_mismatched_alloc(void)
{
    /* Bug: malloc/delete or new/free mismatch (in C++) */
    /* Also applies to: stack vs heap confusion */

    int stack_var = 42;
    /* free(&stack_var);  // CATASTROPHIC BUG */

    int *heap_var = malloc(sizeof(int));
    free(heap_var);
    /* delete heap_var;  // Wrong in C++ if allocated with malloc */
}

/*
 * Bug 8: Pointer arithmetic on void*
 */
void
bug_void_pointer_arithmetic(void)
{
    int arr[] = {1, 2, 3, 4, 5};
    void *vp = arr;

    /* void *vp2 = vp + 1;  // BUG: Can't do arithmetic on void* (size unknown) */

    /* Fix: Cast to specific type first */
    int *ip = (int *)vp;
    int *ip2 = ip + 1;  /* OK: advances by sizeof(int) */
    (void)ip2;

    /* Or cast to char* for byte arithmetic */
    char *cp = (char *)vp;
    char *cp2 = cp + sizeof(int);  /* Manual stride */
    (void)cp2;
}

/*
 * Bug 9: Modifying string literal
 */
void
bug_modify_string_literal(void)
{
    char *str = "Hello";  /* Points to read-only memory */
    /* str[0] = 'h';  // UNDEFINED BEHAVIOR (may crash) */

    /* Fix: Use array instead of pointer */
    char arr[] = "Hello";  /* Creates writable copy */
    arr[0] = 'h';  /* OK */
}

/*
 * Bug 10: Aliasing violation
 */
void
bug_aliasing(void)
{
    int x = 0x12345678;

    /* Bug: Accessing int through char* is OK, but... */
    /* Accessing int through float* violates strict aliasing */
    /* float *fp = (float *)&x; */
    /* float f = *fp;  // UNDEFINED BEHAVIOR */

    /* Fix: Use union for type punning */
    union {
        int i;
        float f;
    } converter;
    converter.i = x;
    float f = converter.f;  /* OK in C99+ */
    (void)f;

    /* Or use memcpy */
    float f2;
    memcpy(&f2, &x, sizeof(f2));
    (void)f2;
}
```

## 11.6 Advanced Pointer Techniques

### 11.6.1 Generic Data Structures

```c
/* generic_structures.c */

/*
 * Using void* to create generic/reusable data structures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Generic linked list
 */
typedef struct GenericNode {
    void               *data;
    struct GenericNode *next;
} GenericNode;

typedef struct GenericList {
    GenericNode *head;
    GenericNode *tail;
    size_t       count;
    size_t       element_size;
    void       (*free_element)(void *);
} GenericList;

static GenericList *
list_create(size_t element_size, void (*free_func)(void *))
{
    GenericList *list = calloc(1, sizeof(GenericList));
    if (list) {
        list->element_size = element_size;
        list->free_element = free_func;
    }
    return list;
}

static bool
list_append(GenericList *list, const void *data)
{
    if (!list || !data) {
        return false;
    }

    GenericNode *node = malloc(sizeof(GenericNode));
    if (!node) {
        return false;
    }

    node->data = malloc(list->element_size);
    if (!node->data) {
        free(node);
        return false;
    }

    memcpy(node->data, data, list->element_size);
    node->next = NULL;

    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    list->count++;

    return true;
}

static void *
list_get(const GenericList *list, size_t index)
{
    if (!list || index >= list->count) {
        return NULL;
    }

    GenericNode *node = list->head;
    for (size_t i = 0; i < index; i++) {
        node = node->next;
    }

    return node->data;
}

static void
list_destroy(GenericList *list)
{
    if (!list) {
        return;
    }

    GenericNode *node = list->head;
    while (node) {
        GenericNode *next = node->next;
        if (list->free_element) {
            list->free_element(node->data);
        }
        free(node->data);
        free(node);
        node = next;
    }

    free(list);
}

/* Usage with integers */
void
demonstrate_int_list(void)
{
    GenericList *list = list_create(sizeof(int), NULL);

    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        list_append(list, &values[i]);
    }

    printf("Integer list: ");
    for (size_t i = 0; i < list->count; i++) {
        int *value = list_get(list, i);
        printf("%d ", *value);
    }
    printf("\n");

    list_destroy(list);
}

/* Usage with structs */
typedef struct Person {
    char name[32];
    int  age;
} Person;

void
demonstrate_struct_list(void)
{
    GenericList *list = list_create(sizeof(Person), NULL);

    Person people[] = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35}
    };

    for (int i = 0; i < 3; i++) {
        list_append(list, &people[i]);
    }

    printf("Person list:\n");
    for (size_t i = 0; i < list->count; i++) {
        Person *p = list_get(list, i);
        printf("  %s, age %d\n", p->name, p->age);
    }

    list_destroy(list);
}

/*
 * Generic hash table (simplified)
 */
typedef struct HashEntry {
    void              *key;
    void              *value;
    struct HashEntry  *next;
} HashEntry;

typedef struct HashTable {
    HashEntry **buckets;
    size_t      bucket_count;
    size_t      key_size;
    size_t      value_size;
    size_t      count;
    size_t    (*hash_func)(const void *key, size_t key_size);
    bool      (*key_eq)(const void *a, const void *b, size_t size);
} HashTable;

/* Default hash function */
static size_t
default_hash(const void *key, size_t key_size)
{
    const uint8_t *bytes = key;
    size_t hash = 5381;
    for (size_t i = 0; i < key_size; i++) {
        hash = hash * 33 + bytes[i];
    }
    return hash;
}

/* Default equality */
static bool
default_eq(const void *a, const void *b, size_t size)
{
    return memcmp(a, b, size) == 0;
}

static HashTable *
hashtable_create(size_t key_size, size_t value_size, size_t bucket_count)
{
    HashTable *ht = calloc(1, sizeof(HashTable));
    if (!ht) return NULL;

    ht->buckets = calloc(bucket_count, sizeof(HashEntry *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->bucket_count = bucket_count;
    ht->key_size = key_size;
    ht->value_size = value_size;
    ht->hash_func = default_hash;
    ht->key_eq = default_eq;

    return ht;
}

static bool
hashtable_put(HashTable *ht, const void *key, const void *value)
{
    if (!ht || !key || !value) return false;

    size_t bucket = ht->hash_func(key, ht->key_size) % ht->bucket_count;

    /* Check for existing key */
    for (HashEntry *e = ht->buckets[bucket]; e; e = e->next) {
        if (ht->key_eq(e->key, key, ht->key_size)) {
            memcpy(e->value, value, ht->value_size);
            return true;
        }
    }

    /* Create new entry */
    HashEntry *entry = malloc(sizeof(HashEntry));
    if (!entry) return false;

    entry->key = malloc(ht->key_size);
    entry->value = malloc(ht->value_size);
    if (!entry->key || !entry->value) {
        free(entry->key);
        free(entry->value);
        free(entry);
        return false;
    }

    memcpy(entry->key, key, ht->key_size);
    memcpy(entry->value, value, ht->value_size);
    entry->next = ht->buckets[bucket];
    ht->buckets[bucket] = entry;
    ht->count++;

    return true;
}

static void *
hashtable_get(const HashTable *ht, const void *key)
{
    if (!ht || !key) return NULL;

    size_t bucket = ht->hash_func(key, ht->key_size) % ht->bucket_count;

    for (HashEntry *e = ht->buckets[bucket]; e; e = e->next) {
        if (ht->key_eq(e->key, key, ht->key_size)) {
            return e->value;
        }
    }

    return NULL;
}

static void
hashtable_destroy(HashTable *ht)
{
    if (!ht) return;

    for (size_t i = 0; i < ht->bucket_count; i++) {
        HashEntry *e = ht->buckets[i];
        while (e) {
            HashEntry *next = e->next;
            free(e->key);
            free(e->value);
            free(e);
            e = next;
        }
    }

    free(ht->buckets);
    free(ht);
}
```

## 11.7 Review Questions

### From "Pointers on C" (Reek)

1. **Pointer fundamentals**: Explain the difference between `int *p = &x` and
   `int *p; *p = x`. Why does the second cause undefined behavior? Draw a
   memory diagram showing what each statement does.

2. **Array decay**: When does an array name NOT decay to a pointer? List all
   the contexts where sizeof(array) gives the full array size rather than
   pointer size.

3. **Pointer arithmetic**: Given `int arr[10]` and `int *p = arr`, what is
   the numeric difference between `(char*)(p+3)` and `(char*)p`? Explain
   why pointer arithmetic is scaled.

4. **Multi-level indirection**: Write a function that takes a `char***` and
   explain a practical use case for triple indirection. Why might you need
   this in a real program?

5. **Function pointers**: Declare a function pointer to a function that takes
   a function pointer as an argument and returns a function pointer. Then
   simplify using typedef.

### From "The C Puzzle Book" (Feuer)

6. **Evaluation order**: What does the following print and why?
   ```c
   int a[] = {0, 1, 2, 3, 4};
   int *p = a;
   printf("%d %d\n", *p++, *p++);
   ```
   (Hint: This is a trick question about undefined behavior.)

7. **Pointer aliasing**: Explain why the compiler might generate incorrect
   code for this function and how to fix it:
   ```c
   void add(int *result, int *a, int *b) {
       *result = *a + *b;
   }
   ```

8. **Memory layout**: Draw the memory layout for:
   ```c
   struct X {
       char a;
       int *b;
       char c[3];
       double d;
   };
   ```
   Show padding and alignment on a 64-bit system.

### Practical Application

9. **PartsDB design**: Design a generic container that can store Part records
   with O(1) lookup by ID and O(log n) lookup by name. What pointer types
   would you need? How would you handle memory ownership?

10. **Memory debugging**: Implement a wrapper around malloc/free that detects
    double-free errors with zero false positives. What data structures would
    you need? What are the performance trade-offs?

---

*Next: Part 12 - SEI CERT C Coding Standard*
