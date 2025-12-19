# The Silmarillion of Algorithms
## Part II: The Valaquenta — Of the Powers of Data

*"In the beginning Eru, the One, who in the Elvish tongue is named Ilúvatar,
made the Ainur of his thought; and they made a great Music before him.
In this Music the World was begun; and it was shaped through algorithms
that held the patterns of all things yet to come."*

---

# Part II-A: Of Arrays and the Halls of Stone

*In which we learn of the foundational craft of the Dwarves:
the carving of great halls with numbered chambers,
each stone in its appointed place.*

## The Philosophy of Contiguous Memory

When the Dwarves of Khazad-dûm first delved into Moria, they discovered a
fundamental truth: the strongest halls are carved from continuous stone.
A hall broken by chasms and gaps is weak; a hall of unbroken granite
endures for ages.

So it is with the **Array** — the most fundamental of all data structures.
An array is a contiguous block of memory, each element adjacent to the next,
like the chambers of Khazad-dûm carved in perfect sequence.

```
The Halls of Moria (Array Visualization):
┌─────────────────────────────────────────────────────────────────┐
│ Base Address: 0x1000                                            │
├────────┬────────┬────────┬────────┬────────┬────────┬────────┬──┤
│ [0]    │ [1]    │ [2]    │ [3]    │ [4]    │ [5]    │ [6]    │…│
│ Hall A │ Hall B │ Hall C │ Hall D │ Hall E │ Hall F │ Hall G │…│
│ 0x1000 │ 0x1004 │ 0x1008 │ 0x100C │ 0x1010 │ 0x1014 │ 0x1018 │…│
└────────┴────────┴────────┴────────┴────────┴────────┴────────┴──┘
   Each chamber is exactly 4 bytes (sizeof(int)), perfectly aligned
```

### The Three Sacred Properties

The Dwarven architects understood that arrays possess three properties
that no other structure can match:

**1. O(1) Random Access — The Dwarven Doorways**

Because every chamber is the same size and carved in sequence,
a Dwarf can calculate the exact location of any chamber instantly:

```c
/* The Formula of Direct Access */
address = base_address + (index × element_size)

/* In C, this is hidden behind the brackets */
int halls[100];
int value = halls[42];  /* Compiler calculates: halls + 42*sizeof(int) */
```

*"To find the forty-second hall, a Dwarf need not walk through halls
one through forty-one. He calculates the distance and steps directly
through the stone."*

**2. Memory Locality — The Warmth of Proximity**

When the great forges of Khazad-dûm burned, their heat spread through
adjacent stone. So too does the CPU cache work — when you access one
element, nearby elements are brought into the cache for free.

```c
/* Cache-friendly traversal (the stones are warm) */
for (int i = 0; i < n; i++) {
    sum += array[i];  /* Each access likely in cache */
}

/* Cache-hostile traversal (cold stone each step) */
for (int i = 0; i < n; i++) {
    sum += array[random_indices[i]];  /* Cache misses likely */
}
```

**3. Simplicity — The Strength of Stone**

Arrays have no overhead. No pointers to maintain, no metadata to store.
Just pure, unadorned data packed into memory. This is the Dwarven way:
*strength through simplicity*.

---

## The Crafting of Arrays in C23

### Static Arrays — Halls Carved at Kingdom's Founding

When a Dwarven kingdom is founded, certain halls are carved immediately,
their number fixed for all time. These are **static arrays**:

```c
/* Static array — size known at compile time */
#define MAX_PARTS 1000

typedef struct {
    char part_number[20];
    char description[100];
    int quantity;
    double price;
} Part;

/* The Great Hall of Parts — carved at program birth */
static Part parts_inventory[MAX_PARTS];
static size_t parts_count = 0;

/* Function to add a part to the halls */
bool add_part(const Part *new_part) {
    if (parts_count >= MAX_PARTS) {
        fprintf(stderr, "The halls are full! No more chambers!\n");
        return false;
    }

    /* Copy the part into the next available chamber */
    parts_inventory[parts_count] = *new_part;
    parts_count++;
    return true;
}
```

### Variable Length Arrays — Halls Carved to Need

C99 introduced Variable Length Arrays (VLAs), allowing the size to be
determined at runtime. But beware! These halls are carved from the stack,
which is limited in size.

```c
/* VLA — size determined at runtime, allocated on stack */
void process_measurements(size_t n) {
    /* WARNING: Large n can overflow the stack! */
    double measurements[n];  /* VLA */

    for (size_t i = 0; i < n; i++) {
        measurements[i] = read_sensor(i);
    }

    /* VLA automatically destroyed when function returns */
}

/* C23 allows VLAs in struct parameters (but not as struct members) */
void analyze_matrix(size_t rows, size_t cols,
                    double matrix[rows][cols]) {  /* C23 VLA parameter */
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            process(matrix[i][j]);
        }
    }
}
```

### Dynamic Arrays — Halls Hewn as the Kingdom Grows

The wisest Dwarven architects know that a kingdom's needs cannot always
be foreseen. They developed techniques to expand their halls as needed.
This is the **dynamic array** — the foundation of nearly every real program.

```c
/*
 * Dynamic Array Implementation
 * Like the ever-expanding halls of Erebor
 */

typedef struct {
    Part *items;        /* Pointer to the hall of parts */
    size_t count;       /* Number of occupied chambers */
    size_t capacity;    /* Total chambers carved */
} PartArray;

/* Create a new array kingdom */
PartArray *part_array_create(size_t initial_capacity) {
    PartArray *arr = malloc(sizeof(PartArray));
    if (!arr) return NULL;

    arr->items = malloc(initial_capacity * sizeof(Part));
    if (!arr->items) {
        free(arr);
        return NULL;
    }

    arr->count = 0;
    arr->capacity = initial_capacity;
    return arr;
}

/* Expand the halls when full */
static bool part_array_grow(PartArray *arr) {
    /* Double the capacity — the geometric growth strategy */
    size_t new_capacity = arr->capacity * 2;
    Part *new_items = realloc(arr->items, new_capacity * sizeof(Part));

    if (!new_items) {
        return false;  /* The mountain yields no more stone */
    }

    arr->items = new_items;
    arr->capacity = new_capacity;
    return true;
}

/* Add a part — O(1) amortized */
bool part_array_push(PartArray *arr, const Part *part) {
    /* If halls are full, carve more */
    if (arr->count >= arr->capacity) {
        if (!part_array_grow(arr)) {
            return false;
        }
    }

    arr->items[arr->count] = *part;
    arr->count++;
    return true;
}

/* Direct access — O(1) */
Part *part_array_get(PartArray *arr, size_t index) {
    if (index >= arr->count) {
        return NULL;  /* Chamber does not exist */
    }
    return &arr->items[index];
}

/* Remove from end — O(1) */
bool part_array_pop(PartArray *arr, Part *out_part) {
    if (arr->count == 0) {
        return false;  /* Halls are empty */
    }

    arr->count--;
    if (out_part) {
        *out_part = arr->items[arr->count];
    }
    return true;
}

/* Destroy the halls */
void part_array_destroy(PartArray *arr) {
    if (arr) {
        free(arr->items);
        free(arr);
    }
}
```

---

## The Operations of the Stone Halls

### Insertion — Carving New Chambers

Inserting at the end is simple — just carve a new chamber. But inserting
in the middle requires moving all subsequent chambers, like shifting
the entire mountain.

```c
/*
 * Insert at position — O(n) in general, O(1) at end
 *
 * This is like carving a new chamber in the middle of
 * existing halls — all subsequent halls must be moved!
 */
bool part_array_insert(PartArray *arr, size_t index, const Part *part) {
    if (index > arr->count) {
        return false;  /* Cannot carve beyond the kingdom */
    }

    /* Ensure capacity */
    if (arr->count >= arr->capacity) {
        if (!part_array_grow(arr)) {
            return false;
        }
    }

    /* Shift all elements from index onward */
    /* This is expensive! O(n) operations */
    for (size_t i = arr->count; i > index; i--) {
        arr->items[i] = arr->items[i - 1];
    }

    arr->items[index] = *part;
    arr->count++;
    return true;
}

/*
 * Visualization of insertion at index 2:
 *
 * Before: [A][B][C][D][E][ ]   insert X at index 2
 *              ↑
 *
 * Step 1: [A][B][C][D][ ][E]   shift E right
 * Step 2: [A][B][C][ ][D][E]   shift D right
 * Step 3: [A][B][ ][C][D][E]   shift C right
 * Step 4: [A][B][X][C][D][E]   insert X
 */
```

### Deletion — Collapsing Empty Chambers

Removing an element from the middle creates a gap that must be filled
by sliding all subsequent elements down.

```c
/*
 * Remove at position — O(n) in general, O(1) at end
 */
bool part_array_remove(PartArray *arr, size_t index, Part *out_part) {
    if (index >= arr->count) {
        return false;
    }

    /* Save the removed item if requested */
    if (out_part) {
        *out_part = arr->items[index];
    }

    /* Shift all elements after index to the left */
    for (size_t i = index; i < arr->count - 1; i++) {
        arr->items[i] = arr->items[i + 1];
    }

    arr->count--;
    return true;
}

/*
 * Alternative: Swap-and-pop — O(1) but destroys order
 * Used when order doesn't matter
 */
bool part_array_remove_unordered(PartArray *arr, size_t index, Part *out) {
    if (index >= arr->count) {
        return false;
    }

    if (out) {
        *out = arr->items[index];
    }

    /* Swap with last element, then pop */
    arr->items[index] = arr->items[arr->count - 1];
    arr->count--;
    return true;
}
```

