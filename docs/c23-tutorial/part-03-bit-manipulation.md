# Part 3: Bit Manipulation & Low-Level Optimization

*Source: Hacker's Delight (Warren)*

> **"Many of the tricks... are ultimately based on the binary representation of numbers."** — Henry S. Warren Jr.

Bit manipulation is the foundation of efficient systems programming. From setting flags to implementing hash functions, understanding binary operations enables optimizations that higher-level abstractions cannot match. This part draws from Warren's comprehensive treatise on low-level arithmetic tricks.

---

## 3.1 Binary Representation Deep Dive

### Two's Complement Representation

```c
/*
 * Two's complement for signed integers:
 * - Positive numbers: standard binary
 * - Negative numbers: invert bits + 1
 *
 * For 8-bit signed:
 *   127 = 0111 1111
 *     1 = 0000 0001
 *     0 = 0000 0000
 *    -1 = 1111 1111
 *  -128 = 1000 0000
 */

/* Sign extension happens automatically */
int8_t small = -1;        /* 0xFF */
int32_t large = small;    /* 0xFFFFFFFF - sign extended */

/* But watch out for unsigned! */
uint8_t usmall = 255;     /* 0xFF */
int32_t ularge = usmall;  /* 0x000000FF - zero extended */
```

### Binary Literals (C23)

```c
/* C23 binary literals make bit patterns clear */
uint8_t flags = 0b00001111;           /* Much clearer than 0x0F */
uint32_t mask = 0b1111'0000'1111'0000; /* Digit separators for readability */

/* Status register example */
#define STATUS_READY    0b0000'0001
#define STATUS_ERROR    0b0000'0010
#define STATUS_BUSY     0b0000'0100
#define STATUS_OVERFLOW 0b0000'1000
#define STATUS_COMPLETE 0b0001'0000

uint8_t status = STATUS_READY | STATUS_BUSY;
```

### Examining Binary Representations

```c
/* Print binary representation */
void print_binary(uint64_t value, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        putchar((value & (1ULL << i)) ? '1' : '0');
        if (i % 8 == 0 && i > 0) putchar(' ');  /* Byte separator */
    }
    putchar('\n');
}

/* Usage */
print_binary(0xDEADBEEF, 32);
/* Output: 11011110 10101101 10111110 11101111 */
```

---

## 3.2 Bit Manipulation Fundamentals

### Basic Operations

```c
/* Set a bit */
#define SET_BIT(x, n)    ((x) | (1ULL << (n)))

/* Clear a bit */
#define CLEAR_BIT(x, n)  ((x) & ~(1ULL << (n)))

/* Toggle a bit */
#define TOGGLE_BIT(x, n) ((x) ^ (1ULL << (n)))

/* Check if bit is set */
#define CHECK_BIT(x, n)  (((x) >> (n)) & 1)

/* Extract bits [high:low] inclusive */
#define EXTRACT_BITS(x, high, low) \
    (((x) >> (low)) & ((1ULL << ((high) - (low) + 1)) - 1))

/* Set bits [high:low] to value */
#define SET_BITS(x, high, low, val) \
    (((x) & ~(((1ULL << ((high) - (low) + 1)) - 1) << (low))) | \
     (((val) & ((1ULL << ((high) - (low) + 1)) - 1)) << (low)))
```

### Flag Manipulation Patterns

```c
/* PartsDB flag system example */
typedef enum {
    PART_FLAG_ACTIVE      = 1 << 0,   /* 0b0001 */
    PART_FLAG_HAZARDOUS   = 1 << 1,   /* 0b0010 */
    PART_FLAG_SERIALIZED  = 1 << 2,   /* 0b0100 */
    PART_FLAG_OBSOLETE    = 1 << 3,   /* 0b1000 */
} part_flags_t;

/* Set multiple flags */
uint32_t flags = PART_FLAG_ACTIVE | PART_FLAG_SERIALIZED;

/* Check if any flag is set */
if (flags & (PART_FLAG_HAZARDOUS | PART_FLAG_OBSOLETE)) {
    /* Handle special parts */
}

/* Check if ALL flags are set */
uint32_t required = PART_FLAG_ACTIVE | PART_FLAG_SERIALIZED;
if ((flags & required) == required) {
    /* All required flags present */
}

/* Clear specific flags */
flags &= ~PART_FLAG_OBSOLETE;

/* Toggle flags */
flags ^= PART_FLAG_ACTIVE;
```

