# Part 13: K&R Design Philosophy & C's Origins

*Drawing from "The C Programming Language" by Kernighan & Ritchie*

## Introduction

"The C Programming Language" (K&R) is not just a programming book—it's a masterclass
in language design, technical writing, and programming style. Understanding the
philosophy behind C helps you write better code and appreciate why the language
works the way it does.

> "C is a general-purpose programming language which features economy of expression,
> modern control flow and data structures, and a rich set of operators." — K&R

## 13.1 The Design Philosophy

### 13.1.1 Trust the Programmer

```c
/* kr_philosophy.c */

/*
 * C's fundamental design principle: Trust the programmer.
 *
 * This manifests in several ways:
 * 1. No array bounds checking
 * 2. No automatic memory management
 * 3. Free type casting
 * 4. Direct hardware access
 * 5. Minimal runtime
 *
 * The consequence: great power, great responsibility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * C trusts you to check bounds
 */
void
demonstrate_trust_bounds(void)
{
    int arr[10];

    /* C doesn't stop you from doing this */
    /* arr[100] = 42;  // No bounds check, may corrupt memory */

    /* Modern C idiom: explicit bounds checking when needed */
    size_t index = 5;
    if (index < 10) {
        arr[index] = 42;  /* Safe because we checked */
    }
}

/*
 * C trusts you to manage memory
 */
char *
demonstrate_trust_memory(const char *input)
{
    /* C doesn't track allocations for you */
    size_t len = strlen(input);
    char *copy = malloc(len + 1);

    if (copy) {
        strcpy(copy, input);
        /* Caller is now responsible for freeing this memory */
    }

    return copy;
}

/*
 * C trusts you with type casting
 */
void
demonstrate_trust_types(void)
{
    int x = 0x12345678;

    /* C lets you view any memory as any type */
    char *bytes = (char *)&x;

    printf("Integer %x as bytes: ", x);
    for (size_t i = 0; i < sizeof(int); i++) {
        printf("%02x ", (unsigned char)bytes[i]);
    }
    printf("\n");

    /* This power enables low-level programming */
    /* But also enables type confusion bugs */
}

/*
 * The benefit: minimal overhead
 */

/*
 * A C program can be as efficient as hand-written assembly.
 * This is why C is used for:
 * - Operating system kernels
 * - Embedded systems
 * - Performance-critical applications
 * - System utilities
 */
```

### 13.1.2 Small is Beautiful

```c
/* small_language.c */

/*
 * K&R Philosophy: Keep the language small.
 *
 * C has relatively few keywords (~32 in C99):
 * auto, break, case, char, const, continue, default, do, double,
 * else, enum, extern, float, for, goto, if, int, long, register,
 * return, short, signed, sizeof, static, struct, switch, typedef,
 * union, unsigned, void, volatile, while
 *
 * C11 added: _Alignas, _Alignof, _Atomic, _Bool, _Complex, _Generic,
 *            _Imaginary, _Noreturn, _Static_assert, _Thread_local
 *
 * C23 added: alignas, alignof, bool, static_assert, thread_local,
 *            true, false, nullptr, typeof, typeof_unqual, constexpr
 *
 * Compare to C++: ~90+ keywords and growing.
 */

/*
 * The standard library provides essential functionality.
 * Complex features are built from simple primitives.
 */

/* Example: Building a dynamic array from primitives */
typedef struct DynamicArray {
    int    *data;
    size_t  size;
    size_t  capacity;
} DynamicArray;

static DynamicArray
dynarray_create(void)
{
    return (DynamicArray){NULL, 0, 0};
}

static int
dynarray_push(DynamicArray *arr, int value)
{
    if (arr->size >= arr->capacity) {
        size_t new_cap = arr->capacity ? arr->capacity * 2 : 4;
        int *new_data = realloc(arr->data, new_cap * sizeof(int));
        if (!new_data) return -1;
        arr->data = new_data;
        arr->capacity = new_cap;
    }
    arr->data[arr->size++] = value;
    return 0;
}

static void
dynarray_free(DynamicArray *arr)
{
    free(arr->data);
    arr->data = NULL;
    arr->size = arr->capacity = 0;
}

/*
 * Everything complex is built from these simple building blocks:
 * - struct for data aggregation
 * - pointers for indirection
 * - malloc/free for dynamic memory
 */
```

### 13.1.3 One Way to Do Things

