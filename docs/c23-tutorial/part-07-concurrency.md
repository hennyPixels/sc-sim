# Part 7: Concurrency, Atomics & Type-Generic Programming

*Source: Modern C (Gustedt)*

> **"C has evolved into a modern programming language."** — Jens Gustedt

C11 introduced native threading and atomic operations, finally giving C first-class support for concurrent programming. This part covers thread-safe programming patterns and C's powerful type-generic features.

---

## 7.1 C11/C23 Thread Support

### The `<threads.h>` Header

```c
#include <threads.h>

/* Thread function signature */
int worker_thread(void *arg) {
    int *data = (int *)arg;
    printf("Worker received: %d\n", *data);
    return 0;  /* Thread exit status */
}

int main(void) {
    thrd_t thread;
    int data = 42;

    /* Create thread */
    if (thrd_create(&thread, worker_thread, &data) != thrd_success) {
        return 1;
    }

    /* Wait for completion */
    int result;
    thrd_join(thread, &result);
    printf("Thread returned: %d\n", result);

    return 0;
}
```

### Thread Creation Options

```c
/* Detached threads - don't need to join */
thrd_t thread;
thrd_create(&thread, worker_thread, nullptr);
thrd_detach(thread);  /* Resources freed automatically on exit */

/* Get current thread ID */
thrd_t self = thrd_current();

/* Compare thread IDs */
if (thrd_equal(self, other_thread)) {
    /* Same thread */
}

/* Yield to other threads */
thrd_yield();

/* Sleep */
struct timespec duration = { .tv_sec = 1, .tv_nsec = 500000000 };  /* 1.5s */
thrd_sleep(&duration, nullptr);

/* Exit current thread */
thrd_exit(42);  /* Returns 42 as exit status */
```

### Thread-Local Storage

```c
/* C11 thread-local keyword */
_Thread_local int per_thread_counter = 0;

/* Or with thread_local macro (C23 keyword) */
thread_local int another_counter = 0;

/* Each thread gets independent copy */
int worker(void *arg) {
    (void)arg;
    per_thread_counter++;  /* Only affects this thread's copy */
    return per_thread_counter;
}

/* Dynamic TLS with tss_t */
tss_t key;

void destructor(void *value) {
    free(value);  /* Called when thread exits */
}

void init_tls(void) {
    tss_create(&key, destructor);
}

void set_thread_data(void *data) {
    tss_set(key, data);
}

void *get_thread_data(void) {
    return tss_get(key);
}
```

---

## 7.2 Atomic Operations and Memory Orders

### Atomic Types

```c
#include <stdatomic.h>

/* Atomic scalar types */
atomic_int counter = 0;
atomic_bool flag = false;
atomic_size_t size = 0;

/* Generic atomic type */
_Atomic int x = 0;
_Atomic(part_t *) atomic_part = nullptr;

/* Check if type is lock-free */
if (atomic_is_lock_free(&counter)) {
    /* Hardware-supported atomic operations */
}
```

### Basic Atomic Operations

```c
/* Atomic load and store */
atomic_int value = 0;

void writer(void) {
    atomic_store(&value, 42);  /* Atomic write */
}

void reader(void) {
    int v = atomic_load(&value);  /* Atomic read */
    printf("Value: %d\n", v);
}

/* Atomic arithmetic */
atomic_int counter = 0;

void increment(void) {
    atomic_fetch_add(&counter, 1);  /* Returns old value */
}

int decrement_and_test(void) {
    return atomic_fetch_sub(&counter, 1) == 1;  /* Was it 1 before? */
}

/* Atomic bitwise operations */
atomic_uint flags = 0;

void set_flag(unsigned int flag) {
    atomic_fetch_or(&flags, flag);  /* flags |= flag */
}

void clear_flag(unsigned int flag) {
    atomic_fetch_and(&flags, ~flag);  /* flags &= ~flag */
}

/* Atomic exchange */
atomic_int owner = -1;

bool try_acquire(int thread_id) {
    int expected = -1;
    return atomic_compare_exchange_strong(&owner, &expected, thread_id);
}

void release(void) {
    atomic_store(&owner, -1);
}
```

### Memory Ordering

