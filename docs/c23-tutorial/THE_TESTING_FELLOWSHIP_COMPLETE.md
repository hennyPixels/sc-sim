# The Testing Fellowship

## A Complete Chronicle of Memory, Debugging, and the Pursuit of Correctness

---

```
    ╔══════════════════════════════════════════════════════════════════════╗
    ║                                                                      ║
    ║                     THE TESTING FELLOWSHIP                           ║
    ║                                                                      ║
    ║              A Chronicle of Memory, Debugging, and                   ║
    ║                  the Pursuit of Correctness                          ║
    ║                                                                      ║
    ║    ─────────────────────────────────────────────────────────────     ║
    ║                                                                      ║
    ║    Part I:   The Forming of the Fellowship                           ║
    ║    Part II:  The Two Allocators                                      ║
    ║    Part III: The Return of the Profiler                              ║
    ║                                                                      ║
    ║    ─────────────────────────────────────────────────────────────     ║
    ║                                                                      ║
    ║    Created during the gperftools integration session                 ║
    ║    December 2024                                                     ║
    ║                                                                      ║
    ╚══════════════════════════════════════════════════════════════════════╝
```

---

<div style="page-break-after: always;"></div>

## Table of Contents

### PART I: THE FORMING OF THE FELLOWSHIP

1. **Prologue: Before the Dawn of Debugging**
2. **Chapter 1: The Ancients of the Kernel**
   - Dennis the Wise (Ritchie)
   - Brian the Scribe (Kernighan)
   - Ken the Architect (Thompson)
3. **Chapter 2: The Keepers of the Memory Palace**
   - Doug Lea - The Allocator
   - Sanjay Ghemawat - The Thread-Weaver
4. **Chapter 3: The Young Upstarts**
   - Kostya Serebryany - The Sanitizer
   - The pprof Collective - The Profilers
5. **Chapter 4: The Gathering at Rivendell (Bell Labs)**
6. **Chapter 5: The Nine Bugs of Power**
7. **Chapter 6: The Fellowship's Oath**
8. **Epilogue: The Road Goes Ever On**

### PART II: THE TWO ALLOCATORS

1. **Prologue: The Schism of Memory Management**
2. **Chapter 1: The Architecture of TCMalloc**
3. **Chapter 2: The Architecture of jemalloc**
4. **Chapter 3: The Battle of Benchmarks**
5. **Chapter 4: The Detailed Statistics**
6. **Chapter 5: Choosing Your Allocator**
7. **Chapter 6: Practical Integration**
8. **Epilogue: The Wisdom of the Two Allocators**

### PART III: THE RETURN OF THE PROFILER

**Book One: The Depths of the Kernel**
1. **Chapter 1: The Kernel's Memory Palace**
2. **Chapter 2: Virtual Memory - The Great Illusion**
3. **Chapter 3: The brk() and mmap() Syscalls**
4. **Chapter 4: The Page Fault Handler**

**Book Two: The SEI CERT Standard**
5. **Chapter 5: The Laws of Safe Memory**
6. **Chapter 6: The Integer Overflow Menace**
7. **Chapter 7: The Array Bounds Fortress**

**Book Three: The Quest for gperftools**
8. **Chapter 8: The Journey Begins**
9. **Chapter 9: The Path Format Peril**
10. **Chapter 10: Installing the Toolchain**
11. **Chapter 11: The Clone and the Build Attempt**
12. **Chapter 12: The Battle with WaitOnAddress**
13. **Chapter 13: Victory and Integration**
14. **Chapter 14: The Benchmarks of Truth**

**Book Four: The Profiler's Arsenal**
15. **Chapter 15: CPU Profiling (The Linux Path)**
16. **Chapter 16: Heap Profiling**
17. **Chapter 17: The Heap Checker**

**Book Five: Wisdom for the Journey**
18. **Chapter 18: The Debugging Manifesto**
19. **Chapter 19: The Complete Toolchain**
20. **Chapter 20: The Final Synthesis**

**Appendices**
- SEI CERT Memory Rules Quick Reference
- TCMalloc API Quick Reference
- Build Commands Quick Reference

---

<div style="page-break-after: always;"></div>

```
    "In the land of Silicon, in the fires of Mount Compiler,
     the Dark Lord Undefined Behavior forged in secret a master bug,
     to corrupt all programs. And into this bug he poured
     his cruelty, his malice, and his will to segfault all of memory."

                                        — The Silmarillion of Systems Programming
```

---

<div style="page-break-after: always;"></div>

# PART I: THE FORMING OF THE FELLOWSHIP

---

## Prologue: Before the Dawn of Debugging

In the beginning, there was only the Machine.

It spoke in voltages—high and low, one and zero—and those who wished to commune with it had to learn its native tongue. There were no error messages, no stack traces, no helpful compiler warnings. There was only the blinking light of a halted processor and the acrid smell of overheated vacuum tubes.

The first programmers were not called programmers at all. They were "computors"—human beings who computed. When the machines came, these humans became translators, mediating between the world of human intention and the world of electronic execution.

And in that translation, bugs were born.

---

<div style="page-break-after: always;"></div>

## Chapter 1: The Ancients of the Kernel

### The First Elder: Dennis the Wise (Ritchie)

```
    "The only way to learn a new programming language is by
     writing programs in it."
                                        — Dennis Ritchie, 1978
```

In the halls of Bell Labs, during the age of the PDP-11, there walked a wizard named Dennis. With his companion Ken (Thompson), he forged a new language—neither the cryptic incantations of Assembly nor the verbose ceremonies of COBOL, but something in between.

They called it **C**.

```c
/* The first C program, written in the tongue of the Ancients */
main()
{
    printf("hello, world\n");
}
```

This simple invocation would echo through the ages. But within its simplicity lay both power and peril. C gave programmers direct access to memory—a gift that was also a curse.

Dennis spoke of this duality:

> "C is quirky, flawed, and an enormous success. While accidents of history surely helped, it evidently satisfied a need for a system implementation language efficient enough to displace assembly language, yet sufficiently abstract and fluent to describe algorithms and interactions in a wide variety of environments."

The Ancients understood that with great power came great responsibility—and great bugs.

### The Second Elder: Brian the Scribe (Kernighan)

While Dennis forged the language, Brian documented it. His tome, written with Dennis, became known simply as **K&R**—a sacred text that would guide generations of programmers.

But Brian's greatest contribution to debugging came from a simple practice he championed:

```c
/* Brian's First Law of Debugging */
#include <stdio.h>

void mysterious_function(int *ptr) {
    printf("DEBUG: ptr = %p, *ptr = %d\n", (void*)ptr, *ptr);  /* The printf debugger */

    /* The actual code */
    *ptr = *ptr + 1;

    printf("DEBUG: after increment, *ptr = %d\n", *ptr);
}
```

This technique—the humble `printf` statement—would become known as "printf debugging" or, more reverently, "The Way of Kernighan." It was primitive, yet effective. It required no special tools, no debugger integration, no complex setup. Just print what you need to see.

Brian also gave us a warning that would prove prophetic:

> "Debugging is twice as hard as writing the code in the first place. Therefore, if you write the code as cleverly as possible, you are, by definition, not smart enough to debug it."

### The Third Elder: Ken the Architect (Thompson)

Ken Thompson, co-creator of Unix and C, understood something fundamental about bugs: they often hide in the tools themselves.

In his famous Turing Award lecture, "Reflections on Trusting Trust," Ken revealed a terrifying truth—a compiler could be modified to insert bugs into any program it compiled, including future versions of itself. The bug would be invisible in the source code.

```c
/* Ken's Paradox: Can you trust your tools? */

/*
 * Imagine a compiler that, when compiling the login program,
 * inserts code to accept a secret password.
 *
 * And when compiling itself, inserts the code to do the above.
 *
 * The source code would be clean.
 * But the binary would be corrupt.
 * Forever.
 */
```

This teaching reminded the Fellowship that debugging was not merely about finding bugs in code—it was about understanding the entire chain of trust from hardware to application.

---

<div style="page-break-after: always;"></div>

## Chapter 2: The Keepers of the Memory Palace

### The Allocator: Doug Lea

In the realm of memory management, one name echoes through the ages: **Doug Lea**. His malloc implementation—dlmalloc—became the foundation upon which countless programs were built.

Doug understood that memory allocation was not merely a technical problem but an *architectural* one. Memory had structure. It had patterns. It had... a palace.

```c
/*
 * THE MEMORY PALACE OF DOUG LEA
 *
 * Imagine memory as a grand palace with many rooms.
 * Each room (chunk) has a header describing its size.
 * Free rooms are linked together in chains (free lists).
 * The allocator is the palace keeper, finding rooms for guests.
 *
 *    ┌─────────────────────────────────────────────────────────┐
 *    │                    THE HEAP                              │
 *    │  ┌──────┐ ┌──────────┐ ┌────┐ ┌──────────────┐ ┌─────┐  │
 *    │  │ USED │ │   FREE   │ │USED│ │     FREE     │ │USED │  │
 *    │  │ 64B  │ │   128B   │ │32B │ │     256B     │ │ 64B │  │
 *    │  └──────┘ └──────────┘ └────┘ └──────────────┘ └─────┘  │
 *    │            ↓                    ↓                        │
 *    │      ┌─────┴────────────────────┴─────┐                  │
 *    │      │        FREE LIST               │                  │
 *    │      │    (chain of empty rooms)      │                  │
 *    │      └────────────────────────────────┘                  │
 *    └─────────────────────────────────────────────────────────┘
 */

struct malloc_chunk {
    size_t prev_size;    /* Size of previous chunk (if free) */
    size_t size;         /* Size of this chunk */

    /* Only in free chunks: */
    struct malloc_chunk *fd;  /* Forward link in free list */
    struct malloc_chunk *bk;  /* Backward link in free list */
};
```

But palaces can be corrupted. The Dark Lord of Undefined Behavior could overflow a buffer and corrupt the metadata, causing the palace to crumble.

### The Thread-Weaver: Sanjay Ghemawat

As programs grew more complex, they needed to do many things at once. Threads emerged—parallel paths of execution that shared the same memory palace.

But threads brought chaos. When two threads tried to allocate memory simultaneously, they would clash at the palace gates, each waiting for the other, slowing everything to a crawl.

Then came **Sanjay Ghemawat** of Google, who created **TCMalloc**—the Thread-Caching Malloc. His innovation was simple yet profound: give each thread its own antechamber.

```c
/*
 * THE THREAD-CACHING INNOVATION
 *
 * Before TCMalloc:
 *     Thread 1 ─┐
 *     Thread 2 ─┼──► [LOCK] ──► Central Heap ──► [UNLOCK]
 *     Thread 3 ─┘
 *                    (everyone waits in line)
 *
 * After TCMalloc:
 *     Thread 1 ──► [Thread Cache 1] ──┐
 *     Thread 2 ──► [Thread Cache 2] ──┼──► Central Cache ──► Page Heap
 *     Thread 3 ──► [Thread Cache 3] ──┘
 *                    (each thread has its own cache)
 */

/*
 * The three tiers of TCMalloc's palace:
 *
 * TIER 1: THREAD CACHE (The Antechamber)
 *   - Each thread has private free lists
 *   - No locks needed for small allocations
 *   - Lightning fast: O(1)
 *
 * TIER 2: CENTRAL CACHE (The Great Hall)
 *   - Shared between threads
 *   - Locked, but transfers happen in batches
 *   - Refills thread caches when empty
 *
 * TIER 3: PAGE HEAP (The Vault)
 *   - Manages large spans of pages
 *   - Interfaces with the operating system
 *   - Where memory truly lives and dies
 */
```