```c
/* consistency.c */

/*
 * K&R style emphasizes consistency and simplicity.
 * There's usually one obvious way to express something.
 */

#include <stdio.h>
#include <string.h>

/*
 * String iteration: pointer idiom is canonical
 */
size_t
kr_strlen(const char *s)
{
    /* Classic K&R style */
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}

/*
 * Array initialization: explicit is clear
 */
void
kr_initialization(void)
{
    /* K&R style: explicit initialization */
    int arr[5] = {1, 2, 3, 4, 5};

    /* For structures, named initializers (C99) */
    struct Point {
        int x, y;
    };
    struct Point p = {.x = 10, .y = 20};

    (void)arr;
    (void)p;
}

/*
 * Loop idioms: for for counting, while for conditions
 */
void
kr_loops(void)
{
    /* for: when you know the count */
    for (int i = 0; i < 10; i++) {
        /* ... */
    }

    /* while: when waiting for a condition */
    int c;
    FILE *fp = stdin;
    while ((c = getchar()) != EOF) {
        putchar(c);
    }

    /* do-while: when you need at least one iteration */
    char buffer[100];
    do {
        if (!fgets(buffer, sizeof(buffer), fp)) break;
    } while (buffer[0] == '#');  /* Skip comments */
}

/*
 * Error handling: return codes are standard
 */
int
kr_error_handling(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);  /* K&R idiom for error messages */
        return -1;
    }

    /* ... work with file ... */

    fclose(fp);
    return 0;  /* Success */
}
```

## 13.2 The UNIX Heritage

### 13.2.1 Pipes and Filters

```c
/* unix_philosophy.c */

/*
 * C was created for UNIX, and UNIX philosophy shaped C:
 *
 * 1. Programs should do one thing well
 * 2. Programs should work together
 * 3. Text streams are the universal interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * Classic UNIX filter: reads stdin, writes stdout
 */

/* count - count lines, words, characters */
static void
count_stdin(void)
{
    long lines = 0, words = 0, chars = 0;
    int c;
    int in_word = 0;

    while ((c = getchar()) != EOF) {
        chars++;
        if (c == '\n') {
            lines++;
        }
        if (isspace(c)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }

    printf("%ld %ld %ld\n", lines, words, chars);
}

/* upper - convert to uppercase */
static void
to_upper_stdin(void)
{
    int c;
    while ((c = getchar()) != EOF) {
        putchar(toupper(c));
    }
}

/* head - print first n lines */
static void
head_stdin(int n)
{
    int c;
    int lines = 0;

    while (lines < n && (c = getchar()) != EOF) {
        putchar(c);
        if (c == '\n') {
            lines++;
        }
    }
}

/*
 * Composition: These simple programs combine via pipes
 *
 * $ cat file.txt | upper | head 10
 *
 * This modularity is the essence of UNIX design.
 */

/*
 * Program entry point: K&R style
 */
int
main(int argc, char *argv[])
{
    /* Parse command line arguments */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [args]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "count") == 0) {
        count_stdin();
    } else if (strcmp(argv[1], "upper") == 0) {
        to_upper_stdin();
    } else if (strcmp(argv[1], "head") == 0) {
        int n = (argc > 2) ? atoi(argv[2]) : 10;
        head_stdin(n);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
```

### 13.2.2 Text Processing

```c
/* text_processing.c */

/*
 * K&R emphasized text processing, which shaped C's string handling.
 * The book is full of practical text utilities.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * Pattern: Line-by-line processing
 */
#define MAX_LINE 1024

static int
getline_kr(char *line, int max)
{
    int c, i;

    for (i = 0; i < max - 1 && (c = getchar()) != EOF && c != '\n'; i++) {
        line[i] = c;
    }

    if (c == '\n') {
        line[i++] = c;
    }

    line[i] = '\0';
    return i;
}

/*
 * Pattern: Simple grep
 */
static void
simple_grep(const char *pattern)
{
    char line[MAX_LINE];

    while (getline_kr(line, MAX_LINE) > 0) {
        if (strstr(line, pattern) != NULL) {
            printf("%s", line);
        }
    }
}

/*
 * Pattern: Tokenization (simple word splitting)
 */
static void
print_words(void)
{
    char line[MAX_LINE];
    char *word;

    while (getline_kr(line, MAX_LINE) > 0) {
        /* Remove newline */
        line[strcspn(line, "\n")] = '\0';

        /* Split on whitespace */
        word = strtok(line, " \t");
        while (word != NULL) {
            printf("%s\n", word);
            word = strtok(NULL, " \t");
        }
    }
}

/*
 * K&R Exercise 1-18: Remove trailing blanks and tabs
 */
static void
squeeze_blanks(void)
{
    char line[MAX_LINE];
    int len;

    while ((len = getline_kr(line, MAX_LINE)) > 0) {
        /* Remove trailing whitespace */
        while (len > 0 && (line[len-1] == ' ' || line[len-1] == '\t' ||
                          line[len-1] == '\n')) {
            len--;
        }

        /* Skip entirely blank lines */
        if (len > 0) {
            line[len] = '\0';
            printf("%s\n", line);
        }
    }
}
```