### Searching — Finding Chambers in the Dark

```c
/*
 * Linear Search — O(n)
 * Walking hall by hall with a torch
 */
int part_array_find_linear(PartArray *arr, const char *part_number) {
    for (size_t i = 0; i < arr->count; i++) {
        if (strcmp(arr->items[i].part_number, part_number) == 0) {
            return (int)i;  /* Found in chamber i */
        }
    }
    return -1;  /* Not in these halls */
}

/*
 * Binary Search — O(log n)
 * Requires sorted array! Like numbered halls in order.
 */
int part_array_find_binary(PartArray *arr, const char *part_number) {
    int low = 0;
    int high = (int)arr->count - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = strcmp(arr->items[mid].part_number, part_number);

        if (cmp == 0) {
            return mid;
        } else if (cmp > 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return -1;
}
```

---

## Multi-Dimensional Arrays — The Stacked Halls of Erebor

The Dwarves of Erebor carved their halls in multiple levels,
creating a three-dimensional kingdom. So too can we create
multi-dimensional arrays.

### Two-Dimensional Arrays — Maps of the Realm

```c
/*
 * 2D Array — Row-major order (C convention)
 * Like a map of the underground kingdom
 */

#define MAP_WIDTH  100
#define MAP_HEIGHT 50

typedef enum {
    TERRAIN_EMPTY = 0,
    TERRAIN_WALL,
    TERRAIN_DOOR,
    TERRAIN_FORGE,
    TERRAIN_TREASURE
} TerrainType;

/* Static 2D array — the map of Khazad-dûm */
static TerrainType dungeon_map[MAP_HEIGHT][MAP_WIDTH];

/* Access element at (row, col) */
TerrainType get_terrain(int row, int col) {
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) {
        return TERRAIN_WALL;  /* Out of bounds = wall */
    }
    return dungeon_map[row][col];
}

/*
 * Memory layout of 2D array (row-major):
 *
 * Logical view:          Memory view:
 * [0,0][0,1][0,2]        [0,0][0,1][0,2][1,0][1,1][1,2][2,0][2,1][2,2]
 * [1,0][1,1][1,2]   →    ← Row 0 →← Row 1 →← Row 2 →
 * [2,0][2,1][2,2]
 *
 * Address of [row][col] = base + (row * WIDTH + col) * sizeof(element)
 */
```

### Dynamic 2D Arrays — Halls Hewn in Layers

```c
/*
 * Dynamic 2D array using array of pointers
 * More flexible but less cache-friendly
 */
typedef struct {
    Part **rows;         /* Array of pointers to rows */
    size_t row_count;
    size_t col_count;
} PartMatrix;

PartMatrix *part_matrix_create(size_t rows, size_t cols) {
    PartMatrix *matrix = malloc(sizeof(PartMatrix));
    if (!matrix) return NULL;

    matrix->rows = malloc(rows * sizeof(Part *));
    if (!matrix->rows) {
        free(matrix);
        return NULL;
    }

    for (size_t i = 0; i < rows; i++) {
        matrix->rows[i] = malloc(cols * sizeof(Part));
        if (!matrix->rows[i]) {
            /* Cleanup on failure */
            for (size_t j = 0; j < i; j++) {
                free(matrix->rows[j]);
            }
            free(matrix->rows);
            free(matrix);
            return NULL;
        }
    }

    matrix->row_count = rows;
    matrix->col_count = cols;
    return matrix;
}

/*
 * Dynamic 2D array as single contiguous block
 * More cache-friendly, same interface
 */
typedef struct {
    Part *data;          /* Single contiguous block */
    size_t row_count;
    size_t col_count;
} PartGrid;

PartGrid *part_grid_create(size_t rows, size_t cols) {
    PartGrid *grid = malloc(sizeof(PartGrid));
    if (!grid) return NULL;

    /* Single allocation for all elements */
    grid->data = malloc(rows * cols * sizeof(Part));
    if (!grid->data) {
        free(grid);
        return NULL;
    }

    grid->row_count = rows;
    grid->col_count = cols;
    return grid;
}

/* Access with manual index calculation */
Part *part_grid_get(PartGrid *grid, size_t row, size_t col) {
    if (row >= grid->row_count || col >= grid->col_count) {
        return NULL;
    }
    return &grid->data[row * grid->col_count + col];
}
```

---

## The Ring Buffer — The Eternal Carousel

In the deepest forges of Khazad-dûm, the Dwarves built great circular
conveyor systems to move ore. When the belt reached the end, it looped
back to the beginning. This is the **Ring Buffer** (Circular Buffer).

```c
/*
 * Ring Buffer — O(1) push and pop at both ends
 * Perfect for streaming data, queues, and buffers
 *
 *     ┌─────────────────────────────┐
 *     │   The Circular Forge Belt   │
 *     │                             │
 *     │    ┌───┬───┬───┬───┬───┐   │
 *     │    │ 0 │ 1 │ 2 │ 3 │ 4 │   │
 *     │    └───┴───┴───┴───┴───┘   │
 *     │      ↑           ↑         │
 *     │    head        tail        │
 *     │                             │
 *     └─────────────────────────────┘
 */

typedef struct {
    Part *buffer;
    size_t capacity;
    size_t head;    /* Index of first element */
    size_t tail;    /* Index after last element */
    size_t count;   /* Number of elements */
} PartRingBuffer;

PartRingBuffer *ring_buffer_create(size_t capacity) {
    PartRingBuffer *rb = malloc(sizeof(PartRingBuffer));
    if (!rb) return NULL;

    rb->buffer = malloc(capacity * sizeof(Part));
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }

    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    return rb;
}

/* Add to tail — O(1) */
bool ring_buffer_push(PartRingBuffer *rb, const Part *part) {
    if (rb->count >= rb->capacity) {
        return false;  /* Belt is full */
    }

    rb->buffer[rb->tail] = *part;
    rb->tail = (rb->tail + 1) % rb->capacity;  /* Wrap around */
    rb->count++;
    return true;
}

/* Remove from head — O(1) */
bool ring_buffer_pop(PartRingBuffer *rb, Part *out_part) {
    if (rb->count == 0) {
        return false;  /* Belt is empty */
    }

    if (out_part) {
        *out_part = rb->buffer[rb->head];
    }
    rb->head = (rb->head + 1) % rb->capacity;  /* Wrap around */
    rb->count--;
    return true;
}

/* Peek at head without removing — O(1) */
Part *ring_buffer_peek(PartRingBuffer *rb) {
    if (rb->count == 0) {
        return NULL;
    }
    return &rb->buffer[rb->head];
}

/* Check if full/empty — O(1) */
bool ring_buffer_is_full(PartRingBuffer *rb) {
    return rb->count == rb->capacity;
}

bool ring_buffer_is_empty(PartRingBuffer *rb) {
    return rb->count == 0;
}
```

### Ring Buffer Power of Two Optimization

```c
/*
 * When capacity is a power of 2, modulo becomes a bitmask
 * This is MUCH faster on most CPUs
 */

typedef struct {
    Part *buffer;
    size_t capacity;    /* MUST be power of 2 */
    size_t mask;        /* capacity - 1 */
    size_t head;
    size_t tail;
} FastRingBuffer;

FastRingBuffer *fast_ring_create(size_t capacity) {
    /* Round up to next power of 2 */
    size_t actual = 1;
    while (actual < capacity) {
        actual *= 2;
    }

    FastRingBuffer *rb = malloc(sizeof(FastRingBuffer));
    if (!rb) return NULL;

    rb->buffer = malloc(actual * sizeof(Part));
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }

    rb->capacity = actual;
    rb->mask = actual - 1;  /* For fast modulo */
    rb->head = 0;
    rb->tail = 0;
    return rb;
}

/* Fast push using bitmask instead of modulo */
bool fast_ring_push(FastRingBuffer *rb, const Part *part) {
    if (((rb->tail - rb->head) & rb->mask) == rb->mask) {
        return false;  /* Full */
    }

    rb->buffer[rb->tail & rb->mask] = *part;
    rb->tail++;  /* Let it overflow naturally */
    return true;
}

/* Fast pop using bitmask */
bool fast_ring_pop(FastRingBuffer *rb, Part *out) {
    if (rb->head == rb->tail) {
        return false;  /* Empty */
    }

    if (out) {
        *out = rb->buffer[rb->head & rb->mask];
    }
    rb->head++;
    return true;
}
```

---

## The Stack — The Dwarven Mine Shaft

In the mines of Moria, Dwarves would descend into shafts on platforms
that could only move up and down. The last platform to descend was
always the first to rise. This is the **Stack** — Last In, First Out (LIFO).

