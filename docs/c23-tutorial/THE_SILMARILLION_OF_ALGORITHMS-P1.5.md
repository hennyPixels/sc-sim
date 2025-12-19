
## Part I-B: The First Algorithms

### Chapter 6: The Forging of the Silmarils — Classic Algorithms

Just as Fëanor crafted the Silmarils—the greatest works of the Elves—so too did the ancient mathematicians and computer scientists forge the classic algorithms that form the foundation of all computation.

```c
/*
 * THE CLASSIC ALGORITHMS
 *
 * These are the "Silmarils" of computer science—
 * timeless creations that capture fundamental truths
 * about computation.
 *
 * From Sedgewick's "Algorithms in C", Part 1:
 *   "These basic algorithms have been studied extensively
 *    and are well understood."
 */

/*
 * ALGORITHM 1: LINEAR SEARCH (The Prospector's Method)
 *
 * The simplest search: examine each item in turn.
 * Like a Dwarf prospector checking each stone for ore.
 */
int linear_search(Part **parts, size_t count, const char *part_number) {
    /*
     * The Prospector's Song:
     *   "Stone by stone and seam by seam,
     *    I search the mountain for my dream.
     *    Each rock I turn, each vein I trace,
     *    Until at last I find the place."
     */
    for (size_t i = 0; i < count; i++) {
        if (strcmp(parts[i]->part_number, part_number) == 0) {
            return (int)i;  /* Found! Return the index */
        }
    }
    return -1;  /* Not found */
}

/*
 * ALGORITHM 2: SWAP (The Exchange)
 *
 * The most fundamental operation: exchange two values.
 * Like Dwarven traders exchanging goods.
 */
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* Pointer version for generic swapping */
void swap_ptr(void **a, void **b) {
    void *temp = *a;
    *a = *b;
    *b = temp;
}

/* XOR swap (no temporary variable needed!) */
void swap_xor(int *a, int *b) {
    /*
     * Dwarven magic! Exchange without a third hand.
     * But beware: fails if a and b point to the same location.
     */
    if (a != b) {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }
}

/*
 * ALGORITHM 3: REVERSE (The Mirror of Galadriel)
 *
 * Reverse an array in place.
 */
void reverse_array(int *arr, size_t n) {
    /*
     * Like viewing the past in Galadriel's mirror—
     * what was first becomes last.
     */
    for (size_t i = 0; i < n / 2; i++) {
        swap(&arr[i], &arr[n - 1 - i]);
    }
}

/*
 * ALGORITHM 4: ROTATE (The Turning of the Seasons)
 *
 * Rotate array elements by k positions.
 */
void rotate_left(int *arr, size_t n, size_t k) {
    /*
     * Like the turning of seasons in Middle-earth—
     * what was at the beginning moves to a new position.
     *
     * Using the "reversal algorithm" (elegant!):
     *   1. Reverse first k elements
     *   2. Reverse remaining elements
     *   3. Reverse entire array
     */
    k = k % n;  /* Handle k > n */
    if (k == 0) return;

    reverse_array(arr, k);
    reverse_array(arr + k, n - k);
    reverse_array(arr, n);
}

/*
 * EXAMPLE TRACE:
 *
 * Original: [1, 2, 3, 4, 5], rotate left by 2
 *
 * Step 1: Reverse first 2: [2, 1, 3, 4, 5]
 * Step 2: Reverse rest:    [2, 1, 5, 4, 3]
 * Step 3: Reverse all:     [3, 4, 5, 1, 2]
 *
 * Result: Elements shifted left by 2 positions!
 */
```

---

### Chapter 7: The Noldor's Craft — Recursive Thinking

The Noldor were the greatest craftsmen among the Elves. Their secret? They understood that complex problems could be broken into simpler parts. This is the essence of recursion.