This architecture achieved something remarkable: **1,852 allocations per millisecond**—each one a guest entering the palace, finding their room, and settling in, all without blocking others at the gate.

---

<div style="page-break-after: always;"></div>

## Chapter 3: The Young Upstarts

### The Sanitizer: Kostya Serebryany

In the modern age, a new generation of debugging tools emerged. They were not content to merely observe bugs—they wanted to *catch* them in the act.

**Kostya Serebryany** at Google created the **Sanitizers**—a family of tools that could detect memory errors, data races, and undefined behavior at runtime.

```c
/*
 * THE ADDRESSSANITIZER (ASan)
 *
 * ASan surrounds every allocation with "poisoned" red zones.
 * Any access to these zones triggers an immediate alert.
 *
 *     ┌─────────────────────────────────────────────────────┐
 *     │ POISON │    YOUR DATA    │ POISON │    NEXT    │    │
 *     │ (red)  │   (allocated)   │ (red)  │  OBJECT    │    │
 *     └─────────────────────────────────────────────────────┘
 *              ↑                 ↑
 *              │                 │
 *        Valid access       Buffer overflow!
 *                            ASan catches it!
 */

/* Example: ASan catching a buffer overflow */
int main() {
    int array[10];

    /* This will be caught by ASan */
    array[10] = 42;  /* ERROR: stack-buffer-overflow */

    return 0;
}

/*
 * ASan output:
 * ==12345==ERROR: AddressSanitizer: stack-buffer-overflow
 * WRITE of size 4 at 0x7ffd12345678
 * #0 0x4005a3 in main test.c:5
 */
```

The Sanitizers could detect:
- **Buffer overflows** (writing beyond array bounds)
- **Use after free** (accessing memory after `free()`)
- **Double free** (freeing the same memory twice)
- **Memory leaks** (allocating without freeing)
- **Data races** (threads accessing shared data unsafely)

### The Profiler: The pprof Collective

While the Sanitizers caught bugs, another group sought to understand *performance*. Where was time being spent? Which functions were slow? Where did memory go?

The **pprof** tool emerged from Google's performance engineering culture. It could visualize program execution as flame graphs—towering infernos where the hottest code burned brightest.

```
/*
 * THE FLAME GRAPH
 *
 * Width = time spent (or memory allocated)
 * Height = call stack depth
 *
 *                    ┌───────────────────┐
 *                    │   expensive_fn()  │
 *              ┌─────┴───────────────────┴─────┐
 *              │        process_data()         │
 *        ┌─────┴───────────────────────────────┴─────┐
 *        │              main_loop()                  │
 *    ────┴───────────────────────────────────────────┴────
 *
 * Reading the flame graph:
 *   - Wide bars = functions consuming lots of time
 *   - Tall stacks = deep call hierarchies
 *   - Look for wide bars at the top = optimization targets
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 4: The Gathering at Rivendell (Bell Labs)

And so it was that the Fellowship came together.

From the East came the **Ancients**:
- **Dennis** with his gift of C
- **Brian** with his printf wisdom
- **Ken** with his trust teachings
- **Doug** with his memory palace

From the West came the **Keepers**:
- **Sanjay** with TCMalloc's thread caches
- **The Valgrind Council** with their memory scrutiny
- **The GDB Order** with their step-by-step interrogation

From the North came the **Young Upstarts**:
- **Kostya** with the Sanitizers
- **The pprof Collective** with their flame graphs
- **The Rust Evangelists** with their borrow checker (though some called them heretics)

```c
/*
 * THE FELLOWSHIP'S TOOLS
 *
 * Each member brought their weapon against bugs:
 */

/* Dennis's Gift: The Language Itself */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Brian's Wisdom: Printf Debugging */
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)

/* Doug's Palace: Memory Management */
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
    DEBUG_PRINT("Allocated %zu bytes at %p", size, ptr);
    return ptr;
}

/* Sanjay's Innovation: TCMalloc Statistics */
#ifdef USE_TCMALLOC
#include <gperftools/malloc_extension_c.h>

void report_memory_stats(void) {
    size_t allocated, heap_size;
    MallocExtension_GetNumericProperty(
        "generic.current_allocated_bytes", &allocated);
    MallocExtension_GetNumericProperty(
        "generic.heap_size", &heap_size);

    DEBUG_PRINT("Memory: %zu / %zu bytes (%.1f%% efficiency)",
                allocated, heap_size,
                (double)allocated / heap_size * 100.0);
}
#endif

/* Kostya's Sentinel: Sanitizer Annotations */
#if defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED 1
#include <sanitizer/asan_interface.h>
#endif

/* The Complete Arsenal */
typedef struct {
    const char *name;
    const char *domain;
    const char *weapon;
} FellowshipMember;

static const FellowshipMember fellowship[] = {
    {"Dennis Ritchie",     "Language Design",    "C Programming Language"},
    {"Brian Kernighan",    "Documentation",      "Printf Debugging"},
    {"Ken Thompson",       "Systems",            "Trust Analysis"},
    {"Doug Lea",           "Memory",             "dlmalloc"},
    {"Sanjay Ghemawat",    "Performance",        "TCMalloc"},
    {"Kostya Serebryany",  "Safety",             "AddressSanitizer"},
    {"Julian Seward",      "Verification",       "Valgrind"},
    {"Richard Stallman",   "Debugging",          "GDB"},
};
```

---

<div style="page-break-after: always;"></div>

## Chapter 5: The Nine Bugs of Power

The Fellowship knew their enemy. The Dark Lord had forged Nine Bugs of Power, each capable of corrupting programs and crashing systems:

### Bug 1: The Buffer Overflow

```c
/*
 * THE BUFFER OVERFLOW
 * First and most ancient of the Nine
 *
 * It strikes when boundaries are forgotten,
 * when arrays are indexed beyond their bounds,
 * when strings grow longer than their homes.
 */

void vulnerable_function(const char *user_input) {
    char buffer[64];

    /* THE CORRUPTION */
    strcpy(buffer, user_input);  /* No length check! */

    /* If user_input > 63 chars, it overwrites:
     *   - Other local variables
     *   - Saved frame pointer
     *   - Return address (!)
     *
     * An attacker can redirect execution anywhere.
     */
}

/* THE DEFENSE */
void safe_function(const char *user_input) {
    char buffer[64];

    /* Bounded copy - the Fellowship's way */
    strncpy(buffer, user_input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    /* Or better: snprintf */
    snprintf(buffer, sizeof(buffer), "%s", user_input);
}
```

### Bug 2: The Use-After-Free

```c
/*
 * THE USE-AFTER-FREE
 * It haunts memory long after it should be forgotten
 */

void haunted_function(void) {
    char *ghost = malloc(100);
    strcpy(ghost, "I am still here");

    free(ghost);  /* The ghost is released... */

    /* THE HAUNTING */
    printf("%s\n", ghost);  /* ...but we still speak to it! */

    /* The memory may have been reused.
     * We might read garbage, or worse,
     * corrupt another allocation's data.
     */
}

/* THE EXORCISM */
void cleansed_function(void) {
    char *spirit = malloc(100);
    strcpy(spirit, "I am here");

    free(spirit);
    spirit = NULL;  /* Sever the connection */

    /* Now any accidental use will crash immediately,
     * rather than corrupting silently.
     */
    if (spirit) {
        printf("%s\n", spirit);
    }
}
```

### Bug 3: The Double Free

```c
/*
 * THE DOUBLE FREE
 * Releasing what has already been released
 * corrupts the memory palace's ledgers
 */

void corrupt_the_palace(void) {
    char *victim = malloc(100);

    free(victim);  /* First release */
    free(victim);  /* THE CORRUPTION - palace ledgers corrupted! */

    /* The free list now contains a cycle.
     * Future allocations may return the same memory twice.
     * Two "owners" will corrupt each other's data.
     */
}

/* THE SAFEGUARD */
void protect_the_palace(void) {
    char *item = malloc(100);

    free(item);
    item = NULL;  /* Mark as released */

    /* Second free is now safe (freeing NULL is a no-op) */
    free(item);
}
```

### Bug 4: The Memory Leak

```c
/*
 * THE MEMORY LEAK
 * Allocations that are forgotten, never to be freed
 * They accumulate like debt, slowly consuming all resources
 */

void leaky_function(void) {
    while (1) {
        char *forgotten = malloc(1024);
        process(forgotten);

        /* THE LEAK - we never free! */
        /* Memory grows without bound */
    }
}

/* THE DISCIPLINE */
void disciplined_function(void) {
    while (1) {
        char *remembered = malloc(1024);
        process(remembered);
        free(remembered);  /* Always release what you claim */
    }
}
```

### Bug 5: The Null Dereference

```c
/*
 * THE NULL DEREFERENCE
 * Reaching into the void and expecting substance
 */

void reach_into_void(int id) {
    Part *part = find_part(id);

    /* THE FALL INTO VOID */
    printf("Part name: %s\n", part->name);  /* If part is NULL... CRASH! */
}

/* THE CAREFUL PATH */
void walk_carefully(int id) {
    Part *part = find_part(id);

    if (part == NULL) {
        printf("Part not found\n");
        return;
    }

    printf("Part name: %s\n", part->name);  /* Safe */
}
```

### Bug 6: The Integer Overflow

```c
/*
 * THE INTEGER OVERFLOW
 * When numbers grow beyond their vessel
 */

void overflow_the_vessel(void) {
    size_t count = get_user_count();      /* User says: 1,000,000,000 */
    size_t size = count * sizeof(Part);   /* THE OVERFLOW if Part is large */

    /* If count * sizeof(Part) > SIZE_MAX, it wraps around!
     * We might allocate a tiny buffer thinking it's huge.
     */
    Part *parts = malloc(size);  /* Allocates wrong size! */
}

/* THE BOUNDED ARITHMETIC */
#include <stdint.h>

void bounded_calculation(void) {
    size_t count = get_user_count();

    /* Check for overflow before multiplication */
    if (count > SIZE_MAX / sizeof(Part)) {
        fprintf(stderr, "Allocation too large\n");
        return;
    }

    size_t size = count * sizeof(Part);  /* Safe */
    Part *parts = malloc(size);
}
```

### Bug 7: The Format String

```c
/*
 * THE FORMAT STRING VULNERABILITY
 * When user input becomes the spell itself
 */

void dangerous_incantation(const char *user_input) {
    /* THE VULNERABILITY */
    printf(user_input);  /* If user_input is "%s%s%s%s%s"... CRASH! */
                         /* If user_input is "%n"... ARBITRARY WRITE! */
}

/* THE SAFE INVOCATION */
void safe_incantation(const char *user_input) {
    printf("%s", user_input);  /* User input is DATA, not FORMAT */
}
```

### Bug 8: The Race Condition

```c
/*
 * THE RACE CONDITION
 * When parallel paths collide without coordination
 */

static int shared_counter = 0;

void *racing_thread(void *arg) {
    for (int i = 0; i < 1000000; i++) {
        /* THE RACE */
        shared_counter++;  /* Read-modify-write is NOT atomic! */

        /* Thread 1: reads 5
         * Thread 2: reads 5
         * Thread 1: writes 6
         * Thread 2: writes 6  <-- Lost update!
         */
    }
    return NULL;
}

/* THE SYNCHRONIZATION */
#include <pthread.h>
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void *coordinated_thread(void *arg) {
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&counter_lock);
        shared_counter++;  /* Protected by lock */
        pthread_mutex_unlock(&counter_lock);
    }
    return NULL;
}
```

### Bug 9: The Uninitialized Memory

```c
/*
 * THE UNINITIALIZED MEMORY
 * Reading from memory before writing to it
 * reveals ghosts of computations past
 */

