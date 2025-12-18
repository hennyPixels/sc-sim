# Appendix C: Quick Reference

## C23 New Features Summary

### Type System Enhancements

```c
/* nullptr - Type-safe null pointer */
int *p = nullptr;          /* Preferred over NULL */
if (p == nullptr) { }

/* auto for type inference (with initializer) */
auto x = 42;               /* int */
auto p = &x;               /* int* */
auto arr = (int[]){1,2,3}; /* int[3] */

/* typeof and typeof_unqual */
int x = 42;
typeof(x) y = 100;          /* int y = 100; */
const int z = 50;
typeof_unqual(z) w = 60;    /* int w = 60; (removes const) */

/* constexpr for compile-time constants */
constexpr int SIZE = 100;
constexpr double PI = 3.14159265358979;

/* true, false, bool as keywords */
bool flag = true;
if (!flag) flag = false;
```

### Attributes

```c
/* Standard attributes in C23 */
[[nodiscard]] int important_func(void);
[[maybe_unused]] static void helper(void);
[[deprecated("Use new_func instead")]] void old_func(void);
[[noreturn]] void fatal_error(const char *msg);
[[fallthrough]]; /* In switch cases */

/* Example usage */
[[nodiscard]] int allocate_resource(void) {
    return 0;
}

void example(int x) {
    [[maybe_unused]] int debug_var = x * 2;

    switch (x) {
    case 1:
        do_something();
        [[fallthrough]];
    case 2:
        do_more();
        break;
    }
}
```

### Other C23 Features

```c
/* Binary literals */
int mask = 0b11110000;
unsigned flags = 0b0001'0010'0100'1000; /* Digit separators */

/* #embed directive */
static const unsigned char icon[] = {
    #embed "icon.png"
};

/* #elifdef and #elifndef */
#ifdef A
    /* ... */
#elifdef B
    /* ... */
#elifndef C
    /* ... */
#endif

/* Empty initializer {} */
struct Point p = {};  /* Zero-initialized */
int arr[10] = {};     /* Zero-initialized */

/* u8 character constants */
char8_t c = u8'A';

/* Bit-precise integers (optional) */
_BitInt(128) big_int;
unsigned _BitInt(256) huge_uint;
```

---

## Memory Layout Reference

### Object File Sections

| Section | Content | Characteristics |
|---------|---------|-----------------|
| `.text` | Executable code | Read-only, executable |
| `.rodata` | String literals, const | Read-only |
| `.data` | Initialized globals | Read-write |
| `.bss` | Uninitialized globals | Read-write, zero-filled |
| `.heap` | Dynamic allocations | Read-write, grows up |
| `.stack` | Local variables | Read-write, grows down |

### x86-64 Calling Convention (System V ABI)

| Argument # | Register | Preserved across calls? |
|------------|----------|------------------------|
| 1st integer | RDI | No |
| 2nd integer | RSI | No |
| 3rd integer | RDX | No |
| 4th integer | RCX | No |
| 5th integer | R8 | No |
| 6th integer | R9 | No |
| 1st float | XMM0 | No |
| 2nd float | XMM1 | No |
| Return value | RAX/XMM0 | N/A |

Callee-saved (must preserve): RBX, RBP, R12-R15

### Data Type Sizes (LP64)

| Type | Size | Alignment |
|------|------|-----------|
| char | 1 | 1 |
| short | 2 | 2 |
| int | 4 | 4 |
| long | 8 | 8 |
| long long | 8 | 8 |
| float | 4 | 4 |
| double | 8 | 8 |
| pointer | 8 | 8 |

---

## Bit Manipulation Cheat Sheet

```c
/* Common bit operations */
x |= (1 << n);           /* Set bit n */
x &= ~(1 << n);          /* Clear bit n */
x ^= (1 << n);           /* Toggle bit n */
(x >> n) & 1;            /* Test bit n */
x & (x - 1);             /* Clear lowest set bit */
x & (-x);                /* Isolate lowest set bit */
x | (x - 1);             /* Set all bits below lowest set bit */

/* Count operations */
__builtin_popcount(x);   /* Count set bits */
__builtin_clz(x);        /* Count leading zeros */
__builtin_ctz(x);        /* Count trailing zeros */
__builtin_ffs(x);        /* Find first set (1-indexed) */

/* Power of 2 checks */
(x & (x - 1)) == 0;      /* Is power of 2 (x > 0) */
(x | (x - 1)) + 1;       /* Next power of 2 */

/* Alignment */
(x + (align - 1)) & ~(align - 1);  /* Round up to alignment */
x & ~(align - 1);                   /* Round down to alignment */

/* Sign manipulation */
(x ^ (x >> 31)) - (x >> 31);  /* Absolute value (int) */
(x ^ y) >= 0;                  /* Same sign */
```

---

## Safe Function Alternatives

### String Functions

```c
/* Unsafe → Safe */
strcpy(dst, src);           →  strncpy(dst, src, sizeof(dst)-1);
                               dst[sizeof(dst)-1] = '\0';

strcat(dst, src);           →  strncat(dst, src, sizeof(dst)-strlen(dst)-1);

sprintf(buf, fmt, ...);     →  snprintf(buf, sizeof(buf), fmt, ...);

gets(buf);                  →  fgets(buf, sizeof(buf), stdin);

scanf("%s", buf);           →  scanf("%63s", buf);  /* specify width */
```

### Memory Functions