```c
/*
 * RECURSION: THE NOLDORIAN METHOD
 *
 * From Grokking Algorithms, Chapter 3:
 *   "Recursion is when a function calls itself."
 *
 * But it's more than that—it's a way of THINKING.
 *
 * The two keys to recursion:
 *   1. BASE CASE: When to stop (the simplest problem)
 *   2. RECURSIVE CASE: How to break down the problem
 */

/*
 * FACTORIAL: The Counting of Days
 *
 * How many ways can n Dwarves be arranged in a line?
 * Answer: n! (n factorial)
 */

/* Recursive version */
unsigned long factorial_recursive(unsigned int n) {
    /*
     * BASE CASE: 0! = 1 and 1! = 1
     * (One Dwarf can be arranged in exactly one way)
     */
    if (n <= 1) {
        return 1;
    }

    /*
     * RECURSIVE CASE: n! = n × (n-1)!
     * (Arrange n-1 Dwarves, then insert the nth in any of n positions)
     */
    return n * factorial_recursive(n - 1);
}

/* Iterative version (for comparison) */
unsigned long factorial_iterative(unsigned int n) {
    unsigned long result = 1;
    for (unsigned int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

/*
 * TRACE OF RECURSIVE FACTORIAL:
 *
 * factorial(5)
 *   → 5 * factorial(4)
 *       → 4 * factorial(3)
 *           → 3 * factorial(2)
 *               → 2 * factorial(1)
 *                   → 1 (base case!)
 *               → 2 * 1 = 2
 *           → 3 * 2 = 6
 *       → 4 * 6 = 24
 *   → 5 * 24 = 120
 *
 * Like a stack of Dwarven mining carts—
 * each waits for the one below to return!
 */

/*
 * FIBONACCI: The Growth of the Mountain-Halls
 *
 * Each new hall requires the combined resources of
 * the previous two halls.
 */

/* Naive recursive (inefficient!) */
unsigned long fib_naive(unsigned int n) {
    if (n <= 1) return n;
    return fib_naive(n - 1) + fib_naive(n - 2);
}

/* Memoized recursive (efficient!) */
unsigned long fib_memo(unsigned int n, unsigned long *cache) {
    if (n <= 1) return n;

    if (cache[n] != 0) {
        return cache[n];  /* Already computed! */
    }

    cache[n] = fib_memo(n - 1, cache) + fib_memo(n - 2, cache);
    return cache[n];
}

/* Iterative (most efficient for simple cases) */
unsigned long fib_iterative(unsigned int n) {
    if (n <= 1) return n;

    unsigned long prev2 = 0;
    unsigned long prev1 = 1;
    unsigned long current;

    for (unsigned int i = 2; i <= n; i++) {
        current = prev1 + prev2;
        prev2 = prev1;
        prev1 = current;
    }

    return current;
}

/*
 * WHY MEMOIZATION MATTERS
 *
 * fib_naive(40) makes over 300 million function calls!
 * fib_memo(40) makes only 39 function calls.
 *
 * As the Dwarves say:
 *   "A wise miner does not dig the same tunnel twice."
 */

/*
 * SCAVENGER HUNT CLUE #3:
 * ════════════════════════════════════════════════════════════════
 * In Grokking Algorithms, Chapter 3, there is a famous story
 * about a farmer counting boxes.
 *
 * What TWO items are inside the boxes that the farmer finds?
 *
 * ANSWER 1: _______
 * ANSWER 2: _______
 *
 * How does this relate to the base case and recursive case?
 * ════════════════════════════════════════════════════════════════
 */
```

---

### Chapter 8: The Rings of Power — Divide and Conquer

The greatest of the Rings were forged using the technique of division—separating the powers and binding them into smaller, manageable forms. This is the Divide and Conquer paradigm.