void read_the_ghosts(void) {
    int array[100];

    /* THE GHOST READING */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i];  /* What values? Who knows! */
    }
    printf("Sum: %d\n", sum);  /* Unpredictable! */
}

/* THE CLEAN SLATE */
void start_fresh(void) {
    int array[100] = {0};  /* Initialize everything */

    /* Or use calloc for heap allocations */
    int *heap_array = calloc(100, sizeof(int));  /* Zero-initialized */
}
```

---

<div style="page-break-after: always;"></div>

## Chapter 6: The Fellowship's Oath

Before departing on their quest, the Fellowship swore an oath—a set of principles that would guide them through the darkest debugging sessions:

```c
/*
 * THE OATH OF THE TESTING FELLOWSHIP
 *
 * We, the defenders of correct code, do solemnly swear:
 */

/* PRINCIPLE 1: Compile with all warnings */
// gcc -Wall -Wextra -Wpedantic -Werror

/* PRINCIPLE 2: Use static analysis */
// clang --analyze source.c

/* PRINCIPLE 3: Test with sanitizers */
// gcc -fsanitize=address,undefined source.c

/* PRINCIPLE 4: Verify with dynamic analysis */
// valgrind --leak-check=full ./program

/* PRINCIPLE 5: Profile before optimizing */
// CPUPROFILE=output.prof ./program
// pprof --text ./program output.prof

/* PRINCIPLE 6: Document assumptions */
#include <assert.h>
void process_part(Part *part) {
    assert(part != NULL && "Part must not be null");
    assert(part->quantity >= 0 && "Quantity must be non-negative");
    /* ... */
}

/* PRINCIPLE 7: Fail fast and loud */
#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FATAL: %s at %s:%d\n", \
                message, __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)

/* PRINCIPLE 8: Leave the code cleaner than you found it */
/* (The Boy Scout Rule) */

/* PRINCIPLE 9: When in doubt, write a test */
void test_part_creation(void) {
    Part *part = create_part("TEST-001", "Test Part", 100);
    assert(part != NULL);
    assert(strcmp(part->part_number, "TEST-001") == 0);
    assert(part->quantity == 100);
    free_part(part);
    printf("test_part_creation: PASSED\n");
}
```

---

<div style="page-break-after: always;"></div>

## Epilogue: The Road Goes Ever On

And so the Fellowship was formed.

They carried with them the wisdom of the Ancients, the techniques of the Keepers, and the tools of the Young Upstarts. They knew the Nine Bugs of Power and how to defeat each one.

But their quest was not to destroy all bugs—for that would be impossible. Their quest was to build systems that could detect bugs early, contain their damage, and recover gracefully.

```c
/*
 * THE FELLOWSHIP'S CREED
 *
 * We cannot prevent all bugs.
 * But we can:
 *   - Catch them early (static analysis, compile warnings)
 *   - Detect them quickly (sanitizers, assertions)
 *   - Understand them deeply (debuggers, profilers)
 *   - Fix them permanently (tests, documentation)
 *
 * The road goes ever on,
 * From bug to fix, from crash to stability,
 * Each commit a step forward,
 * Each test a guardian against regression.
 */

int main(int argc, char *argv[]) {
    /* Initialize the Fellowship's tools */
    init_logging();
    init_memory_tracking();

    /* The quest begins */
    printf("The Testing Fellowship stands ready.\n");
    printf("May your builds be clean and your tests be green.\n");

    /* Run all tests */
    run_test_suite();

    /* Report memory status */
    report_memory_stats();

    return 0;
}
```

---

```
    "Not all bugs that crash are lost,
     Not all memory that's freed is reclaimed,
     The code that is strong does not wither,
     Deep roots are not reached by the frost."

                                        — The Fellowship of the Test
```

---

# END OF PART I

---

<div style="page-break-after: always;"></div>

```
    "The world is changed. I feel it in the heap.
     I feel it in the stack. I smell it in the registers.
     Much that once was allocated is lost,
     for none now live who remember to free it."

                                        — Galadriel, Lady of the Memory Pool
```

---

<div style="page-break-after: always;"></div>

# PART II: THE TWO ALLOCATORS

---

## Prologue: The Schism of Memory Management

In the ages after Doug Lea created his malloc, the world of memory allocation split into two great kingdoms.

In the East rose **TCMalloc**, forged in the fires of Google's data centers, optimized for the many-threaded workloads of web services.

In the West grew **jemalloc**, born in the halls of FreeBSD, adopted by Facebook, designed for the complex allocation patterns of modern applications.

Both claimed to be the true heir to Doug Lea's legacy. Both promised performance. Both had their champions.

This is the tale of their rivalry, and how the Fellowship learned from both.

---

<div style="page-break-after: always;"></div>

## Chapter 1: The Architecture of TCMalloc

### The Google Way

TCMalloc was born from necessity. Google's servers handled billions of requests, each spawning threads, each thread allocating memory. The old ways—a single lock protecting the heap—could not scale.

Sanjay Ghemawat and Paul Menage designed a new architecture: give each thread its own cache.

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                     THE REALM OF TCMALLOC                            ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                                                                      ┃
┃    ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                ┃
┃    │  Thread 1   │  │  Thread 2   │  │  Thread 3   │                ┃
┃    │   Cache     │  │   Cache     │  │   Cache     │   TIER 1       ┃
┃    │ ┌─────────┐ │  │ ┌─────────┐ │  │ ┌─────────┐ │   Thread       ┃
┃    │ │ 8 byte  │ │  │ │ 8 byte  │ │  │ │ 8 byte  │ │   Caches       ┃
┃    │ │ 16 byte │ │  │ │ 16 byte │ │  │ │ 16 byte │ │   (No locks!)  ┃
┃    │ │ 32 byte │ │  │ │ 32 byte │ │  │ │ 32 byte │ │                ┃
┃    │ │   ...   │ │  │ │   ...   │ │  │ │   ...   │ │                ┃
┃    │ └─────────┘ │  │ └─────────┘ │  │ └─────────┘ │                ┃
┃    └──────┬──────┘  └──────┬──────┘  └──────┬──────┘                ┃
┃           │                │                │                        ┃
┃           └────────────────┼────────────────┘                        ┃
┃                            ▼                                         ┃
┃    ┌──────────────────────────────────────────────────────┐         ┃
┃    │                  CENTRAL CACHE                        │  TIER 2 ┃
┃    │  ┌────────────────────────────────────────────────┐  │  Central ┃
┃    │  │  Transfer Cache (batch transfers)              │  │  Cache   ┃
┃    │  │  Central Free Lists (size-segregated)          │  │  (Locked)┃
┃    │  └────────────────────────────────────────────────┘  │         ┃
┃    └──────────────────────────┬───────────────────────────┘         ┃
┃                               ▼                                      ┃
┃    ┌──────────────────────────────────────────────────────┐         ┃
┃    │                    PAGE HEAP                          │  TIER 3 ┃
┃    │  ┌────────────────────────────────────────────────┐  │  Page    ┃
┃    │  │  Span Management (contiguous pages)            │  │  Heap    ┃
┃    │  │  Page Map (virtual address → span lookup)      │  │  (Rare)  ┃
┃    │  │  System Allocator Interface (mmap/VirtualAlloc)│  │         ┃
┃    │  └────────────────────────────────────────────────┘  │         ┃
┃    └──────────────────────────────────────────────────────┘         ┃
┃                                                                      ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### The Size Classes

TCMalloc does not allocate exact sizes. It rounds up to predefined "size classes"—a wisdom learned from Doug Lea. This reduces fragmentation and allows objects of the same size to be managed together.

```c
/*
 * THE SIZE CLASSES OF TCMALLOC
 *
 * When you request memory, TCMalloc rounds up to the next size class.
 * This is not waste—it is wisdom.
 *
 * By grouping similar sizes together:
 *   - Free lists are more efficient
 *   - Fragmentation is reduced
 *   - Cache locality improves
 */

/*
 * Observed size classes from our profiling session:
 *
 * ┌────────────────────────────────────────────────────────────┐
 * │  Requested  │  Allocated  │  Overhead  │  Efficiency       │
 * ├─────────────┼─────────────┼────────────┼───────────────────┤
 * │    1 byte   │    8 bytes  │   7 bytes  │   12.5%           │
 * │    8 bytes  │    8 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │   15 bytes  │   16 bytes  │   1 byte   │   93.8%           │
 * │   16 bytes  │   16 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │   17 bytes  │   32 bytes  │  15 bytes  │   53.1%           │
 * │   32 bytes  │   32 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │  100 bytes  │  112 bytes  │  12 bytes  │   89.3%           │
 * │ 1000 bytes  │ 1024 bytes  │  24 bytes  │   97.7%           │
 * │ 4096 bytes  │ 4096 bytes  │   0 bytes  │  100.0%  ← Perfect│
 * │10000 bytes  │10240 bytes  │ 240 bytes  │   97.7%           │
 * └────────────────────────────────────────────────────────────┘
 *
 * THE WISDOM: Align your structures to power-of-2 sizes!
 */
```

### The Thread Cache Dance

When a thread requests memory, a beautiful dance occurs:

```c
/*
 * THE ALLOCATION DANCE
 *
 * Step 1: Check Thread Cache (fast path)
 *   - No locks! Thread-local storage.
 *   - If object available in free list → return immediately
 *   - Time: ~10-20 nanoseconds
 *
 * Step 2: Refill from Central Cache (slow path)
 *   - Acquire lock on central cache
 *   - Transfer batch of objects to thread cache
 *   - Release lock
 *   - Time: ~100-200 nanoseconds
 *
 * Step 3: Grow from Page Heap (rare path)
 *   - Acquire page heap lock
 *   - Allocate new span of pages
 *   - Carve into objects, populate central cache
 *   - Time: ~1000+ nanoseconds
 */

/*
 * Pseudocode for TCMalloc's allocation:
 */
void *tcmalloc_allocate(size_t size) {
    /* Round up to size class */
    size_t size_class = SizeToClass(size);

    /* FAST PATH: Thread cache */
    ThreadCache *tc = GetThreadCache();
    if (tc->free_list[size_class] != NULL) {
        void *result = tc->free_list[size_class];
        tc->free_list[size_class] = *(void **)result;
        return result;  /* No locks! */
    }

    /* SLOW PATH: Refill from central cache */
    return RefillThreadCache(tc, size_class);
}