```c
/*
 * Memory orders (weakest to strongest):
 *
 * memory_order_relaxed - No ordering constraints
 * memory_order_consume - Data-dependency ordering (deprecated)
 * memory_order_acquire - Reads after this see prior writes
 * memory_order_release - Writes before this visible after acquire
 * memory_order_acq_rel - Both acquire and release
 * memory_order_seq_cst - Total ordering (default, safest)
 */

/* Producer-consumer pattern */
atomic_int data = 0;
atomic_bool ready = false;

void producer(void) {
    data = 42;  /* Regular write */
    atomic_store_explicit(&ready, true, memory_order_release);
    /* release: data write visible before ready write */
}

void consumer(void) {
    while (!atomic_load_explicit(&ready, memory_order_acquire)) {
        thrd_yield();
    }
    /* acquire: sees all writes before producer's release */
    int value = data;  /* Guaranteed to see 42 */
}

/* Relaxed ordering for counters (order doesn't matter) */
atomic_uint64_t stats_counter = 0;

void record_event(void) {
    atomic_fetch_add_explicit(&stats_counter, 1, memory_order_relaxed);
    /* Fastest, but no ordering guarantees */
}
```

### Compare-and-Swap Patterns

```c
/* Lock-free stack push */
typedef struct node {
    int value;
    struct node *next;
} node_t;

_Atomic(node_t *) stack_top = nullptr;

void push(int value) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->value = value;

    node_t *old_top = atomic_load(&stack_top);
    do {
        new_node->next = old_top;
    } while (!atomic_compare_exchange_weak(&stack_top, &old_top, new_node));
    /* Retry if another thread modified stack_top */
}

int pop(int *value) {
    node_t *old_top = atomic_load(&stack_top);
    node_t *new_top;

    do {
        if (old_top == nullptr) {
            return 0;  /* Empty */
        }
        new_top = old_top->next;
    } while (!atomic_compare_exchange_weak(&stack_top, &old_top, new_top));

    *value = old_top->value;
    free(old_top);
    return 1;
}
```

---

## 7.3 Lock-Free Data Structures

### Spinlock Implementation

```c
typedef atomic_flag spinlock_t;

#define SPINLOCK_INIT ATOMIC_FLAG_INIT

void spinlock_lock(spinlock_t *lock) {
    while (atomic_flag_test_and_set(lock)) {
        /* Spin until we acquire the lock */
        thrd_yield();  /* Be nice to other threads */
    }
}

void spinlock_unlock(spinlock_t *lock) {
    atomic_flag_clear(lock);
}

/* Usage */
spinlock_t lock = SPINLOCK_INIT;

void critical_section(void) {
    spinlock_lock(&lock);
    /* Protected code */
    spinlock_unlock(&lock);
}
```

### Sequence Lock (SeqLock)

```c
/* Reader-writer lock optimized for frequent reads */
typedef struct {
    atomic_uint sequence;
    /* Protected data follows */
} seqlock_t;

void seqlock_write_begin(seqlock_t *lock) {
    unsigned seq = atomic_load(&lock->sequence);
    while ((seq & 1) || !atomic_compare_exchange_weak(
            &lock->sequence, &seq, seq + 1)) {
        seq = atomic_load(&lock->sequence);
    }
    atomic_thread_fence(memory_order_release);
}

void seqlock_write_end(seqlock_t *lock) {
    atomic_fetch_add(&lock->sequence, 1);
}

unsigned seqlock_read_begin(seqlock_t *lock) {
    unsigned seq;
    do {
        seq = atomic_load(&lock->sequence);
    } while (seq & 1);  /* Wait if write in progress */
    atomic_thread_fence(memory_order_acquire);
    return seq;
}

bool seqlock_read_retry(seqlock_t *lock, unsigned start_seq) {
    atomic_thread_fence(memory_order_acquire);
    return atomic_load(&lock->sequence) != start_seq;
}

/* Usage for read-heavy data */
typedef struct {
    seqlock_t lock;
    int64_t timestamp;
    double value;
} sensor_reading_t;

double read_sensor(sensor_reading_t *sensor) {
    double result;
    unsigned seq;
    do {
        seq = seqlock_read_begin(&sensor->lock);
        result = sensor->value;  /* Copy data */
    } while (seqlock_read_retry(&sensor->lock, seq));
    return result;
}
```

---

## 7.4 Thread-Local Storage

### Static Thread-Local Variables