## 13.3 Classic Algorithms in K&R Style

### 13.3.1 Binary Search

```c
/* classic_algorithms.c */

#include <stdio.h>
#include <stdlib.h>

/*
 * Binary search - K&R style
 * Clean, efficient, and well-documented
 */
int
binsearch(int x, const int v[], int n)
{
    int low, high, mid;

    low = 0;
    high = n - 1;

    while (low <= high) {
        mid = (low + high) / 2;
        if (x < v[mid])
            high = mid - 1;
        else if (x > v[mid])
            low = mid + 1;
        else
            return mid;  /* Found */
    }

    return -1;  /* Not found */
}

/*
 * Quicksort - recursive K&R style
 */
static void
swap(int v[], int i, int j)
{
    int temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}

void
qsort_kr(int v[], int left, int right)
{
    int i, last;

    if (left >= right)
        return;

    swap(v, left, (left + right) / 2);  /* Move pivot to v[left] */
    last = left;

    for (i = left + 1; i <= right; i++) {
        if (v[i] < v[left]) {
            swap(v, ++last, i);
        }
    }

    swap(v, left, last);      /* Restore pivot */
    qsort_kr(v, left, last - 1);
    qsort_kr(v, last + 1, right);
}

/*
 * Hash table - K&R style
 * Simple and practical
 */
#define HASHSIZE 101

typedef struct HashNode {
    struct HashNode *next;
    char *name;
    char *defn;
} HashNode;

static HashNode *hashtab[HASHSIZE];

static unsigned
hash(const char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++) {
        hashval = *s + 31 * hashval;
    }

    return hashval % HASHSIZE;
}

static HashNode *
lookup(const char *s)
{
    HashNode *np;

    for (np = hashtab[hash(s)]; np != NULL; np = np->next) {
        if (strcmp(s, np->name) == 0) {
            return np;
        }
    }

    return NULL;
}

static HashNode *
install(const char *name, const char *defn)
{
    HashNode *np;
    unsigned hashval;

    if ((np = lookup(name)) == NULL) {
        np = malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL) {
            return NULL;
        }
        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else {
        free(np->defn);
    }

    if ((np->defn = strdup(defn)) == NULL) {
        return NULL;
    }

    return np;
}
```

### 13.3.2 Expression Parsing

```c
/* expression_parser.c */

/*
 * K&R's calculator example is a masterclass in recursive descent parsing.
 * It demonstrates:
 * - Clean separation of concerns
 * - State machines
 * - Recursive structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAXOP 100
#define NUMBER '0'

static int sp = 0;
static double stack[100];

static void
push(double f)
{
    if (sp < 100) {
        stack[sp++] = f;
    } else {
        printf("error: stack full\n");
    }
}

static double
pop(void)
{
    if (sp > 0) {
        return stack[--sp];
    } else {
        printf("error: stack empty\n");
        return 0.0;
    }
}

/* getop: get next operator or numeric operand */
static int bufp = 0;
static char buf[100];

static int
getch(void)
{
    return (bufp > 0) ? buf[--bufp] : getchar();
}

static void
ungetch(int c)
{
    if (bufp < 100) {
        buf[bufp++] = c;
    }
}

static int
getop(char s[])
{
    int i, c;

    while ((s[0] = c = getch()) == ' ' || c == '\t')
        ;

    s[1] = '\0';

    if (!isdigit(c) && c != '.') {
        return c;
    }

    i = 0;
    if (isdigit(c)) {
        while (isdigit(s[++i] = c = getch()))
            ;
    }

    if (c == '.') {
        while (isdigit(s[++i] = c = getch()))
            ;
    }

    s[i] = '\0';

    if (c != EOF) {
        ungetch(c);
    }

    return NUMBER;
}

/* Reverse Polish notation calculator */
void
calculator(void)
{
    int type;
    double op2;
    char s[MAXOP];

    while ((type = getop(s)) != EOF) {
        switch (type) {
        case NUMBER:
            push(atof(s));
            break;
        case '+':
            push(pop() + pop());
            break;
        case '*':
            push(pop() * pop());
            break;
        case '-':
            op2 = pop();
            push(pop() - op2);
            break;
        case '/':
            op2 = pop();
            if (op2 != 0.0) {
                push(pop() / op2);
            } else {
                printf("error: zero divisor\n");
            }
            break;
        case '\n':
            printf("\t%.8g\n", pop());
            break;
        default:
            printf("error: unknown command %s\n", s);
            break;
        }
    }
}
```