void tcmalloc_free(void *ptr) {
    size_t size_class = GetSizeClass(ptr);
    ThreadCache *tc = GetThreadCache();

    /* Return to thread cache (no locks!) */
    *(void **)ptr = tc->free_list[size_class];
    tc->free_list[size_class] = ptr;

    /* If thread cache is too full, return some to central cache */
    if (tc->size[size_class] > tc->max_size[size_class]) {
        ReleaseToCentralCache(tc, size_class);
    }
}
```

---

<div style="page-break-after: always;"></div>

## Chapter 2: The Architecture of jemalloc

### The Facebook Way

While Google built TCMalloc, Jason Evans at FreeBSD created jemalloc. Later adopted by Facebook (now Meta), jemalloc took a different approach to the same problem.

Where TCMalloc uses thread caches, jemalloc uses **arenas**—independent heaps that threads can access.

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                      THE REALM OF JEMALLOC                           ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                                                                      ┃
┃    ┌─────────────────────────────────────────────────────────┐      ┃
┃    │                      ARENAS                              │      ┃
┃    │  ┌───────────┐  ┌───────────┐  ┌───────────┐            │      ┃
┃    │  │  Arena 0  │  │  Arena 1  │  │  Arena 2  │  ...       │      ┃
┃    │  │  ┌─────┐  │  │  ┌─────┐  │  │  ┌─────┐  │            │      ┃
┃    │  │  │Bins │  │  │  │Bins │  │  │  │Bins │  │            │      ┃
┃    │  │  │ 8B  │  │  │  │ 8B  │  │  │  │ 8B  │  │            │      ┃
┃    │  │  │16B  │  │  │  │16B  │  │  │  │16B  │  │            │      ┃
┃    │  │  │32B  │  │  │  │32B  │  │  │  │32B  │  │            │      ┃
┃    │  │  │...  │  │  │  │...  │  │  │  │...  │  │            │      ┃
┃    │  │  └─────┘  │  │  └─────┘  │  │  └─────┘  │            │      ┃
┃    │  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘            │      ┃
┃    └────────┼──────────────┼──────────────┼──────────────────┘      ┃
┃             │              │              │                          ┃
┃    Thread 1─┘    Thread 2──┴──Thread 3    └─Thread 4                 ┃
┃    (bound to     (can use any arena,      (bound to                  ┃
┃     Arena 0)      picks least contended)   Arena 2)                  ┃
┃                                                                      ┃
┃    ┌─────────────────────────────────────────────────────────┐      ┃
┃    │                    THREAD CACHES                         │      ┃
┃    │   (Optional layer, similar to TCMalloc's thread caches)  │      ┃
┃    │   Introduced in later versions for even faster allocs    │      ┃
┃    └─────────────────────────────────────────────────────────┘      ┃
┃                                                                      ┃
┃    ┌─────────────────────────────────────────────────────────┐      ┃
┃    │                       EXTENTS                            │      ┃
┃    │   Large allocations managed separately                   │      ┃
┃    │   Uses red-black trees for coalescing                    │      ┃
┃    └─────────────────────────────────────────────────────────┘      ┃
┃                                                                      ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### The Philosophy Difference

```c
/*
 * TCMALLOC vs JEMALLOC: A PHILOSOPHICAL DIFFERENCE
 *
 * TCMalloc says: "Give each thread its own cache."
 *   - Thread-local storage for maximum speed
 *   - Threads are isolated, no sharing
 *   - Simple mental model
 *
 * jemalloc says: "Create multiple arenas, let threads choose."
 *   - Arenas can be shared or dedicated
 *   - More flexible for diverse workloads
 *   - Better for producer-consumer patterns
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    ALLOCATION PATTERNS                        │
 * ├──────────────────────────────────────────────────────────────┤
 * │                                                               │
 * │  TCMALLOC excels when:                                       │
 * │    - Threads allocate and free their own memory              │
 * │    - Workload is homogeneous (similar allocation sizes)      │
 * │    - High allocation rate, many small objects                │
 * │                                                               │
 * │  jemalloc excels when:                                       │
 * │    - Memory is passed between threads                        │
 * │    - Workload is heterogeneous (varied allocation sizes)     │
 * │    - Fragmentation is a concern                              │
 * │    - Memory needs to be returned to OS predictably           │
 * │                                                               │
 * └──────────────────────────────────────────────────────────────┘
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 3: The Battle of Benchmarks

### The Fellowship's Test

The Fellowship decided to put both allocators to the test. They created a benchmark that would reveal the strengths and weaknesses of each.

```c
/*
 * ACTUAL RESULTS FROM THE FELLOWSHIP'S SESSION
 * Platform: Windows 10, MSYS2/MinGW64, GCC 15.2.0
 * Allocator: TCMalloc (tcmalloc_minimal)
 *
 * ┌────────────────────────────────────────────────────────────────┐
 * │                    TCMALLOC RESULTS                             │
 * ├────────────────────────────────────────────────────────────────┤
 * │  Metric                    │  Value                            │
 * ├────────────────────────────┼───────────────────────────────────┤
 * │  Allocation Time           │  54 ms                            │
 * │  Allocations per ms        │  1,852                            │
 * │  Deallocation Time         │  2 ms                             │
 * │  Deallocations per ms      │  50,000                           │
 * │  Memory Efficiency         │  96.6%                            │
 * │  Peak Heap Size            │  227 MB                           │
 * │  Thread Cache Usage        │  4.8 MB                           │
 * └────────────────────────────┴───────────────────────────────────┘
 *
 * INTERPRETATION:
 *   - Deallocation is 27x faster than allocation
 *   - This is because freed memory goes to thread cache (no syscall)
 *   - 96.6% efficiency means minimal fragmentation
 *   - Thread cache holds recently freed objects for instant reuse
 */
```

### The Memory Efficiency Mystery

```c
/*
 * THE MEMORY EFFICIENCY MYSTERY
 *
 * After allocating 100,000 objects totaling ~210 MB of requested memory,
 * the heap size was 227 MB. That's 96.6% efficiency.
 *
 * But after freeing everything, the heap remained at 227 MB!
 * Only 272 bytes were "in use," yet 227 MB was still claimed.
 *
 * This is not a bug. This is TCMalloc's CACHING STRATEGY.
 */

/*
 * MEMORY STATES OBSERVED:
 *
 * ┌─────────────────────────────────────────────────────────────────┐
 * │  State                    │ Allocated │ Heap Size │ Efficiency  │
 * ├───────────────────────────┼───────────┼───────────┼─────────────┤
 * │  Initial                  │    272 B  │    1 MB   │    0.0%     │
 * │  After 100K allocations   │   219 MB  │  227 MB   │   96.6%     │
 * │  After freeing all        │    272 B  │  227 MB   │    0.0%     │
 * │  After ReleaseFreeMemory  │    272 B  │  227 MB   │    0.0%     │
 * └─────────────────────────────────────────────────────────────────┘
 *
 * WHERE DID THE MEMORY GO?
 *
 * After freeing:
 *   - Page heap freelist:     193 MB  (ready for reuse)
 *   - Central cache freelist:  12 MB  (ready for reuse)
 *   - Transfer cache:          16 MB  (ready for reuse)
 *   - Thread cache:             5 MB  (ready for reuse)
 *   - Metadata overhead:        3 MB  (bookkeeping)
 *
 * The memory is not lost—it's CACHED for future allocations.
 * This makes subsequent allocations blazingly fast.
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 4: The Detailed Statistics

### Reading the TCMalloc Oracle

TCMalloc provides an oracle—a way to see inside the memory palace and understand its structure.

```c
/*
 * ACTUAL OUTPUT FROM THE FELLOWSHIP'S SESSION:
 *
 * ────────────────────────────────────────────────────────────────
 * MALLOC:            272 (    0.0 MiB) Bytes in use by application
 * MALLOC: +    193,658,880 (  184.7 MiB) Bytes in page heap freelist
 * MALLOC: +     12,122,496 (   11.6 MiB) Bytes in central cache freelist
 * MALLOC: +     16,807,424 (   16.0 MiB) Bytes in transfer cache freelist
 * MALLOC: +      4,951,920 (    4.7 MiB) Bytes in thread cache freelists
 * MALLOC: +      3,276,864 (    3.1 MiB) Bytes in malloc metadata
 * MALLOC:   ────────────
 * MALLOC: =    230,817,856 (  220.1 MiB) Actual memory used (physical + swap)
 * MALLOC: +            0 (    0.0 MiB) Bytes released to OS (aka unmapped)
 * MALLOC:   ────────────
 * MALLOC: =    230,817,856 (  220.1 MiB) Virtual address space used
 * MALLOC:
 * MALLOC:           4,188              Spans in use
 * MALLOC:               1              Thread heaps in use
 * MALLOC:           8,192              Tcmalloc page size
 * ────────────────────────────────────────────────────────────────
 *
 * INTERPRETING THE ORACLE:
 *
 * "Bytes in use by application" (272)
 *   → The memory YOU allocated and haven't freed
 *
 * "Bytes in page heap freelist" (184.7 MiB)
 *   → Free pages ready for large allocations
 *
 * "Bytes in central cache freelist" (11.6 MiB)
 *   → Free objects in the central cache
 *
 * "Bytes in transfer cache freelist" (16.0 MiB)
 *   → Objects being transferred between caches
 *
 * "Bytes in thread cache freelists" (4.7 MiB)
 *   → Free objects in per-thread caches
 *
 * "Bytes in malloc metadata" (3.1 MiB)
 *   → Bookkeeping overhead
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 5: Choosing Your Allocator

### The Decision Tree

```
                    ┌─────────────────────────────────┐
                    │   What is your primary concern?  │
                    └─────────────────┬───────────────┘
                                      │
              ┌───────────────────────┼───────────────────────┐
              │                       │                       │
              ▼                       ▼                       ▼
     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
     │  Raw Speed     │     │ Memory Return  │     │ Fragmentation  │
     │  (throughput)  │     │   to OS        │     │   Control      │
     └───────┬────────┘     └───────┬────────┘     └───────┬────────┘
             │                      │                      │
             ▼                      ▼                      ▼
     ┌────────────────┐     ┌────────────────┐     ┌────────────────┐
     │   TCMalloc     │     │   jemalloc     │     │   jemalloc     │
     │                │     │   (with decay) │     │   (arenas)     │
     │ - Thread cache │     │                │     │                │
     │ - Size classes │     │ - muzzy pages  │     │ - Dedicated    │
     │ - Batch xfer   │     │ - dirty decay  │     │   arenas       │
     └────────────────┘     └────────────────┘     └────────────────┘
```

### The Comparison Table

```c
/*
 * THE GREAT COMPARISON
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │  Feature              │  TCMalloc          │  jemalloc           │
 * ├───────────────────────┼────────────────────┼─────────────────────┤
 * │  Thread-local cache   │  ✓ Built-in        │  ✓ Optional (tcache)│
 * │  Arena support        │  ✗ Single heap     │  ✓ Multiple arenas  │
 * │  Size classes         │  ~85 classes       │  ~100+ classes      │
 * │  Page size            │  8 KB              │  4 KB (default)     │
 * │  Memory return to OS  │  Manual            │  Automatic (decay)  │
 * │  Profiling built-in   │  ✓ Heap profiler   │  ✓ Stats + profiling│
 * │  Windows support      │  Minimal (no prof) │  Better             │
 * │  Origin               │  Google            │  FreeBSD/Facebook   │
 * │  License              │  BSD-3             │  BSD-2              │
 * └───────────────────────┴────────────────────┴─────────────────────┘
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 6: Practical Integration

### Using TCMalloc in Your Project

```c
/*
 * INTEGRATING TCMALLOC: A PRACTICAL GUIDE
 */