```c
/* Safe allocation pattern */
ptr = malloc(n * sizeof(*ptr));  /* Use sizeof(*ptr), not sizeof(type) */
if (ptr == NULL) {
    /* Handle error */
}

/* Overflow-safe allocation */
if (n > SIZE_MAX / sizeof(*ptr)) {
    /* Overflow would occur */
    return NULL;
}
ptr = calloc(n, sizeof(*ptr));  /* calloc checks for overflow */

/* Safe free pattern */
free(ptr);
ptr = NULL;  /* Prevent use-after-free */
```

---

## Common Compiler Flags

### GCC/Clang Warning Flags

```bash
# Essential warnings
-Wall               # Common warnings
-Wextra             # Extra warnings
-Wpedantic          # Strict ISO compliance
-Werror             # Treat warnings as errors

# Recommended additions
-Wshadow            # Variable shadowing
-Wconversion        # Implicit conversions
-Wsign-conversion   # Sign conversion warnings
-Wnull-dereference  # Null pointer dereference
-Wdouble-promotion  # Float to double promotion
-Wformat=2          # Format string issues
-Wstrict-overflow=2 # Signed overflow issues
-Wcast-align        # Pointer alignment issues
-Wstrict-prototypes # Function prototype issues
-Wmissing-prototypes # Missing prototypes
```

### Security Hardening Flags

```bash
# Compile-time protection
-D_FORTIFY_SOURCE=2    # Buffer overflow detection
-fstack-protector-strong  # Stack canaries
-fPIE                  # Position independent executable

# Link-time protection
-pie                   # PIE executable
-Wl,-z,relro           # Partial RELRO
-Wl,-z,now             # Full RELRO
-Wl,-z,noexecstack     # Non-executable stack
```

### Sanitizer Flags

```bash
# AddressSanitizer (memory errors)
-fsanitize=address -fno-omit-frame-pointer

# UndefinedBehaviorSanitizer
-fsanitize=undefined

# ThreadSanitizer (data races)
-fsanitize=thread

# MemorySanitizer (uninitialized reads)
-fsanitize=memory

# Combined (not all can be combined)
-fsanitize=address,undefined
```

---

## Error Handling Patterns

### Return Code Pattern

```c
typedef enum {
    SUCCESS = 0,
    ERR_NULL_ARGUMENT,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_INPUT,
    ERR_IO_ERROR
} Result;

Result do_something(int *out) {
    if (out == NULL) {
        return ERR_NULL_ARGUMENT;
    }
    *out = 42;
    return SUCCESS;
}

/* Usage */
int value;
Result r = do_something(&value);
if (r != SUCCESS) {
    fprintf(stderr, "Error: %d\n", r);
    return 1;
}
```

### GOTO Cleanup Pattern

```c
int process_file(const char *path) {
    int result = -1;
    FILE *f = NULL;
    char *buf = NULL;

    f = fopen(path, "r");
    if (!f) goto cleanup;

    buf = malloc(4096);
    if (!buf) goto cleanup;

    /* ... processing ... */

    result = 0;  /* Success */

cleanup:
    free(buf);
    if (f) fclose(f);
    return result;
}
```

---

## Printf Format Specifiers

| Specifier | Type | Example |
|-----------|------|---------|
| `%d`, `%i` | int | -42 |
| `%u` | unsigned int | 42 |
| `%ld` | long | -42L |
| `%lu` | unsigned long | 42UL |
| `%lld` | long long | -42LL |
| `%llu` | unsigned long long | 42ULL |
| `%zu` | size_t | (size value) |
| `%td` | ptrdiff_t | (pointer difference) |
| `%f` | double | 3.14 |
| `%e` | double (scientific) | 3.14e+00 |
| `%g` | double (auto) | 3.14 |
| `%x`, `%X` | hex | 0x2a |
| `%o` | octal | 052 |
| `%c` | char | 'A' |
| `%s` | string | "hello" |
| `%p` | pointer | 0x7fff... |
| `%%` | literal % | % |

### Fixed-width integers (from `<inttypes.h>`)

```c
#include <inttypes.h>

int32_t i = -42;
uint64_t u = 42;

printf("%" PRId32 "\n", i);   /* int32_t */
printf("%" PRIu64 "\n", u);   /* uint64_t */
printf("%" PRIx64 "\n", u);   /* hex */

/* Scanning */
scanf("%" SCNd32, &i);
scanf("%" SCNu64, &u);
```

---

## Static Assert Examples

```c
#include <limits.h>
#include <stddef.h>

/* Verify sizes */
static_assert(sizeof(int) == 4, "int must be 4 bytes");
static_assert(CHAR_BIT == 8, "char must be 8 bits");

/* Verify struct layout */
struct Packet {
    uint32_t header;
    uint64_t payload;
    uint32_t checksum;
};
static_assert(sizeof(struct Packet) == 24, "Packet size mismatch");
static_assert(offsetof(struct Packet, payload) == 8, "Bad alignment");

/* Verify assumptions */
static_assert(-1 == ~0, "Requires two's complement");
```

---

## Atomic Operations Quick Reference

```c
#include <stdatomic.h>

atomic_int counter = ATOMIC_VAR_INIT(0);

/* Basic operations */
atomic_store(&counter, 42);
int val = atomic_load(&counter);
int old = atomic_fetch_add(&counter, 1);
int prev = atomic_exchange(&counter, 100);

/* Compare-and-swap */
int expected = 42;
bool success = atomic_compare_exchange_strong(&counter, &expected, 50);

/* Memory ordering */
atomic_store_explicit(&counter, 42, memory_order_release);
int v = atomic_load_explicit(&counter, memory_order_acquire);

/* Fence */
atomic_thread_fence(memory_order_seq_cst);
```