### Bitmask Patterns

```c
/* Create mask of n bits */
#define MASK(n) ((1ULL << (n)) - 1)
/* MASK(4) = 0b1111 = 15 */

/* Mask for bits [high:low] */
#define RANGE_MASK(high, low) (MASK((high) - (low) + 1) << (low))

/* Sign extend from bit n */
static inline int32_t sign_extend(uint32_t x, int n) {
    uint32_t sign_bit = 1U << (n - 1);
    return (int32_t)((x ^ sign_bit) - sign_bit);
}

/* Example: sign extend 5-bit value */
int32_t extended = sign_extend(0b11111, 5);  /* -1 */
```

---

## 3.3 Population Count and Leading Zeros

### Population Count (Counting Set Bits)

```c
/* Naive implementation */
int popcount_naive(uint32_t x) {
    int count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

/* Brian Kernighan's algorithm - O(number of set bits) */
int popcount_kernighan(uint32_t x) {
    int count = 0;
    while (x) {
        x &= x - 1;  /* Clear lowest set bit */
        count++;
    }
    return count;
}

/* Parallel bit counting (Hacker's Delight) */
int popcount_parallel(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x3F;
}

/* Use built-in when available (fastest) */
#ifdef __GNUC__
#define POPCOUNT(x) __builtin_popcount(x)
#define POPCOUNTL(x) __builtin_popcountl(x)
#define POPCOUNTLL(x) __builtin_popcountll(x)
#else
#define POPCOUNT(x) popcount_parallel(x)
#endif
```

### Leading/Trailing Zero Count

```c
/* Count leading zeros */
#ifdef __GNUC__
#define CLZ(x) __builtin_clz(x)       /* Undefined for x=0 */
#define CLZL(x) __builtin_clzl(x)
#define CLZLL(x) __builtin_clzll(x)
#else
int clz_portable(uint32_t x) {
    if (x == 0) return 32;
    int n = 0;
    if (x <= 0x0000FFFF) { n += 16; x <<= 16; }
    if (x <= 0x00FFFFFF) { n += 8;  x <<= 8; }
    if (x <= 0x0FFFFFFF) { n += 4;  x <<= 4; }
    if (x <= 0x3FFFFFFF) { n += 2;  x <<= 2; }
    if (x <= 0x7FFFFFFF) { n += 1; }
    return n;
}
#define CLZ(x) clz_portable(x)
#endif

/* Count trailing zeros */
#ifdef __GNUC__
#define CTZ(x) __builtin_ctz(x)       /* Undefined for x=0 */
#else
int ctz_portable(uint32_t x) {
    if (x == 0) return 32;
    return 31 - CLZ(x & -x);  /* Isolate lowest bit, then CLZ */
}
#define CTZ(x) ctz_portable(x)
#endif

/* Find position of highest set bit (0-indexed) */
#define BIT_WIDTH(x) (32 - CLZ(x))
#define LOG2_FLOOR(x) (31 - CLZ(x))   /* Floor of log2 */

/* Find position of lowest set bit (0-indexed) */
#define LOWEST_SET_BIT_POS(x) CTZ(x)
```

### Applications

```c
/* Find next power of 2 >= x */
uint32_t next_power_of_2(uint32_t x) {
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

/* Or using CLZ */
uint32_t next_power_of_2_clz(uint32_t x) {
    if (x <= 1) return 1;
    return 1U << (32 - CLZ(x - 1));
}

/* Check if x is a power of 2 */
#define IS_POWER_OF_2(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

/* Round up to next multiple of power of 2 */
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
```