/* === METHOD 1: Link-time replacement === */

/*
 * Makefile approach (simplest):
 *
 *   # Link against tcmalloc
 *   LDFLAGS += -ltcmalloc_minimal
 *
 *   # Or with full profiling support (Linux only)
 *   LDFLAGS += -ltcmalloc
 *
 * Your code doesn't change—TCMalloc replaces malloc/free automatically.
 */

/* === METHOD 2: LD_PRELOAD (Linux) === */

/*
 * No recompilation needed:
 *
 *   $ LD_PRELOAD=/usr/lib/libtcmalloc_minimal.so ./your_program
 */

/* === METHOD 3: Explicit API usage === */

#include <gperftools/tcmalloc.h>
#include <gperftools/malloc_extension_c.h>

void *optimized_allocate(size_t requested_size) {
    void *ptr = tc_malloc(requested_size);

    if (ptr) {
        /* Get actual allocation size (may be larger) */
        size_t actual_size = tc_malloc_size(ptr);

        /* You can use the extra space! */
        if (actual_size > requested_size) {
            /* actual_size - requested_size bytes are bonus */
        }
    }

    return ptr;
}

void periodic_maintenance(void) {
    /* Check memory efficiency */
    size_t allocated, heap_size;
    MallocExtension_GetNumericProperty(
        "generic.current_allocated_bytes", &allocated);
    MallocExtension_GetNumericProperty(
        "generic.heap_size", &heap_size);

    double efficiency = (double)allocated / heap_size;

    /* If efficiency drops below 50%, release memory */
    if (efficiency < 0.5) {
        MallocExtension_ReleaseFreeMemory();
        printf("[MAINTENANCE] Released cached memory to OS\n");
    }
}
```

---

<div style="page-break-after: always;"></div>

## Epilogue: The Wisdom of the Two Allocators

The Fellowship emerged from their study of the Two Allocators with hard-won wisdom:

```c
/*
 * THE TEN COMMANDMENTS OF MEMORY ALLOCATION
 *
 * I.    Thou shalt align thy structures to size classes.
 * II.   Thou shalt not assume allocation is free.
 * III.  Thou shalt measure before thou optimize.
 * IV.   Thou shalt understand thy workload pattern.
 * V.    Thou shalt release memory in long-running services.
 * VI.   Thou shalt not fight the allocator's design.
 * VII.  Thou shalt use thread-local allocation when possible.
 * VIII. Thou shalt batch allocations when feasible.
 * IX.   Thou shalt consult the oracle (statistics) often.
 * X.    Thou shalt test with multiple allocators.
 */

/*
 * FINAL BENCHMARK SUMMARY
 *
 * ┌────────────────────────────────────────────────────────────┐
 * │  Achievement              │  Value                         │
 * ├───────────────────────────┼────────────────────────────────┤
 * │  Allocation rate          │  1,852 allocs/ms               │
 * │  Deallocation rate        │  50,000 frees/ms               │
 * │  Memory efficiency        │  96.6%                         │
 * │  Thread cache hit rate    │  >99% (estimated)              │
 * │  Lock contention          │  Near zero (single-threaded)   │
 * └───────────────────────────┴────────────────────────────────┘
 */
```

---

```
    "The Two Allocators are not enemies, but different paths
     up the same mountain. TCMalloc races up the rocky face,
     swift and direct. jemalloc takes the winding trail,
     careful of the terrain, leaving no trace behind.
     Both reach the summit. Choose the path that suits your journey."

                                        — Gandalf the Memory-Grey
```

---

# END OF PART II

---

<div style="page-break-after: always;"></div>

```
    "I am a servant of the Secret Fire, wielder of the flame of Anor.
     The dark fire will not avail you, flame of Udûn!
     Go back to the Shadow! You cannot pass!"

                    — Gandalf, confronting undefined behavior at the Bridge of Khazad-dûm
```

---

<div style="page-break-after: always;"></div>

# PART III: THE RETURN OF THE PROFILER

---

## Prologue: The Quest for gperftools

In the previous chapters, the Fellowship learned of the Two Allocators and their powers. But knowledge alone was not enough. They needed to *wield* these powers—to build, to integrate, to profile.

This is the tale of that quest: the building of gperftools from source on the hostile terrain of Windows, the battles with linker errors, the triumph over undefined references, and the wisdom gained from the kernel itself.

But first, we must descend deeper—into the very foundations of memory itself.

---

<div style="page-break-after: always;"></div>

# Book One: The Depths of the Kernel

---

## Chapter 1: The Kernel's Memory Palace

Before one can truly understand memory allocation, one must understand how the operating system kernel manages memory. The Fellowship descended into these depths, guided by ancient texts and kernel source code.

```c
/*
 * THE KERNEL'S VIEW OF MEMORY
 *
 * When your program calls malloc(), a long chain of events unfolds:
 *
 *    User Space                         Kernel Space
 *    ──────────                         ────────────
 *
 *    malloc(1024)
 *         │
 *         ▼
 *    ┌─────────────┐
 *    │  TCMalloc   │ ──► Returns cached memory if available
 *    │  (or libc)  │
 *    └──────┬──────┘
 *           │ (if cache empty)
 *           ▼
 *    ┌─────────────┐
 *    │   brk()     │ ──► Extends the heap (small allocations)
 *    │   mmap()    │ ──► Maps new pages (large allocations)
 *    └──────┬──────┘
 *           │
 *    ═══════╪═══════════════════════════════════════════
 *           │  SYSTEM CALL BOUNDARY
 *    ═══════╪═══════════════════════════════════════════
 *           │
 *           ▼
 *    ┌─────────────┐
 *    │   Kernel    │
 *    │   Memory    │
 *    │  Management │
 *    └──────┬──────┘
 *           │
 *           ▼
 *    ┌─────────────────────────────────────────────────┐
 *    │              PAGE TABLE MANIPULATION             │
 *    │   • Allocate physical page frames               │
 *    │   • Update page tables                          │
 *    │   • Set permissions (read/write/execute)        │
 *    │   • Handle TLB (Translation Lookaside Buffer)   │
 *    └─────────────────────────────────────────────────┘
 */
```

---

## Chapter 2: Virtual Memory—The Great Illusion

The kernel maintains a grand illusion: each process believes it has access to a vast, contiguous address space. In truth, physical memory is fragmented, shared, and scarce.

```c
/*
 * THE VIRTUAL MEMORY ILLUSION
 *
 * Your process sees:          Reality (Physical Memory):
 *
 * 0x0000000000000000          ┌─────────────────────────┐
 *         │                   │ Kernel Code & Data      │
 *         │ (unmapped)        ├─────────────────────────┤
 *         ▼                   │ Process A: Page 0x1000  │
 * 0x0000555500000000          ├─────────────────────────┤
 *    ┌─────────┐              │ Process B: Page 0x2000  │
 *    │  Code   │ ◄────────────├─────────────────────────┤
 *    ├─────────┤              │ Process A: Page 0x3000  │
 *    │  Data   │ ◄────────────├─────────────────────────┤
 *    ├─────────┤              │ (free)                  │
 *    │   BSS   │              ├─────────────────────────┤
 *    ├─────────┤              │ Process C: Page 0x5000  │
 *    │         │              ├─────────────────────────┤
 *    │  Heap   │ ◄────────────│ Process A: Heap         │
 *    │   ↓     │              ├─────────────────────────┤
 *    │         │              │ (free)                  │
 *    │         │              ├─────────────────────────┤
 *    │         │              │ Shared Libraries        │
 *    │   ↑     │              ├─────────────────────────┤
 *    │  Stack  │ ◄────────────│ Process A: Stack        │
 *    └─────────┘              └─────────────────────────┘
 * 0x00007FFFFFFFFFFF
 *
 * The PAGE TABLE is the map that connects virtual to physical.
 */

/*
 * Simplified page table entry (x86-64 style):
 */
typedef struct {
    uint64_t present      : 1;   /* Page is in physical memory */
    uint64_t writable     : 1;   /* Page can be written */
    uint64_t user         : 1;   /* Accessible from user mode */
    uint64_t write_through: 1;   /* Write-through caching */
    uint64_t cache_disable: 1;   /* Disable caching */
    uint64_t accessed     : 1;   /* Page has been read */
    uint64_t dirty        : 1;   /* Page has been written */
    uint64_t huge_page    : 1;   /* 2MB or 1GB page */
    uint64_t global       : 1;   /* Don't flush from TLB */
    uint64_t available    : 3;   /* Available for OS use */
    uint64_t phys_addr    : 40;  /* Physical page frame number */
    uint64_t reserved     : 11;  /* Reserved bits */
    uint64_t no_execute   : 1;   /* NX bit - prevent code execution */
} PageTableEntry;
```

---

<div style="page-break-after: always;"></div>

## Chapter 3: The brk() and mmap() Syscalls

When the allocator needs more memory from the kernel, it has two ancient rituals: `brk()` and `mmap()`.

```c
/*
 * THE TWO RITUALS OF MEMORY ACQUISITION
 *
 * brk() - The Heap Extension Ritual
 * ─────────────────────────────────
 * Moves the "program break" - the end of the data segment.
 * Simple, fast, but memory can only be returned from the top.
 *
 *    Before brk():              After brk(new_break):
 *    ┌─────────────┐            ┌─────────────┐
 *    │    Code     │            │    Code     │
 *    ├─────────────┤            ├─────────────┤
 *    │    Data     │            │    Data     │
 *    ├─────────────┤            ├─────────────┤
 *    │    Heap     │            │    Heap     │
 *    │             │            │             │
 *    │             │            │  (new area) │
 *    ├─────────────┤◄─ break    ├─────────────┤
 *    │  (unmapped) │            │  (unmapped) │◄─ new break
 *    └─────────────┘            └─────────────┘
 */

#include <unistd.h>

void demonstrate_brk(void) {
    void *current_break = sbrk(0);  /* Get current break */
    printf("Current break: %p\n", current_break);

    /* Extend heap by 4096 bytes */
    void *new_memory = sbrk(4096);
    if (new_memory == (void *)-1) {
        perror("sbrk failed");
        return;
    }

    printf("Allocated at: %p\n", new_memory);
    printf("New break: %p\n", sbrk(0));
}

/*
 * mmap() - The Page Mapping Ritual
 * ────────────────────────────────
 * Creates a new mapping in the virtual address space.
 * More flexible, can be placed anywhere, easily unmapped.
 */

#include <sys/mman.h>

