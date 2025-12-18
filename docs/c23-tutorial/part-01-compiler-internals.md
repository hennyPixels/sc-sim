# Part 1: Compiler Internals & Memory Layout

*Sources: Expert C Programming (van der Linden), Computer Systems: A Programmer's Perspective (Bryant & O'Hallaron)*

> **"C is quirky, flawed, and an enormous success."** — Peter van der Linden

Understanding how C code transforms into executable programs is fundamental to writing secure, efficient systems software. This part explores the journey from source code to running process, drawing from van der Linden's insider knowledge of compiler design and Bryant & O'Hallaron's systems-level perspective.

---

## 1.1 The Compilation Pipeline

### The Four Stages of Compilation

Every C program passes through four distinct transformation stages:

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   Source     │───▶│ Preprocessed │───▶│   Assembly   │───▶│    Object    │
│    (.c)      │    │    (.i)      │    │    (.s)      │    │    (.o)      │
└──────────────┘    └──────────────┘    └──────────────┘    └──────────────┘
       │                   │                   │                   │
       ▼                   ▼                   ▼                   ▼
   Preprocessor        Compiler           Assembler            Linker
     (cpp)              (cc1)              (as)                (ld)
                                                                  │
                                                                  ▼
                                                          ┌──────────────┐
                                                          │  Executable  │
                                                          │   (a.out)    │
                                                          └──────────────┘
```

**Stage 1: Preprocessing (cpp)**

The preprocessor handles all directives beginning with `#`:

```c
/* Before preprocessing */
#include <stdio.h>
#define MAX_PARTS 1000
#define SQUARE(x) ((x) * (x))

int buffer[MAX_PARTS];

/* After preprocessing - header expanded, macros replaced */
/* ... thousands of lines from stdio.h ... */
int buffer[1000];
```

**Examine preprocessor output:**
```bash
# GCC: stop after preprocessing
gcc -E database.c -o database.i

# Clang: same behavior
clang -E database.c -o database.i

# View macro expansions only
gcc -E -dM database.c | grep PARTSDB
```

> **Deep C Secrets Insight**: "The preprocessor knows nothing about C. It's a text processor that happens to be run before the C compiler."

**Stage 2: Compilation (cc1)**

The compiler proper transforms preprocessed C into assembly language:

```bash
# Stop after compilation to assembly
gcc -S database.c -o database.s

# With optimizations visible
gcc -S -O2 -fverbose-asm database.c -o database.s
```

**Sample assembly output (x86-64):**
```asm
partsdb_close:
    push    rbp
    mov     rbp, rsp
    mov     QWORD PTR [rbp-8], rdi    ; db parameter
    cmp     QWORD PTR [rbp-8], 0      ; if (db == nullptr)
    je      .L1                        ; return early
    ; ... cleanup code ...
.L1:
    pop     rbp
    ret
```

**Stage 3: Assembly (as)**

The assembler converts assembly language to machine code in object file format:

```bash
# Create object file
gcc -c database.c -o database.o

# Examine object file contents
objdump -d database.o      # Disassembly
objdump -t database.o      # Symbol table
objdump -r database.o      # Relocations
nm database.o              # Symbol listing
```

**Stage 4: Linking (ld)**

The linker combines object files and resolves external references:

```bash
# Link manually (educational - normally gcc handles this)
ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
   /usr/lib/crt1.o /usr/lib/crti.o \
   database.o main.o \
   -lsqlite3 -lc \
   /usr/lib/crtn.o \
   -o partsdb

# See what gcc actually invokes
gcc -v database.c main.c -lsqlite3 -o partsdb 2>&1 | grep collect2
```

### Compiler Optimizations

Modern compilers transform code dramatically at higher optimization levels:

```c
/* Original code */
int sum_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* At -O0: Literal translation, every variable in memory */
/* At -O2: Loop unrolling, register allocation, strength reduction */
/* At -O3: Vectorization (SIMD), aggressive inlining */
```