---

## 3.4 Power of Two Operations

### Fast Division and Modulo

```c
/* Division by power of 2 (unsigned) */
#define DIV_POW2(x, n) ((x) >> (n))

/* Modulo power of 2 (unsigned) */
#define MOD_POW2(x, n) ((x) & ((1 << (n)) - 1))

/* Examples */
uint32_t x = 1000;
uint32_t div8 = x >> 3;        /* x / 8 */
uint32_t mod8 = x & 7;         /* x % 8 */
uint32_t mod1024 = x & 0x3FF;  /* x % 1024 */

/* Signed division rounds toward negative infinity, not zero!
 * For signed, use regular division or adjust manually */
int32_t signed_div_pow2(int32_t x, int n) {
    /* Add (2^n - 1) if negative before shifting */
    return (x + ((x >> 31) & ((1 << n) - 1))) >> n;
}
```

### Alignment Calculations

```c
/* PartsDB: align allocation to cache line */
#define CACHE_LINE_SIZE 64

void *aligned_alloc_cache_line(size_t size) {
    size_t aligned_size = ALIGN_UP(size, CACHE_LINE_SIZE);
    return aligned_alloc(CACHE_LINE_SIZE, aligned_size);
}

/* Check alignment */
#define IS_ALIGNED(ptr, align) (((uintptr_t)(ptr) & ((align) - 1)) == 0)

void *safe_aligned_access(void *ptr, size_t align) {
    if (!IS_ALIGNED(ptr, align)) {
        /* Handle misalignment */
        return nullptr;
    }
    return ptr;
}
```

---

## 3.5 Division and Remainder Tricks

### Division by Constant

Compilers optimize division by constants using multiplication by magic numbers:

```c
/* The compiler transforms this automatically: */
uint32_t div_by_10(uint32_t x) {
    return x / 10;
}

/* Into something like: */
uint32_t div_by_10_magic(uint32_t x) {
    /* Multiply by magic number, then shift */
    return (uint32_t)(((uint64_t)x * 0xCCCCCCCD) >> 35);
}

/* Manual magic number calculation for runtime divisors */
typedef struct {
    uint32_t magic;
    int shift;
    int add;
} div_magic_t;

/* Use libdivide for general case */
```

### Fast Modulo for Hash Tables

```c
/* For hash tables with power-of-2 size */
size_t hash_index_pow2(uint64_t hash, size_t table_size) {
    /* table_size must be power of 2 */
    return hash & (table_size - 1);
}

/* Fibonacci hashing for better distribution */
size_t hash_index_fibonacci(uint64_t hash, int bits) {
    /* Golden ratio: 2^64 / phi */
    const uint64_t GOLDEN = 0x9E3779B97F4A7C15ULL;
    return (hash * GOLDEN) >> (64 - bits);
}

/* PartsDB: compute bucket for part ID */
size_t get_bucket(int64_t part_id, size_t num_buckets) {
    if (IS_POWER_OF_2(num_buckets)) {
        return (size_t)part_id & (num_buckets - 1);
    }
    return (size_t)(part_id % (int64_t)num_buckets);
}
```

---

## 3.6 Overflow Detection

### Detecting Signed Overflow