void demonstrate_mmap(void) {
    size_t size = 1024 * 1024;  /* 1 MB */

    /* Anonymous mapping (not backed by a file) */
    void *region = mmap(
        NULL,                   /* Let kernel choose address */
        size,                   /* Size of mapping */
        PROT_READ | PROT_WRITE, /* Read and write permissions */
        MAP_PRIVATE | MAP_ANONYMOUS,  /* Private, not file-backed */
        -1,                     /* No file descriptor */
        0                       /* No offset */
    );

    if (region == MAP_FAILED) {
        perror("mmap failed");
        return;
    }

    printf("Mapped 1 MB at: %p\n", region);

    /* Use the memory */
    memset(region, 0, size);

    /* Return to OS (unlike brk, this is immediate) */
    if (munmap(region, size) == -1) {
        perror("munmap failed");
    }
}
```

---

<div style="page-break-after: always;"></div>

## Chapter 4: The Page Fault Handler

When a program accesses memory, the CPU consults the page table. If the page is not present, a **page fault** occurs, and the kernel must intervene.

```c
/*
 * THE PAGE FAULT RITUAL
 *
 * Page faults are not errors—they are how the kernel implements:
 *   - Demand paging (load pages only when accessed)
 *   - Copy-on-write (COW) for fork()
 *   - Memory-mapped files
 *   - Swap space management
 *
 * ┌─────────────────────────────────────────────────────────────────┐
 * │                      PAGE FAULT FLOW                            │
 * ├─────────────────────────────────────────────────────────────────┤
 * │                                                                 │
 * │   CPU accesses virtual address 0x7fff12340000                   │
 * │                       │                                         │
 * │                       ▼                                         │
 * │   ┌─────────────────────────────────────┐                       │
 * │   │ Check TLB (Translation Lookaside    │                       │
 * │   │ Buffer) - hardware cache of recent  │                       │
 * │   │ virtual→physical translations       │                       │
 * │   └─────────────────┬───────────────────┘                       │
 * │                     │                                           │
 * │           TLB miss? │                                           │
 * │                     ▼                                           │
 * │   ┌─────────────────────────────────────┐                       │
 * │   │ Walk page tables                    │                       │
 * │   │ (4 levels on x86-64)               │                       │
 * │   └─────────────────┬───────────────────┘                       │
 * │                     │                                           │
 * │        Page not present?                                        │
 * │                     │                                           │
 * │                     ▼                                           │
 * │   ╔═════════════════════════════════════╗                       │
 * │   ║        PAGE FAULT EXCEPTION         ║                       │
 * │   ║   Trap to kernel fault handler      ║                       │
 * │   ╚═════════════════╤═══════════════════╝                       │
 * │                     │                                           │
 * │                     ▼                                           │
 * │   ┌─────────────────────────────────────┐                       │
 * │   │ Kernel determines fault type:       │                       │
 * │   │   • Valid but not loaded → load it  │                       │
 * │   │   • Copy-on-write → copy the page   │                       │
 * │   │   • Invalid access → SIGSEGV        │                       │
 * │   └─────────────────────────────────────┘                       │
 * │                                                                 │
 * └─────────────────────────────────────────────────────────────────┘
 */
```

---

<div style="page-break-after: always;"></div>

# Book Two: The SEI CERT Standard

---

## Chapter 5: The Laws of Safe Memory

The Fellowship carried with them the **SEI CERT C Coding Standard**—a set of rules forged by the Software Engineering Institute to prevent the most dangerous memory errors.

```c
/*═══════════════════════════════════════════════════════════════════════
 * MEM30-C: Do not access freed memory
 *═══════════════════════════════════════════════════════════════════════*/

/* VIOLATION: Use after free */
void mem30_violation(void) {
    char *data = malloc(100);
    strcpy(data, "sensitive information");

    free(data);

    /* CERT VIOLATION: Accessing freed memory */
    printf("%s\n", data);  /* Undefined behavior! */
}

/* COMPLIANT: Null after free */
void mem30_compliant(void) {
    char *data = malloc(100);
    if (data == NULL) return;

    strcpy(data, "sensitive information");

    /* Clear sensitive data before freeing (MEM03-C) */
    memset(data, 0, 100);

    free(data);
    data = NULL;  /* Prevent use after free */

    /* Any accidental use now causes immediate, debuggable crash */
    if (data != NULL) {
        printf("%s\n", data);
    }
}

/*═══════════════════════════════════════════════════════════════════════
 * MEM31-C: Free dynamically allocated memory exactly once
 *═══════════════════════════════════════════════════════════════════════*/

/* VIOLATION: Double free */
void mem31_violation(void) {
    char *buffer = malloc(256);

    free(buffer);
    free(buffer);  /* Corrupts heap metadata! */
}

/* COMPLIANT: Track allocation state */
void mem31_compliant(void) {
    char *buffer = malloc(256);
    if (buffer == NULL) return;

    free(buffer);
    buffer = NULL;  /* Prevent double free */

    free(buffer);  /* Does nothing (freeing NULL is safe) */
}

/*═══════════════════════════════════════════════════════════════════════
 * MEM32-C: Detect and handle memory allocation errors
 *═══════════════════════════════════════════════════════════════════════*/