**Optimization flags for security-critical code:**
```makefile
# PartsDB Makefile hardening
CFLAGS = -O2                    # Moderate optimization (predictable)
CFLAGS += -fstack-protector-strong  # Stack canaries
CFLAGS += -D_FORTIFY_SOURCE=2   # Buffer overflow checks
CFLAGS += -fPIE                 # Position independent executable
CFLAGS += -fno-strict-aliasing  # Safer pointer handling
```

> **CS:APP Insight**: "Optimization can change program behavior in subtle ways. Always test optimized code as thoroughly as debug builds."

---

## 1.2 Object Files and Linking

### ELF Object File Format

Linux uses the Executable and Linkable Format (ELF) for object files:

```
┌─────────────────────────────────────┐
│           ELF Header                 │  Magic number, architecture, entry point
├─────────────────────────────────────┤
│     Program Header Table             │  Segment descriptions (for execution)
├─────────────────────────────────────┤
│           .text                      │  Executable code
├─────────────────────────────────────┤
│           .rodata                    │  Read-only data (string literals)
├─────────────────────────────────────┤
│           .data                      │  Initialized global variables
├─────────────────────────────────────┤
│           .bss                       │  Uninitialized global variables
├─────────────────────────────────────┤
│           .symtab                    │  Symbol table
├─────────────────────────────────────┤
│           .rel.text                  │  Relocation entries for .text
├─────────────────────────────────────┤
│           .strtab                    │  String table
├─────────────────────────────────────┤
│     Section Header Table             │  Section descriptions (for linking)
└─────────────────────────────────────┘
```

**Examining ELF files:**
```bash
# ELF header
readelf -h partsdb

# Section headers
readelf -S partsdb

# Program headers (segments)
readelf -l partsdb

# Symbol table
readelf -s partsdb | grep partsdb_

# Dynamic dependencies
ldd partsdb
```

### Symbol Resolution

The linker resolves symbols according to specific rules:

```c
/* database.c */
int global_count = 0;           /* Strong symbol - initialized */
int uninitialized;              /* Weak symbol - uninitialized (tentative) */
static int file_local = 42;     /* Local - not exported */

void partsdb_open(void);        /* Strong symbol - function definition */
extern int external_var;        /* Reference - must be resolved */
```

**Symbol visibility control (security best practice):**
```c
/* Hide internal symbols from dynamic linking */
#if defined(__GNUC__) && __GNUC__ >= 4
    #define PARTSDB_PUBLIC __attribute__((visibility("default")))
    #define PARTSDB_PRIVATE __attribute__((visibility("hidden")))
#else
    #define PARTSDB_PUBLIC
    #define PARTSDB_PRIVATE
#endif

/* In partsdb.h - public API */
PARTSDB_PUBLIC partsdb_error_t partsdb_open(const char *path, partsdb_t **db);

/* In database.c - internal helper */
PARTSDB_PRIVATE static const char *sqlite_text(sqlite3_stmt *stmt, int col);
```

**Compile with hidden visibility by default:**
```makefile
CFLAGS += -fvisibility=hidden
```

### Static vs Dynamic Linking

```bash
# Static linking - all code included in executable
gcc -static database.c main.c -lsqlite3 -o partsdb-static
# Result: ~2MB executable, no runtime dependencies

# Dynamic linking - shared libraries loaded at runtime
gcc database.c main.c -lsqlite3 -o partsdb-dynamic
# Result: ~50KB executable, requires libsqlite3.so at runtime

# Hybrid - static SQLite, dynamic libc
gcc database.c main.c /usr/lib/libsqlite3.a -o partsdb-hybrid
```

**Security implications:**
| Aspect | Static | Dynamic |
|--------|--------|---------|
| Update vulnerabilities | Must recompile | Update library only |
| Attack surface | Self-contained | Shared library attacks possible |
| ASLR effectiveness | Limited | Full randomization |
| Deployment | Single file | Dependency management |

---

## 1.3 Memory Segments in Detail

### Process Virtual Address Space