```c
/* GCC/Clang built-ins for overflow checking */
#ifdef __GNUC__

bool safe_add_i32(int32_t a, int32_t b, int32_t *result) {
    return !__builtin_add_overflow(a, b, result);
}

bool safe_sub_i32(int32_t a, int32_t b, int32_t *result) {
    return !__builtin_sub_overflow(a, b, result);
}

bool safe_mul_i32(int32_t a, int32_t b, int32_t *result) {
    return !__builtin_mul_overflow(a, b, result);
}

#else

/* Portable overflow detection */
bool safe_add_i32(int32_t a, int32_t b, int32_t *result) {
    if (b > 0 && a > INT32_MAX - b) return false;  /* Overflow */
    if (b < 0 && a < INT32_MIN - b) return false;  /* Underflow */
    *result = a + b;
    return true;
}

bool safe_mul_i32(int32_t a, int32_t b, int32_t *result) {
    if (a > 0 && b > 0 && a > INT32_MAX / b) return false;
    if (a > 0 && b < 0 && b < INT32_MIN / a) return false;
    if (a < 0 && b > 0 && a < INT32_MIN / b) return false;
    if (a < 0 && b < 0 && b < INT32_MAX / a) return false;
    *result = a * b;
    return true;
}

#endif

/* PartsDB: safe quantity calculation */
partsdb_error_t update_quantity_safe(int32_t current, int32_t change,
                                      int32_t *new_quantity) {
    if (!safe_add_i32(current, change, new_quantity)) {
        return PARTSDB_ERROR_OVERFLOW;
    }
    if (*new_quantity < 0) {
        return PARTSDB_ERROR_INVALID;  /* Can't have negative inventory */
    }
    return PARTSDB_OK;
}
```

### Detecting Unsigned Overflow

```c
/* Unsigned overflow wraps - detect by checking result */
bool safe_add_u64(uint64_t a, uint64_t b, uint64_t *result) {
    *result = a + b;
    return *result >= a;  /* If wrapped, result < a */
}

bool safe_mul_u64(uint64_t a, uint64_t b, uint64_t *result) {
    if (a != 0 && b > UINT64_MAX / a) return false;
    *result = a * b;
    return true;
}

/* Size calculation with overflow check */
void *safe_calloc(size_t nmemb, size_t size) {
    size_t total;
    if (!safe_mul_u64(nmemb, size, &total)) {
        return nullptr;  /* Would overflow */
    }
    return calloc(nmemb, size);
}
```

---

## 3.7 Branch-Free Programming

### Conditional Move

```c
/* Branching version */
int max_branch(int a, int b) {
    if (a > b) return a;
    return b;
}

/* Branch-free version */
int max_branchfree(int a, int b) {
    int diff = a - b;
    int mask = diff >> 31;  /* 0 if a >= b, -1 if a < b */
    return a - (diff & mask);
}

/* Or simpler, letting compiler optimize */
int max_ternary(int a, int b) {
    return a > b ? a : b;  /* Compiler may use CMOV */
}
```

### Branch-Free Absolute Value

```c
/* Branching version */
int abs_branch(int x) {
    if (x < 0) return -x;
    return x;
}

/* Branch-free version */
int abs_branchfree(int x) {
    int mask = x >> 31;      /* 0 if positive, -1 if negative */
    return (x + mask) ^ mask; /* Equivalent to conditional negate */
}

/* Or: */
int abs_branchfree2(int x) {
    int mask = x >> 31;
    return (x ^ mask) - mask;
}
```

### Branch-Free Selection

```c
/* Select a or b based on condition (condition must be 0 or 1) */
int select_branchfree(int condition, int a, int b) {
    /* Create mask: condition ? -1 : 0 */
    int mask = -condition;
    return (a & mask) | (b & ~mask);
}

/* PartsDB: branch-free clamp */
int32_t clamp_branchfree(int32_t value, int32_t min_val, int32_t max_val) {
    /* Clamp to [min_val, max_val] */
    int32_t t = value < min_val ? min_val : value;
    return t > max_val ? max_val : t;
    /* Compiler often optimizes this well */
}

/* Fully branch-free clamp */
int32_t clamp_fully_branchfree(int32_t x, int32_t lo, int32_t hi) {
    /* Assumes no overflow */
    int32_t mask_lo = (x - lo) >> 31;  /* -1 if x < lo */
    x = (lo & mask_lo) | (x & ~mask_lo);
    int32_t mask_hi = (hi - x) >> 31;  /* -1 if hi < x */
    return (hi & mask_hi) | (x & ~mask_hi);
}
```

---

## 3.8 SIMD and Vectorization Basics

### Auto-Vectorization