/* COMPLIANT: Always check allocation results */
void mem32_compliant(size_t count) {
    /* Also check for integer overflow (INT30-C) */
    if (count > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Allocation size overflow\n");
        return;
    }

    int *array = malloc(count * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    array[0] = 42;  /* Safe */

    free(array);
}

/*═══════════════════════════════════════════════════════════════════════
 * MEM35-C: Allocate sufficient memory for an object
 *═══════════════════════════════════════════════════════════════════════*/

/* COMPLIANT: Use sizeof on the dereferenced pointer */
void mem35_compliant(void) {
    /* Best practice: sizeof(*ptr) always matches ptr's type */
    long *lptr = malloc(sizeof(*lptr));  /* Always correct size */

    if (lptr != NULL) {
        *lptr = LONG_MAX;  /* Safe */
        free(lptr);
    }
}
```

---

<div style="page-break-after: always;"></div>

## Chapter 6: The Integer Overflow Menace

The SEI CERT Standard also warns of integer overflows—a subtle evil that corrupts size calculations.

```c
/*═══════════════════════════════════════════════════════════════════════
 * INT30-C: Ensure unsigned integer operations do not wrap
 *═══════════════════════════════════════════════════════════════════════*/

/*
 * INTEGER OVERFLOW IN MEMORY ALLOCATION
 *
 * This is one of the most dangerous vulnerabilities.
 * It was used in real-world exploits against:
 *   - OpenSSH (2002)
 *   - Windows ANI handler (2007)
 *   - iOS libpng (2011)
 */

/* COMPLIANT: Check for overflow before multiplication */
void *int30_compliant(size_t count, size_t element_size) {
    /* Check for overflow: if count > SIZE_MAX / element_size, overflow */
    if (element_size != 0 && count > SIZE_MAX / element_size) {
        errno = ENOMEM;
        return NULL;
    }

    size_t size = count * element_size;  /* Safe - no overflow */

    void *buffer = malloc(size);
    return buffer;  /* Caller should check for NULL */
}

/* EVEN BETTER: Use calloc, which checks internally */
void *int30_best(size_t count, size_t element_size) {
    /* calloc checks for overflow internally and zero-initializes */
    return calloc(count, element_size);
}

/*
 * THE FELLOWSHIP'S OVERFLOW CHECKER
 */

#include <stdbool.h>
#include <stdint.h>

bool safe_multiply_size(size_t a, size_t b, size_t *result) {
    if (a == 0 || b == 0) {
        *result = 0;
        return true;
    }

    if (a > SIZE_MAX / b) {
        return false;  /* Would overflow */
    }

    *result = a * b;
    return true;
}
```

---

<div style="page-break-after: always;"></div>

## Chapter 7: The Array Bounds Fortress

```c
/*═══════════════════════════════════════════════════════════════════════
 * ARR30-C: Do not form or use out-of-bounds pointers or array subscripts
 *═══════════════════════════════════════════════════════════════════════*/

/*
 * THE FELLOWSHIP'S SAFE ARRAY PATTERN
 */

typedef struct {
    size_t length;
    size_t capacity;
    int *data;
} SafeArray;

SafeArray *safe_array_create(size_t initial_capacity) {
    SafeArray *arr = malloc(sizeof(SafeArray));
    if (arr == NULL) return NULL;

    arr->data = malloc(initial_capacity * sizeof(int));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->length = 0;
    arr->capacity = initial_capacity;
    return arr;
}

bool safe_array_get(SafeArray *arr, size_t index, int *out_value) {
    if (arr == NULL || index >= arr->length) {
        return false;  /* Bounds check failed */
    }

    *out_value = arr->data[index];
    return true;
}

bool safe_array_set(SafeArray *arr, size_t index, int value) {
    if (arr == NULL || index >= arr->length) {
        return false;  /* Bounds check failed */
    }

    arr->data[index] = value;
    return true;
}

bool safe_array_push(SafeArray *arr, int value) {
    if (arr == NULL) return false;

    /* Grow if needed */
    if (arr->length >= arr->capacity) {
        size_t new_capacity = arr->capacity * 2;

        /* Check for overflow */
        if (new_capacity < arr->capacity) {
            return false;
        }

        int *new_data = realloc(arr->data, new_capacity * sizeof(int));
        if (new_data == NULL) {
            return false;
        }

        arr->data = new_data;
        arr->capacity = new_capacity;
    }

    arr->data[arr->length++] = value;
    return true;
}

void safe_array_destroy(SafeArray *arr) {
    if (arr != NULL) {
        free(arr->data);
        free(arr);
    }
}
```

---

<div style="page-break-after: always;"></div>

# Book Three: The Quest for gperftools

---

## Chapter 8: The Journey Begins

The Fellowship's quest to integrate gperftools began on a hostile land: Windows, where C compilers were scarce and build systems arcane.

```c
/*
 * THE QUEST LOG
 *
 * Date: December 2024
 * Location: Windows 10, Build 26100.7462
 * Mission: Build gperftools from source, integrate with PartsDB
 *
 * Initial reconnaissance revealed:
 *   - No GCC in PATH
 *   - No MSVC cl.exe available
 *   - No CMake installed
 *
 * The Fellowship was undeterred.
 */

/*
 * STEP 1: FINDING A COMPILER
 *
 * $ where gcc.exe
 * INFO: Could not find files for the given pattern(s).
 *
 * The land was barren.
 */

/*
 * STEP 2: DISCOVERING WINGET
 *
 * $ winget --version
 * v1.9.25200
 *
 * "We have found a way," said the wizard.
 */

/*
 * STEP 3: INSTALLING MSYS2
 *
 * $ winget install MSYS2.MSYS2
 *
 * Found MSYS2 Installer [MSYS2.MSYS2] Version 20250830
 * Starting package install...
 * Successfully installed
 *
 * The gates of MinGW opened before them.
 */
```

---

## Chapter 9: The Path Format Peril

```c
/*
 * THE PATH FORMAT PERIL
 *
 * In Git Bash, a treacherous trap awaited.
 * Windows paths spoke a different tongue.
 */

/*
 * ATTEMPT 1: Windows path syntax
 *
 * $ C:\msys64\usr\bin\pacman.exe --version
 * bash: C:msys64usrbinpacman.exe: command not found
 *
 * The backslashes were consumed by the shell.
 */

/*
 * REVELATION: Unix-style paths in Git Bash
 *
 * $ /c/msys64/usr/bin/pacman.exe --version
 * Pacman v6.1.0 - libalpm v14.0.0
 *
 * The wizard spoke: "In this realm, C:\ becomes /c/.
 * Remember this, or be forever lost."
 */

/*
 * THE PATH TRANSLATION TABLE
 *
 * Windows Path              Git Bash Path
 * ────────────────────────  ─────────────────────
 * C:\Users\name             /c/Users/name
 * C:\msys64\mingw64\bin     /c/msys64/mingw64/bin
 * D:\Projects               /d/Projects
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 10: Installing the Toolchain

```c
/*
 * THE TOOLCHAIN INSTALLATION
 *
 * Installing the MinGW-w64 compiler:
 *
 * $ /c/msys64/usr/bin/pacman.exe -S --noconfirm \
 *     mingw-w64-x86_64-gcc \
 *     mingw-w64-x86_64-cmake \
 *     mingw-w64-x86_64-ninja
 *
 * Packages (84):
 *   mingw-w64-x86_64-gcc-15.2.0-8
 *   mingw-w64-x86_64-cmake-4.2.1-1
 *   mingw-w64-x86_64-ninja-1.13.2-1
 *
 * Total Installed Size: 777.65 MiB
 *
 * "GCC 15.2.0," marveled the Fellowship. "The latest incarnation."
 */

/*
 * VERIFICATION OF ARMS
 *
 * $ /c/msys64/mingw64/bin/gcc.exe --version
 * gcc.exe (Rev8, Built by MSYS2 project) 15.2.0
 *
 * $ /c/msys64/mingw64/bin/cmake.exe --version
 * cmake version 4.2.1
 *
 * $ /c/msys64/mingw64/bin/ninja.exe --version
 * 1.13.2
 *
 * The weapons were sharp and ready.
 */
```

---

## Chapter 11: The Clone and the Build Attempt

```c
/*
 * CLONING GPERFTOOLS
 */

/*
 * Attempt 1: MSYS2 git (failed)
 *
 * $ /c/msys64/usr/bin/git.exe clone https://github.com/gperftools/gperftools
 * fatal: remote helper 'https' aborted session
 *
 * The MSYS2 git lacked the proper SSL incantations.
 */

/*
 * Attempt 2: Windows git (success)
 *
 * $ git clone --depth 1 https://github.com/gperftools/gperftools
 * Cloning into 'gperftools'...
 * done.
 *
 * The source code was theirs.
 */

/*
 * THE FIRST BUILD ATTEMPT
 *
 * $ mkdir build && cd build
 * $ cmake -G "Ninja" ../gperftools
 *
 * -- The C compiler identification is GNU 15.2.0
 * -- Configuring done (13.8s)
 *
 * $ ninja
 *
 * [78/123] Linking CXX shared library libtcmalloc_minimal.dll
 * FAILED: libtcmalloc_minimal.dll
 *
 * undefined reference to `WaitOnAddress'
 *
 * The build had failed. A dark shadow fell upon the Fellowship.
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 12: The Battle with WaitOnAddress

```c
/*
 * THE LINKER ERROR MYSTERY
 *
 *   undefined reference to `WaitOnAddress'
 *   undefined reference to `WakeByAddressAll'
 *   undefined reference to `WakeByAddressSingle'
 *
 * These were Windows 8+ synchronization primitives.
 */

/*
 * INVESTIGATION PHASE 1: Does the library exist?
 *
 * $ ls /c/msys64/mingw64/lib | grep -i sync
 * libsynchronization.a    ◄── It exists!
 */

/*
 * INVESTIGATION PHASE 2: Does it contain the symbols?
 *
 * $ nm libsynchronization.a | grep -i WaitOnAddress
 * 0000000000000000 T WaitOnAddress    ◄── Symbol is there!
 */

/*
 * INVESTIGATION PHASE 3: Link order analysis
 *
 * Examining the failed link command:
 *
 *   g++.exe ... -lsynchronization -lshlwapi libcommon.a ...
 *                      ↑                        ↑
 *                      │                        │
 *              Comes BEFORE          Needs the symbols
 *
 * THE ROOT CAUSE REVEALED:
 * In static library linking, the library providing symbols
 * must come AFTER the object that needs them!
 */

/*
 * THE SOLUTION
 *
 * The Fellowship modified CMakeLists.txt, adding after line 432:
 *
 *   target_link_libraries(common INTERFACE synchronization)
 *
 * This tells CMake: "Anyone linking against 'common'
 * must also link against 'synchronization', IN THE CORRECT ORDER."
 */
```

---

## Chapter 13: Victory and Integration

```c
/*
 * THE SUCCESSFUL BUILD
 *
 * $ rm -rf * && cmake -G "Ninja" ../gperftools
 * $ ninja
 *
 * [123/123] Linking CXX executable stack_trace_table_test.exe
 *
 * All 123 targets built successfully!
 */

/*
 * THE SPOILS OF VICTORY
 *
 * $ ls -la *.dll *.a
 *
 * libtcmalloc_minimal.dll      3,178,943 bytes  ◄── The prize!
 * libtcmalloc_minimal.dll.a      131,010 bytes  ◄── Import library
 *
 * The TCMalloc library was theirs.
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 14: The Benchmarks of Truth

```c
/*
 * ACTUAL RESULTS
 *
 * ╔════════════════════════════════════════════════════════════════╗
 * ║              TCMALLOC BENCHMARK RESULTS                        ║
 * ╠════════════════════════════════════════════════════════════════╣
 * ║                                                                ║
 * ║  Platform:     Windows 10, MSYS2/MinGW64, GCC 15.2.0          ║
 * ║  Allocator:    TCMalloc (tcmalloc_minimal)                    ║
 * ║  Test:         100,000 random allocations (16-4096 bytes)     ║
 * ║                                                                ║
 * ║  ┌────────────────────┬───────────────────────────────────┐   ║
 * ║  │ Metric             │ Result                            │   ║
 * ║  ├────────────────────┼───────────────────────────────────┤   ║
 * ║  │ Allocation Time    │ 54 ms                             │   ║
 * ║  │ Allocation Rate    │ 1,852 allocs/ms                   │   ║
 * ║  │ Deallocation Time  │ 2 ms                              │   ║
 * ║  │ Deallocation Rate  │ 50,000 frees/ms                   │   ║
 * ║  │ Memory Efficiency  │ 96.6%                             │   ║
 * ║  │ Peak Memory        │ 219 MB allocated / 227 MB heap    │   ║
 * ║  └────────────────────┴───────────────────────────────────┘   ║
 * ║                                                                ║
 * ║  INTERPRETATION:                                              ║
 * ║  • Deallocation is 27x faster than allocation                 ║
 * ║  • Free returns to thread cache (no syscall)                  ║
 * ║  • 96.6% efficiency = minimal fragmentation                   ║
 * ║  • Thread cache enables instant reuse                         ║
 * ║                                                                ║
 * ╚════════════════════════════════════════════════════════════════╝
 */
```

---

<div style="page-break-after: always;"></div>

# Book Four: The Profiler's Arsenal

---

## Chapter 15: CPU Profiling (The Linux Path)

Though the Fellowship's Windows journey could not use CPU profiling, they documented the technique for future quests on Linux.

```c
/*
 * CPU PROFILING WITH GPERFTOOLS
 *
 * Available on Linux and macOS.
 * Not available on Windows (uses ITIMER_PROF signal).
 */

/*
 * METHOD 1: Environment Variable Activation
 *
 * $ export LD_PRELOAD=/usr/lib/libprofiler.so
 * $ export CPUPROFILE=/tmp/myapp.prof
 * $ ./myapp
 *
 * The profiler samples the call stack at regular intervals.
 */

/*
 * METHOD 2: Programmatic Control
 */

#include <gperftools/profiler.h>

void profile_specific_section(void) {
    /* Start profiling */
    ProfilerStart("section_profile.prof");

    /* The code to profile */
    expensive_computation();
    heavy_allocation_workload();

    /* Stop profiling */
    ProfilerStop();
}

/*
 * ANALYZING CPU PROFILES WITH PPROF
 *
 * $ pprof --text ./myapp profile.prof         # Text report
 * $ pprof --web ./myapp profile.prof          # Web visualization
 * $ pprof --http=:8080 ./myapp profile.prof   # Interactive web UI
 */
```

---

## Chapter 16: Heap Profiling

```c
/*
 * HEAP PROFILING WITH GPERFTOOLS
 *
 * Tracks memory allocations over time.
 */

/*
 * ENABLING HEAP PROFILING
 *
 * $ export LD_PRELOAD=/usr/lib/libtcmalloc.so
 * $ export HEAPPROFILE=/tmp/heap
 * $ ./myapp
 *
 * Generates: /tmp/heap.0001.heap, /tmp/heap.0002.heap, ...
 */

/*
 * PROGRAMMATIC HEAP PROFILING
 */

#include <gperftools/heap-profiler.h>

void profile_memory_usage(void) {
    /* Start heap profiling */
    HeapProfilerStart("memory_profile");

    /* Checkpoint 1: Initial state */
    HeapProfilerDump("initial");

    /* Phase 1: Load data */
    load_large_dataset();
    HeapProfilerDump("after_load");

    /* Phase 2: Cleanup */
    cleanup_data();
    HeapProfilerDump("after_cleanup");

    /* Stop profiling */
    HeapProfilerStop();
}

/*
 * ANALYZING HEAP PROFILES
 *
 * $ pprof --inuse_space --text ./myapp heap.0001.heap
 *
 * Shows currently in-use memory (not freed).
 *
 * $ pprof --base=heap.0001.heap ./myapp heap.0003.heap
 *
 * Shows difference between two profiles (find memory growth).
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 17: The Heap Checker

```c
/*
 * HEAP CHECKING (MEMORY LEAK DETECTION)
 *
 * $ export LD_PRELOAD=/usr/lib/libtcmalloc.so
 * $ export HEAPCHECK=normal
 * $ ./myapp
 *
 * Modes:
 *   minimal   - Check only at program exit
 *   normal    - Standard checking
 *   strict    - More aggressive
 *   draconian - Report all potential leaks
 */

/*
 * HEAP CHECK OUTPUT EXAMPLE
 *
 * WARNING: Perftools heap leak checker is active
 *
 * Have memory regions from 1 allocations, size 1024
 *   1024 bytes in 1 objects from:
 *     @ 0x4012ab main
 *     @ 0x7f8a3c leaky_function
 *     @ 0x7f8a2d allocate_buffer
 *
 * Memory leak of 1024 bytes detected.
 *
 * This tells you:
 *   - 1024 bytes were leaked
 *   - Allocated in allocate_buffer()
 *   - Called from leaky_function()
 *   - Called from main()
 */
```

---

<div style="page-break-after: always;"></div>

# Book Five: Wisdom for the Journey

---

## Chapter 18: The Debugging Manifesto

```c
/*
 * THE DEBUGGING MANIFESTO
 */

/*
 * PRINCIPLE 1: Understand Before You Optimize
 *
 * "Premature optimization is the root of all evil."
 *     — Donald Knuth
 */

/*
 * PRINCIPLE 2: The Scientific Method of Debugging
 *
 *   1. OBSERVE    - What exactly is happening?
 *   2. HYPOTHESIZE - What could cause this behavior?
 *   3. PREDICT    - If my hypothesis is true, what else should happen?
 *   4. TEST       - Run an experiment to test the prediction
 *   5. ITERATE    - Refine hypothesis based on results
 */

/*
 * PRINCIPLE 3: Minimize, Isolate, Reproduce
 *
 *   MINIMIZE  - Remove code until bug disappears, then add back
 *   ISOLATE   - Create minimal test case that shows the bug
 *   REPRODUCE - Ensure bug is consistently reproducible
 *
 * A bug you can't reproduce is a bug you can't fix.
 */

/*
 * PRINCIPLE 4: Trust Nothing, Verify Everything
 *
 * Ken Thompson taught us that even compilers can be compromised.
 * In debugging, verify your assumptions.
 */

/*
 * PRINCIPLE 5: The Five Whys
 *
 *   BUG: Program crashes
 *   Why? → Null pointer dereference
 *   Why? → find_part() returned NULL
 *   Why? → Part was not in database
 *   Why? → Part ID was corrupted
 *   Why? → Integer overflow in ID calculation
 *
 *   ROOT CAUSE: Integer overflow
 *   FIX: Add overflow checking
 */

/*
 * PRINCIPLE 6: Make It Fail Loudly
 */

#include <assert.h>
#include <stdio.h>

#define LOUD_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "ASSERTION FAILED at %s:%d\n", __FILE__, __LINE__); \
        fprintf(stderr, "  Condition: %s\n", #cond); \
        fprintf(stderr, "  Message: %s\n", msg); \
        abort(); \
    } \
} while(0)
```

---

<div style="page-break-after: always;"></div>

## Chapter 19: The Complete Toolchain

```c
/*
 * THE FELLOWSHIP'S COMPLETE TOOLCHAIN
 */