```c
/* Per-thread error information */
_Thread_local partsdb_error_t last_error = PARTSDB_OK;
_Thread_local char error_message[256] = "";

void set_thread_error(partsdb_error_t err, const char *msg) {
    last_error = err;
    strncpy(error_message, msg, sizeof(error_message) - 1);
}

partsdb_error_t get_thread_error(char *buf, size_t size) {
    if (buf != nullptr && size > 0) {
        strncpy(buf, error_message, size - 1);
    }
    return last_error;
}
```

### Per-Thread Caches

```c
/* Thread-local database connection cache */
_Thread_local partsdb_t *tls_db = nullptr;

partsdb_t *get_thread_db(const char *path) {
    if (tls_db == nullptr) {
        partsdb_open(path, &tls_db);
    }
    return tls_db;
}

void cleanup_thread_db(void) {
    if (tls_db != nullptr) {
        partsdb_close(tls_db);
        tls_db = nullptr;
    }
}
```

---

## 7.5 Synchronization Primitives

### Mutexes

```c
#include <threads.h>

mtx_t mutex;

void init(void) {
    /* Plain mutex */
    mtx_init(&mutex, mtx_plain);

    /* Recursive mutex (can be locked multiple times by same thread) */
    mtx_init(&mutex, mtx_recursive);

    /* Timed mutex */
    mtx_init(&mutex, mtx_timed);
}

void critical_section(void) {
    mtx_lock(&mutex);  /* Block until available */
    /* Protected code */
    mtx_unlock(&mutex);
}

bool try_critical_section(void) {
    if (mtx_trylock(&mutex) == thrd_success) {
        /* Got the lock */
        mtx_unlock(&mutex);
        return true;
    }
    return false;  /* Lock was busy */
}

bool timed_critical_section(void) {
    struct timespec deadline;
    timespec_get(&deadline, TIME_UTC);
    deadline.tv_sec += 5;  /* 5 second timeout */

    if (mtx_timedlock(&mutex, &deadline) == thrd_success) {
        /* Got the lock within timeout */
        mtx_unlock(&mutex);
        return true;
    }
    return false;  /* Timeout */
}

void cleanup(void) {
    mtx_destroy(&mutex);
}
```

### Condition Variables

```c
/* Producer-consumer queue */
typedef struct {
    part_t *buffer;
    size_t capacity;
    size_t head, tail, count;
    mtx_t mutex;
    cnd_t not_empty;
    cnd_t not_full;
} part_queue_t;

void queue_init(part_queue_t *q, size_t capacity) {
    q->buffer = malloc(capacity * sizeof(part_t));
    q->capacity = capacity;
    q->head = q->tail = q->count = 0;
    mtx_init(&q->mutex, mtx_plain);
    cnd_init(&q->not_empty);
    cnd_init(&q->not_full);
}

void queue_push(part_queue_t *q, const part_t *part) {
    mtx_lock(&q->mutex);

    while (q->count == q->capacity) {
        cnd_wait(&q->not_full, &q->mutex);  /* Wait for space */
    }

    q->buffer[q->tail] = *part;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;

    cnd_signal(&q->not_empty);  /* Wake one waiting consumer */
    mtx_unlock(&q->mutex);
}

bool queue_pop(part_queue_t *q, part_t *part) {
    mtx_lock(&q->mutex);

    while (q->count == 0) {
        cnd_wait(&q->not_empty, &q->mutex);  /* Wait for item */
    }

    *part = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;

    cnd_signal(&q->not_full);  /* Wake one waiting producer */
    mtx_unlock(&q->mutex);
    return true;
}

void queue_destroy(part_queue_t *q) {
    cnd_destroy(&q->not_full);
    cnd_destroy(&q->not_empty);
    mtx_destroy(&q->mutex);
    free(q->buffer);
}
```

### Once Initialization

```c
once_flag init_flag = ONCE_FLAG_INIT;
partsdb_t *global_db = nullptr;

void do_init(void) {
    partsdb_open("default.db", &global_db);
}

partsdb_t *get_global_db(void) {
    call_once(&init_flag, do_init);  /* Thread-safe one-time init */
    return global_db;
}
```

---

## 7.6 Type-Generic Macros (_Generic)

### Basic _Generic Usage

```c
/* Type-generic absolute value */
#define abs_generic(x) _Generic((x), \
    int: abs, \
    long: labs, \
    long long: llabs, \
    float: fabsf, \
    double: fabs, \
    long double: fabsl \
)(x)

/* Usage */
int i = abs_generic(-5);       /* Calls abs() */
double d = abs_generic(-3.14); /* Calls fabs() */
```