```c
/*
 * DIVIDE AND CONQUER
 *
 * From Sedgewick, Part 3:
 *   "Divide-and-conquer algorithms break up a problem
 *    into smaller subproblems, solve the subproblems,
 *    and combine the solutions."
 *
 * The Three Steps:
 *   1. DIVIDE:  Split the problem into smaller subproblems
 *   2. CONQUER: Solve each subproblem (recursively)
 *   3. COMBINE: Merge the solutions together
 */

/*
 * MERGE SORT: The Assembly of Armies
 *
 * To organize a great army, first organize smaller units,
 * then combine them in proper order.
 */

void merge(Part **arr, Part **temp, size_t left, size_t mid, size_t right) {
    /*
     * COMBINE STEP: Merge two sorted halves into one sorted whole
     *
     * Like merging two lines of Dwarven warriors,
     * each already in order by height,
     * into a single ordered line.
     */
    size_t i = left;      /* Index for left half */
    size_t j = mid + 1;   /* Index for right half */
    size_t k = left;      /* Index for merged result */

    while (i <= mid && j <= right) {
        if (strcmp(arr[i]->part_number, arr[j]->part_number) <= 0) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    /* Copy remaining elements from left half */
    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    /* Copy remaining elements from right half */
    while (j <= right) {
        temp[k++] = arr[j++];
    }

    /* Copy merged result back to original array */
    for (size_t idx = left; idx <= right; idx++) {
        arr[idx] = temp[idx];
    }
}

void merge_sort_recursive(Part **arr, Part **temp, size_t left, size_t right) {
    if (left >= right) {
        return;  /* BASE CASE: Single element is already sorted */
    }

    size_t mid = left + (right - left) / 2;

    /* DIVIDE: Split into two halves */
    merge_sort_recursive(arr, temp, left, mid);      /* Sort left half */
    merge_sort_recursive(arr, temp, mid + 1, right); /* Sort right half */

    /* COMBINE: Merge the sorted halves */
    merge(arr, temp, left, mid, right);
}

void merge_sort(Part **arr, size_t n) {
    if (n <= 1) return;

    Part **temp = malloc(n * sizeof(Part *));
    if (temp == NULL) return;

    merge_sort_recursive(arr, temp, 0, n - 1);

    free(temp);
}

/*
 * VISUALIZATION OF MERGE SORT:
 *
 * Original:     [D, B, A, C, F, E]
 *
 * DIVIDE:       [D, B, A]          [C, F, E]
 *               [D, B]  [A]        [C, F]  [E]
 *               [D] [B]            [C] [F]
 *
 * CONQUER:      (single elements are sorted)
 *
 * COMBINE:      [B, D]  [A]        [C, F]  [E]
 *               [A, B, D]          [C, E, F]
 *               [A, B, C, D, E, F]
 *
 * Each level does O(n) work, and there are O(log n) levels.
 * Total: O(n log n)
 */

/*
 * BINARY SEARCH (Divide and Conquer Version)
 *
 * We saw this earlier, but here's the recursive form:
 */
int binary_search_recursive(Part **parts, const char *target,
                            int low, int high) {
    /* BASE CASE: Search space is empty */
    if (low > high) {
        return -1;
    }

    int mid = low + (high - low) / 2;
    int cmp = strcmp(parts[mid]->part_number, target);

    if (cmp == 0) {
        return mid;  /* Found! */
    } else if (cmp > 0) {
        /* DIVIDE: Search left half */
        return binary_search_recursive(parts, target, low, mid - 1);
    } else {
        /* DIVIDE: Search right half */
        return binary_search_recursive(parts, target, mid + 1, high);
    }
}

/*
 * MAXIMUM SUBARRAY: The Richest Vein
 *
 * Find the contiguous subarray with the largest sum.
 * Like finding the richest vein of ore in a mountain.
 */
typedef struct {
    int left;
    int right;
    int sum;
} SubarrayResult;

SubarrayResult max_crossing_subarray(int *arr, int low, int mid, int high) {
    /* Find max subarray crossing the midpoint */
    int left_sum = INT_MIN;
    int sum = 0;
    int max_left = mid;

    for (int i = mid; i >= low; i--) {
        sum += arr[i];
        if (sum > left_sum) {
            left_sum = sum;
            max_left = i;
        }
    }

    int right_sum = INT_MIN;
    sum = 0;
    int max_right = mid + 1;

    for (int i = mid + 1; i <= high; i++) {
        sum += arr[i];
        if (sum > right_sum) {
            right_sum = sum;
            max_right = i;
        }
    }

    return (SubarrayResult){max_left, max_right, left_sum + right_sum};
}

SubarrayResult max_subarray(int *arr, int low, int high) {
    /* BASE CASE: Single element */
    if (low == high) {
        return (SubarrayResult){low, high, arr[low]};
    }

    int mid = low + (high - low) / 2;

    /* DIVIDE: Check left, right, and crossing */
    SubarrayResult left = max_subarray(arr, low, mid);
    SubarrayResult right = max_subarray(arr, mid + 1, high);
    SubarrayResult cross = max_crossing_subarray(arr, low, mid, high);

    /* COMBINE: Return the maximum of the three */
    if (left.sum >= right.sum && left.sum >= cross.sum) {
        return left;
    } else if (right.sum >= left.sum && right.sum >= cross.sum) {
        return right;
    } else {
        return cross;
    }
}
```

---

### Chapter 9: The Palantíri — Understanding Pointers

The Palantíri were seeing-stones that could show distant places. In C, pointers serve a similar function—they "see" memory locations elsewhere.