```c
/* Compiler can auto-vectorize simple loops */
void add_arrays(float *restrict a, float *restrict b,
                float *restrict result, size_t n) {
    /* 'restrict' tells compiler arrays don't overlap */
    for (size_t i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
    /* Compile with: gcc -O3 -march=native -ftree-vectorize */
}

/* Help compiler vectorize */
void add_arrays_aligned(float *restrict a, float *restrict b,
                        float *restrict result, size_t n) {
    /* Assume 32-byte alignment for AVX */
    a = __builtin_assume_aligned(a, 32);
    b = __builtin_assume_aligned(b, 32);
    result = __builtin_assume_aligned(result, 32);

    for (size_t i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}
```

### Manual SIMD with Intrinsics

```c
#include <immintrin.h>  /* AVX/AVX2 intrinsics */

/* AVX2: process 8 floats at once */
void add_arrays_avx(float *restrict a, float *restrict b,
                    float *restrict result, size_t n) {
    size_t i = 0;

    /* Process 8 floats at a time with AVX */
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vr = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&result[i], vr);
    }

    /* Handle remaining elements */
    for (; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Check CPU support at runtime */
#include <cpuid.h>

bool has_avx2(void) {
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid(7, &eax, &ebx, &ecx, &edx)) {
        return false;
    }
    return (ebx & bit_AVX2) != 0;
}
```

---

## 3.9 PartsDB Case Study: Optimizing Search

### Bit-Packed Flags

```c
/* Compact part flags using bitfield */
typedef struct {
    uint8_t is_active     : 1;
    uint8_t is_hazardous  : 1;
    uint8_t is_serialized : 1;
    uint8_t is_obsolete   : 1;
    uint8_t reserved      : 4;
} part_flags_packed_t;

/* Or as a single byte with masks */
typedef uint8_t part_flags_t;
#define PFLAG_ACTIVE     0x01
#define PFLAG_HAZARDOUS  0x02
#define PFLAG_SERIALIZED 0x04
#define PFLAG_OBSOLETE   0x08

/* Fast filtering with bitwise AND */
bool matches_filter(part_flags_t part_flags, part_flags_t required_flags) {
    return (part_flags & required_flags) == required_flags;
}

/* Filter parts by flags - vectorizable */
size_t filter_parts_by_flags(const part_flags_t *flags, size_t n,
                              part_flags_t required, size_t *indices) {
    size_t count = 0;
    for (size_t i = 0; i < n; i++) {
        if ((flags[i] & required) == required) {
            indices[count++] = i;
        }
    }
    return count;
}
```

### Fast String Hashing

```c
/* FNV-1a hash - fast and simple */
uint64_t fnv1a_hash(const char *str) {
    uint64_t hash = 0xCBF29CE484222325ULL;  /* FNV offset basis */
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 0x100000001B3ULL;  /* FNV prime */
    }
    return hash;
}

/* Case-insensitive hash */
uint64_t fnv1a_hash_nocase(const char *str) {
    uint64_t hash = 0xCBF29CE484222325ULL;
    while (*str) {
        uint8_t c = *str++;
        /* ASCII lowercase: set bit 5 if letter */
        if (c >= 'A' && c <= 'Z') c |= 0x20;
        hash ^= c;
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

/* PartsDB: hash part number for quick lookup */
typedef struct {
    uint64_t hash;
    size_t index;
} hash_entry_t;

/* Build hash index */
void build_part_number_index(const part_t *parts, size_t n,
                              hash_entry_t *index) {
    for (size_t i = 0; i < n; i++) {
        index[i].hash = fnv1a_hash(parts[i].part_number);
        index[i].index = i;
    }
    /* Sort by hash for binary search */
    qsort(index, n, sizeof(hash_entry_t), compare_hash_entries);
}
```

### Bloom Filter for Fast Negative Lookup