### Type-Generic Print

```c
#define print_value(x) _Generic((x), \
    int: print_int, \
    unsigned int: print_uint, \
    long: print_long, \
    double: print_double, \
    char *: print_string, \
    const char *: print_string, \
    default: print_unknown \
)(x)

static void print_int(int x) { printf("%d", x); }
static void print_uint(unsigned int x) { printf("%u", x); }
static void print_long(long x) { printf("%ld", x); }
static void print_double(double x) { printf("%f", x); }
static void print_string(const char *x) { printf("%s", x); }
static void print_unknown(void *x) { printf("%p", x); }

/* Usage */
print_value(42);        /* 42 */
print_value(3.14);      /* 3.140000 */
print_value("hello");   /* hello */
```

### Type-Generic Container

```c
/* Type-generic min/max */
#define min(a, b) _Generic((a) + (b), \
    int: min_int, \
    long: min_long, \
    double: min_double \
)((a), (b))

static int min_int(int a, int b) { return a < b ? a : b; }
static long min_long(long a, long b) { return a < b ? a : b; }
static double min_double(double a, double b) { return a < b ? a : b; }

/* Type-safe swap */
#define swap(a, b) do { \
    _Static_assert(_Generic((a), \
        typeof(b): 1, default: 0), \
        "swap: types must match"); \
    typeof(a) _tmp = (a); \
    (a) = (b); \
    (b) = _tmp; \
} while (0)
```

---

## 7.7 Static Assertions and Compile-Time Checks

### _Static_assert

```c
/* Compile-time assertions */
_Static_assert(sizeof(int) >= 4, "int must be at least 32 bits");
_Static_assert(sizeof(part_t) == 1048, "part_t size changed - check ABI");

/* Verify struct layout */
_Static_assert(offsetof(part_t, id) == 0, "id must be first field");
_Static_assert(offsetof(part_t, quantity) % 4 == 0, "quantity must be aligned");

/* Verify enum values */
_Static_assert(PARTSDB_OK == 0, "PARTSDB_OK must be zero for boolean checks");

/* Check array sizes */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define STATIC_ARRAY_SIZE(arr) \
    (_Static_assert(sizeof(arr) > sizeof(void*), "Not an array"), \
     sizeof(arr) / sizeof((arr)[0]))
```

### Compile-Time Type Checking

```c
/* Ensure pointer type at compile time */
#define CHECK_POINTER(ptr) \
    _Static_assert(_Generic((ptr), \
        void *: 1, \
        const void *: 1, \
        default: 0), \
        "Expected pointer type")

/* Type compatibility check */
#define SAME_TYPE(a, b) \
    _Generic((a), typeof(b): 1, default: 0)

/* Usage in API */
#define partsdb_free_checked(ptr) do { \
    _Static_assert(SAME_TYPE(ptr, part_t *) || \
                   SAME_TYPE(ptr, category_t *), \
                   "partsdb_free_checked: invalid type"); \
    free(ptr); \
} while (0)
```

---

## 7.8 PartsDB Case Study: Thread-Safe Database Pool

### Connection Pool Implementation

```c
/* Thread-safe connection pool for PartsDB */
typedef struct {
    partsdb_t **connections;
    size_t pool_size;
    size_t available;
    mtx_t mutex;
    cnd_t available_cond;
    const char *db_path;
} partsdb_pool_t;

partsdb_pool_t *partsdb_pool_create(const char *path, size_t size) {
    partsdb_pool_t *pool = malloc(sizeof(partsdb_pool_t));
    if (pool == nullptr) return nullptr;

    pool->connections = calloc(size, sizeof(partsdb_t *));
    if (pool->connections == nullptr) {
        free(pool);
        return nullptr;
    }

    pool->db_path = strdup(path);
    pool->pool_size = size;
    pool->available = 0;

    mtx_init(&pool->mutex, mtx_plain);
    cnd_init(&pool->available_cond);

    /* Pre-create connections */
    for (size_t i = 0; i < size; i++) {
        if (partsdb_open(path, &pool->connections[i]) == PARTSDB_OK) {
            pool->available++;
        }
    }

    return pool;
}

partsdb_t *partsdb_pool_acquire(partsdb_pool_t *pool) {
    mtx_lock(&pool->mutex);

    while (pool->available == 0) {
        cnd_wait(&pool->available_cond, &pool->mutex);
    }

    /* Find available connection */
    partsdb_t *conn = nullptr;
    for (size_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i] != nullptr) {
            conn = pool->connections[i];
            pool->connections[i] = nullptr;
            pool->available--;
            break;
        }
    }

    mtx_unlock(&pool->mutex);
    return conn;
}

void partsdb_pool_release(partsdb_pool_t *pool, partsdb_t *conn) {
    mtx_lock(&pool->mutex);

    /* Return to pool */
    for (size_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i] == nullptr) {
            pool->connections[i] = conn;
            pool->available++;
            break;
        }
    }

    cnd_signal(&pool->available_cond);
    mtx_unlock(&pool->mutex);
}

void partsdb_pool_destroy(partsdb_pool_t *pool) {
    if (pool == nullptr) return;

    for (size_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i] != nullptr) {
            partsdb_close(pool->connections[i]);
        }
    }

    cnd_destroy(&pool->available_cond);
    mtx_destroy(&pool->mutex);
    free((void *)pool->db_path);
    free(pool->connections);
    free(pool);
}
```