```c
/*
 * POINTERS: THE SEEING-STONES OF C
 *
 * A pointer does not contain data directly.
 * It contains the ADDRESS of data—a way to find it.
 *
 * Like a Palantír showing a distant location,
 * a pointer lets you access memory elsewhere.
 */

/*
 * THE THREE POINTER OPERATIONS
 */

void pointer_fundamentals(void) {
    int treasure = 100;        /* The actual gold */
    int *map = &treasure;      /* Map showing where gold is */

    /*
     * OPERATION 1: Address-of (&)
     * "Where is this treasure?"
     */
    printf("Treasure is at address: %p\n", (void *)&treasure);

    /*
     * OPERATION 2: Dereference (*)
     * "What is at this location?"
     */
    printf("Treasure value: %d\n", *map);

    /*
     * OPERATION 3: Assignment
     * "Put new treasure here"
     */
    *map = 200;  /* Change the treasure through the map */
    printf("New treasure value: %d\n", treasure);  /* Prints 200 */
}

/*
 * POINTER ARITHMETIC: Navigating the Mines
 *
 * When you add to a pointer, you move by the SIZE of what it points to.
 */

void pointer_arithmetic(void) {
    int chambers[5] = {10, 20, 30, 40, 50};  /* Five mine chambers */
    int *explorer = chambers;                 /* Start at first chamber */

    /*
     * Chamber layout in memory:
     *
     * Address:  0x1000  0x1004  0x1008  0x100C  0x1010
     *           ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
     *           │  10 │ │  20 │ │  30 │ │  40 │ │  50 │
     *           └─────┘ └─────┘ └─────┘ └─────┘ └─────┘
     * Index:       0       1       2       3       4
     */

    printf("Chamber 0: %d\n", *explorer);       /* 10 */
    printf("Chamber 2: %d\n", *(explorer + 2)); /* 30 */
    printf("Chamber 4: %d\n", explorer[4]);     /* 50 (same as *(explorer+4)) */

    /* Move the explorer forward */
    explorer++;
    printf("Now at Chamber 1: %d\n", *explorer); /* 20 */
}

/*
 * DOUBLE POINTERS: Maps to Maps
 *
 * Sometimes you need a map that shows where other maps are.
 * This is essential for dynamic arrays of strings, for example.
 */

void double_pointer_example(void) {
    /*
     * The Library of Khazad-dûm contains books.
     * Each shelf contains pointers to books.
     * The library catalog is a pointer to shelves.
     */

    char *shelf1[] = {"Mining", "Smithing", "Masonry"};
    char *shelf2[] = {"History", "Legends", "Songs"};

    char **library[2] = {shelf1, shelf2};

    /* Access "Smithing" (shelf 0, book 1) */
    printf("Book: %s\n", library[0][1]);  /* Smithing */

    /* How it works:
     * library[0]    → shelf1 (pointer to first shelf)
     * library[0][1] → shelf1[1] → "Smithing"
     */
}

/*
 * FUNCTION POINTERS: The Command Runes
 *
 * A function pointer holds the address of executable code.
 * Like a Dwarven command rune that invokes a specific action.
 */

typedef int (*Comparator)(const void *, const void *);

int compare_by_quantity(const void *a, const void *b) {
    const Part *pa = *(const Part **)a;
    const Part *pb = *(const Part **)b;
    return pa->quantity - pb->quantity;
}

int compare_by_name(const void *a, const void *b) {
    const Part *pa = *(const Part **)a;
    const Part *pb = *(const Part **)b;
    return strcmp(pa->name, pb->name);
}

void sort_parts(Part **parts, size_t count, Comparator cmp) {
    /*
     * The sorting algorithm doesn't need to know HOW to compare—
     * it just calls the provided comparison function.
     */
    qsort(parts, count, sizeof(Part *), cmp);
}

void demonstrate_function_pointers(Part **parts, size_t count) {
    /* Sort by quantity */
    sort_parts(parts, count, compare_by_quantity);
    printf("Sorted by quantity.\n");

    /* Sort by name */
    sort_parts(parts, count, compare_by_name);
    printf("Sorted by name.\n");
}

/*
 * SCAVENGER HUNT CLUE #4:
 * ════════════════════════════════════════════════════════════════
 * In Sedgewick's "Algorithms in C", there is a discussion of
 * "handles" as a way to manage dynamic data structures.
 *
 * What is the relationship between handles and double pointers?
 *
 * HINT: Look in the chapter on Abstract Data Types.
 *
 * A handle is like: _______________________________________
 * ════════════════════════════════════════════════════════════════
 */
```

---

### Summary of Part I-B

```
╔═══════════════════════════════════════════════════════════════════════╗
║                      THE FIRST ALGORITHMS                              ║
║                         Key Concepts                                   ║
╠═══════════════════════════════════════════════════════════════════════╣
║                                                                        ║
║  1. CLASSIC ALGORITHMS                                                 ║
║     • Linear Search — O(n), simple but thorough                       ║
║     • Swap — The fundamental exchange operation                        ║
║     • Reverse — Using swap to invert order                            ║
║     • Rotate — The reversal algorithm technique                        ║
║                                                                        ║
║  2. RECURSION                                                          ║
║     • Base case: When to stop                                          ║
║     • Recursive case: How to reduce the problem                        ║
║     • Factorial, Fibonacci examples                                    ║
║     • Memoization for efficiency                                       ║
║                                                                        ║
║  3. DIVIDE AND CONQUER                                                 ║
║     • Divide: Split into subproblems                                   ║
║     • Conquer: Solve subproblems recursively                          ║
║     • Combine: Merge solutions                                         ║
║     • Merge Sort: O(n log n) guaranteed                               ║
║                                                                        ║
║  4. POINTERS                                                           ║
║     • Address-of (&) and Dereference (*)                              ║
║     • Pointer arithmetic                                               ║
║     • Double pointers for indirect access                              ║
║     • Function pointers for callbacks                                  ║
║                                                                        ║
╚═══════════════════════════════════════════════════════════════════════╝
```

---

<div style="page-break-after: always;"></div>

## Part I-C: The Complexity of Arda

### Chapter 10: The Counting of Years — Big-O Notation

In the First Age of Middle-earth, the Elves counted time differently than Men. Similarly, in computer science, we count the "time" an algorithm takes not in seconds, but in *operations*—and we use Big-O notation to describe how this count grows.