```
High Address (0x7FFFFFFFFFFF on x86-64)
┌─────────────────────────────────────┐
│            Kernel Space              │  Not accessible from user mode
├─────────────────────────────────────┤  0x7FFFFFFFFFFF (user/kernel boundary)
│         Stack (grows down)           │  ← RSP (stack pointer)
│              ↓                       │
├─────────────────────────────────────┤
│                                      │
│      Unmapped Region (grows)         │  Guard pages, future allocations
│                                      │
├─────────────────────────────────────┤
│              ↑                       │
│          Heap (grows up)             │  malloc/calloc allocations
├─────────────────────────────────────┤
│           .bss                       │  Uninitialized globals (zeroed)
├─────────────────────────────────────┤
│           .data                      │  Initialized globals
├─────────────────────────────────────┤
│          .rodata                     │  String literals, const data
├─────────────────────────────────────┤
│           .text                      │  Executable code
├─────────────────────────────────────┤
│      Reserved (unmapped)             │  Catch NULL pointer dereferences
└─────────────────────────────────────┘
Low Address (0x0)
```

**Examine actual memory layout:**
```bash
# View memory map of running process
cat /proc/$(pgrep partsdb)/maps

# Output example:
# 555555554000-555555556000 r--p 00000000  .text (read, execute)
# 555555556000-555555558000 r-xp 00002000  .rodata (read only)
# 555555758000-555555759000 rw-p 00003000  .data (read, write)
# 7ffff7c00000-7ffff7c28000 r--p 00000000  libc.so.6
# 7ffffffde000-7ffffffff000 rw-p 00000000  [stack]
```

### Segment Permissions and Security

Each segment has specific permissions:

```c
/* .text - executable code (r-x) */
int main(void) {
    /* This code resides in .text */
    return 0;
}

/* .rodata - read-only data (r--) */
const char *message = "Hello, World!";  /* String in .rodata */
static const int MAGIC = 0xDEADBEEF;    /* Also .rodata */

/* .data - initialized read-write (rw-) */
int global_count = 42;                   /* Initialized global */
static char buffer[100] = "initial";     /* Initialized static */

/* .bss - uninitialized read-write (rw-), zeroed at load */
int uninitialized_global;                /* Goes to .bss, will be 0 */
static char large_buffer[10000];         /* .bss - doesn't increase file size */
```

**Security hardening with segment permissions:**
```makefile
# Enable NX (No-Execute) bit - prevent code execution on stack/heap
LDFLAGS += -z noexecstack

# Mark GOT as read-only after relocation (RELRO)
LDFLAGS += -z relro -z now

# Stack protector
CFLAGS += -fstack-protector-strong
```

---

## 1.4 Stack Frames and Calling Conventions

### x86-64 Calling Convention (System V AMD64 ABI)

```
┌─────────────────────────────────────┐
│      Caller's Stack Frame            │
├─────────────────────────────────────┤
│         Return Address               │  ← Pushed by CALL instruction
├─────────────────────────────────────┤
│         Saved RBP                    │  ← Frame pointer (optional)
├─────────────────────────────────────┤  ← RBP points here
│         Local Variable 1             │
├─────────────────────────────────────┤
│         Local Variable 2             │
├─────────────────────────────────────┤
│         ...                          │
├─────────────────────────────────────┤
│      Red Zone (128 bytes)            │  ← Usable without adjusting RSP
└─────────────────────────────────────┘  ← RSP points here
```

**Argument passing:**
- First 6 integer/pointer args: RDI, RSI, RDX, RCX, R8, R9
- First 8 floating point args: XMM0-XMM7
- Additional args: pushed on stack (right-to-left)
- Return value: RAX (integer), XMM0 (floating point)

```c
/* Example: partsdb_get_by_id calling convention */
partsdb_error_t partsdb_get_by_id(
    partsdb_t *db,      /* RDI - first arg */
    int64_t id,         /* RSI - second arg */
    part_t *part        /* RDX - third arg */
);

/* Return value in RAX (partsdb_error_t is int) */
```