### Thread-Safe Statistics

```c
/* Atomic statistics for PartsDB */
typedef struct {
    atomic_uint64_t queries_total;
    atomic_uint64_t queries_failed;
    atomic_uint64_t bytes_read;
    atomic_uint64_t bytes_written;
    atomic_uint64_t cache_hits;
    atomic_uint64_t cache_misses;
} partsdb_stats_t;

static partsdb_stats_t global_stats = {0};

void stats_record_query(bool success) {
    atomic_fetch_add_explicit(&global_stats.queries_total, 1,
                              memory_order_relaxed);
    if (!success) {
        atomic_fetch_add_explicit(&global_stats.queries_failed, 1,
                                  memory_order_relaxed);
    }
}

void stats_get(partsdb_stats_t *out) {
    out->queries_total = atomic_load(&global_stats.queries_total);
    out->queries_failed = atomic_load(&global_stats.queries_failed);
    /* ... copy other fields ... */
}
```

---

## 7.9 Review Questions - Part 7

> **Instructions**: Research these questions using *Modern C*. Implement solutions in the PartsDB codebase where applicable.

### Conceptual Questions

1. **[Modern C, Ch. 17]** Explain the difference between `memory_order_acquire` and `memory_order_consume`. Why is `memory_order_consume` discouraged?

2. **[Modern C, Ch. 18]** What is the ABA problem in lock-free programming? How can it be mitigated?

3. **[Modern C, Ch. 19]** Explain why `atomic_compare_exchange_weak` exists alongside `atomic_compare_exchange_strong`. When would you use each?

4. **[Modern C, Ch. 20]** What is a "data race" vs a "race condition"? Give examples of each. Which is undefined behavior in C?

5. **[Modern C, Ch. 15]** Explain how `_Generic` selection works. What happens if no matching type is found and there's no `default`?

### Practical Exercises

6. **Thread-Safe Cache**: Implement a thread-safe LRU cache for PartsDB query results using mutexes and condition variables.

7. **Lock-Free Counter**: Implement a lock-free hit counter for PartsDB that supports increment and snapshot operations.

8. **Read-Write Lock**: Implement a read-write lock (multiple readers OR one writer) using C11 primitives. Use it to protect PartsDB's configuration.

9. **Type-Generic API**: Create type-generic macros for PartsDB that can handle both `part_t` and `category_t` with the same interface.

10. **Thread Pool**: Implement a thread pool that can execute PartsDB queries in parallel:
    ```c
    typedef void (*task_fn)(void *);
    void threadpool_submit(threadpool_t *pool, task_fn fn, void *arg);
    ```

### Research Topics

11. **[Modern C, Ch. 17]** Research the C11 memory model. How does it relate to hardware memory models (x86-TSO, ARM, etc.)? What guarantees does C11 provide?

12. **[Modern C, Ch. 18]** Research hazard pointers and RCU (Read-Copy-Update). How do they solve the memory reclamation problem in lock-free data structures?

13. **[Modern C, Ch. 16]** Research transactional memory. How does it compare to lock-based synchronization? What compiler support exists?

---

[← Part 6: API Design](part-06-api-design.md) | [Back to Index](README.md) | [Part 8: Memory Hierarchy →](part-08-memory-hierarchy.md)