```c
/*
 * BIG-O NOTATION: THE MEASURE OF ALGORITHM EFFICIENCY
 *
 * From Grokking Algorithms, Chapter 1:
 *   "Big O notation tells you how fast an algorithm is."
 *
 * But it's more nuanced than that:
 *   Big O describes how the running time GROWS
 *   as the input size GROWS.
 *
 * It's not about seconds—it's about SCALING.
 */

/*
 * THE DWARVEN INTERPRETATION
 *
 * Imagine you're a Dwarven architect planning to organize a treasury.
 *
 * Small treasury (100 items): Any method works fine
 * Medium treasury (10,000 items): Method matters
 * Khazad-dûm treasury (1,000,000 items): Only efficient methods work
 *
 * Big-O tells you which methods will work at scale.
 */

/*
 * THE COMMON COMPLEXITY CLASSES
 *
 * Listed from fastest to slowest:
 */

/*
 * O(1) — CONSTANT TIME
 * "The speed of the wind"
 *
 * No matter how large the input, takes the same time.
 * Like a Dwarf king's decree: one word, heard by all.
 */
int get_first_element(int *arr, size_t n) {
    (void)n;  /* n doesn't affect runtime */
    return arr[0];  /* Always one operation */
}

int hash_table_lookup(HashTable *ht, const char *key) {
    /* Average case: O(1) */
    size_t index = hash(key) % ht->capacity;
    return ht->buckets[index].value;
}

/*
 * O(log n) — LOGARITHMIC TIME
 * "The halving of distances"
 *
 * Each step cuts the problem in half.
 * Like navigating the levels of Moria—each choice halves the remaining depth.
 */
int binary_search_complexity(int *arr, int n, int target) {
    /* Each iteration halves the search space */
    int low = 0, high = n - 1;
    int comparisons = 0;

    while (low <= high) {
        comparisons++;
        int mid = low + (high - low) / 2;

        if (arr[mid] == target) {
            printf("Found in %d comparisons (log₂(%d) ≈ %.1f)\n",
                   comparisons, n, log2(n));
            return mid;
        }
        if (arr[mid] < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

/*
 * LOGARITHM INTUITION
 *
 * n = 1     → log₂(n) = 0
 * n = 2     → log₂(n) = 1
 * n = 4     → log₂(n) = 2
 * n = 8     → log₂(n) = 3
 * n = 16    → log₂(n) = 4
 * n = 1024  → log₂(n) = 10
 * n = 1M    → log₂(n) ≈ 20
 * n = 1B    → log₂(n) ≈ 30
 *
 * Even a billion items only needs ~30 steps!
 */

/*
 * O(n) — LINEAR TIME
 * "The march of the army"
 *
 * Must touch each element once.
 * Like counting every soldier in an army—one by one.
 */
int linear_sum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {  /* n iterations */
        sum += arr[i];              /* O(1) work each */
    }
    return sum;
}

int find_maximum(int *arr, int n) {
    int max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
    /* Must check every element—can't skip any */
}

/*
 * O(n log n) — LINEARITHMIC TIME
 * "The organized march"
 *
 * Do O(log n) work for each of n elements, or
 * Do O(n) work for O(log n) levels.
 *
 * This is the best possible for comparison-based sorting!
 */
void efficient_sort(Part **parts, size_t n) {
    /*
     * Merge Sort: O(n log n)
     *   - log n levels of recursion
     *   - n work at each level (merging)
     *
     * Quick Sort: O(n log n) average
     *   - log n levels of recursion (average)
     *   - n work at each level (partitioning)
     */
    merge_sort(parts, n);
}

/*
 * O(n²) — QUADRATIC TIME
 * "The great assembly"
 *
 * For each element, examine all other elements.
 * Like each Dwarf greeting every other Dwarf at a feast.
 */
void bubble_sort_demo(int *arr, int n) {
    for (int i = 0; i < n; i++) {           /* n iterations */
        for (int j = 0; j < n - 1; j++) {   /* n-1 iterations each */
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
    /* Total: n × (n-1) ≈ n² comparisons */
}

bool has_duplicates(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (arr[i] == arr[j]) {
                return true;
            }
        }
    }
    return false;
    /* Worst case: check all n×(n-1)/2 pairs */
}

/*
 * O(2ⁿ) — EXPONENTIAL TIME
 * "The branching paths of fate"
 *
 * Each element doubles the work.
 * Like the exponential branching of possibilities in the Song of Creation.
 */
int fibonacci_naive_complexity(int n) {
    if (n <= 1) return n;
    return fibonacci_naive_complexity(n - 1) +
           fibonacci_naive_complexity(n - 2);
    /*
     * fib(5) calls fib(4) and fib(3)
     * fib(4) calls fib(3) and fib(2)
     * fib(3) calls fib(2) and fib(1)
     * ...
     * Exponential explosion!
     */
}

void all_subsets(int *arr, int n) {
    /* Generate all 2ⁿ subsets */
    for (unsigned int mask = 0; mask < (1u << n); mask++) {
        printf("{ ");
        for (int i = 0; i < n; i++) {
            if (mask & (1u << i)) {
                printf("%d ", arr[i]);
            }
        }
        printf("}\n");
    }
    /* For n=20: over 1 million subsets */
    /* For n=30: over 1 billion subsets */
}

/*
 * O(n!) — FACTORIAL TIME
 * "The doom of permutations"
 *
 * All possible orderings.
 * As the Naugrim say: "This way lies madness."
 */
void all_permutations(int *arr, int n, int depth) {
    if (depth == n) {
        /* Print this permutation */
        for (int i = 0; i < n; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
        return;
    }

    for (int i = depth; i < n; i++) {
        swap(&arr[depth], &arr[i]);
        all_permutations(arr, n, depth + 1);
        swap(&arr[depth], &arr[i]);  /* Backtrack */
    }
    /*
     * n=10:  3,628,800 permutations
     * n=12: 479,001,600 permutations
     * n=20: 2.4 × 10¹⁸ permutations (more than grains of sand on Earth)
     */
}
```