```c
/*
 * Stack Implementation using Dynamic Array
 *
 *     ┌─────────────────────┐
 *     │   The Mine Shaft    │
 *     │                     │
 *     │   ┌───────────┐     │  ← top (push/pop here)
 *     │   │  Part D   │     │
 *     │   ├───────────┤     │
 *     │   │  Part C   │     │
 *     │   ├───────────┤     │
 *     │   │  Part B   │     │
 *     │   ├───────────┤     │
 *     │   │  Part A   │     │  ← bottom
 *     │   └───────────┘     │
 *     └─────────────────────┘
 */

typedef struct {
    Part *items;
    size_t count;
    size_t capacity;
} PartStack;

PartStack *part_stack_create(size_t initial_capacity) {
    PartStack *stack = malloc(sizeof(PartStack));
    if (!stack) return NULL;

    stack->items = malloc(initial_capacity * sizeof(Part));
    if (!stack->items) {
        free(stack);
        return NULL;
    }

    stack->count = 0;
    stack->capacity = initial_capacity;
    return stack;
}

/* Push — O(1) amortized */
bool part_stack_push(PartStack *stack, const Part *part) {
    if (stack->count >= stack->capacity) {
        size_t new_cap = stack->capacity * 2;
        Part *new_items = realloc(stack->items, new_cap * sizeof(Part));
        if (!new_items) return false;
        stack->items = new_items;
        stack->capacity = new_cap;
    }

    stack->items[stack->count++] = *part;
    return true;
}

/* Pop — O(1) */
bool part_stack_pop(PartStack *stack, Part *out_part) {
    if (stack->count == 0) return false;

    if (out_part) {
        *out_part = stack->items[--stack->count];
    } else {
        stack->count--;
    }
    return true;
}

/* Peek — O(1) */
Part *part_stack_peek(PartStack *stack) {
    if (stack->count == 0) return NULL;
    return &stack->items[stack->count - 1];
}
```

### Stack Applications in PartsDB

```c
/*
 * Undo System — Each operation pushed onto the stack
 */
typedef enum {
    OP_ADD_PART,
    OP_DELETE_PART,
    OP_MODIFY_PART
} OperationType;

typedef struct {
    OperationType type;
    Part old_state;
    Part new_state;
    size_t index;
} UndoOperation;

typedef struct {
    UndoOperation *items;
    size_t count;
    size_t capacity;
} UndoStack;

void record_operation(UndoStack *undo, OperationType type,
                      Part *old_state, Part *new_state, size_t index) {
    UndoOperation op = {
        .type = type,
        .index = index
    };
    if (old_state) op.old_state = *old_state;
    if (new_state) op.new_state = *new_state;

    /* Push to undo stack */
    if (undo->count >= undo->capacity) {
        undo->capacity *= 2;
        undo->items = realloc(undo->items,
                              undo->capacity * sizeof(UndoOperation));
    }
    undo->items[undo->count++] = op;
}

void perform_undo(UndoStack *undo, PartArray *inventory) {
    if (undo->count == 0) {
        printf("Nothing to undo.\n");
        return;
    }

    UndoOperation op = undo->items[--undo->count];

    switch (op.type) {
        case OP_ADD_PART:
            /* Undo an add by removing */
            part_array_remove(inventory, op.index, NULL);
            break;

        case OP_DELETE_PART:
            /* Undo a delete by re-adding */
            part_array_insert(inventory, op.index, &op.old_state);
            break;

        case OP_MODIFY_PART:
            /* Undo a modify by restoring old state */
            inventory->items[op.index] = op.old_state;
            break;
    }
}
```

---

## The Queue — The Dwarven Assembly Line

In the great forges, ore arrived at one end and finished goods emerged
from the other. The first ore to enter was the first to be processed.
This is the **Queue** — First In, First Out (FIFO).

```c
/*
 * Queue Implementation using Ring Buffer
 * (See Ring Buffer section above)
 *
 *     ┌───────────────────────────────────────┐
 *     │      The Forge Assembly Line          │
 *     │                                       │
 *     │  enqueue →  [A][B][C][D][E]  → dequeue│
 *     │             ↑           ↑             │
 *     │           tail        head            │
 *     └───────────────────────────────────────┘
 */

/* Queue is just a ring buffer with different semantics */
typedef PartRingBuffer PartQueue;

#define part_queue_create ring_buffer_create
#define part_queue_enqueue ring_buffer_push
#define part_queue_dequeue ring_buffer_pop
#define part_queue_peek ring_buffer_peek
#define part_queue_is_empty ring_buffer_is_empty
#define part_queue_is_full ring_buffer_is_full
```

### Priority Queue Preview

We'll cover the full Priority Queue (Heap) in Part III-C,
but here's a preview of the interface:

```c
/*
 * Priority Queue — Items with higher priority dequeued first
 * Used for scheduling, pathfinding (Dijkstra), etc.
 */

typedef int (*PartComparator)(const Part *a, const Part *b);

typedef struct {
    Part *heap;          /* Binary heap storage */
    size_t count;
    size_t capacity;
    PartComparator cmp;  /* Comparison function */
} PartPriorityQueue;

/* Operations preview — full implementation in Part III-C */
bool pq_insert(PartPriorityQueue *pq, const Part *part);  /* O(log n) */
bool pq_extract_min(PartPriorityQueue *pq, Part *out);    /* O(log n) */
Part *pq_peek_min(PartPriorityQueue *pq);                 /* O(1) */
```

---

## Scavenger Hunt — Clue #6

*"In the ancient texts of the Algorithmist Sedgewick, Chapter 3 speaks
of Elementary Data Structures. There, he reveals the secret of using
an array as a stack with but a single pointer. What is this pointer called,
and what does it track?"*

🔍 **Your Quest**: Open Sedgewick's "Algorithms in C" to Chapter 3.
Find the implementation of a stack using arrays. The answer lies
in understanding what single variable controls all stack operations.

*The answer shall be recorded in the Chronicle of Nauglamír...*

---

## Scavenger Hunt — Clue #7

*"Bhargava, the wise chronicler of Grokking, speaks in Chapter 4
of 'the call stack.' He uses the tale of a simple box search to
reveal the danger of infinite recursion. What happens to the stack
when recursion has no end?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 4. Find the
warning about the call stack and recursion. What catastrophe awaits
those who forget their base case?

---

## The Complexity Tablet of Arrays

```
╔══════════════════════════════════════════════════════════════════╗
║           THE STONE TABLET OF ARRAY COMPLEXITIES                 ║
╠══════════════════════════════════════════════════════════════════╣
║  Operation              │ Time      │ Space   │ Notes            ║
╠═════════════════════════╪═══════════╪═════════╪══════════════════╣
║  Access by index        │ O(1)      │ O(1)    │ Direct address   ║
║  Search (unsorted)      │ O(n)      │ O(1)    │ Linear scan      ║
║  Search (sorted)        │ O(log n)  │ O(1)    │ Binary search    ║
║  Insert at end          │ O(1)*     │ O(1)    │ *Amortized       ║
║  Insert at beginning    │ O(n)      │ O(1)    │ Shift all        ║
║  Insert in middle       │ O(n)      │ O(1)    │ Shift half       ║
║  Delete from end        │ O(1)      │ O(1)    │ Just decrement   ║
║  Delete from beginning  │ O(n)      │ O(1)    │ Shift all        ║
║  Delete from middle     │ O(n)      │ O(1)    │ Shift half       ║
║  Swap-remove (unordered)│ O(1)      │ O(1)    │ Destroys order   ║
╠═════════════════════════╧═══════════╧═════════╧══════════════════╣
║  Space overhead: O(1) — Just capacity and count                  ║
║  Cache performance: Excellent — Contiguous memory                ║
╚══════════════════════════════════════════════════════════════════╝
```

---

## Exercises — The First Trials of the Stone Halls

### Trial 1: The Inventory Ledger
```c
/*
 * Implement a parts inventory using a dynamic array
 * Requirements:
 * 1. Support adding parts (push to end)
 * 2. Support finding parts by part_number (linear search)
 * 3. Support removing parts by part_number
 * 4. Support sorting by part_number (we'll implement sorts in Part III)
 */
```

### Trial 2: The Mine Cart Queue
```c
/*
 * Implement a ring buffer for tracking the last N sensor readings
 * Requirements:
 * 1. Fixed capacity (configurable at creation)
 * 2. When full, oldest reading is overwritten
 * 3. Support iterating over all readings (oldest to newest)
 * 4. Support calculating running average
 */
```

### Trial 3: The Expression Evaluator
```c
/*
 * Use a stack to evaluate postfix expressions
 * Examples:
 *   "3 4 +" → 7
 *   "3 4 2 * +" → 11 (3 + (4 * 2))
 *   "5 1 2 + 4 * + 3 -" → 14 (5 + ((1 + 2) * 4) - 3)
 */
```

---

*Thus ends the first chapter of the Valaquenta.
The foundations have been laid in stone.
In the next chapter, we shall learn of Linked Lists,
structures of chain and mail, flexible where arrays are rigid.*

---

---

# Part II-B: Of Linked Lists and Chain-Mail

*In which we learn of the Dwarven smiths who forged
chains of mithril, each link connected to the next,
flexible where stone is rigid, strong where it must bend.*

## The Philosophy of Links

When the Dwarves needed armor that could move with the body,
they could not use solid stone. They forged chain-mail —
thousands of small rings, each linked to its neighbors,
flexible yet strong.

So it is with the **Linked List**. Where an array is a solid
block of stone, a linked list is a chain of nodes, each pointing
to the next. It trades the array's fast random access for the
ability to insert and delete without shifting the world.

```
The Chain of Nodes:

     ┌──────────────────────────────────────────────────────────┐
     │                  The Mithril Chain                       │
     │                                                          │
     │   ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌────┐   │
     │   │  Data   │    │  Data   │    │  Data   │    │NULL│   │
     │   │ "Bolt"  │───▶│ "Nut"   │───▶│ "Washer"│───▶│    │   │
     │   │  next ──┘    │  next ──┘    │  next ──┘    └────┘   │
     │   └─────────┘    └─────────┘    └─────────┘             │
     │       ↑                                                  │
     │      head                                                │
     └──────────────────────────────────────────────────────────┘
```

### The Trade-Offs of Chain-Mail