**Corresponding assembly:**
```asm
partsdb_get_by_id:
    push    rbp                 ; Save caller's frame pointer
    mov     rbp, rsp            ; Establish our frame
    sub     rsp, 32             ; Allocate local variables

    mov     QWORD PTR [rbp-8], rdi    ; Save db
    mov     QWORD PTR [rbp-16], rsi   ; Save id
    mov     QWORD PTR [rbp-24], rdx   ; Save part

    ; ... function body ...

    mov     eax, 0              ; Return PARTSDB_OK
    leave                       ; Restore RSP, pop RBP
    ret                         ; Return to caller
```

### Stack Buffer Overflow Anatomy

```c
/* VULNERABLE CODE - DO NOT USE */
void vulnerable_function(const char *input) {
    char buffer[64];           /* Stack buffer */
    strcpy(buffer, input);     /* No bounds checking! */
}

/* Stack layout during overflow:

   Before overflow:          After overflow:
   ┌────────────────┐        ┌────────────────┐
   │ Return Address │        │ AAAA (attacker)│ ← Hijacked!
   ├────────────────┤        ├────────────────┤
   │   Saved RBP    │        │ AAAA (attacker)│
   ├────────────────┤        ├────────────────┤
   │                │        │ AAAAAAAAAAAAAA │
   │   buffer[64]   │        │ AAAAAAAAAAAAAA │
   │                │        │ AAAAAAAAAAAAAA │
   └────────────────┘        └────────────────┘
*/
```

**PartsDB secure alternative:**
```c
/* From database.c - safe string copy */
static void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= dest_size) {
        len = dest_size - 1;  /* Truncate to fit */
    }
    memcpy(dest, src, len);
    dest[len] = '\0';         /* Always null-terminate */
}
```

### Stack Canaries (Stack Protector)

GCC/Clang can insert "canary" values to detect stack smashing:

```c
void protected_function(void) {
    char buffer[64];
    /* Compiler inserts:
       1. Save random canary value on stack
       2. Before return, verify canary unchanged
       3. If changed, call __stack_chk_fail() - abort
    */
}
```

**Enable stack protection:**
```makefile
CFLAGS += -fstack-protector-strong  # Protect functions with arrays
# Or
CFLAGS += -fstack-protector-all     # Protect all functions (slower)
```

---

## 1.5 The Data Segment and BSS

### Understanding .data vs .bss

```c
/* .data segment - initialized data, stored in executable */
int initialized_global = 42;              /* 4 bytes in file */
char greeting[] = "Hello";                /* 6 bytes in file */
static double pi = 3.14159;               /* 8 bytes in file */

/* .bss segment - uninitialized data, NOT stored in file */
int uninitialized_global;                 /* 0 bytes in file, 4 bytes in memory */
static char big_buffer[1000000];          /* 0 bytes in file, 1MB in memory */
int zero_initialized = 0;                 /* Goes to .bss (optimization) */
```

**Why .bss matters:**
```bash
# Create test file
cat > test_bss.c << 'EOF'
int small_data = 42;
char big_bss[10000000];  /* 10MB uninitialized */
int main(void) { return 0; }
EOF

# Compile and check size
gcc test_bss.c -o test_bss
ls -la test_bss
# Output: ~16KB (not 10MB!) because .bss isn't stored

# View section sizes
size test_bss
#    text    data     bss     dec     hex filename
#    1234      56 10000004 10001294  989c2e test_bss
```

### Thread-Local Storage (.tdata, .tbss)

C11/C23 `_Thread_local` variables get their own segments:

```c
#include <threads.h>

/* Each thread gets its own copy */
_Thread_local int thread_counter = 0;           /* .tdata */
_Thread_local char thread_buffer[1024];         /* .tbss */

/* Compiler generates code to access via thread pointer (FS segment on x86-64) */
```

**Thread-local in PartsDB (hypothetical connection pool):**
```c
/* Per-thread database connection cache */
_Thread_local partsdb_t *tls_connection = nullptr;

partsdb_t *get_thread_connection(void) {
    if (tls_connection == nullptr) {
        partsdb_open(db_path, &tls_connection);
    }
    return tls_connection;
}
```