```c
/* Simple bloom filter for part number existence check */
typedef struct {
    uint64_t *bits;
    size_t size_bits;
    int num_hashes;
} bloom_filter_t;

bloom_filter_t *bloom_create(size_t expected_items, double false_positive_rate) {
    bloom_filter_t *bf = malloc(sizeof(bloom_filter_t));

    /* Calculate optimal size and hash count */
    size_t m = (size_t)(-1.0 * expected_items * log(false_positive_rate) /
                        (log(2) * log(2)));
    bf->num_hashes = (int)(m / expected_items * log(2));
    bf->size_bits = next_power_of_2(m);

    bf->bits = calloc(bf->size_bits / 64 + 1, sizeof(uint64_t));
    return bf;
}

void bloom_add(bloom_filter_t *bf, const char *key) {
    uint64_t h1 = fnv1a_hash(key);
    uint64_t h2 = fnv1a_hash(key + strlen(key)/2);  /* Simple double hashing */

    for (int i = 0; i < bf->num_hashes; i++) {
        uint64_t hash = (h1 + i * h2) & (bf->size_bits - 1);
        bf->bits[hash / 64] |= (1ULL << (hash % 64));
    }
}

bool bloom_maybe_contains(bloom_filter_t *bf, const char *key) {
    uint64_t h1 = fnv1a_hash(key);
    uint64_t h2 = fnv1a_hash(key + strlen(key)/2);

    for (int i = 0; i < bf->num_hashes; i++) {
        uint64_t hash = (h1 + i * h2) & (bf->size_bits - 1);
        if (!(bf->bits[hash / 64] & (1ULL << (hash % 64)))) {
            return false;  /* Definitely not present */
        }
    }
    return true;  /* Maybe present - need to verify */
}
```

---

## 3.10 Review Questions - Part 3

> **Instructions**: Research these questions using *Hacker's Delight*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[Hacker's Delight, Ch. 2]** Explain why `x & (x - 1)` clears the lowest set bit. Derive this result from the binary representation.

2. **[Hacker's Delight, Ch. 3]** How does the parallel population count algorithm work? Trace through `popcount_parallel(0xABCD1234)` step by step.

3. **[Hacker's Delight, Ch. 5]** Explain the magic number technique for division by constant. How would you compute the magic number for division by 7?

4. **[Hacker's Delight, Ch. 7]** What is the difference between arithmetic right shift and logical right shift for negative numbers? How does this affect the sign extension trick for branch-free absolute value?

5. **[Hacker's Delight, Ch. 10]** Why is the Fibonacci hashing constant `0x9E3779B97F4A7C15`? How does it relate to the golden ratio?

### Practical Exercises

6. **Implement Bit Reversal**: Write a function to reverse the bits of a 32-bit integer using the parallel technique from Hacker's Delight. Benchmark against the naive loop.

7. **Fast Log2**: Implement floor(log2(x)) for 64-bit integers using CLZ. Handle the x=0 case appropriately.

8. **Gray Code**: Implement conversion between binary and Gray code. Use this to implement a rotary encoder decoder for PartsDB location tracking.

9. **CRC-32 Optimization**: Implement CRC-32 using the table-based method. Then optimize using the carry-less multiplication instruction (PCLMULQDQ) if available.

10. **Parallel Search**: Implement a function that searches for a byte value in a 64-bit word simultaneously using the SWAR (SIMD Within A Register) technique.

### Research Topics

11. **[Hacker's Delight, Ch. 8]** Research the "magic" division algorithm for signed integers. How does it differ from unsigned? Implement a general-purpose function that computes magic numbers for any divisor.

12. **[Hacker's Delight, Ch. 11]** Research bit matrix multiplication. How can you use it to implement GF(2^8) multiplication for AES? Implement a constant-time AES SubBytes using bit-slicing.

13. **[Hacker's Delight, Ch. 15]** Research the POPCNT, LZCNT, and TZCNT instructions. When are built-ins faster than lookup tables? Benchmark on your CPU.

---

[← Part 2: Modern Tooling](part-02-modern-tooling.md) | [Back to Index](README.md) | [Part 4: Security →](part-04-security.md)