| Property          | Array (Stone)     | Linked List (Chain) |
|-------------------|-------------------|---------------------|
| Memory layout     | Contiguous        | Scattered           |
| Access by index   | O(1)              | O(n)                |
| Insert at head    | O(n)              | O(1)                |
| Insert at tail    | O(1) amortized    | O(1) with tail ptr  |
| Insert in middle  | O(n)              | O(1) if at position |
| Memory overhead   | Low               | Higher (pointers)   |
| Cache performance | Excellent         | Poor                |

*"The stone halls are faster to navigate, but the chain-mail
adapts to the wearer's movements."*

---

## The Singly Linked List — A Simple Chain

### The Node Structure

```c
/*
 * A single link in the chain
 * Each node holds data and a pointer to the next node
 */
typedef struct PartNode {
    Part data;              /* The cargo in this link */
    struct PartNode *next;  /* The next link in the chain */
} PartNode;

/*
 * The chain itself — we track the head (and optionally tail)
 */
typedef struct {
    PartNode *head;   /* First link in the chain */
    PartNode *tail;   /* Last link (optional optimization) */
    size_t count;     /* Number of links */
} PartList;
```

### Creating and Destroying the Chain

```c
/*
 * Forge a new empty chain
 */
PartList *part_list_create(void) {
    PartList *list = malloc(sizeof(PartList));
    if (!list) return NULL;

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

/*
 * Forge a new link (node)
 */
static PartNode *create_node(const Part *part) {
    PartNode *node = malloc(sizeof(PartNode));
    if (!node) return NULL;

    node->data = *part;
    node->next = NULL;
    return node;
}

/*
 * Melt down the chain — free all links
 */
void part_list_destroy(PartList *list) {
    if (!list) return;

    PartNode *current = list->head;
    while (current != NULL) {
        PartNode *next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
```

### Insertion Operations

```c
/*
 * Add a link at the head — O(1)
 * The simplest and fastest insertion
 */
bool part_list_push_front(PartList *list, const Part *part) {
    PartNode *node = create_node(part);
    if (!node) return false;

    node->next = list->head;  /* Point to old head */
    list->head = node;        /* New node becomes head */

    if (list->tail == NULL) {
        list->tail = node;    /* First node is also tail */
    }

    list->count++;
    return true;
}

/*
 * Add a link at the tail — O(1) with tail pointer
 */
bool part_list_push_back(PartList *list, const Part *part) {
    PartNode *node = create_node(part);
    if (!node) return false;

    if (list->tail == NULL) {
        /* Empty list */
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;  /* Old tail points to new node */
        list->tail = node;        /* New node becomes tail */
    }

    list->count++;
    return true;
}

/*
 * Insert after a given node — O(1)
 * This is where linked lists shine!
 *
 *   Before: [A] → [B] → [C]
 *                  ↑
 *                after
 *
 *   After:  [A] → [B] → [X] → [C]
 */
bool part_list_insert_after(PartList *list, PartNode *after,
                            const Part *part) {
    if (!after) return false;

    PartNode *node = create_node(part);
    if (!node) return false;

    node->next = after->next;  /* New node points to what after pointed to */
    after->next = node;        /* after now points to new node */

    if (list->tail == after) {
        list->tail = node;     /* Update tail if we inserted at end */
    }

    list->count++;
    return true;
}
```

### Deletion Operations

```c
/*
 * Remove from head — O(1)
 */
bool part_list_pop_front(PartList *list, Part *out_part) {
    if (list->head == NULL) return false;

    PartNode *old_head = list->head;
    if (out_part) {
        *out_part = old_head->data;
    }

    list->head = old_head->next;

    if (list->head == NULL) {
        list->tail = NULL;  /* List is now empty */
    }

    free(old_head);
    list->count--;
    return true;
}

/*
 * Remove after a given node — O(1)
 *
 *   Before: [A] → [B] → [C] → [D]
 *                  ↑
 *                after  (remove C)
 *
 *   After:  [A] → [B] → [D]
 */
bool part_list_remove_after(PartList *list, PartNode *after, Part *out) {
    if (!after || !after->next) return false;

    PartNode *to_remove = after->next;
    if (out) {
        *out = to_remove->data;
    }

    after->next = to_remove->next;

    if (list->tail == to_remove) {
        list->tail = after;  /* Update tail if we removed it */
    }

    free(to_remove);
    list->count--;
    return true;
}

/*
 * Remove a specific node — O(n)
 * Must traverse to find the previous node
 */
bool part_list_remove(PartList *list, PartNode *node, Part *out) {
    if (!node || !list->head) return false;

    /* Special case: removing head */
    if (list->head == node) {
        return part_list_pop_front(list, out);
    }

    /* Find the node before the one we want to remove */
    PartNode *prev = list->head;
    while (prev->next != NULL && prev->next != node) {
        prev = prev->next;
    }

    if (prev->next != node) {
        return false;  /* Node not found in list */
    }

    return part_list_remove_after(list, prev, out);
}
```

### Traversal and Search

```c
/*
 * Traverse the chain — O(n)
 * Apply a function to each element
 */
typedef void (*PartVisitor)(Part *part, void *context);

void part_list_foreach(PartList *list, PartVisitor visitor, void *context) {
    for (PartNode *node = list->head; node != NULL; node = node->next) {
        visitor(&node->data, context);
    }
}

/* Example visitor: print all parts */
void print_part(Part *part, void *context) {
    (void)context;  /* Unused */
    printf("Part: %s - %s\n", part->part_number, part->description);
}

void print_all_parts(PartList *list) {
    part_list_foreach(list, print_part, NULL);
}

/*
 * Find a node by predicate — O(n)
 */
typedef bool (*PartPredicate)(const Part *part, const void *key);

PartNode *part_list_find(PartList *list, PartPredicate pred, const void *key) {
    for (PartNode *node = list->head; node != NULL; node = node->next) {
        if (pred(&node->data, key)) {
            return node;
        }
    }
    return NULL;
}

/* Predicate for finding by part number */
bool match_part_number(const Part *part, const void *key) {
    return strcmp(part->part_number, (const char *)key) == 0;
}

/* Usage */
PartNode *found = part_list_find(list, match_part_number, "BOLT-001");
```

### Getting Elements by Index — The Slow Path

```c
/*
 * Access by index — O(n)
 * This is where linked lists are weak!
 */
PartNode *part_list_get(PartList *list, size_t index) {
    if (index >= list->count) return NULL;

    PartNode *node = list->head;
    for (size_t i = 0; i < index; i++) {
        node = node->next;
    }
    return node;
}

/*
 * WARNING: This common pattern is O(n²)!
 * Never do this with a linked list:
 */
void bad_iteration(PartList *list) {
    for (size_t i = 0; i < list->count; i++) {
        PartNode *node = part_list_get(list, i);  /* O(n) each time! */
        process(&node->data);
    }
    /* Total: O(n²) — very slow for large lists */
}

/*
 * GOOD: Use the natural traversal
 */
void good_iteration(PartList *list) {
    for (PartNode *node = list->head; node != NULL; node = node->next) {
        process(&node->data);  /* O(1) each time */
    }
    /* Total: O(n) — much better! */
}
```

---

## The Doubly Linked List — Chain-Mail with Two Rings

A doubly linked list has links pointing both forward and backward,
like a chain where each ring is connected to both neighbors.

```
The Double Chain:

     ┌────────────────────────────────────────────────────────────┐
     │              The Two-Way Mithril Chain                     │
     │                                                            │
     │   ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌────┐     │
     │   │  prev ◀─┼────│  prev ◀─┼────│  prev ◀─┼────│NULL│     │
     │   │  Data   │    │  Data   │    │  Data   │    │    │     │
     │   │ "Bolt"  │    │ "Nut"   │    │ "Washer"│    │    │     │
     │   │  next ──┼───▶│  next ──┼───▶│  next ──┼───▶│NULL│     │
     │   └─────────┘    └─────────┘    └─────────┘    └────┘     │
     │       ↑                              ↑                     │
     │      head                          tail                    │
     └────────────────────────────────────────────────────────────┘
```

```c
/*
 * Doubly Linked List Node
 */
typedef struct DPartNode {
    Part data;
    struct DPartNode *prev;  /* Previous link */
    struct DPartNode *next;  /* Next link */
} DPartNode;

typedef struct {
    DPartNode *head;
    DPartNode *tail;
    size_t count;
} DPartList;

/*
 * Create a new doubly linked node
 */
static DPartNode *create_dnode(const Part *part) {
    DPartNode *node = malloc(sizeof(DPartNode));
    if (!node) return NULL;

    node->data = *part;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

/*
 * Insert at head — O(1)
 */
bool dpart_list_push_front(DPartList *list, const Part *part) {
    DPartNode *node = create_dnode(part);
    if (!node) return false;

    node->next = list->head;

    if (list->head != NULL) {
        list->head->prev = node;
    } else {
        list->tail = node;  /* First node is also tail */
    }

    list->head = node;
    list->count++;
    return true;
}

/*
 * Insert at tail — O(1)
 */
bool dpart_list_push_back(DPartList *list, const Part *part) {
    DPartNode *node = create_dnode(part);
    if (!node) return false;

    node->prev = list->tail;

    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        list->head = node;  /* First node is also head */
    }

    list->tail = node;
    list->count++;
    return true;
}

/*
 * Remove from tail — O(1) with doubly linked!
 * This is impossible in O(1) with singly linked lists
 */
bool dpart_list_pop_back(DPartList *list, Part *out_part) {
    if (list->tail == NULL) return false;

    DPartNode *old_tail = list->tail;
    if (out_part) {
        *out_part = old_tail->data;
    }

    list->tail = old_tail->prev;

    if (list->tail != NULL) {
        list->tail->next = NULL;
    } else {
        list->head = NULL;  /* List is now empty */
    }

    free(old_tail);
    list->count--;
    return true;
}

/*
 * Remove a specific node — O(1)!
 * With a doubly linked list, we can remove without traversing
 */
bool dpart_list_remove_node(DPartList *list, DPartNode *node, Part *out) {
    if (!node) return false;

    if (out) {
        *out = node->data;
    }

    /* Update previous node's next pointer */
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;  /* Removing head */
    }

    /* Update next node's prev pointer */
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;  /* Removing tail */
    }

    free(node);
    list->count--;
    return true;
}

/*
 * Insert before a specific node — O(1)
 */
bool dpart_list_insert_before(DPartList *list, DPartNode *before,
                              const Part *part) {
    if (!before) return false;

    DPartNode *node = create_dnode(part);
    if (!node) return false;

    node->next = before;
    node->prev = before->prev;

    if (before->prev != NULL) {
        before->prev->next = node;
    } else {
        list->head = node;  /* Inserting before head */
    }

    before->prev = node;
    list->count++;
    return true;
}
```