---

## 1.6 Virtual Memory and Address Spaces

### Page Tables and Address Translation

```
Virtual Address (48 bits used on x86-64)
┌───────┬───────┬───────┬───────┬──────────────┐
│ PML4  │ PDPT  │  PD   │  PT   │   Offset     │
│ 9 bits│ 9 bits│ 9 bits│ 9 bits│   12 bits    │
└───┬───┴───┬───┴───┬───┴───┬───┴──────┬───────┘
    │       │       │       │          │
    ▼       ▼       ▼       ▼          ▼
┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ ┌───────────┐
│ PML4  │─▶│ PDPT  │─▶│  PD   │─▶│  PT   │─▶│ 4KB Page  │
│ Table │  │ Table │  │ Table │  │ Table │  │ in RAM    │
└───────┘  └───────┘  └───────┘  └───────┘  └───────────┘
```

**Key concepts:**
- Page size: 4KB (default), 2MB (huge pages), 1GB (gigantic)
- Each process has its own page table hierarchy
- Copy-on-write (COW) enables efficient fork()
- Demand paging - pages loaded only when accessed

### Memory Mapped Files

```c
#include <sys/mman.h>
#include <fcntl.h>

/* Memory-map a database file for read-only access */
void *map_database(const char *path, size_t *size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return nullptr;

    struct stat st;
    fstat(fd, &st);
    *size = st.st_size;

    void *mapped = mmap(
        nullptr,            /* Let kernel choose address */
        st.st_size,        /* Map entire file */
        PROT_READ,         /* Read-only access */
        MAP_PRIVATE,       /* Private copy-on-write */
        fd,                /* File descriptor */
        0                  /* Offset in file */
    );

    close(fd);  /* Can close fd after mmap */

    if (mapped == MAP_FAILED) return nullptr;
    return mapped;
}

/* Unmap when done */
void unmap_database(void *mapped, size_t size) {
    munmap(mapped, size);
}
```

### Address Space Layout Randomization (ASLR)

ASLR randomizes memory layout to thwart exploits:

```bash
# Check ASLR status
cat /proc/sys/kernel/randomize_va_space
# 0 = disabled, 1 = stack/libs, 2 = full (includes heap)

# View randomization in action
for i in 1 2 3; do
    cat /proc/self/maps | grep stack
done
# Different addresses each time with ASLR enabled
```

**Compile for full ASLR support:**
```makefile
# Position Independent Executable
CFLAGS += -fPIE
LDFLAGS += -pie

# Verify PIE is enabled
readelf -h partsdb | grep Type
# Should show: Type: DYN (Position-Independent Executable file)
```

---

## 1.7 PartsDB Case Study: Memory Layout Analysis

### Analyzing PartsDB Memory Usage

```bash
# Compile PartsDB with debug symbols
cd src/partsdb-cli
make clean && make debug

# Run with memory analysis
valgrind --tool=massif ./bin/partsdb ../database/partsdb.sqlite

# View memory profile
ms_print massif.out.*
```

### Structure Padding Analysis

```c
/* Examine PartsDB's part_t structure */
#include <stddef.h>
#include <stdio.h>

void analyze_part_t_layout(void) {
    printf("part_t layout analysis:\n");
    printf("  sizeof(part_t) = %zu\n", sizeof(part_t));
    printf("  offsetof(id) = %zu\n", offsetof(part_t, id));
    printf("  offsetof(part_number) = %zu\n", offsetof(part_t, part_number));
    printf("  offsetof(name) = %zu\n", offsetof(part_t, name));
    printf("  offsetof(quantity) = %zu\n", offsetof(part_t, quantity));
    printf("  offsetof(unit_price) = %zu\n", offsetof(part_t, unit_price));

    /* Check for padding holes */
    printf("\nPadding analysis:\n");
    printf("  After id: %zu bytes\n",
           offsetof(part_t, part_number) - offsetof(part_t, id) - sizeof(int64_t));
    /* ... etc ... */
}
```

### Security-Focused Memory Hardening