---

### Chapter 11: The Doom of Time — Comparing Complexities

```c
/*
 * THE GREAT COMPARISON TABLE
 *
 * How long does each complexity take for different input sizes?
 * Assume 1 operation = 1 nanosecond (10⁻⁹ seconds)
 *
 * ┌────────────┬─────────────┬─────────────┬─────────────┬─────────────┐
 * │ Complexity │   n = 10    │  n = 100    │  n = 1000   │  n = 1M     │
 * ├────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
 * │ O(1)       │    1 ns     │    1 ns     │    1 ns     │    1 ns     │
 * │ O(log n)   │    3 ns     │    7 ns     │   10 ns     │   20 ns     │
 * │ O(n)       │   10 ns     │  100 ns     │    1 µs     │    1 ms     │
 * │ O(n log n) │   33 ns     │  664 ns     │   10 µs     │   20 ms     │
 * │ O(n²)      │  100 ns     │   10 µs     │    1 ms     │   17 min    │
 * │ O(2ⁿ)      │    1 µs     │ 10¹⁵ years  │   ∞         │   ∞         │
 * │ O(n!)      │    4 ms     │   ∞         │   ∞         │   ∞         │
 * └────────────┴─────────────┴─────────────┴─────────────┴─────────────┘
 *
 * KEY INSIGHT: The difference between O(n) and O(n²) seems small
 * for n=100, but becomes catastrophic for n=1,000,000.
 */

/*
 * VISUAL COMPARISON: Growth Rates
 *
 *     ops
 *      │
 *      │                                              n!
 *      │                                         ╱
 *      │                                    ╱
 *      │                              2ⁿ ╱
 *      │                           ╱
 *      │                      ╱
 *      │              n² ╱
 *      │            ╱
 *      │      n log n
 *      │    ╱
 *      │  ╱ n
 *      │╱ log n
 *      │─────────────────────────────────── 1
 *      └──────────────────────────────────────── n
 *
 * As n increases, the curves separate dramatically!
 */

/*
 * PRACTICAL IMPLICATIONS FOR PARTSDB
 */

typedef struct {
    const char *operation;
    const char *complexity;
    const char *explanation;
} ComplexityAnalysis;

static const ComplexityAnalysis partsdb_operations[] = {
    {
        "Add part to unsorted list",
        "O(1)",
        "Just append to end—no searching needed"
    },
    {
        "Find part in unsorted list",
        "O(n)",
        "Must check each part—no shortcuts"
    },
    {
        "Find part in sorted list",
        "O(log n)",
        "Binary search halves search space each step"
    },
    {
        "Sort the parts list",
        "O(n log n)",
        "Merge sort or quick sort"
    },
    {
        "Find part in hash table",
        "O(1) average",
        "Direct lookup by hash—like a Dwarven index"
    },
    {
        "Insert into sorted list",
        "O(n)",
        "Must shift elements to make room"
    },
    {
        "Check for duplicate parts",
        "O(n²) naive, O(n) with hash set",
        "Choose your data structure wisely!"
    },
};

void print_complexity_analysis(void) {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║          PARTSDB OPERATION COMPLEXITY ANALYSIS            ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");

    for (size_t i = 0; i < sizeof(partsdb_operations) / sizeof(partsdb_operations[0]); i++) {
        printf("║ %-25s │ %-10s                  ║\n",
               partsdb_operations[i].operation,
               partsdb_operations[i].complexity);
        printf("║   → %-50s ║\n", partsdb_operations[i].explanation);
        printf("╟───────────────────────────────────────────────────────────╢\n");
    }
    printf("╚═══════════════════════════════════════════════════════════╝\n");
}
```

---

### Chapter 12: The Rules of Counting — Analyzing Algorithms