---

## The Sentinel Node — The Guardian of the Gates

The Dwarves placed guardians at the gates of their halls,
not to store treasure but to simplify passage. A **sentinel node**
is a dummy node that simplifies linked list operations by
eliminating special cases for empty lists.

```c
/*
 * Linked List with Sentinel Nodes
 * No more NULL checks for head/tail!
 *
 *   ┌────────────┐    ┌─────────┐    ┌────────────┐
 *   │  SENTINEL  │───▶│  Data   │───▶│  SENTINEL  │
 *   │   (head)   │◀───│  Node   │◀───│   (tail)   │
 *   └────────────┘    └─────────┘    └────────────┘
 */

typedef struct {
    DPartNode head_sentinel;  /* Not a pointer — always exists */
    DPartNode tail_sentinel;
    size_t count;
} SentinelList;

/*
 * Initialize with sentinels pointing to each other
 */
void sentinel_list_init(SentinelList *list) {
    list->head_sentinel.prev = NULL;
    list->head_sentinel.next = &list->tail_sentinel;

    list->tail_sentinel.prev = &list->head_sentinel;
    list->tail_sentinel.next = NULL;

    list->count = 0;
}

/*
 * Insert after any node — no special cases!
 */
void sentinel_insert_after(SentinelList *list, DPartNode *after,
                           DPartNode *node) {
    node->prev = after;
    node->next = after->next;
    after->next->prev = node;
    after->next = node;
    list->count++;
    /* No NULL checks needed — sentinels are always there */
}

/*
 * Remove any node — no special cases!
 */
void sentinel_remove(SentinelList *list, DPartNode *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->count--;
    /* No NULL checks — sentinels handle the edges */
}

/*
 * Push to front — insert after head sentinel
 */
void sentinel_push_front(SentinelList *list, DPartNode *node) {
    sentinel_insert_after(list, &list->head_sentinel, node);
}

/*
 * Push to back — insert before tail sentinel
 */
void sentinel_push_back(SentinelList *list, DPartNode *node) {
    sentinel_insert_after(list, list->tail_sentinel.prev, node);
}

/*
 * Is the list empty?
 */
bool sentinel_is_empty(SentinelList *list) {
    return list->head_sentinel.next == &list->tail_sentinel;
}

/*
 * Get first real node (or NULL if empty)
 */
DPartNode *sentinel_first(SentinelList *list) {
    if (sentinel_is_empty(list)) return NULL;
    return list->head_sentinel.next;
}
```

---

## The Circular List — The Endless Ring

In the deepest forges, Dwarves created perfect rings with no beginning
and no end. The **Circular Linked List** connects the tail back to
the head, forming an endless loop.

```
The Endless Ring:

          ┌─────────────────────────────────────┐
          │                                     │
          ▼                                     │
     ┌─────────┐    ┌─────────┐    ┌─────────┐ │
     │  Data A │───▶│  Data B │───▶│  Data C │─┘
     │         │◀───│         │◀───│         │
     └─────────┘    └─────────┘    └─────────┘
          ↑
          │
     (entry point)
```

```c
/*
 * Circular Doubly Linked List
 */
typedef struct CNode {
    Part data;
    struct CNode *prev;
    struct CNode *next;
} CNode;

typedef struct {
    CNode *current;  /* Our entry point into the ring */
    size_t count;
} CircularList;

/*
 * Create an empty circular list
 */
CircularList *circular_create(void) {
    CircularList *list = malloc(sizeof(CircularList));
    if (!list) return NULL;

    list->current = NULL;
    list->count = 0;
    return list;
}

/*
 * Insert into the ring
 */
bool circular_insert(CircularList *list, const Part *part) {
    CNode *node = malloc(sizeof(CNode));
    if (!node) return false;

    node->data = *part;

    if (list->current == NULL) {
        /* First node points to itself */
        node->next = node;
        node->prev = node;
        list->current = node;
    } else {
        /* Insert after current */
        node->next = list->current->next;
        node->prev = list->current;
        list->current->next->prev = node;
        list->current->next = node;
    }

    list->count++;
    return true;
}

/*
 * Rotate the ring — move to next/prev
 */
void circular_rotate_forward(CircularList *list) {
    if (list->current) {
        list->current = list->current->next;
    }
}

void circular_rotate_backward(CircularList *list) {
    if (list->current) {
        list->current = list->current->prev;
    }
}

/*
 * Application: Round-Robin Scheduling
 * Each "part" represents a task to process
 */
void round_robin_schedule(CircularList *tasks, int time_slice) {
    if (!tasks->current) return;

    CNode *start = tasks->current;
    do {
        Part *task = &tasks->current->data;
        printf("Processing task %s for %d units\n",
               task->part_number, time_slice);
        /* Process the task... */

        circular_rotate_forward(tasks);
    } while (tasks->current != start);
}
```

---

## The Intrusive List — Armor Worn, Not Carried

Normal linked lists allocate nodes separately from data.
**Intrusive lists** embed the link pointers directly into
the data structure, like chain-mail worn rather than carried.

```c
/*
 * Intrusive List Node — embedded in the data itself
 */
typedef struct IntrusiveNode {
    struct IntrusiveNode *prev;
    struct IntrusiveNode *next;
} IntrusiveNode;

/*
 * Part with embedded list node
 */
typedef struct {
    char part_number[20];
    char description[100];
    int quantity;
    double price;

    IntrusiveNode list_node;  /* Embedded link! */
} IntrusivePart;

/*
 * Intrusive List Header
 */
typedef struct {
    IntrusiveNode head;  /* Sentinel */
    IntrusiveNode tail;  /* Sentinel */
    size_t count;
} IntrusiveList;

/*
 * Container_of macro — find the Part from the node
 * This is the key to intrusive data structures!
 */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/*
 * Initialize intrusive list with sentinels
 */
void intrusive_init(IntrusiveList *list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
    list->count = 0;
}

/*
 * Add a part to the list — no memory allocation!
 */
void intrusive_add(IntrusiveList *list, IntrusivePart *part) {
    IntrusiveNode *node = &part->list_node;
    IntrusiveNode *tail = list->tail.prev;

    node->prev = tail;
    node->next = &list->tail;
    tail->next = node;
    list->tail.prev = node;
    list->count++;
}

/*
 * Remove a part — no memory deallocation from list!
 */
void intrusive_remove(IntrusiveList *list, IntrusivePart *part) {
    IntrusiveNode *node = &part->list_node;
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->count--;
}

/*
 * Iterate over parts
 */
#define intrusive_foreach(pos, list) \
    for (pos = container_of((list)->head.next, IntrusivePart, list_node); \
         &pos->list_node != &(list)->tail; \
         pos = container_of(pos->list_node.next, IntrusivePart, list_node))

/*
 * Usage example
 */
void process_intrusive_parts(IntrusiveList *list) {
    IntrusivePart *part;
    intrusive_foreach(part, list) {
        printf("Part: %s - Qty: %d\n", part->part_number, part->quantity);
    }
}
```

### Benefits of Intrusive Lists

1. **No extra allocation** — Nodes live inside the data
2. **Better cache locality** — Data and links are together
3. **Knowing the part gives you the node** — O(1) removal if you have the item
4. **Used by Linux kernel** — The `list_head` in Linux is intrusive

---

## Skip Lists — Express Lanes in the Chain

*A preview of advanced structures — covered fully in Part III*

The Skip List is like a linked list with multiple layers,
allowing faster traversal at the cost of more pointers.

```
Skip List Visualization (searching for 50):

Level 3:  HEAD ─────────────────────────▶ 50 ────────────▶ NIL
                                          ↓
Level 2:  HEAD ───────▶ 25 ─────────────▶ 50 ───▶ 75 ───▶ NIL
                        ↓                 ↓       ↓
Level 1:  HEAD ──▶ 10 ─▶ 25 ──▶ 40 ────▶ 50 ──▶ 75 ─▶ 90 ▶ NIL

We can skip directly to 50 via Level 3!
Search complexity: O(log n) on average
```

---

## Scavenger Hunt — Clue #8

*"In Sedgewick's tome, Chapter 3 reveals a profound truth about
linked lists. He speaks of 'list heads' and 'dummy nodes.'
Find where he explains why keeping a dummy head node simplifies
the code. What operations become simpler?"*

🔍 **Your Quest**: Study Sedgewick's Algorithms in C, Chapter 3.
Find the discussion of dummy nodes and sentinels. Count the
special cases that disappear when using a sentinel.