```c
/* Secure memory clearing for PartsDB */
#ifdef _WIN32
#include <windows.h>
#define secure_zero(ptr, size) SecureZeroMemory(ptr, size)
#else
/* Volatile pointer prevents optimization */
static void secure_zero(void *ptr, size_t size) {
    volatile unsigned char *p = ptr;
    while (size--) {
        *p++ = 0;
    }
}
#endif

/* Use when closing database with sensitive data */
void partsdb_close_secure(partsdb_t *db) {
    if (db == nullptr) return;

    if (db->db != nullptr) {
        sqlite3_close(db->db);
        db->db = nullptr;
    }

    if (db->path != nullptr) {
        size_t path_len = strlen(db->path);
        secure_zero(db->path, path_len);  /* Clear path from memory */
        free(db->path);
        db->path = nullptr;
    }

    secure_zero(db, sizeof(partsdb_t));  /* Clear struct */
    free(db);
}
```

---

## 1.8 Review Questions - Part 1

> **Instructions**: Research these questions using *Expert C Programming* and *CS:APP*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[Deep C Secrets, Ch. 6]** Explain why the following code might behave differently at `-O0` vs `-O3`:
   ```c
   int *p = malloc(sizeof(int));
   *p = 42;
   free(p);
   printf("%d\n", *p);  /* What happens? */
   ```

2. **[CS:APP, Ch. 7]** What is the difference between a *strong symbol* and a *weak symbol*? How does the linker resolve conflicts between them? Give an example where this could cause subtle bugs.

3. **[Deep C Secrets, Ch. 4]** Why does C have both `.data` and `.bss` segments? Calculate the executable size difference for a program with `int array[1000000] = {0};` vs `int array[1000000];`.

4. **[CS:APP, Ch. 9]** Explain how copy-on-write (COW) allows `fork()` to be efficient. What happens to shared pages when either process writes to them?

5. **[Deep C Secrets, Ch. 5]** What is the "red zone" in the x86-64 ABI? Why does it exist, and when can it cause problems?

### Practical Exercises

6. **Memory Map Investigation**: Use `/proc/[pid]/maps` to analyze the PartsDB process memory layout. Identify:
   - The location of the SQLite library
   - The heap boundaries
   - Stack location and size limit
   - Any memory-mapped files

7. **Symbol Visibility Audit**: Examine PartsDB's exported symbols with `nm -D partsdb`. Identify symbols that should be hidden and implement `__attribute__((visibility("hidden")))` for internal functions.

8. **Stack Frame Analysis**: Disassemble `partsdb_search()` with `objdump -d` and draw the complete stack frame layout. Identify all saved registers, local variables, and the return address location.

9. **ASLR Verification**: Write a test program that prints the addresses of:
   - A function in .text
   - A global variable in .data
   - A local variable on the stack
   - A heap allocation

   Run it multiple times and verify ASLR randomization.

10. **Security Hardening**: Add the following security features to the PartsDB Makefile:
    - Full RELRO (`-Wl,-z,relro,-z,now`)
    - Stack canaries (`-fstack-protector-strong`)
    - FORTIFY_SOURCE (`-D_FORTIFY_SOURCE=2`)
    - Position independent code (`-fPIE -pie`)

    Verify each with `checksec` or `readelf`.

### Research Topics

11. **[CS:APP, Ch. 7]** Research *interpositioning* - how can you intercept calls to `malloc()` in a running program without modifying its source code? Implement a memory usage tracker for PartsDB using `LD_PRELOAD`.

12. **[Deep C Secrets, Ch. 3]** The C standard allows implementations to use different representations for pointers to different types. Research *segmented memory models* and explain why `void *` is guaranteed to hold any object pointer but not necessarily a function pointer.

13. **[CS:APP, Ch. 9]** Research *huge pages* (2MB and 1GB pages). When would using huge pages benefit PartsDB? Implement a compile-time option to use `madvise(MADV_HUGEPAGE)` for large allocations.

---

[← Back to Index](README.md) | [Part 2: Modern Tooling →](part-02-modern-tooling.md)