## 13.4 Memory Allocation: K&R malloc

### 13.4.1 The Classic Implementation

```c
/* kr_malloc.c */

/*
 * K&R's malloc implementation is a masterpiece of systems programming.
 * It demonstrates:
 * - Free list management
 * - Memory coalescing
 * - System call interaction (sbrk)
 *
 * This is educational code showing the concepts.
 */

#include <stddef.h>

typedef long Align;  /* For alignment to long boundary */

union header {
    struct {
        union header *ptr;  /* Next block on free list */
        unsigned size;      /* Size of this block */
    } s;
    Align x;  /* Force alignment */
};

typedef union header Header;

static Header base;           /* Empty list to get started */
static Header *freep = NULL;  /* Start of free list */

/* morecore: ask system for more memory */
#define NALLOC 1024  /* Minimum units to request */

static Header *
morecore(unsigned nu)
{
    void *sbrk(intptr_t);
    char *cp;
    Header *up;

    if (nu < NALLOC) {
        nu = NALLOC;
    }

    cp = sbrk(nu * sizeof(Header));
    if (cp == (char *)-1) {
        return NULL;
    }

    up = (Header *)cp;
    up->s.size = nu;
    free((void *)(up + 1));  /* Add to free list */

    return freep;
}

/* malloc: general-purpose storage allocator */
void *
kr_malloc(unsigned nbytes)
{
    Header *p, *prevp;
    unsigned nunits;

    /* Round up to allocate in units of Header size */
    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    if ((prevp = freep) == NULL) {
        /* No free list yet */
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) {
            /* Big enough */
            if (p->s.size == nunits) {
                /* Exactly */
                prevp->s.ptr = p->s.ptr;
            } else {
                /* Allocate tail end */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p + 1);
        }
        if (p == freep) {
            /* Wrapped around free list */
            if ((p = morecore(nunits)) == NULL) {
                return NULL;
            }
        }
    }
}

/* free: put block ap in free list */
void
kr_free(void *ap)
{
    Header *bp, *p;

    bp = (Header *)ap - 1;  /* Point to block header */

    /* Find where to insert */
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
            break;  /* Freed block at start or end of arena */
        }
    }

    /* Join to upper neighbor */
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }

    /* Join to lower neighbor */
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }

    freep = p;
}
```

## 13.5 The Preprocessor

### 13.5.1 K&R Macro Techniques