/*
 * COMPILATION FLAGS FOR DEBUGGING
 *
 * Debug build:
 *   gcc -g3 -O0 -DDEBUG -fno-omit-frame-pointer \
 *       -Wall -Wextra -Wpedantic \
 *       -fsanitize=address,undefined \
 *       -o program_debug program.c
 *
 * Profile build:
 *   gcc -g -O2 -fno-omit-frame-pointer \
 *       -o program_profile program.c \
 *       -lprofiler -ltcmalloc
 *
 * Release build:
 *   gcc -O2 -DNDEBUG -flto \
 *       -fstack-protector-strong \
 *       -D_FORTIFY_SOURCE=2 \
 *       -o program program.c \
 *       -ltcmalloc_minimal
 */

/*
 * STATIC ANALYSIS TOOLS
 *
 * Clang Static Analyzer:
 *   $ clang --analyze -Xanalyzer -analyzer-output=text source.c
 *
 * Cppcheck:
 *   $ cppcheck --enable=all --inconclusive source.c
 */

/*
 * DYNAMIC ANALYSIS TOOLS
 *
 * AddressSanitizer (compile-time):
 *   $ gcc -fsanitize=address program.c
 *
 * Valgrind (runtime):
 *   $ valgrind --leak-check=full ./program
 */

/*
 * PROFILING TOOLS
 *
 * gperftools CPU profiler:
 *   $ CPUPROFILE=cpu.prof ./program
 *   $ pprof --text ./program cpu.prof
 *
 * gperftools Heap profiler:
 *   $ HEAPPROFILE=heap ./program
 *   $ pprof --inuse_space --text ./program heap.0001.heap
 *
 * Linux perf:
 *   $ perf record ./program
 *   $ perf report
 */

/*
 * BINARY ANALYSIS TOOLS
 *
 * nm - Symbol listing:
 *   $ nm -C ./program | grep malloc
 *
 * objdump - Disassembly:
 *   $ objdump -d -M intel ./program | less
 *
 * ldd - Dynamic dependencies:
 *   $ ldd ./program
 *
 * strace - System call tracing:
 *   $ strace ./program
 */
```

---

<div style="page-break-after: always;"></div>

## Chapter 20: The Final Synthesis

```c
/*
 * THE FELLOWSHIP'S FINAL SYNTHESIS
 */

/*
 * FROM THE KERNEL:
 *   - Memory is virtualized through page tables
 *   - brk() and mmap() are the syscall foundations
 *   - Page faults enable demand paging and COW
 */

/*
 * FROM SEI CERT:
 *   - MEM30-C: Never access freed memory
 *   - MEM31-C: Free memory exactly once
 *   - MEM32-C: Always check allocation results
 *   - MEM35-C: Allocate sufficient memory
 *   - INT30-C: Prevent integer overflow in size calculations
 *   - ARR30-C: Check array bounds before access
 */

/*
 * FROM TCMALLOC:
 *   - Thread caches eliminate lock contention
 *   - Size classes reduce fragmentation
 *   - Memory is cached for fast reuse
 *   - Statistics reveal allocator behavior
 */

/*
 * FROM THE BUILD JOURNEY:
 *   - Link order matters for static libraries
 *   - Platform differences require adaptation
 *   - Persistence and investigation solve problems
 */

/*
 * THE OATH RENEWED
 */

static const char *fellowship_oath[] = {
    "We will compile with all warnings enabled.",
    "We will test with sanitizers and static analyzers.",
    "We will profile before we optimize.",
    "We will check all return values.",
    "We will validate all inputs.",
    "We will free exactly what we allocate.",
    "We will document our assumptions.",
    "We will write tests for our fixes.",
    "We will learn from every bug.",
    "We will share our knowledge with others."
};
```

---

<div style="page-break-after: always;"></div>

## Epilogue: The Return Journey

The Fellowship emerged from their quest transformed. They had descended into the depths of the kernel, learned the laws of CERT, battled linker errors, and emerged with a working gperftools integration.

But more importantly, they had gained wisdom:

- **Memory is not magic.** It is a carefully managed resource, virtualized by the kernel, allocated by libraries, and ultimately the programmer's responsibility.

- **Bugs are not random.** They follow patterns. The Nine Bugs of Power—overflow, use-after-free, double-free, leak, null-deref, integer-overflow, format-string, race, uninitialized—account for most vulnerabilities.

- **Tools are allies.** Sanitizers, profilers, static analyzers, and debuggers are force multipliers. Use them.

- **Standards are wisdom encoded.** The SEI CERT rules exist because thousands of programmers made the same mistakes. Learn from their experience.

The road goes ever on. New bugs will emerge. New tools will be forged. But the Fellowship's principles endure:

```
    "All that is gold does not glitter,
     Not all those who wander are lost;
     The old code that is strong does not wither,
     Deep tests are not reached by the frost."

                                        — The Testing Fellowship
```

---

```
    ┌─────────────────────────────────────────────────────────────────┐
    │                                                                 │
    │   "I will not say: do not weep; for not all bugs                │
    │    are an evil to be wept for."                                 │
    │                                                                 │
    │    Some bugs teach us. Some bugs make us stronger.              │
    │    Some bugs reveal flaws in our designs that,                  │
    │    once fixed, make our code immortal.                          │
    │                                                                 │
    │    The quest is not to eliminate all bugs—                      │
    │    that is impossible.                                          │
    │                                                                 │
    │    The quest is to build systems that detect bugs early,        │
    │    contain their damage, and recover gracefully.                │
    │                                                                 │
    │    This is the way of the Testing Fellowship.                   │
    │                                                                 │
    │                                        — Gandalf the Debugger   │
    │                                                                 │
    └─────────────────────────────────────────────────────────────────┘
```

---

# END OF PART III

# END OF THE TESTING FELLOWSHIP

---

<div style="page-break-after: always;"></div>

# Appendices

---

## Appendix A: SEI CERT Memory Rules Quick Reference

| Rule | Title | Key Point |
|------|-------|-----------|
| MEM30-C | No use after free | Set pointer to NULL after free |
| MEM31-C | Free exactly once | Track ownership, NULL after free |
| MEM32-C | Check allocations | Always test malloc return value |
| MEM33-C | Flexible arrays | Allocate sizeof(struct) + array_size |
| MEM34-C | Free only heap | Never free stack/static/literal |
| MEM35-C | Sufficient size | Use sizeof(*ptr) pattern |
| MEM36-C | Realloc invalidates | Recalculate pointers after realloc |
| INT30-C | No unsigned wrap | Check before multiplication |
| INT32-C | No signed overflow | Use safe arithmetic functions |
| ARR30-C | Bounds checking | Validate all array indices |

---

## Appendix B: TCMalloc API Quick Reference

```c
/* Basic allocation (drop-in replacement) */
void *ptr = malloc(size);
void *ptr = calloc(count, size);
void *ptr = realloc(ptr, new_size);
void free(ptr);

/* TCMalloc extensions */
#include <gperftools/tcmalloc.h>
size_t tc_malloc_size(ptr);          /* Get actual allocation size */

/* Statistics API */
#include <gperftools/malloc_extension_c.h>
MallocExtension_GetNumericProperty(name, &value);
MallocExtension_GetStats(buffer, size);
MallocExtension_ReleaseFreeMemory();

/* Useful properties */
"generic.current_allocated_bytes"     /* In-use memory */
"generic.heap_size"                   /* Total heap */
"tcmalloc.pageheap_free_bytes"        /* Cached pages */
"tcmalloc.current_total_thread_cache_bytes"  /* Thread cache */
```

---

## Appendix C: Build Commands Quick Reference

```bash
# Debug with sanitizers
gcc -g3 -O0 -fsanitize=address,undefined -o prog_debug prog.c

# With TCMalloc
gcc -g -O2 -o prog prog.c -ltcmalloc_minimal

# Profile build (Linux)
gcc -g -O2 -o prog prog.c -lprofiler -ltcmalloc

# Run with CPU profiling
CPUPROFILE=cpu.prof ./prog
pprof --text ./prog cpu.prof

# Run with heap profiling
HEAPPROFILE=heap ./prog
pprof --inuse_space --text ./prog heap.0001.heap

# Valgrind memory check
valgrind --leak-check=full --show-leak-kinds=all ./prog
```

---

<div style="page-break-after: always;"></div>

```
    ╔══════════════════════════════════════════════════════════════════════╗
    ║                                                                      ║
    ║                     THE TESTING FELLOWSHIP                           ║
    ║                                                                      ║
    ║                            ~ FIN ~                                   ║
    ║                                                                      ║
    ║    ─────────────────────────────────────────────────────────────     ║
    ║                                                                      ║
    ║    Created during the gperftools integration session                 ║
    ║    December 2024                                                     ║
    ║                                                                      ║
    ║    May your builds be clean and your tests be green.                 ║
    ║                                                                      ║
    ╚══════════════════════════════════════════════════════════════════════╝
```

---

*This document is part of the C23 Tutorial series.*
*All benchmark results are from actual measurements on Windows 10, MSYS2/MinGW64, GCC 15.2.0.*