---

## Scavenger Hunt — Clue #9

*"Bhargava speaks in Chapter 2 of linked lists with a tale
of adding elements. He shows an image of computers connected
like a chain. But he warns of a limitation. What takes longer
in a linked list than in an array?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 2.
Find the trade-off table comparing arrays and linked lists.
What is O(n) in linked lists that is O(1) in arrays?

---

## The Complexity Tablet of Linked Lists

```
╔═══════════════════════════════════════════════════════════════════════╗
║              THE MITHRIL TABLET OF LIST COMPLEXITIES                  ║
╠═══════════════════════════════════════════════════════════════════════╣
║  Operation              │ Singly    │ Doubly    │ Notes               ║
╠═════════════════════════╪═══════════╪═══════════╪═════════════════════╣
║  Access by index        │ O(n)      │ O(n)      │ Must traverse       ║
║  Search                 │ O(n)      │ O(n)      │ Must traverse       ║
║  Insert at head         │ O(1)      │ O(1)      │ Just adjust pointers║
║  Insert at tail         │ O(1)*     │ O(1)      │ *If tail pointer    ║
║  Insert after node      │ O(1)      │ O(1)      │ Given the node      ║
║  Insert before node     │ O(n)      │ O(1)      │ Doubly: has prev    ║
║  Remove head            │ O(1)      │ O(1)      │ Just adjust head    ║
║  Remove tail            │ O(n)      │ O(1)      │ Doubly: has prev    ║
║  Remove given node      │ O(n)      │ O(1)      │ Doubly: has prev    ║
║  Find predecessor       │ O(n)      │ O(1)      │ Doubly: has prev    ║
╠═════════════════════════╧═══════════╧═══════════╧═════════════════════╣
║  Space overhead: O(n) — One pointer per node (singly)                 ║
║                         Two pointers per node (doubly)                ║
║  Cache performance: Poor — Elements scattered in memory               ║
╚═══════════════════════════════════════════════════════════════════════╝
```

---

## Exercises — The Trials of the Chain

### Trial 1: The Supply Chain
```c
/*
 * Implement a singly linked list for tracking parts through
 * a manufacturing process. Each part moves from station to station.
 *
 * Requirements:
 * 1. Add parts to the end of the queue (O(1) with tail)
 * 2. Remove parts from the front (O(1))
 * 3. Find a part by part_number (O(n))
 * 4. Count total parts (O(1) with count)
 */
```

### Trial 2: The Undo History
```c
/*
 * Implement a doubly linked list for undo/redo functionality.
 *
 * Requirements:
 * 1. Each node stores an operation (add, delete, modify)
 * 2. Undo moves backward in the list
 * 3. Redo moves forward in the list
 * 4. A new action after undo truncates the redo history
 */
```

### Trial 3: The LRU Cache
```c
/*
 * Implement a Least Recently Used (LRU) cache using a doubly
 * linked list + hash map.
 *
 * Requirements:
 * 1. Fixed maximum capacity
 * 2. Get: Move accessed item to head (most recent)
 * 3. Put: Add to head, evict from tail if at capacity
 * 4. All operations should be O(1)
 */
```

---

*Thus ends the second chapter of the Valaquenta.
We have learned the ways of chain and mail,
flexible structures for the changing world.
In the next chapter, we ascend to the Trees,
hierarchical structures reaching toward the light.*

---