```c
/* kr_macros.c */

/*
 * K&R showed how to use the preprocessor effectively.
 * These patterns are still relevant today.
 */

#include <stdio.h>

/*
 * Simple object-like macros
 */
#define MAXLINE 1000
#define YES     1
#define NO      0

/*
 * Function-like macros with parentheses
 */
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define ABS(X)    ((X) < 0 ? -(X) : (X))
#define SQUARE(X) ((X) * (X))

/*
 * The dangers of macros (K&R discusses these)
 */
void
macro_dangers(void)
{
    int x = 5;

    /* Problem: side effects */
    /* int y = SQUARE(x++);  // Evaluates x++ twice! */

    /* Problem: operator precedence without parentheses */
    /* #define BAD_SQUARE(X) X * X */
    /* int z = BAD_SQUARE(1 + 2);  // Expands to 1 + 2 * 1 + 2 = 5 */

    /* Correct usage */
    int a = 3, b = 4;
    int max = MAX(a, b);      /* OK: no side effects */
    int sq = SQUARE(a);       /* OK: no side effects */

    (void)x;
    (void)max;
    (void)sq;
}

/*
 * String operations with # and ##
 */
#define STR(X) #X                       /* Stringify */
#define CONCAT(A, B) A##B               /* Token paste */
#define MAKE_VAR(name) var_##name       /* Create variable name */

void
stringification_example(void)
{
    printf("Value of MAX: %s\n", STR(MAX));  /* Prints "MAX" */

    int MAKE_VAR(count) = 10;  /* Creates var_count */
    printf("var_count = %d\n", var_count);
}

/*
 * Conditional compilation - K&R style
 */
#define DEBUG 1

#if DEBUG
    #define LOG(msg) fprintf(stderr, "DEBUG: %s\n", msg)
#else
    #define LOG(msg) /* nothing */
#endif

/*
 * Header guard pattern (K&R convention)
 */
#ifndef MYHEADER_H
#define MYHEADER_H

/* Header contents */

#endif /* MYHEADER_H */

/*
 * X-Macros: A K&R-era pattern still used today
 */
#define ERROR_LIST \
    X(SUCCESS,       "Operation succeeded") \
    X(NOT_FOUND,     "Item not found") \
    X(OUT_OF_MEMORY, "Memory allocation failed") \
    X(INVALID_INPUT, "Invalid input parameter")

/* Generate enum */
typedef enum {
    #define X(code, msg) ERROR_##code,
    ERROR_LIST
    #undef X
    ERROR_COUNT
} ErrorCode;

/* Generate string array */
static const char *error_messages[] = {
    #define X(code, msg) msg,
    ERROR_LIST
    #undef X
};

const char *
get_error_message(ErrorCode code)
{
    if (code >= 0 && code < ERROR_COUNT) {
        return error_messages[code];
    }
    return "Unknown error";
}
```

## 13.6 Style and Formatting

### 13.6.1 K&R Coding Style

```c
/* kr_style.c */

/*
 * The K&R style is still influential today.
 * Key characteristics:
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * 1. Opening brace on same line (for control structures)
 */
void
style_braces(int n)
{
    if (n > 0) {
        /* K&R: brace on same line as if */
        printf("positive\n");
    } else if (n < 0) {
        printf("negative\n");
    } else {
        printf("zero\n");
    }

    for (int i = 0; i < n; i++) {
        printf("%d ", i);
    }
}

/*
 * 2. Function return type on separate line
 */
static int
max(int a, int b)
{
    return (a > b) ? a : b;
}

/*
 * 3. One space after keywords, no space after function names
 */
void
style_spacing(void)
{
    int x = 10;

    if (x > 0)      /* Space after if */
        printf("positive\n");  /* No space after printf */

    while (x > 0)   /* Space after while */
        x--;

    for (;;)        /* Space after for */
        break;
}

/*
 * 4. Declarations at start of block (traditional C)
 */
void
style_declarations(int n)
{
    int i, j, k;     /* All declarations at top */
    char buffer[100];
    int *ptr;

    /* Then code */
    i = 0;
    j = n;
    k = i + j;
    ptr = &i;

    (void)buffer;
    (void)k;
    (void)ptr;
}

/*
 * 5. Compact but readable expressions
 */
void
style_compact(void)
{
    int c, nl, nw, nc;

    nl = nw = nc = 0;

    /* Classic K&R idiom */
    while ((c = getchar()) != EOF) {
        ++nc;
        if (c == '\n')
            ++nl;
    }

    (void)nl;
    (void)nw;
    (void)nc;
}

/*
 * 6. Meaningful but concise names
 */
/*
 * K&R favors short names for local variables (i, j, p, s)
 * but longer names for functions and globals (getchar, MAXLINE)
 */
int
count_words(const char *s)
{
    int n = 0;
    int inword = 0;

    for ( ; *s; s++) {
        if (*s == ' ' || *s == '\n' || *s == '\t') {
            inword = 0;
        } else if (!inword) {
            inword = 1;
            ++n;
        }
    }

    return n;
}

/*
 * 7. Comment style: /* ... */ for multi-line, sparingly used
 */
/* Copy string t to s; assume s is big enough */
void
strcpy_kr(char *s, const char *t)
{
    while ((*s++ = *t++) != '\0')
        ;
}
```

## 13.7 Lessons for Modern C

### 13.7.1 What K&R Got Right