```c
/*
 * THE RULES OF BIG-O ANALYSIS
 *
 * When calculating Big-O, follow these Dwarven rules:
 */

/*
 * RULE 1: DROP CONSTANTS
 *
 * O(2n) = O(n)
 * O(100n) = O(n)
 * O(n/2) = O(n)
 *
 * Constants don't affect how the algorithm SCALES.
 */
void rule_1_example(int *arr, int n) {
    /* This loop runs 3n times */
    for (int i = 0; i < n; i++) {
        process(arr[i]);
    }
    for (int i = 0; i < n; i++) {
        verify(arr[i]);
    }
    for (int i = 0; i < n; i++) {
        finalize(arr[i]);
    }
    /* Complexity: O(3n) = O(n) */
}

/*
 * RULE 2: DROP LOWER-ORDER TERMS
 *
 * O(n² + n) = O(n²)
 * O(n³ + n² + n) = O(n³)
 * O(2ⁿ + n²) = O(2ⁿ)
 *
 * The highest-order term dominates as n grows.
 */
void rule_2_example(int *arr, int n) {
    /* Outer loop: n iterations */
    for (int i = 0; i < n; i++) {
        /* Inner loop: n iterations each */
        for (int j = 0; j < n; j++) {
            combine(arr[i], arr[j]);
        }
    }
    /* Plus a linear scan */
    for (int i = 0; i < n; i++) {
        finalize(arr[i]);
    }
    /* Complexity: O(n² + n) = O(n²) */
}

/*
 * RULE 3: DIFFERENT INPUTS = DIFFERENT VARIABLES
 *
 * If you have two different inputs, use two variables.
 */
void rule_3_example(int *arr1, int n, int *arr2, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            compare(arr1[i], arr2[j]);
        }
    }
    /* Complexity: O(n × m), NOT O(n²) */
}

/*
 * RULE 4: SEQUENTIAL = ADD, NESTED = MULTIPLY
 */
void rule_4_sequential(int *arr, int n) {
    /* First loop: O(n) */
    for (int i = 0; i < n; i++) {
        step1(arr[i]);
    }
    /* Second loop: O(n) */
    for (int i = 0; i < n; i++) {
        step2(arr[i]);
    }
    /* Total: O(n) + O(n) = O(2n) = O(n) */
}

void rule_4_nested(int *arr, int n) {
    /* Outer loop: n iterations */
    for (int i = 0; i < n; i++) {
        /* Inner loop: n iterations */
        for (int j = 0; j < n; j++) {
            combine(arr[i], arr[j]);
        }
    }
    /* Total: O(n) × O(n) = O(n²) */
}

/*
 * RULE 5: RECURSIVE CALLS — THE RECURRENCE RELATION
 *
 * For recursion, count the TOTAL work across all calls.
 */

/*
 * Example: Binary Search
 *
 * T(n) = T(n/2) + O(1)
 *
 * Each call does O(1) work and makes one recursive call
 * with half the input size.
 *
 * Solution: T(n) = O(log n)
 */

/*
 * Example: Merge Sort
 *
 * T(n) = 2T(n/2) + O(n)
 *
 * Each call does O(n) work (merging) and makes two
 * recursive calls with half the input size.
 *
 * Solution: T(n) = O(n log n)
 */

/*
 * THE MASTER THEOREM (Simplified)
 *
 * For recurrences of the form: T(n) = aT(n/b) + O(nᶜ)
 *
 * Let d = log_b(a)
 *
 * If c < d: T(n) = O(nᵈ)
 * If c = d: T(n) = O(nᵈ log n)
 * If c > d: T(n) = O(nᶜ)
 *
 * Examples:
 *   Binary Search: a=1, b=2, c=0 → d=0, c=d → O(log n)
 *   Merge Sort:    a=2, b=2, c=1 → d=1, c=d → O(n log n)
 *   Strassen:      a=7, b=2, c=2 → d≈2.8, c<d → O(n^2.8)
 */

/*
 * SCAVENGER HUNT CLUE #5:
 * ════════════════════════════════════════════════════════════════
 * In Sedgewick's "Algorithms in C", Part 1, Chapter 2,
 * there is a discussion of the "inner loop" and why it matters.
 *
 * What percentage of execution time typically occurs in
 * the inner loop of a program, according to Sedgewick?
 *
 * ANSWER: _______%
 *
 * This is why optimizing the inner loop is so crucial!
 * ════════════════════════════════════════════════════════════════
 */
```

---

### Chapter 13: The Three Measures — Best, Average, and Worst Case