[Continue to Part II-C: Trees and the Two Trees of Valinor →](#part-ii-c-of-trees-and-the-two-trees-of-valinor)

---

# Part II-C: Of Trees and the Two Trees of Valinor

*In which we learn of the great Trees that grew in the light
of the Blessed Realm, their branches reaching ever upward,
each node a kingdom connected to its children.*

## The Philosophy of Hierarchy

In the days before the Sun and Moon, two great Trees lit the
Blessed Realm of Valinor: Telperion the Silver and Laurelin the Gold.
From a single root, each Tree rose and branched, growing ever more
complex yet remaining connected to its source.

So it is with the **Tree** data structure. From a single root,
nodes branch outward, each parent connected to its children,
forming a hierarchy of relationships. Trees model natural hierarchies:
file systems, organizational charts, and the very syntax of
programming languages.

```
The Tree of Telperion (File System):

                         [/]  (root)
                          │
            ┌─────────────┼─────────────┐
            │             │             │
          [usr]         [home]        [etc]
            │             │             │
       ┌────┴────┐    ┌───┴───┐    ┌───┴───┐
       │         │    │       │    │       │
    [bin]     [lib]  [gd]  [guest] [passwd] [hosts]
       │
   ┌───┴───┐
   │       │
 [gcc]   [make]
```

### Tree Terminology — The Language of the Forests

```
                    ┌───────────────────┐
                    │       ROOT        │  ← Level 0 (depth 0)
                    │   (the ancestor)  │
                    └─────────┬─────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
        ┌─────┴─────┐   ┌─────┴─────┐   ┌─────┴─────┐
        │  CHILD A  │   │  CHILD B  │   │  CHILD C  │  ← Level 1
        │ (internal)│   │ (internal)│   │  (leaf)   │
        └─────┬─────┘   └─────┬─────┘   └───────────┘
              │               │
        ┌─────┴─────┐   ┌─────┴─────┐
        │  LEAF     │   │  LEAF     │  ← Level 2 (leaves)
        │ (no child)│   │ (no child)│
        └───────────┘   └───────────┘

Terminology:
- Root:     The topmost node (no parent)
- Leaf:     A node with no children
- Internal: A node with at least one child
- Parent:   The node directly above
- Child:    A node directly below
- Sibling:  Nodes sharing the same parent
- Ancestor: Any node on the path to root
- Descendant: Any node in the subtree below
- Depth:    Distance from root (root = 0)
- Height:   Distance to deepest leaf
- Subtree:  A node and all its descendants
```

---

## The Binary Tree — The Simplest Branch

A **Binary Tree** is a tree where each node has at most two children:
a left child and a right child. It is the foundation of many
more complex structures.

```c
/*
 * Binary Tree Node
 */
typedef struct BTreeNode {
    Part data;
    struct BTreeNode *left;   /* Left child */
    struct BTreeNode *right;  /* Right child */
} BTreeNode;

/*
 * Create a new tree node
 */
BTreeNode *btree_create_node(const Part *part) {
    BTreeNode *node = malloc(sizeof(BTreeNode));
    if (!node) return NULL;

    node->data = *part;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/*
 * Destroy a tree recursively
 */
void btree_destroy(BTreeNode *root) {
    if (root == NULL) return;

    btree_destroy(root->left);   /* Destroy left subtree */
    btree_destroy(root->right);  /* Destroy right subtree */
    free(root);                   /* Destroy this node */
}
```

### Tree Traversals — Walking Through the Forest

There are three fundamental ways to visit every node in a binary tree,
each useful for different purposes:

```c
/*
 * In-Order Traversal: Left, Root, Right
 * For a BST, this visits nodes in sorted order!
 */
void btree_inorder(BTreeNode *root, PartVisitor visit, void *ctx) {
    if (root == NULL) return;

    btree_inorder(root->left, visit, ctx);   /* Visit left subtree */
    visit(&root->data, ctx);                  /* Visit this node */
    btree_inorder(root->right, visit, ctx);  /* Visit right subtree */
}

/*
 * Pre-Order Traversal: Root, Left, Right
 * Useful for copying trees or prefix expressions
 */
void btree_preorder(BTreeNode *root, PartVisitor visit, void *ctx) {
    if (root == NULL) return;

    visit(&root->data, ctx);                  /* Visit this node first */
    btree_preorder(root->left, visit, ctx);  /* Then left subtree */
    btree_preorder(root->right, visit, ctx); /* Then right subtree */
}

/*
 * Post-Order Traversal: Left, Right, Root
 * Useful for deleting trees or postfix expressions
 */
void btree_postorder(BTreeNode *root, PartVisitor visit, void *ctx) {
    if (root == NULL) return;

    btree_postorder(root->left, visit, ctx);  /* Visit left subtree */
    btree_postorder(root->right, visit, ctx); /* Visit right subtree */
    visit(&root->data, ctx);                   /* Visit this node last */
}

/*
 * Level-Order Traversal (Breadth-First)
 * Visit nodes level by level using a queue
 */
void btree_levelorder(BTreeNode *root, PartVisitor visit, void *ctx) {
    if (root == NULL) return;

    /* Use a simple queue (we'd use our ring buffer in practice) */
    BTreeNode *queue[1000];  /* Fixed size for simplicity */
    int front = 0, back = 0;

    queue[back++] = root;

    while (front < back) {
        BTreeNode *node = queue[front++];
        visit(&node->data, ctx);

        if (node->left) queue[back++] = node->left;
        if (node->right) queue[back++] = node->right;
    }
}

/*
 * Example: Print traversals for this tree:
 *
 *           50
 *          /  \
 *        30    70
 *       /  \     \
 *      20  40    80
 *
 * In-Order:    20, 30, 40, 50, 70, 80  (sorted!)
 * Pre-Order:   50, 30, 20, 40, 70, 80  (root first)
 * Post-Order:  20, 40, 30, 80, 70, 50  (leaves first)
 * Level-Order: 50, 30, 70, 20, 40, 80  (breadth-first)
 */
```

### Iterative Traversal with a Stack

Sometimes recursion isn't ideal (deep trees can overflow the call stack).
Here's an iterative in-order traversal:

```c
/*
 * Iterative In-Order Traversal using explicit stack
 */
void btree_inorder_iterative(BTreeNode *root, PartVisitor visit, void *ctx) {
    BTreeNode *stack[1000];  /* Explicit stack */
    int top = -1;

    BTreeNode *current = root;

    while (current != NULL || top >= 0) {
        /* Go as far left as possible */
        while (current != NULL) {
            stack[++top] = current;
            current = current->left;
        }

        /* Pop and visit */
        current = stack[top--];
        visit(&current->data, ctx);

        /* Go right */
        current = current->right;
    }
}
```

---

## The Binary Search Tree — The Ordered Realm

A **Binary Search Tree (BST)** is a binary tree with a special property:
for every node, all values in the left subtree are smaller, and all
values in the right subtree are larger. This enables O(log n) search
when the tree is balanced.

```
The Ordered Tree (BST Property):

              ┌──────────────────────┐
              │   All values < 50    │
              │                      │
              └───────────┬──────────┘
                          │
                    ┌─────┴─────┐
                    │    50     │  ← Root
                    └─────┬─────┘
                          │
    ┌─────────────────────┼─────────────────────┐
    │                                           │
    ▼                                           ▼
┌──────┐                                   ┌──────┐
│  30  │ ← Left subtree (all < 50)         │  70  │ ← Right subtree (all > 50)
└──┬───┘                                   └───┬──┘
   │                                           │
   ├─────────────┐                     ┌───────┘
   │             │                     │
   ▼             ▼                     ▼
┌──────┐     ┌──────┐             ┌──────┐
│  20  │     │  40  │             │  80  │
└──────┘     └──────┘             └──────┘
```

```c
/*
 * BST Operations
 */

/* Comparator for parts (by part_number) */
typedef int (*PartComparator)(const Part *a, const Part *b);

int compare_by_part_number(const Part *a, const Part *b) {
    return strcmp(a->part_number, b->part_number);
}

/*
 * Search — O(log n) average, O(n) worst case
 */
BTreeNode *bst_search(BTreeNode *root, const Part *target,
                      PartComparator cmp) {
    while (root != NULL) {
        int result = cmp(target, &root->data);

        if (result == 0) {
            return root;         /* Found! */
        } else if (result < 0) {
            root = root->left;   /* Go left */
        } else {
            root = root->right;  /* Go right */
        }
    }
    return NULL;  /* Not found */
}

/*
 * Insert — O(log n) average, O(n) worst case
 */
BTreeNode *bst_insert(BTreeNode *root, const Part *part,
                      PartComparator cmp) {
    if (root == NULL) {
        return btree_create_node(part);  /* Create new leaf */
    }

    int result = cmp(part, &root->data);

    if (result < 0) {
        root->left = bst_insert(root->left, part, cmp);
    } else if (result > 0) {
        root->right = bst_insert(root->right, part, cmp);
    }
    /* If result == 0, part already exists — do nothing */

    return root;
}

/*
 * Find minimum — O(log n) average
 * The leftmost node
 */
BTreeNode *bst_find_min(BTreeNode *root) {
    if (root == NULL) return NULL;

    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}

/*
 * Find maximum — O(log n) average
 * The rightmost node
 */
BTreeNode *bst_find_max(BTreeNode *root) {
    if (root == NULL) return NULL;

    while (root->right != NULL) {
        root = root->right;
    }
    return root;
}

/*
 * Delete — O(log n) average, O(n) worst case
 * The most complex BST operation
 */
BTreeNode *bst_delete(BTreeNode *root, const Part *target,
                      PartComparator cmp) {
    if (root == NULL) return NULL;

    int result = cmp(target, &root->data);

    if (result < 0) {
        /* Target is in left subtree */
        root->left = bst_delete(root->left, target, cmp);
    } else if (result > 0) {
        /* Target is in right subtree */
        root->right = bst_delete(root->right, target, cmp);
    } else {
        /* Found the node to delete */

        /* Case 1: No children (leaf) */
        if (root->left == NULL && root->right == NULL) {
            free(root);
            return NULL;
        }

        /* Case 2: One child */
        if (root->left == NULL) {
            BTreeNode *right = root->right;
            free(root);
            return right;
        }
        if (root->right == NULL) {
            BTreeNode *left = root->left;
            free(root);
            return left;
        }

        /* Case 3: Two children */
        /* Replace with in-order successor (min of right subtree) */
        BTreeNode *successor = bst_find_min(root->right);
        root->data = successor->data;
        root->right = bst_delete(root->right, &successor->data, cmp);
    }

    return root;
}
```

### The Problem of Balance

A BST's performance depends on its shape. If we insert elements
in sorted order, the tree degenerates into a linked list:

```
Balanced Tree (O(log n)):    Degenerate Tree (O(n)):

        50                    10
       /  \                     \
      30   70                   20
     / \   / \                    \
   20  40 60  80                  30
                                    \
                                    40
                                      \
                                      50

   Height: log₂(7) ≈ 3            Height: 5 (n-1)
```

This is why we need **self-balancing trees** like AVL and Red-Black trees,
which automatically maintain balance after insertions and deletions.

---

## The AVL Tree — The Perfectly Balanced Realm

Named after its inventors Adelson-Velsky and Landis, the **AVL Tree**
is a BST that maintains strict balance: for every node, the heights
of its left and right subtrees differ by at most 1.

```c
/*
 * AVL Tree Node — includes height for balance calculation
 */
typedef struct AVLNode {
    Part data;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;  /* Height of this node's subtree */
} AVLNode;

/*
 * Get height (NULL nodes have height -1 or 0 depending on convention)
 */
static int avl_height(AVLNode *node) {
    return node ? node->height : -1;
}

/*
 * Update height based on children
 */
static void avl_update_height(AVLNode *node) {
    int left_h = avl_height(node->left);
    int right_h = avl_height(node->right);
    node->height = 1 + (left_h > right_h ? left_h : right_h);
}

/*
 * Balance factor: height(left) - height(right)
 * Should be in range [-1, 0, 1] for balanced
 */
static int avl_balance_factor(AVLNode *node) {
    if (!node) return 0;
    return avl_height(node->left) - avl_height(node->right);
}

/*
 * Right Rotation — fixes left-heavy imbalance
 *
 *        Y                X
 *       / \              / \
 *      X   C    →       A   Y
 *     / \                  / \
 *    A   B                B   C
 */
static AVLNode *avl_rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *b = x->right;

    x->right = y;
    y->left = b;

    avl_update_height(y);
    avl_update_height(x);

    return x;  /* New root */
}

/*
 * Left Rotation — fixes right-heavy imbalance
 *
 *      X                  Y
 *     / \                / \
 *    A   Y      →       X   C
 *       / \            / \
 *      B   C          A   B
 */
static AVLNode *avl_rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *b = y->left;

    y->left = x;
    x->right = b;

    avl_update_height(x);
    avl_update_height(y);

    return y;  /* New root */
}

/*
 * Rebalance after insertion or deletion
 */
static AVLNode *avl_rebalance(AVLNode *node) {
    avl_update_height(node);

    int balance = avl_balance_factor(node);

    /* Left-heavy (balance > 1) */
    if (balance > 1) {
        if (avl_balance_factor(node->left) < 0) {
            /* Left-Right case: rotate left child left first */
            node->left = avl_rotate_left(node->left);
        }
        /* Left-Left case (or after Left-Right conversion) */
        return avl_rotate_right(node);
    }

    /* Right-heavy (balance < -1) */
    if (balance < -1) {
        if (avl_balance_factor(node->right) > 0) {
            /* Right-Left case: rotate right child right first */
            node->right = avl_rotate_right(node->right);
        }
        /* Right-Right case (or after Right-Left conversion) */
        return avl_rotate_left(node);
    }

    /* Already balanced */
    return node;
}

/*
 * Insert with rebalancing — O(log n) guaranteed
 */
AVLNode *avl_insert(AVLNode *root, const Part *part, PartComparator cmp) {
    if (root == NULL) {
        AVLNode *node = malloc(sizeof(AVLNode));
        node->data = *part;
        node->left = node->right = NULL;
        node->height = 0;
        return node;
    }

    int result = cmp(part, &root->data);

    if (result < 0) {
        root->left = avl_insert(root->left, part, cmp);
    } else if (result > 0) {
        root->right = avl_insert(root->right, part, cmp);
    } else {
        return root;  /* Duplicate */
    }

    return avl_rebalance(root);
}
```

---

## The Red-Black Tree — The Chromatic Kingdom

The **Red-Black Tree** is another self-balancing BST, but with
a different strategy: instead of height, it uses colors (red and black)
to maintain approximate balance. It's used in many standard libraries
(C++ `std::map`, Java `TreeMap`).

```
Red-Black Tree Rules:
1. Every node is either red or black
2. The root is always black
3. Every leaf (NULL) is black
4. Red nodes cannot have red children (no red-red)
5. All paths from any node to its descendant NULLs
   have the same number of black nodes

These rules ensure: height ≤ 2 × log₂(n + 1)
```

```c
/*
 * Red-Black Tree Node
 */
typedef enum { RED, BLACK } RBColor;

typedef struct RBNode {
    Part data;
    struct RBNode *left;
    struct RBNode *right;
    struct RBNode *parent;  /* RB trees typically track parent */
    RBColor color;
} RBNode;

/*
 * Color utilities
 */
static RBColor rb_color(RBNode *node) {
    return (node == NULL) ? BLACK : node->color;
}

static bool rb_is_red(RBNode *node) {
    return node != NULL && node->color == RED;
}

/*
 * Left Rotation (similar to AVL but with parent pointers)
 */
static RBNode *rb_rotate_left(RBNode *root, RBNode *x) {
    RBNode *y = x->right;
    x->right = y->left;

    if (y->left != NULL) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == NULL) {
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;

    return root;
}

/* Right rotation is symmetric... */

/*
 * Fix violations after insertion
 * (The full implementation is quite involved)
 */
static RBNode *rb_insert_fixup(RBNode *root, RBNode *z) {
    while (rb_is_red(z->parent)) {
        if (z->parent == z->parent->parent->left) {
            RBNode *uncle = z->parent->parent->right;

            if (rb_is_red(uncle)) {
                /* Case 1: Uncle is red — recolor */
                z->parent->color = BLACK;
                uncle->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* Case 2: z is right child — rotate to case 3 */
                    z = z->parent;
                    root = rb_rotate_left(root, z);
                }
                /* Case 3: z is left child — rotate and recolor */
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                root = rb_rotate_right(root, z->parent->parent);
            }
        } else {
            /* Symmetric cases for right child of grandparent */
            /* ... */
        }
    }

    root->color = BLACK;  /* Root is always black */
    return root;
}
```

---

## N-ary Trees — Many Branches from Each Node

Not all trees are binary. **N-ary trees** allow any number of children.

```c
/*
 * N-ary Tree using child-sibling representation
 * Each node has a pointer to its first child and next sibling
 *
 *           [A]                    [A]
 *          / | \                    |
 *        [B][C][D]      →         [B]→[C]→[D]
 *        / \                        |
 *      [E] [F]                    [E]→[F]
 *
 * Left-child, right-sibling representation
 */

typedef struct NaryNode {
    Part data;
    struct NaryNode *first_child;  /* First child */
    struct NaryNode *next_sibling; /* Next sibling */
} NaryNode;

/*
 * Add a child to a parent
 */
void nary_add_child(NaryNode *parent, NaryNode *child) {
    child->next_sibling = parent->first_child;
    parent->first_child = child;
}

/*
 * Traverse all children of a node
 */
void nary_foreach_child(NaryNode *parent, PartVisitor visit, void *ctx) {
    for (NaryNode *child = parent->first_child;
         child != NULL;
         child = child->next_sibling) {
        visit(&child->data, ctx);
    }
}

/*
 * Recursive depth-first traversal of entire tree
 */
void nary_traverse(NaryNode *node, PartVisitor visit, void *ctx) {
    if (node == NULL) return;

    visit(&node->data, ctx);

    /* Visit all children */
    for (NaryNode *child = node->first_child;
         child != NULL;
         child = child->next_sibling) {
        nary_traverse(child, visit, ctx);
    }
}
```

---

## The Trie — The Tree of Words

A **Trie** (from "retrieval") is a tree for storing strings where
each edge represents a character. It's perfect for dictionaries,
autocomplete, and prefix matching.

```
Trie storing: "BOLT", "BOLTS", "BOX", "NUT"

                    [root]
                    /    \
                  'B'    'N'
                   |      |
                  [.]    [.]
                 / \      |
               'O' 'U'   'U'
                |   |     |
               [.]  |    [.]
              / \   |     |
            'L' 'X' |    'T'
             |   |  |     |
            [.] [✓] |    [✓]
             |      |
            'T'    'T'
             |      |
            [✓]    [✓]   (✓ = end of word)
             |
            'S'
             |
            [✓]
```

```c
/*
 * Trie Node — one node per character in alphabet
 */
#define ALPHABET_SIZE 36  /* A-Z, 0-9 */

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    bool is_end_of_word;
    Part *data;  /* Associated data (e.g., the part) */
} TrieNode;

/* Map character to index */
static int char_to_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    return -1;  /* Invalid character */
}

/*
 * Create a new trie node
 */
TrieNode *trie_create_node(void) {
    TrieNode *node = calloc(1, sizeof(TrieNode));
    /* calloc zeros all pointers and is_end_of_word */
    return node;
}

/*
 * Insert a part into the trie (keyed by part_number)
 */
bool trie_insert(TrieNode *root, Part *part) {
    TrieNode *current = root;

    for (const char *p = part->part_number; *p; p++) {
        int index = char_to_index(*p);
        if (index < 0) continue;  /* Skip invalid chars */

        if (current->children[index] == NULL) {
            current->children[index] = trie_create_node();
            if (!current->children[index]) return false;
        }
        current = current->children[index];
    }

    current->is_end_of_word = true;
    current->data = part;
    return true;
}

/*
 * Search for a part by exact part_number — O(m) where m = key length
 */
Part *trie_search(TrieNode *root, const char *part_number) {
    TrieNode *current = root;

    for (const char *p = part_number; *p; p++) {
        int index = char_to_index(*p);
        if (index < 0) continue;

        if (current->children[index] == NULL) {
            return NULL;  /* Not found */
        }
        current = current->children[index];
    }

    if (current->is_end_of_word) {
        return current->data;
    }
    return NULL;
}

/*
 * Find all parts with a given prefix
 */
static void trie_collect(TrieNode *node, Part **results,
                         int *count, int max_results) {
    if (node == NULL || *count >= max_results) return;

    if (node->is_end_of_word) {
        results[(*count)++] = node->data;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        trie_collect(node->children[i], results, count, max_results);
    }
}

int trie_prefix_search(TrieNode *root, const char *prefix,
                       Part **results, int max_results) {
    /* Navigate to prefix endpoint */
    TrieNode *current = root;
    for (const char *p = prefix; *p; p++) {
        int index = char_to_index(*p);
        if (index < 0) continue;

        if (current->children[index] == NULL) {
            return 0;  /* No matches */
        }
        current = current->children[index];
    }

    /* Collect all words from this point */
    int count = 0;
    trie_collect(current, results, &count, max_results);
    return count;
}
```

---

## Scavenger Hunt — Clue #10

*"In Sedgewick's Chapter 12, he speaks of Binary Search Trees.
There he reveals the expected height of a randomly built BST.
What is this expected height in terms of n, and why does
insertion order matter so much?"*

🔍 **Your Quest**: Study Sedgewick's Algorithms in C, Chapter 12.
Find the analysis of random BSTs. What theorem relates height
to the natural logarithm?

---

## Scavenger Hunt — Clue #11

*"Bhargava, in Chapter 11, speaks of trees in the context
of Breadth-First Search. He uses the metaphor of 'mango sellers'
and levels of friendship. How many levels until you find a mango seller,
and why does BFS find the shortest path?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 6 (BFS).
Find the mango seller example. Why must BFS use a queue, not a stack?

---

## The Complexity Tablet of Trees

```
╔══════════════════════════════════════════════════════════════════════════╗
║             THE TWO TREES TABLET OF TREE COMPLEXITIES                    ║
╠══════════════════════════════════════════════════════════════════════════╣
║  Operation        │ BST Avg │ BST Worst │ AVL/RB  │ Notes                ║
╠═══════════════════╪═════════╪═══════════╪═════════╪══════════════════════╣
║  Search           │ O(log n)│ O(n)      │ O(log n)│ Balanced: guaranteed ║
║  Insert           │ O(log n)│ O(n)      │ O(log n)│ Plus rotations       ║
║  Delete           │ O(log n)│ O(n)      │ O(log n)│ Most complex         ║
║  Min/Max          │ O(log n)│ O(n)      │ O(log n)│ Go left/right        ║
║  In-order traverse│ O(n)    │ O(n)      │ O(n)    │ Visit all nodes      ║
╠═══════════════════╧═════════╧═══════════╧═════════╧══════════════════════╣
║  Space: O(n) for all tree types                                          ║
║  Balance matters: Random BST ≈ O(log n), Sorted input → O(n)             ║
╠══════════════════════════════════════════════════════════════════════════╣
║  Trie Operations                                                         ║
╠══════════════════════════════════════════════════════════════════════════╣
║  Search           │ O(m)    │ m = key length, independent of n!          ║
║  Insert           │ O(m)    │ Follow/create path for each char           ║
║  Prefix search    │ O(m+k)  │ k = number of matches                      ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Exercises — The Trials of the Two Trees

### Trial 1: The Parts Catalog BST
```c
/*
 * Build a BST for the parts inventory.
 *
 * Requirements:
 * 1. Insert parts ordered by part_number
 * 2. Search for a part by part_number
 * 3. Print all parts in sorted order (in-order traversal)
 * 4. Find parts in a range (e.g., BOLT-001 to BOLT-099)
 */
```

### Trial 2: The Autocomplete System
```c
/*
 * Build a Trie for part number autocomplete.
 *
 * Requirements:
 * 1. Insert all parts into the trie
 * 2. Given a prefix, return all matching parts
 * 3. Track insertion count for popularity ranking
 * 4. Support deletion of discontinued parts
 */
```

### Trial 3: The Expression Parser
```c
/*
 * Build an expression tree for parsing equations.
 *
 * Example: "3 + 4 * 2" becomes:
 *
 *          [+]
 *         /   \
 *       [3]   [*]
 *            /   \
 *          [4]   [2]
 *
 * Requirements:
 * 1. Parse infix expression to tree
 * 2. Evaluate the tree recursively
 * 3. Print in prefix notation (Polish)
 * 4. Print in postfix notation (Reverse Polish)
 */
```

---

*Thus ends the Valaquenta of Data Structures.
We have learned of stone halls, mithril chains,
and the great Trees that organize our knowledge.
In the next part, we shall descend into Khazad-dûm
to learn the sorting arts of the Dwarven miners.*

---

[Continue to Part III: Khazadgarmâ — The Sorting Arts →](./THE_SILMARILLION_OF_ALGORITHMS_PART3.md)

---