```c
/* modern_lessons.c */

/*
 * Lessons from K&R that remain relevant:
 *
 * 1. Simplicity: Simple code is easier to understand, debug, and maintain
 * 2. Efficiency: Don't add unnecessary overhead
 * 3. Composability: Build complex systems from simple parts
 * 4. Portability: Write code that works across systems
 * 5. Documentation: Code should be self-documenting where possible
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Modern update: Add const correctness (not in original K&R)
 */
size_t
modern_strlen(const char *s)  /* const was added in ANSI C */
{
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}

/*
 * Modern update: Use size_t for sizes (not int)
 */
void *
modern_memcpy(void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;

    while (n--)
        *d++ = *s++;

    return dest;
}

/*
 * Modern update: Bounds checking
 */
int
modern_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    /* Unlike sprintf, won't overflow buffer */
    /* This is the modern version of K&R's sprintf */
    (void)buf;
    (void)size;
    (void)fmt;
    return 0;
}

/*
 * Modern update: Error handling with errno
 */
#include <errno.h>

FILE *
modern_fopen(const char *path, const char *mode)
{
    FILE *fp = fopen(path, mode);
    if (fp == NULL) {
        /* K&R style error reporting */
        fprintf(stderr, "%s: cannot open (%s)\n", path, strerror(errno));
    }
    return fp;
}

/*
 * The K&R approach, modernized for PartsDB
 */
typedef struct Part {
    unsigned int id;
    char name[64];
    double price;
} Part;

/* K&R style: simple, direct, efficient */
static Part *parts = NULL;
static size_t nparts = 0;
static size_t maxparts = 0;

int
addpart(unsigned int id, const char *name, double price)
{
    if (nparts >= maxparts) {
        size_t newmax = maxparts ? maxparts * 2 : 16;
        Part *newparts = realloc(parts, newmax * sizeof(Part));
        if (newparts == NULL)
            return -1;
        parts = newparts;
        maxparts = newmax;
    }

    Part *p = &parts[nparts++];
    p->id = id;
    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->price = price;

    return 0;
}

Part *
findpart(unsigned int id)
{
    for (size_t i = 0; i < nparts; i++) {
        if (parts[i].id == id)
            return &parts[i];
    }
    return NULL;
}

void
freeparts(void)
{
    free(parts);
    parts = NULL;
    nparts = maxparts = 0;
}
```

## 13.8 Review Questions

### Historical Context

1. **Language design**: K&R says C is "not a very high level language." What do they mean?
   How does C's level of abstraction compare to assembly on one end and Python on the
   other? What trade-offs does this position imply?

2. **UNIX influence**: C was created to implement UNIX. How did this goal shape the
   language? Give specific examples of C features that reflect operating system needs.

3. **Trust the programmer**: This philosophy is both C's strength and weakness. Describe
   three scenarios where this philosophy benefits programmers and three where it causes
   problems. How do modern C standards (C11, C23) address the downsides?

### Code Style

4. **K&R style**: Compare K&R brace style with "Allman style" (opening brace on new line).
   What are the arguments for each? Which does the Linux kernel use and why?

5. **Naming conventions**: K&R uses short names like `i`, `j`, `p`, `s`. When is this
   appropriate and when should longer names be used? Give guidelines for name length
   based on scope and usage frequency.

6. **Macro discipline**: K&R's macros like `MAX(A, B)` have potential problems. Explain
   why the parentheses around `A` and `B` are necessary, and when even parentheses
   aren't sufficient protection.

### Programming Techniques

7. **Memory management**: Study the K&R malloc implementation. Explain:
   - Why is the Header union used for alignment?
   - What is the "free list" and how does coalescing work?
   - How does the algorithm prevent memory fragmentation?

8. **Recursive descent**: The K&R calculator uses a different approach than the stack-based
   one shown. Implement a recursive descent parser for simple arithmetic expressions
   following K&R style.

9. **Text processing**: K&R emphasizes text as a universal format. Design a simple data
   format for storing PartsDB records as text. What are the advantages over binary
   formats? The disadvantages?

10. **Modern application**: Take the K&R hash table implementation and modernize it with:
    - Generic keys and values (void *)
    - Dynamic resizing
    - Thread safety
    - Memory leak prevention
    Maintain the K&R style and simplicity while adding these features.

---

*This concludes the main tutorial content. See the Appendices for reference materials,
security checklists, and comprehensive review questions.*