```c
/*
 * THE THREE FATES OF ALGORITHM ANALYSIS
 *
 * Like the fates that govern the lives of Men in Arda,
 * algorithms have three possible destinies for any input.
 */

/*
 * BEST CASE (Ω — Omega)
 * "The blessed path"
 *
 * The minimum time the algorithm could take.
 * Often not very informative—usually O(1) for finding.
 */

/*
 * AVERAGE CASE (Θ — Theta)
 * "The common path"
 *
 * The expected time for a "typical" input.
 * Requires assumptions about input distribution.
 */

/*
 * WORST CASE (O — Big-O)
 * "The cursed path"
 *
 * The maximum time the algorithm could take.
 * This is what we usually focus on—it's a GUARANTEE.
 */

/*
 * EXAMPLE: Linear Search
 */
typedef struct {
    int best;
    int average;
    int worst;
} SearchResult;

SearchResult analyze_linear_search(int *arr, int n, int target) {
    SearchResult result = {0, 0, 0};

    /*
     * BEST CASE: O(1)
     * Target is the first element.
     */
    if (arr[0] == target) {
        result.best = 1;
    }

    /*
     * AVERAGE CASE: O(n/2) = O(n)
     * On average, we check half the elements.
     * (Assuming target is equally likely to be anywhere)
     */
    result.average = n / 2;

    /*
     * WORST CASE: O(n)
     * Target is the last element, or not present at all.
     */
    result.worst = n;

    return result;
}

/*
 * EXAMPLE: Quick Sort
 *
 * ┌──────────────┬────────────────┬──────────────────────────────────┐
 * │    Case      │   Complexity   │   When it happens                │
 * ├──────────────┼────────────────┼──────────────────────────────────┤
 * │   Best       │   O(n log n)   │   Pivot always splits evenly     │
 * │   Average    │   O(n log n)   │   Random input, random pivot     │
 * │   Worst      │   O(n²)        │   Already sorted, bad pivot      │
 * └──────────────┴────────────────┴──────────────────────────────────┘
 */

void quicksort_analysis(int *arr, int low, int high, int depth) {
    if (low >= high) return;

    /*
     * The partition step is O(n) for the current subarray.
     *
     * BEST CASE: Pivot is always median
     *   → Each partition splits array in half
     *   → Depth is O(log n)
     *   → Total: O(n log n)
     *
     * WORST CASE: Pivot is always min or max
     *   → Each partition removes only 1 element
     *   → Depth is O(n)
     *   → Total: O(n²)
     */

    int pivot_idx = partition(arr, low, high);
    quicksort_analysis(arr, low, pivot_idx - 1, depth + 1);
    quicksort_analysis(arr, pivot_idx + 1, high, depth + 1);
}

/*
 * AMORTIZED ANALYSIS
 * "The average over many operations"
 *
 * Sometimes a single operation is expensive, but if we spread
 * the cost over many operations, the average is low.
 */

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} DynamicArray;

void dynamic_array_push(DynamicArray *arr, int value) {
    if (arr->size >= arr->capacity) {
        /*
         * EXPENSIVE: Double the capacity O(n)
         * Must copy all existing elements.
         */
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(int));
    }

    /*
     * CHEAP: Add the element O(1)
     */
    arr->data[arr->size++] = value;

    /*
     * AMORTIZED ANALYSIS:
     *
     * Insert 1: no resize, cost = 1
     * Insert 2: resize, copy 1, insert, cost = 2
     * Insert 3: no resize, cost = 1
     * Insert 4: resize, copy 3, insert, cost = 4
     * Insert 5: no resize, cost = 1
     * Insert 6: no resize, cost = 1
     * Insert 7: no resize, cost = 1
     * Insert 8: resize, copy 7, insert, cost = 8
     * ...
     *
     * Total cost for n insertions ≈ n + n/2 + n/4 + ... ≈ 2n
     * Amortized cost per insertion = 2n/n = O(1)
     *
     * Even though resizing is O(n), it happens rarely enough
     * that the AVERAGE cost per operation is O(1).
     */
}
```

---

### Summary of Part I-C

```
╔═══════════════════════════════════════════════════════════════════════╗
║                    THE COMPLEXITY OF ARDA                              ║
║                         Key Concepts                                   ║
╠═══════════════════════════════════════════════════════════════════════╣
║                                                                        ║
║  1. BIG-O COMPLEXITY CLASSES                                          ║
║     • O(1)       — Constant (hash table lookup)                       ║
║     • O(log n)   — Logarithmic (binary search)                        ║
║     • O(n)       — Linear (single pass)                               ║
║     • O(n log n) — Linearithmic (efficient sorting)                   ║
║     • O(n²)      — Quadratic (nested loops)                           ║
║     • O(2ⁿ)      — Exponential (subset generation)                    ║
║     • O(n!)      — Factorial (permutations)                           ║
║                                                                        ║
║  2. ANALYSIS RULES                                                     ║
║     • Drop constants: O(3n) = O(n)                                    ║
║     • Drop lower terms: O(n² + n) = O(n²)                             ║
║     • Sequential = Add: O(n) + O(n) = O(n)                            ║
║     • Nested = Multiply: O(n) × O(n) = O(n²)                          ║
║                                                                        ║
║  3. THREE CASES                                                        ║
║     • Best case (Ω) — Minimum time                                    ║
║     • Average case (Θ) — Expected time                                ║
║     • Worst case (O) — Maximum time (guarantee)                       ║
║                                                                        ║
║  4. AMORTIZED ANALYSIS                                                 ║
║     • Average cost over sequence of operations                        ║
║     • Dynamic array push: O(1) amortized                              ║
║                                                                        ║
╚═══════════════════════════════════════════════════════════════════════╝
```

---

# END OF THE AINULINDALË (PART I)

---

*Continue to The Valaquenta (Part II): The Powers of Data Structures*

---
