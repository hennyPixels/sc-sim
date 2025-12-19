# The Silmarillion of Algorithms
## Part III: Khazadgarmâ — The Sorting Arts of the Dwarves

*"And the Dwarves delved deep into the mountains of Arda,
and there they found ores of many kinds: gold and silver,
mithril and iron, gems of every hue. But the ores came forth
in chaos, and the Dwarves learned the arts of sorting —
to bring order from disorder, value from raw earth."*

---

# Part III-A: The Mining Sorts — Simple Tools for Simple Tasks

*In which we learn the basic sorting arts of the miners:
Bubble Sort, Selection Sort, and Insertion Sort —
slow but steady methods for small collections.*

## The Philosophy of Sorting

When a Dwarven miner emerges from the depths with a cart of ore,
the first task is to sort the stones: precious metals here,
common iron there, worthless rock to the slag heap. Sorting
is perhaps the most fundamental operation in all of computing.

**Why Sort?**

1. **Binary Search** — O(log n) search requires sorted data
2. **Duplicate Detection** — Adjacent elements after sorting
3. **Finding Min/Max** — First and last elements
4. **Efficient Merging** — Two sorted lists merge in O(n)
5. **Human Readability** — Sorted output is easier to scan

### The Properties of a Good Sort

```
Stability:   Does the sort preserve the relative order of equal elements?
             [3a, 1, 3b, 2] → [1, 2, 3a, 3b]  (stable)
             [3a, 1, 3b, 2] → [1, 2, 3b, 3a]  (unstable)

In-place:    Does it sort using only O(1) extra memory?

Adaptive:    Is it faster when data is already partially sorted?

Online:      Can it sort as data arrives (streaming)?

Comparison-based: Does it compare elements, or use other properties?
```

---

## Bubble Sort — The Rising Bubbles

*"As bubbles rise through water, so do larger values
rise through the array, floating to their rightful place."*

The simplest of all sorts. Compare adjacent elements and swap
if out of order. Repeat until no swaps are needed.

```
Bubble Sort Visualization:

Pass 1: [5, 3, 8, 4, 2]
        [3, 5, 8, 4, 2]  swap 5,3
        [3, 5, 8, 4, 2]  no swap
        [3, 5, 4, 8, 2]  swap 8,4
        [3, 5, 4, 2, 8]  swap 8,2  ← 8 bubbled to end!

Pass 2: [3, 5, 4, 2, 8]
        [3, 5, 4, 2, 8]  no swap
        [3, 4, 5, 2, 8]  swap 5,4
        [3, 4, 2, 5, 8]  swap 5,2  ← 5 in place!

Pass 3: [3, 4, 2, 5, 8]
        [3, 4, 2, 5, 8]  no swap
        [3, 2, 4, 5, 8]  swap 4,2  ← 4 in place!

Pass 4: [3, 2, 4, 5, 8]
        [2, 3, 4, 5, 8]  swap 3,2  ← Done!
```

```c
/*
 * Bubble Sort — O(n²) average and worst, O(n) best
 *
 * Like bubbles rising through molten ore,
 * larger elements float to the surface.
 *
 * Properties:
 * - Stable: Yes
 * - In-place: Yes
 * - Adaptive: Yes (with early termination)
 * - Online: No
 */
void bubble_sort(Part *parts, size_t n, PartComparator cmp) {
    if (n <= 1) return;

    for (size_t i = 0; i < n - 1; i++) {
        bool swapped = false;

        /* Each pass bubbles the largest unsorted element to the end */
        for (size_t j = 0; j < n - 1 - i; j++) {
            if (cmp(&parts[j], &parts[j + 1]) > 0) {
                /* Swap adjacent elements */
                Part temp = parts[j];
                parts[j] = parts[j + 1];
                parts[j + 1] = temp;
                swapped = true;
            }
        }

        /* If no swaps occurred, array is sorted */
        if (!swapped) break;  /* Adaptive optimization */
    }
}

/*
 * Optimized Bubble Sort — tracks last swap position
 */
void bubble_sort_optimized(Part *parts, size_t n, PartComparator cmp) {
    size_t last_unsorted = n - 1;

    while (last_unsorted > 0) {
        size_t last_swap = 0;

        for (size_t j = 0; j < last_unsorted; j++) {
            if (cmp(&parts[j], &parts[j + 1]) > 0) {
                Part temp = parts[j];
                parts[j] = parts[j + 1];
                parts[j + 1] = temp;
                last_swap = j;
            }
        }

        /* Everything after last_swap is sorted */
        last_unsorted = last_swap;
    }
}
```

### When to Use Bubble Sort

Almost never in production. But it has educational value and
one practical use: detecting if data is already sorted in O(n).

```c
/*
 * Check if array is sorted — Bubble Sort's one true calling
 */
bool is_sorted(Part *parts, size_t n, PartComparator cmp) {
    for (size_t i = 0; i < n - 1; i++) {
        if (cmp(&parts[i], &parts[i + 1]) > 0) {
            return false;
        }
    }
    return true;
}
```

---

## Selection Sort — The Miner's Eye

*"The master miner surveys the ore pile with a keen eye,
selecting the largest nugget each time, placing it aside."*

Selection Sort finds the minimum (or maximum) element in the
unsorted portion and swaps it to its final position.

```
Selection Sort Visualization:

[5, 3, 8, 4, 2]  Find min in [0..4]: 2 at index 4
[2, 3, 8, 4, 5]  Swap with position 0

[2, 3, 8, 4, 5]  Find min in [1..4]: 3 at index 1
[2, 3, 8, 4, 5]  Already in place

[2, 3, 8, 4, 5]  Find min in [2..4]: 4 at index 3
[2, 3, 4, 8, 5]  Swap with position 2

[2, 3, 4, 8, 5]  Find min in [3..4]: 5 at index 4
[2, 3, 4, 5, 8]  Swap with position 3

Done!
```

```c
/*
 * Selection Sort — O(n²) always
 *
 * The miner's eye surveys the unsorted pile,
 * selecting the smallest ore each pass.
 *
 * Properties:
 * - Stable: No (swapping can reorder equals)
 * - In-place: Yes
 * - Adaptive: No (always n² comparisons)
 * - Online: No
 *
 * Advantage: Minimum number of swaps (exactly n-1)
 */
void selection_sort(Part *parts, size_t n, PartComparator cmp) {
    for (size_t i = 0; i < n - 1; i++) {
        /* Find minimum in unsorted portion */
        size_t min_idx = i;

        for (size_t j = i + 1; j < n; j++) {
            if (cmp(&parts[j], &parts[min_idx]) < 0) {
                min_idx = j;
            }
        }

        /* Swap minimum to its final position */
        if (min_idx != i) {
            Part temp = parts[i];
            parts[i] = parts[min_idx];
            parts[min_idx] = temp;
        }
    }
}

/*
 * Double Selection Sort — finds both min and max each pass
 * Reduces passes by half, same O(n²) complexity
 */
void double_selection_sort(Part *parts, size_t n, PartComparator cmp) {
    size_t left = 0;
    size_t right = n - 1;

    while (left < right) {
        size_t min_idx = left;
        size_t max_idx = left;

        /* Find both min and max in one pass */
        for (size_t i = left; i <= right; i++) {
            if (cmp(&parts[i], &parts[min_idx]) < 0) {
                min_idx = i;
            }
            if (cmp(&parts[i], &parts[max_idx]) > 0) {
                max_idx = i;
            }
        }

        /* Swap min to left */
        if (min_idx != left) {
            Part temp = parts[left];
            parts[left] = parts[min_idx];
            parts[min_idx] = temp;

            /* If max was at left, it's now at min_idx */
            if (max_idx == left) {
                max_idx = min_idx;
            }
        }

        /* Swap max to right */
        if (max_idx != right) {
            Part temp = parts[right];
            parts[right] = parts[max_idx];
            parts[max_idx] = temp;
        }

        left++;
        right--;
    }
}
```

### When to Use Selection Sort

When writes are expensive (e.g., flash memory, EEPROM).
Selection Sort makes exactly n-1 swaps regardless of input.

---

## Insertion Sort — The Card Player's Method

*"As a Dwarf arranges his hand of Khazâd-ai cards,
each new card is inserted into its proper place
among those already sorted."*

Insertion Sort builds the sorted array one element at a time,
inserting each new element into its correct position.

```
Insertion Sort Visualization:

[5, 3, 8, 4, 2]  Start: [5] is trivially sorted
     ↑
[5, 3, 8, 4, 2]  Insert 3: shift 5 right, place 3
[3, 5, 8, 4, 2]  Sorted: [3, 5]
        ↑
[3, 5, 8, 4, 2]  Insert 8: already in place
[3, 5, 8, 4, 2]  Sorted: [3, 5, 8]
           ↑
[3, 5, 8, 4, 2]  Insert 4: shift 8,5 right, place 4
[3, 4, 5, 8, 2]  Sorted: [3, 4, 5, 8]
              ↑
[3, 4, 5, 8, 2]  Insert 2: shift all right, place 2
[2, 3, 4, 5, 8]  Done!
```

```c
/*
 * Insertion Sort — O(n²) average/worst, O(n) best
 *
 * Like sorting cards in your hand,
 * each new element finds its place among the sorted.
 *
 * Properties:
 * - Stable: Yes
 * - In-place: Yes
 * - Adaptive: Yes (O(n) for nearly sorted data!)
 * - Online: Yes (can sort as data arrives)
 */
void insertion_sort(Part *parts, size_t n, PartComparator cmp) {
    for (size_t i = 1; i < n; i++) {
        Part key = parts[i];  /* Element to insert */
        size_t j = i;

        /* Shift larger elements right */
        while (j > 0 && cmp(&parts[j - 1], &key) > 0) {
            parts[j] = parts[j - 1];
            j--;
        }

        parts[j] = key;  /* Insert in correct position */
    }
}

/*
 * Binary Insertion Sort — O(n log n) comparisons, still O(n²) shifts
 * Useful when comparison is expensive
 */
void binary_insertion_sort(Part *parts, size_t n, PartComparator cmp) {
    for (size_t i = 1; i < n; i++) {
        Part key = parts[i];

        /* Binary search for insertion point */
        size_t left = 0;
        size_t right = i;

        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (cmp(&parts[mid], &key) <= 0) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        /* Shift elements right */
        for (size_t j = i; j > left; j--) {
            parts[j] = parts[j - 1];
        }

        parts[left] = key;
    }
}

/*
 * Insertion Sort with Sentinel — eliminates inner loop bound check
 */
void insertion_sort_sentinel(Part *parts, size_t n, PartComparator cmp) {
    if (n <= 1) return;

    /* Find minimum and place at position 0 as sentinel */
    size_t min_idx = 0;
    for (size_t i = 1; i < n; i++) {
        if (cmp(&parts[i], &parts[min_idx]) < 0) {
            min_idx = i;
        }
    }

    /* Swap minimum to front */
    Part temp = parts[0];
    parts[0] = parts[min_idx];
    parts[min_idx] = temp;

    /* Now we can skip the j > 0 check */
    for (size_t i = 2; i < n; i++) {
        Part key = parts[i];
        size_t j = i;

        /* No bounds check needed — sentinel stops us */
        while (cmp(&parts[j - 1], &key) > 0) {
            parts[j] = parts[j - 1];
            j--;
        }

        parts[j] = key;
    }
}
```

### When to Use Insertion Sort

1. **Small arrays** (n < 20): Cache-friendly, low overhead
2. **Nearly sorted data**: O(n) performance
3. **Online sorting**: Can sort as data arrives
4. **Hybrid sorts**: Used as base case in Quicksort/Mergesort

---

## Shell Sort — The Diminishing Increment

*"The wise miner Donaldus Shell discovered that by
pre-sorting with larger gaps, the final passes require
fewer moves, as stones are already near their homes."*

Shell Sort is a generalization of Insertion Sort that allows
exchanges of elements far apart. The gap shrinks over passes.

```
Shell Sort with gaps [5, 3, 1]:

Original: [9, 8, 3, 7, 5, 6, 4, 1]

Gap 5: Compare elements 5 apart
       [9,.,.,.,.,6,.,.] → [6,.,.,.,.,9,.,.]  swap
       [.,8,.,.,.,.,4,.] → [.,4,.,.,.,.,8,.]  swap
       [.,.,3,.,.,.,.,1] → [.,.,1,.,.,.,.,3]  swap
       Result: [6, 4, 1, 7, 5, 9, 8, 3]

Gap 3: Compare elements 3 apart
       [6,.,.,7,.,.,8,.] → [6,.,.,7,.,.,8,.]  OK
       [.,4,.,.,5,.,.,3] → [.,3,.,.,4,.,.,5]  (3 swaps)
       [.,.,1,.,.,9,.,.]  → [.,.,1,.,.,9,.,.]  OK
       Result: [6, 3, 1, 7, 4, 9, 8, 5]

Gap 1: Standard insertion sort (but nearly sorted!)
       Result: [1, 3, 4, 5, 6, 7, 8, 9]
```

```c
/*
 * Shell Sort — O(n^1.5) to O(n log² n) depending on gap sequence
 *
 * The wisdom of gaps: pre-sorting with large gaps
 * means the final insertion sort has less work.
 *
 * Properties:
 * - Stable: No (elements jump over each other)
 * - In-place: Yes
 * - Adaptive: Somewhat
 */

/* Shell's original sequence: n/2, n/4, ..., 1 */
void shell_sort_original(Part *parts, size_t n, PartComparator cmp) {
    for (size_t gap = n / 2; gap > 0; gap /= 2) {
        /* Insertion sort with this gap */
        for (size_t i = gap; i < n; i++) {
            Part temp = parts[i];
            size_t j = i;

            while (j >= gap && cmp(&parts[j - gap], &temp) > 0) {
                parts[j] = parts[j - gap];
                j -= gap;
            }

            parts[j] = temp;
        }
    }
}

/*
 * Ciura's gap sequence — empirically optimal
 * Gaps: 1, 4, 10, 23, 57, 132, 301, 701, 1750, ...
 */
static const size_t CIURA_GAPS[] = {
    701, 301, 132, 57, 23, 10, 4, 1
};
static const size_t CIURA_GAP_COUNT = 8;

void shell_sort_ciura(Part *parts, size_t n, PartComparator cmp) {
    /* Find first gap smaller than n */
    size_t gap_idx = 0;
    while (gap_idx < CIURA_GAP_COUNT && CIURA_GAPS[gap_idx] >= n) {
        gap_idx++;
    }

    /* Apply each gap */
    for (; gap_idx < CIURA_GAP_COUNT; gap_idx++) {
        size_t gap = CIURA_GAPS[gap_idx];

        for (size_t i = gap; i < n; i++) {
            Part temp = parts[i];
            size_t j = i;

            while (j >= gap && cmp(&parts[j - gap], &temp) > 0) {
                parts[j] = parts[j - gap];
                j -= gap;
            }

            parts[j] = temp;
        }
    }
}

/*
 * Sedgewick's gap sequence — O(n^(4/3)) worst case
 * Gaps: 1, 5, 19, 41, 109, 209, 505, 929, ...
 * Formula: 9*(4^k - 2^k) + 1 or 2^(k+2)*(2^(k+2) - 3) + 1
 */
void shell_sort_sedgewick(Part *parts, size_t n, PartComparator cmp) {
    /* Generate Sedgewick gaps */
    size_t gaps[32];
    size_t gap_count = 0;

    for (size_t k = 0; ; k++) {
        size_t gap;
        if (k % 2 == 0) {
            gap = 9 * ((1ULL << (2*k)) - (1ULL << k)) + 1;
        } else {
            size_t pow = (k + 1) / 2;
            gap = (1ULL << (2*pow + 2)) - 3 * (1ULL << (pow + 1)) + 1;
        }

        if (gap >= n) break;
        gaps[gap_count++] = gap;
    }

    /* Apply gaps in reverse order (largest first) */
    for (size_t g = gap_count; g > 0; g--) {
        size_t gap = gaps[g - 1];

        for (size_t i = gap; i < n; i++) {
            Part temp = parts[i];
            size_t j = i;

            while (j >= gap && cmp(&parts[j - gap], &temp) > 0) {
                parts[j] = parts[j - gap];
                j -= gap;
            }

            parts[j] = temp;
        }
    }
}
```

---

## Comparison of Mining Sorts

```
╔══════════════════════════════════════════════════════════════════════════╗
║                 THE MINING TABLET OF SIMPLE SORTS                        ║
╠══════════════════════════════════════════════════════════════════════════╣
║  Algorithm       │ Best    │ Average │ Worst   │ Space │ Stable │ Notes  ║
╠══════════════════╪═════════╪═════════╪═════════╪═══════╪════════╪════════╣
║  Bubble Sort     │ O(n)    │ O(n²)   │ O(n²)   │ O(1)  │ Yes    │Adaptive║
║  Selection Sort  │ O(n²)   │ O(n²)   │ O(n²)   │ O(1)  │ No     │Min swap║
║  Insertion Sort  │ O(n)    │ O(n²)   │ O(n²)   │ O(1)  │ Yes    │Online  ║
║  Shell Sort      │ O(n log)│ O(n^1.3)│ O(n^1.5)│ O(1)  │ No     │Gap-dep ║
╠══════════════════╧═════════╧═════════╧═════════╧═══════╧════════╧════════╣
║  For small n (< 20): Insertion Sort is fastest due to cache efficiency  ║
║  For nearly sorted: Insertion Sort achieves O(n)                         ║
║  For minimal writes: Selection Sort makes only n-1 swaps                 ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Scavenger Hunt — Clue #12

*"In Sedgewick's Chapter 6, he reveals a profound truth about
Insertion Sort. He shows that the number of exchanges equals
the number of inversions in the input. What is an inversion,
and why does this make Insertion Sort optimal for nearly sorted data?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 6.
Find the definition of "inversion" and understand why counting
inversions predicts Insertion Sort's performance.

---

## Scavenger Hunt — Clue #13

*"Bhargava opens Chapter 2 with a simple question: how many
guesses to find a name in a phone book? He then pivots to
selection sort with a tale of music playlists. What metaphor
does he use, and why is it O(n²)?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 2.
Find the music playlist example. How many operations
does it take to sort a list of n songs?

---

## Exercises — The Trials of the Mining Sorts

### Trial 1: The Ore Sorter
```c
/*
 * Sort a pile of ore samples by density.
 * Use Insertion Sort since samples arrive one at a time.
 *
 * typedef struct {
 *     char sample_id[10];
 *     double density;  // g/cm³
 *     OreType type;
 * } OreSample;
 */
```

### Trial 2: The Stability Test
```c
/*
 * Sort parts first by category, then by price within category.
 * Demonstrate why stability matters:
 *
 * 1. Sort by price (stable sort)
 * 2. Sort by category (stable sort)
 * Result: Parts sorted by category, with prices sorted within
 */
```

### Trial 3: The Gap Experiment
```c
/*
 * Implement Shell Sort with three different gap sequences:
 * 1. Shell's original: n/2, n/4, ...
 * 2. Hibbard's: 2^k - 1 (1, 3, 7, 15, 31, ...)
 * 3. Ciura's: 1, 4, 10, 23, 57, 132, 301, 701
 *
 * Compare performance on random, sorted, and reverse-sorted data.
 */
```

---

*Thus ends the first chapter of Khazadgarmâ.
We have learned the simple arts of the miners,
sorts that work well for small collections.
In the next chapter, we shall learn the construction sorts,
powerful algorithms built through division and conquest.*

---

# Part III-B: The Construction Sorts — Divide and Conquer

*In which we learn the architectural methods of the Dwarves:
Merge Sort and Quick Sort — dividing great problems
into smaller ones, then reassembling the solution.*

## The Philosophy of Divide and Conquer

When the Dwarves carved Khazad-dûm, they did not attempt to
hollow the entire mountain at once. They divided the work:
teams carved separate halls, and the halls were connected
into the great kingdom. This is **Divide and Conquer**.

```
The Three Steps of Divide and Conquer:

1. DIVIDE:   Split the problem into smaller subproblems
2. CONQUER:  Solve each subproblem (recursively or directly)
3. COMBINE:  Merge solutions into the final answer

                    [Problem]
                    /       \
              [Sub-1]       [Sub-2]        ← DIVIDE
              /    \        /    \
          [S-1a] [S-1b]  [S-2a] [S-2b]     ← DIVIDE
            ↓      ↓        ↓      ↓
          [Sol]  [Sol]    [Sol]  [Sol]     ← CONQUER (base case)
            \      /        \      /
          [Solution]      [Solution]       ← COMBINE
                \            /
               [Final Solution]            ← COMBINE
```

---

## Merge Sort — The Master Builder's Method

*"First, sort each stone individually (trivial).
Then, merge pairs of sorted piles into larger sorted piles.
Continue until all stones form one great ordered wall."*

Merge Sort divides the array in half, recursively sorts each half,
then merges the two sorted halves into one.

```
Merge Sort Visualization:

           [38, 27, 43, 3, 9, 82, 10]
                    DIVIDE
           /                    \
    [38, 27, 43, 3]        [9, 82, 10]
         DIVIDE                DIVIDE
       /        \            /        \
  [38, 27]    [43, 3]    [9, 82]    [10]
    DIVIDE      DIVIDE     DIVIDE    (base)
   /    \      /    \     /    \
 [38]  [27]  [43]  [3]  [9]  [82]    [10]
  \    /      \    /     \    /
   MERGE       MERGE      MERGE
  [27, 38]    [3, 43]    [9, 82]     [10]
      \        /            \        /
       MERGE                 MERGE
    [3, 27, 38, 43]        [9, 10, 82]
           \                  /
                 MERGE
        [3, 9, 10, 27, 38, 43, 82]
```

```c
/*
 * Merge Sort — O(n log n) always
 *
 * The master builder's wisdom: divide the work,
 * sort the pieces, merge the results.
 *
 * Properties:
 * - Stable: Yes
 * - In-place: No (requires O(n) extra space)
 * - Adaptive: No (always n log n)
 * - Parallelizable: Excellent
 */

/*
 * Merge two sorted subarrays into one
 * Left: arr[left..mid], Right: arr[mid+1..right]
 */
static void merge(Part *arr, Part *temp, size_t left,
                  size_t mid, size_t right, PartComparator cmp) {
    size_t i = left;      /* Index into left subarray */
    size_t j = mid + 1;   /* Index into right subarray */
    size_t k = left;      /* Index into temp array */

    /* Merge while both subarrays have elements */
    while (i <= mid && j <= right) {
        if (cmp(&arr[i], &arr[j]) <= 0) {
            temp[k++] = arr[i++];  /* Take from left (stable) */
        } else {
            temp[k++] = arr[j++];  /* Take from right */
        }
    }

    /* Copy remaining elements from left subarray */
    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    /* Copy remaining elements from right subarray */
    while (j <= right) {
        temp[k++] = arr[j++];
    }

    /* Copy merged result back to original array */
    for (size_t x = left; x <= right; x++) {
        arr[x] = temp[x];
    }
}

/*
 * Recursive Merge Sort implementation
 */
static void merge_sort_recursive(Part *arr, Part *temp,
                                  size_t left, size_t right,
                                  PartComparator cmp) {
    if (left >= right) return;  /* Base case: 0 or 1 element */

    size_t mid = left + (right - left) / 2;

    /* Divide and conquer */
    merge_sort_recursive(arr, temp, left, mid, cmp);
    merge_sort_recursive(arr, temp, mid + 1, right, cmp);

    /* Combine */
    merge(arr, temp, left, mid, right, cmp);
}

/*
 * Public interface for Merge Sort
 */
bool merge_sort(Part *arr, size_t n, PartComparator cmp) {
    if (n <= 1) return true;

    /* Allocate temporary array */
    Part *temp = malloc(n * sizeof(Part));
    if (!temp) return false;

    merge_sort_recursive(arr, temp, 0, n - 1, cmp);

    free(temp);
    return true;
}

/*
 * Optimized Merge Sort with Insertion Sort for small subarrays
 */
#define INSERTION_THRESHOLD 16

static void merge_sort_optimized(Part *arr, Part *temp,
                                  size_t left, size_t right,
                                  PartComparator cmp) {
    /* Use insertion sort for small subarrays */
    if (right - left + 1 <= INSERTION_THRESHOLD) {
        for (size_t i = left + 1; i <= right; i++) {
            Part key = arr[i];
            size_t j = i;
            while (j > left && cmp(&arr[j - 1], &key) > 0) {
                arr[j] = arr[j - 1];
                j--;
            }
            arr[j] = key;
        }
        return;
    }

    size_t mid = left + (right - left) / 2;

    merge_sort_optimized(arr, temp, left, mid, cmp);
    merge_sort_optimized(arr, temp, mid + 1, right, cmp);

    /* Skip merge if already sorted */
    if (cmp(&arr[mid], &arr[mid + 1]) <= 0) return;

    merge(arr, temp, left, mid, right, cmp);
}
```

### Bottom-Up Merge Sort — Iterative Version

```c
/*
 * Bottom-Up Merge Sort — No recursion
 *
 * Start with subarrays of size 1, merge into size 2,
 * then 4, then 8, ... until entire array is sorted.
 */
bool merge_sort_bottom_up(Part *arr, size_t n, PartComparator cmp) {
    if (n <= 1) return true;

    Part *temp = malloc(n * sizeof(Part));
    if (!temp) return false;

    /* Start with width 1, double each iteration */
    for (size_t width = 1; width < n; width *= 2) {
        /* Merge subarrays of current width */
        for (size_t left = 0; left < n; left += 2 * width) {
            size_t mid = left + width - 1;
            size_t right = left + 2 * width - 1;

            /* Clamp to array bounds */
            if (mid >= n) mid = n - 1;
            if (right >= n) right = n - 1;

            if (mid < right) {
                merge(arr, temp, left, mid, right, cmp);
            }
        }
    }

    free(temp);
    return true;
}
```

### Natural Merge Sort — Exploiting Existing Order

```c
/*
 * Natural Merge Sort — Finds existing sorted runs
 *
 * If data is partially sorted, this can be faster than O(n log n)
 */
bool natural_merge_sort(Part *arr, size_t n, PartComparator cmp) {
    if (n <= 1) return true;

    Part *temp = malloc(n * sizeof(Part));
    if (!temp) return false;

    bool sorted = false;

    while (!sorted) {
        sorted = true;
        size_t i = 0;

        while (i < n) {
            /* Find first run */
            size_t run1_start = i;
            while (i < n - 1 && cmp(&arr[i], &arr[i + 1]) <= 0) {
                i++;
            }
            size_t run1_end = i++;

            if (i >= n) break;  /* Only one run — we're done */

            /* Find second run */
            size_t run2_start = i;
            while (i < n - 1 && cmp(&arr[i], &arr[i + 1]) <= 0) {
                i++;
            }
            size_t run2_end = i++;

            /* Merge the two runs */
            merge(arr, temp, run1_start, run1_end, run2_end, cmp);
            sorted = false;
        }
    }

    free(temp);
    return true;
}
```

---

## Quick Sort — The Pivot Master's Art

*"Choose a pivot stone. Cast all lesser stones to the left,
all greater stones to the right. Then recursively order
each pile. The pivot remains forever in its final place."*

Quick Sort selects a "pivot" element, partitions the array
so all elements less than pivot are left, greater are right,
then recursively sorts each partition.

```
Quick Sort Visualization (pivot = last element):

[3, 7, 8, 5, 2, 1, 9, 5, 4]  pivot = 4
                         ↑
Partition around 4:
[3, 2, 1] [4] [7, 8, 5, 9, 5]  ← 4 is in final position!
    ↑            ↑
 all ≤ 4      all > 4

Recursively sort left: [3, 2, 1]  pivot = 1
[1] [3, 2] → [1] [2, 3] → [1, 2, 3]

Recursively sort right: [7, 8, 5, 9, 5]  pivot = 5
[5] [5] [7, 8, 9] → [5, 5] [7, 8, 9]
                          ↓
                    [7, 8, 9]  pivot = 9
                    [7, 8] [9] → [7, 8, 9]

Final: [1, 2, 3, 4, 5, 5, 7, 8, 9]
```

```c
/*
 * Quick Sort — O(n log n) average, O(n²) worst
 *
 * The pivot master's wisdom: one well-chosen pivot
 * divides the problem nearly in half.
 *
 * Properties:
 * - Stable: No (partitioning reorders equals)
 * - In-place: Yes (O(log n) stack space)
 * - Adaptive: Somewhat
 * - Cache-friendly: Excellent
 */

/*
 * Swap helper
 */
static inline void swap(Part *a, Part *b) {
    Part temp = *a;
    *a = *b;
    *b = temp;
}

/*
 * Lomuto Partition Scheme
 * Simpler but slower; pivot goes to final position
 */
static size_t partition_lomuto(Part *arr, size_t low, size_t high,
                               PartComparator cmp) {
    Part *pivot = &arr[high];  /* Use last element as pivot */
    size_t i = low;            /* Index of smaller element */

    for (size_t j = low; j < high; j++) {
        if (cmp(&arr[j], pivot) < 0) {
            swap(&arr[i], &arr[j]);
            i++;
        }
    }

    swap(&arr[i], &arr[high]);  /* Place pivot in final position */
    return i;
}

/*
 * Hoare Partition Scheme
 * Original, faster, but more complex; pivot NOT in final position
 */
static size_t partition_hoare(Part *arr, size_t low, size_t high,
                              PartComparator cmp) {
    Part pivot = arr[low + (high - low) / 2];  /* Middle element */
    size_t i = low;
    size_t j = high;

    while (true) {
        /* Find element >= pivot from left */
        while (cmp(&arr[i], &pivot) < 0) i++;

        /* Find element <= pivot from right */
        while (cmp(&arr[j], &pivot) > 0) j--;

        if (i >= j) return j;

        swap(&arr[i], &arr[j]);
        i++;
        j--;
    }
}

/*
 * Recursive Quick Sort with Lomuto partitioning
 */
static void quicksort_lomuto(Part *arr, size_t low, size_t high,
                              PartComparator cmp) {
    if (low >= high) return;

    size_t pivot_idx = partition_lomuto(arr, low, high, cmp);

    if (pivot_idx > 0) {
        quicksort_lomuto(arr, low, pivot_idx - 1, cmp);
    }
    quicksort_lomuto(arr, pivot_idx + 1, high, cmp);
}

/*
 * Recursive Quick Sort with Hoare partitioning
 */
static void quicksort_hoare(Part *arr, size_t low, size_t high,
                             PartComparator cmp) {
    if (low >= high) return;

    size_t pivot_idx = partition_hoare(arr, low, high, cmp);

    quicksort_hoare(arr, low, pivot_idx, cmp);
    quicksort_hoare(arr, pivot_idx + 1, high, cmp);
}

/*
 * Public Quick Sort interface
 */
void quick_sort(Part *arr, size_t n, PartComparator cmp) {
    if (n <= 1) return;
    quicksort_hoare(arr, 0, n - 1, cmp);
}
```

### Pivot Selection Strategies

The choice of pivot determines Quick Sort's efficiency.
A bad pivot (smallest or largest) gives O(n²).
A good pivot (median) gives O(n log n).

```c
/*
 * Median-of-Three Pivot Selection
 * Choose median of first, middle, and last elements
 */
static size_t median_of_three(Part *arr, size_t low, size_t high,
                              PartComparator cmp) {
    size_t mid = low + (high - low) / 2;

    /* Sort low, mid, high */
    if (cmp(&arr[mid], &arr[low]) < 0) swap(&arr[mid], &arr[low]);
    if (cmp(&arr[high], &arr[low]) < 0) swap(&arr[high], &arr[low]);
    if (cmp(&arr[high], &arr[mid]) < 0) swap(&arr[high], &arr[mid]);

    /* Place median at high-1 for Lomuto partitioning */
    swap(&arr[mid], &arr[high - 1]);
    return high - 1;
}

/*
 * Quick Sort with Median-of-Three
 */
static void quicksort_median3(Part *arr, size_t low, size_t high,
                               PartComparator cmp) {
    if (high - low + 1 <= INSERTION_THRESHOLD) {
        /* Use insertion sort for small subarrays */
        for (size_t i = low + 1; i <= high; i++) {
            Part key = arr[i];
            size_t j = i;
            while (j > low && cmp(&arr[j - 1], &key) > 0) {
                arr[j] = arr[j - 1];
                j--;
            }
            arr[j] = key;
        }
        return;
    }

    size_t pivot_idx = median_of_three(arr, low, high, cmp);
    Part pivot = arr[pivot_idx];

    /* Partition using Hoare-like scheme */
    size_t i = low;
    size_t j = high - 1;

    while (true) {
        while (cmp(&arr[++i], &pivot) < 0);
        while (cmp(&arr[--j], &pivot) > 0);

        if (i >= j) break;
        swap(&arr[i], &arr[j]);
    }

    swap(&arr[i], &arr[high - 1]);  /* Restore pivot */

    if (i > low + 1) quicksort_median3(arr, low, i - 1, cmp);
    if (i < high - 1) quicksort_median3(arr, i + 1, high, cmp);
}
```

### Three-Way Partition (Dutch National Flag)

Handles many duplicate elements efficiently:

```c
/*
 * Three-Way Partition (Dijkstra's Dutch National Flag)
 *
 * Partitions into: [< pivot] [== pivot] [> pivot]
 * Excellent for arrays with many duplicates
 */
static void partition_three_way(Part *arr, size_t low, size_t high,
                                 size_t *lt, size_t *gt,
                                 PartComparator cmp) {
    Part pivot = arr[low];
    size_t i = low;
    *lt = low;
    *gt = high;

    while (i <= *gt) {
        int result = cmp(&arr[i], &pivot);

        if (result < 0) {
            swap(&arr[*lt], &arr[i]);
            (*lt)++;
            i++;
        } else if (result > 0) {
            swap(&arr[i], &arr[*gt]);
            (*gt)--;
        } else {
            i++;
        }
    }
}

/*
 * Quick Sort with Three-Way Partition
 */
static void quicksort_3way(Part *arr, size_t low, size_t high,
                            PartComparator cmp) {
    if (low >= high) return;

    size_t lt, gt;
    partition_three_way(arr, low, high, &lt, &gt, cmp);

    /* All elements in [lt..gt] are equal to pivot */
    if (lt > 0) quicksort_3way(arr, low, lt - 1, cmp);
    quicksort_3way(arr, gt + 1, high, cmp);
}
```

### Introsort — The Best of Both Worlds

```c
/*
 * Introsort — Quick Sort with depth limit, falls back to Heap Sort
 *
 * Used by: C++ std::sort, .NET Array.Sort
 * Guarantees O(n log n) worst case while being fast on average
 */
static void introsort_impl(Part *arr, size_t low, size_t high,
                            size_t depth_limit, PartComparator cmp);

/* Forward declaration — Heap Sort will be in Part III-C */
void heap_sort(Part *arr, size_t n, PartComparator cmp);

static void introsort_impl(Part *arr, size_t low, size_t high,
                            size_t depth_limit, PartComparator cmp) {
    size_t size = high - low + 1;

    /* Use insertion sort for small subarrays */
    if (size <= INSERTION_THRESHOLD) {
        for (size_t i = low + 1; i <= high; i++) {
            Part key = arr[i];
            size_t j = i;
            while (j > low && cmp(&arr[j - 1], &key) > 0) {
                arr[j] = arr[j - 1];
                j--;
            }
            arr[j] = key;
        }
        return;
    }

    /* Fall back to heap sort if recursion too deep */
    if (depth_limit == 0) {
        /* Sort this subarray with heap sort */
        heap_sort(&arr[low], size, cmp);
        return;
    }

    /* Quick Sort with median-of-three pivot */
    size_t pivot_idx = partition_hoare(arr, low, high, cmp);

    introsort_impl(arr, low, pivot_idx, depth_limit - 1, cmp);
    introsort_impl(arr, pivot_idx + 1, high, depth_limit - 1, cmp);
}

void introsort(Part *arr, size_t n, PartComparator cmp) {
    if (n <= 1) return;

    /* Depth limit: 2 * log2(n) */
    size_t depth_limit = 0;
    for (size_t k = n; k > 0; k >>= 1) depth_limit++;
    depth_limit *= 2;

    introsort_impl(arr, 0, n - 1, depth_limit, cmp);
}
```

---

## Comparison of Construction Sorts

```
╔══════════════════════════════════════════════════════════════════════════════╗
║              THE CONSTRUCTION TABLET OF DIVIDE AND CONQUER                   ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  Algorithm    │ Best      │ Average   │ Worst     │ Space   │ Stable │ Notes║
╠══════════════╪═══════════╪═══════════╪═══════════╪═════════╪════════╪══════╣
║  Merge Sort  │ O(n log n)│ O(n log n)│ O(n log n)│ O(n)    │ Yes    │ Pred.║
║  Quick Sort  │ O(n log n)│ O(n log n)│ O(n²)     │ O(log n)│ No     │ Fast ║
║  Introsort   │ O(n log n)│ O(n log n)│ O(n log n)│ O(log n)│ No     │ Hybrid
╠══════════════╧═══════════╧═══════════╧═══════════╧═════════╧════════╧══════╣
║  Merge Sort: Predictable, stable, great for external sorting, linked lists ║
║  Quick Sort: Fastest in practice, cache-friendly, but O(n²) possible       ║
║  Introsort: Production choice — Quick Sort speed with guaranteed O(n log n)║
╚══════════════════════════════════════════════════════════════════════════════╝
```

---

## Scavenger Hunt — Clue #14

*"In Sedgewick's Chapter 7, he reveals the true cost of Quick Sort.
He shows that on average, Quick Sort makes about 2N ln N comparisons.
But what happens with the 'killer' input? What sequence of elements
causes Quick Sort to descend into O(n²) madness?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 7.
Find the analysis of Quick Sort's worst case. What input
causes the worst performance with Lomuto's partition?

---

## Scavenger Hunt — Clue #15

*"Bhargava in Chapter 4 teaches recursion through a tale of boxes
within boxes. He then applies this wisdom to Quick Sort.
He uses the metaphor of a 'base case' and 'recursive case.'
What is the base case of Quick Sort?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 4.
Find the Quick Sort explanation. What happens when
the array has 0 or 1 elements?

---

## Exercises — The Trials of Construction

### Trial 1: The Stable Parts Sorter
```c
/*
 * Sort parts by multiple keys using stable sort:
 * 1. Primary: category (string)
 * 2. Secondary: price (ascending)
 * 3. Tertiary: part_number (alphabetical)
 *
 * Use Merge Sort's stability to achieve this.
 */
```

### Trial 2: The Quick Select Challenge
```c
/*
 * Implement Quick Select to find the k-th smallest part
 * without fully sorting the array. O(n) average time.
 *
 * Part *quick_select(Part *arr, size_t n, size_t k, PartComparator cmp);
 */
```

### Trial 3: The External Sort
```c
/*
 * Sort a file too large to fit in memory using Merge Sort.
 *
 * 1. Read chunks that fit in memory, sort, write to temp files
 * 2. Merge sorted temp files using k-way merge
 * 3. Handle memory efficiently with buffered I/O
 */
```

---

*Thus ends the second chapter of Khazadgarmâ.
We have learned the powerful arts of division,
Merge Sort and Quick Sort — the workhorses of computation.
In the final chapter, we ascend to the master crafts,
Heap Sort and the non-comparison sorts.*

---

# Part III-C: The Master Crafts — Heaps and Beyond

*In which we learn the highest arts of the Dwarven smiths:
Heap Sort, and the magical non-comparison sorts that
transcend the n log n barrier.*

## The Binary Heap — The Pyramid of Priorities

*"In the great halls of Erebor, the Dwarves built pyramids
of treasure, where the most valuable gem always sat atop.
No matter how gems were added or removed, the greatest
always rose to the peak."*

A **Binary Heap** is a complete binary tree stored in an array,
with the heap property: each parent is greater (max-heap)
or smaller (min-heap) than its children.

```
Max-Heap Visualization:

        Tree View:              Array View:
           [90]                 [90, 80, 70, 50, 60, 40, 30]
          /    \                  0   1   2   3   4   5   6
        [80]   [70]
       /   \   /   \           Parent of i: (i-1)/2
     [50] [60][40] [30]        Left child:  2*i + 1
                               Right child: 2*i + 2

Heap Property: arr[parent] >= arr[left] && arr[parent] >= arr[right]
```

```c
/*
 * Binary Heap Operations
 */

/* Get parent index */
static inline size_t heap_parent(size_t i) {
    return (i - 1) / 2;
}

/* Get left child index */
static inline size_t heap_left(size_t i) {
    return 2 * i + 1;
}

/* Get right child index */
static inline size_t heap_right(size_t i) {
    return 2 * i + 2;
}

/*
 * Heapify Down (Sift Down)
 * Restore heap property by moving element down
 */
static void heapify_down(Part *arr, size_t n, size_t i,
                         PartComparator cmp) {
    size_t largest = i;
    size_t left = heap_left(i);
    size_t right = heap_right(i);

    /* Find largest among parent and children */
    if (left < n && cmp(&arr[left], &arr[largest]) > 0) {
        largest = left;
    }
    if (right < n && cmp(&arr[right], &arr[largest]) > 0) {
        largest = right;
    }

    /* If largest is not parent, swap and continue */
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify_down(arr, n, largest, cmp);
    }
}

/*
 * Heapify Up (Sift Up)
 * Restore heap property by moving element up
 */
static void heapify_up(Part *arr, size_t i, PartComparator cmp) {
    while (i > 0) {
        size_t parent = heap_parent(i);

        if (cmp(&arr[i], &arr[parent]) <= 0) {
            break;  /* Heap property satisfied */
        }

        swap(&arr[i], &arr[parent]);
        i = parent;
    }
}

/*
 * Build Heap from unordered array — O(n)
 *
 * Start from last non-leaf node and heapify down each.
 * Counter-intuitively, this is O(n), not O(n log n)!
 */
static void build_heap(Part *arr, size_t n, PartComparator cmp) {
    /* Start from last non-leaf node */
    for (size_t i = n / 2; i > 0; i--) {
        heapify_down(arr, n, i - 1, cmp);
    }
}
```

---

## Heap Sort — Extracting Order from the Pyramid

*"Extract the greatest gem from the peak, place it at the end.
Restore the heap. Repeat until all gems are sorted."*

```
Heap Sort Visualization:

Build max-heap:
[4, 10, 3, 5, 1] → [10, 5, 3, 4, 1]

Extract max, rebuild:
[10, 5, 3, 4, 1] → swap 10 with 1 → [1, 5, 3, 4, 10] → heapify → [5, 4, 3, 1, |10]
[5, 4, 3, 1] → swap 5 with 1 → [1, 4, 3, 5] → heapify → [4, 1, 3, |5, 10]
[4, 1, 3] → swap 4 with 3 → [3, 1, 4] → heapify → [3, 1, |4, 5, 10]
[3, 1] → swap 3 with 1 → [1, 3] → [1, |3, 4, 5, 10]
[1] → done → [1, 3, 4, 5, 10]
```

```c
/*
 * Heap Sort — O(n log n) always
 *
 * The pyramid of priorities: build a heap,
 * then repeatedly extract the maximum.
 *
 * Properties:
 * - Stable: No
 * - In-place: Yes
 * - Adaptive: No
 * - Cache-unfriendly: Random access pattern
 */
void heap_sort(Part *arr, size_t n, PartComparator cmp) {
    if (n <= 1) return;

    /* Build max-heap — O(n) */
    build_heap(arr, n, cmp);

    /* Extract elements one by one — O(n log n) */
    for (size_t i = n - 1; i > 0; i--) {
        /* Move current max to end */
        swap(&arr[0], &arr[i]);

        /* Restore heap property for reduced heap */
        heapify_down(arr, i, 0, cmp);
    }
}

/*
 * Iterative Heapify Down — avoids recursion overhead
 */
static void heapify_down_iterative(Part *arr, size_t n, size_t i,
                                    PartComparator cmp) {
    while (true) {
        size_t largest = i;
        size_t left = heap_left(i);
        size_t right = heap_right(i);

        if (left < n && cmp(&arr[left], &arr[largest]) > 0) {
            largest = left;
        }
        if (right < n && cmp(&arr[right], &arr[largest]) > 0) {
            largest = right;
        }

        if (largest == i) break;

        swap(&arr[i], &arr[largest]);
        i = largest;
    }
}

/*
 * Bottom-Up Heap Sort (Floyd's improvement)
 * Faster in practice — fewer comparisons during heapify
 */
static void heapify_bottom_up(Part *arr, size_t n, size_t i,
                               PartComparator cmp) {
    Part value = arr[i];
    size_t child;

    /* Sift hole down to leaf */
    while ((child = heap_left(i)) < n) {
        /* Choose larger child */
        if (child + 1 < n && cmp(&arr[child + 1], &arr[child]) > 0) {
            child++;
        }
        arr[i] = arr[child];
        i = child;
    }

    /* Sift value up from the leaf */
    while (i > 0) {
        size_t parent = heap_parent(i);
        if (cmp(&value, &arr[parent]) <= 0) break;
        arr[i] = arr[parent];
        i = parent;
    }

    arr[i] = value;
}
```

---

## Priority Queue — The Heap's True Purpose

*"The heap is not merely for sorting, but for maintaining
priority among ever-changing elements."*

```c
/*
 * Priority Queue using Binary Heap
 */
typedef struct {
    Part *heap;
    size_t count;
    size_t capacity;
    PartComparator cmp;
} PriorityQueue;

PriorityQueue *pq_create(size_t initial_capacity, PartComparator cmp) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;

    pq->heap = malloc(initial_capacity * sizeof(Part));
    if (!pq->heap) {
        free(pq);
        return NULL;
    }

    pq->count = 0;
    pq->capacity = initial_capacity;
    pq->cmp = cmp;
    return pq;
}

void pq_destroy(PriorityQueue *pq) {
    if (pq) {
        free(pq->heap);
        free(pq);
    }
}

/*
 * Insert — O(log n)
 */
bool pq_insert(PriorityQueue *pq, const Part *part) {
    /* Grow if needed */
    if (pq->count >= pq->capacity) {
        size_t new_capacity = pq->capacity * 2;
        Part *new_heap = realloc(pq->heap, new_capacity * sizeof(Part));
        if (!new_heap) return false;
        pq->heap = new_heap;
        pq->capacity = new_capacity;
    }

    /* Add at end and sift up */
    pq->heap[pq->count] = *part;
    heapify_up(pq->heap, pq->count, pq->cmp);
    pq->count++;
    return true;
}

/*
 * Extract Maximum — O(log n)
 */
bool pq_extract_max(PriorityQueue *pq, Part *out_part) {
    if (pq->count == 0) return false;

    if (out_part) {
        *out_part = pq->heap[0];
    }

    /* Move last to root and sift down */
    pq->count--;
    pq->heap[0] = pq->heap[pq->count];
    heapify_down(pq->heap, pq->count, 0, pq->cmp);

    return true;
}

/*
 * Peek Maximum — O(1)
 */
Part *pq_peek_max(PriorityQueue *pq) {
    if (pq->count == 0) return NULL;
    return &pq->heap[0];
}

/*
 * Increase Key — O(log n)
 * Increase priority of element at index i
 */
void pq_increase_key(PriorityQueue *pq, size_t i, const Part *new_part) {
    if (pq->cmp(new_part, &pq->heap[i]) < 0) {
        return;  /* New value is not greater */
    }

    pq->heap[i] = *new_part;
    heapify_up(pq->heap, i, pq->cmp);
}
```

---

## The Non-Comparison Sorts — Breaking the Barrier

*"The wisest of the Dwarven smiths discovered that some treasures
need not be compared to be sorted. If you know the treasure's
properties, you can place it directly in its destined chamber."*

### The Lower Bound of Comparison Sorts

Any comparison-based sort must make at least Ω(n log n) comparisons
in the worst case. This is because n! permutations require
log₂(n!) ≈ n log n comparisons to distinguish.

But non-comparison sorts use other properties (integer values,
digit structure) to achieve O(n) time!

---

## Counting Sort — The Census Method

*"Count the occurrence of each ore type, then place them
in order by their counts."*

```c
/*
 * Counting Sort — O(n + k) where k is the range of values
 *
 * Works only for integer keys with known, limited range.
 *
 * Properties:
 * - Stable: Yes (with proper implementation)
 * - In-place: No (requires O(n + k) space)
 * - Only for integers with bounded range
 */

/* Sort parts by quantity (assumed to be in range 0..max_qty) */
bool counting_sort_by_quantity(Part *arr, size_t n, int max_qty) {
    if (n <= 1) return true;

    /* Allocate count array */
    int *count = calloc(max_qty + 1, sizeof(int));
    if (!count) return false;

    /* Allocate output array */
    Part *output = malloc(n * sizeof(Part));
    if (!output) {
        free(count);
        return false;
    }

    /* Count occurrences */
    for (size_t i = 0; i < n; i++) {
        count[arr[i].quantity]++;
    }

    /* Transform count to positions (cumulative sum) */
    for (int i = 1; i <= max_qty; i++) {
        count[i] += count[i - 1];
    }

    /* Build output array (iterate backwards for stability) */
    for (size_t i = n; i > 0; i--) {
        int qty = arr[i - 1].quantity;
        output[count[qty] - 1] = arr[i - 1];
        count[qty]--;
    }

    /* Copy back to original array */
    memcpy(arr, output, n * sizeof(Part));

    free(output);
    free(count);
    return true;
}

/*
 * Counting Sort Visualization:
 *
 * Input: [4, 2, 2, 8, 3, 3, 1]
 *
 * Step 1: Count occurrences
 * count[]: [0, 1, 2, 2, 1, 0, 0, 0, 1]
 *              1  2  3  4           8
 *
 * Step 2: Cumulative sum (positions)
 * count[]: [0, 1, 3, 5, 6, 6, 6, 6, 7]
 *              ↑  ↑  ↑  ↑           ↑
 *              1 goes at 0
 *                 2s go at 1-2
 *                    3s go at 3-4
 *                       4 goes at 5
 *                                   8 goes at 6
 *
 * Step 3: Place elements
 * Output: [1, 2, 2, 3, 3, 4, 8]
 */
```

---

## Radix Sort — The Digit-by-Digit Method

*"The Dwarven scribes sorted treasure records by examining
each digit of the vault number, from least significant to most."*

```c
/*
 * Radix Sort (LSD - Least Significant Digit first)
 * O(d × (n + b)) where d = digits, b = base
 *
 * Uses Counting Sort as a stable subroutine for each digit.
 */

/* Get digit at position (0 = least significant) */
static int get_digit(int num, int position, int base) {
    for (int i = 0; i < position; i++) {
        num /= base;
    }
    return num % base;
}

/* Get number of digits in max value */
static int get_max_digits(Part *arr, size_t n, int base) {
    int max_val = 0;
    for (size_t i = 0; i < n; i++) {
        if (arr[i].quantity > max_val) {
            max_val = arr[i].quantity;
        }
    }

    int digits = 0;
    while (max_val > 0) {
        digits++;
        max_val /= base;
    }
    return digits > 0 ? digits : 1;
}

/*
 * Radix Sort using base 10
 */
bool radix_sort_by_quantity(Part *arr, size_t n) {
    if (n <= 1) return true;

    const int BASE = 10;
    int max_digits = get_max_digits(arr, n, BASE);

    Part *output = malloc(n * sizeof(Part));
    int *count = malloc(BASE * sizeof(int));
    if (!output || !count) {
        free(output);
        free(count);
        return false;
    }

    /* Sort by each digit, starting from least significant */
    for (int d = 0; d < max_digits; d++) {
        /* Reset count array */
        memset(count, 0, BASE * sizeof(int));

        /* Count occurrences of each digit */
        for (size_t i = 0; i < n; i++) {
            int digit = get_digit(arr[i].quantity, d, BASE);
            count[digit]++;
        }

        /* Transform to positions */
        for (int i = 1; i < BASE; i++) {
            count[i] += count[i - 1];
        }

        /* Build output (backwards for stability) */
        for (size_t i = n; i > 0; i--) {
            int digit = get_digit(arr[i - 1].quantity, d, BASE);
            output[count[digit] - 1] = arr[i - 1];
            count[digit]--;
        }

        /* Copy back */
        memcpy(arr, output, n * sizeof(Part));
    }

    free(output);
    free(count);
    return true;
}

/*
 * Radix Sort Visualization (base 10):
 *
 * Input: [170, 45, 75, 90, 802, 24, 2, 66]
 *
 * Sort by ones digit (d=0):
 * [170, 90, 802, 2, 24, 45, 75, 66]
 *    0   0    2  2   4   5   5   6
 *
 * Sort by tens digit (d=1):
 * [802, 2, 24, 45, 66, 170, 75, 90]
 *   0   0   2   4   6    7   7   9
 *
 * Sort by hundreds digit (d=2):
 * [2, 24, 45, 66, 75, 90, 170, 802]
 *  0   0   0   0   0   0    1    8
 *
 * Done!
 */
```

---

## Bucket Sort — The Compartment System

*"Divide the treasures into compartments by value range,
sort each compartment, then concatenate."*

```c
/*
 * Bucket Sort — O(n + k) average for uniformly distributed data
 *
 * Assumes input is uniformly distributed in [0, 1) or can be mapped to it.
 */

typedef struct BucketNode {
    Part data;
    struct BucketNode *next;
} BucketNode;

bool bucket_sort_by_price(Part *arr, size_t n, double max_price) {
    if (n <= 1) return true;

    /* Create n buckets */
    BucketNode **buckets = calloc(n, sizeof(BucketNode *));
    if (!buckets) return false;

    /* Distribute elements into buckets */
    for (size_t i = 0; i < n; i++) {
        /* Map price to bucket index */
        size_t bucket_idx = (size_t)((arr[i].price / max_price) * (n - 1));
        if (bucket_idx >= n) bucket_idx = n - 1;

        /* Insert into bucket (using insertion sort order) */
        BucketNode *node = malloc(sizeof(BucketNode));
        if (!node) {
            /* Cleanup on error */
            for (size_t j = 0; j < n; j++) {
                BucketNode *curr = buckets[j];
                while (curr) {
                    BucketNode *next = curr->next;
                    free(curr);
                    curr = next;
                }
            }
            free(buckets);
            return false;
        }
        node->data = arr[i];

        /* Insert in sorted order within bucket */
        BucketNode **pp = &buckets[bucket_idx];
        while (*pp && (*pp)->data.price < node->data.price) {
            pp = &(*pp)->next;
        }
        node->next = *pp;
        *pp = node;
    }

    /* Concatenate buckets back to array */
    size_t idx = 0;
    for (size_t i = 0; i < n; i++) {
        BucketNode *curr = buckets[i];
        while (curr) {
            arr[idx++] = curr->data;
            BucketNode *next = curr->next;
            free(curr);
            curr = next;
        }
    }

    free(buckets);
    return true;
}
```

---

## Comparison of Master Crafts

```
╔══════════════════════════════════════════════════════════════════════════════╗
║              THE MASTER CRAFT TABLET OF ADVANCED SORTS                       ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  Algorithm     │ Best      │ Average   │ Worst     │ Space  │ Stable │ Notes║
╠══════════════╪═══════════╪═══════════╪═══════════╪════════╪════════╪══════╣
║  Heap Sort    │ O(n log n)│ O(n log n)│ O(n log n)│ O(1)   │ No     │In-pl.║
║  Counting Sort│ O(n + k)  │ O(n + k)  │ O(n + k)  │ O(n+k) │ Yes    │ Int  ║
║  Radix Sort   │ O(d(n+b)) │ O(d(n+b)) │ O(d(n+b)) │ O(n+b) │ Yes    │ Int  ║
║  Bucket Sort  │ O(n + k)  │ O(n + k)  │ O(n²)     │ O(n+k) │ Yes    │ Unif.║
╠══════════════╧═══════════╧═══════════╧═══════════╧════════╧════════╧══════╣
║  k = range of values, d = number of digits, b = base                        ║
║  Heap Sort: Guaranteed O(n log n), but cache-unfriendly                     ║
║  Counting/Radix: O(n) when k is O(n), only for integers                     ║
║  Bucket Sort: O(n) average for uniform data, O(n²) worst                    ║
╚══════════════════════════════════════════════════════════════════════════════╝
```

---

## The Complete Sorting Decision Tree

```
                    ┌─────────────────────────────────────┐
                    │        How should I sort?           │
                    └─────────────────┬───────────────────┘
                                      │
                    ┌─────────────────▼───────────────────┐
                    │  Is n small (< 20)?                 │
                    └─────────────────┬───────────────────┘
                           Yes        │        No
                            │         │         │
                    ┌───────▼───────┐ │ ┌───────▼───────────────────┐
                    │ Insertion Sort│ │ │ Are elements integers     │
                    │ (cache-fast)  │ │ │ with known, small range?  │
                    └───────────────┘ │ └───────┬───────────────────┘
                                      │    Yes  │           No
                                      │    │    │            │
                              ┌───────▼────▼┐   │   ┌────────▼────────┐
                              │Counting/Radix│   │   │ Need stability? │
                              │  O(n) sorts  │   │   └────────┬────────┘
                              └──────────────┘   │       Yes  │    No
                                                 │        │   │     │
                                        ┌────────▼────────▼┐  │ ┌───▼────────┐
                                        │   Merge Sort     │  │ │ Introsort  │
                                        │ (stable O(n lg n)│  │ │(Quick+Heap)│
                                        └──────────────────┘  │ └────────────┘
                                                              │
                                        ┌─────────────────────▼────────────────┐
                                        │ Memory constrained?                  │
                                        └─────────────────────┬────────────────┘
                                                 Yes          │          No
                                                  │           │           │
                                          ┌───────▼────┐      │   ┌───────▼──────┐
                                          │ Heap Sort  │      │   │ Quick Sort   │
                                          │ (in-place) │      │   │ (cache-fast) │
                                          └────────────┘      │   └──────────────┘
```

---

## Scavenger Hunt — Clue #16

*"In Sedgewick's Chapter 9, he reveals the secret of Radix Sort.
He shows that for sorting n integers in the range 0 to M,
we can achieve O(n) time when M is O(n^k) for constant k.
What is the relationship between the number of passes
and the base used for digit extraction?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 9.
Find the analysis of LSD Radix Sort. How do you choose
the optimal base?

---

## Scavenger Hunt — Clue #17

*"Bhargava speaks of a farmer counting boxes in Chapter 4,
but in Chapter 5 hash tables, he hints at how counting
can be used for sorting. He also mentions that sometimes
the simplest approach (like selection sort) is good enough
for small datasets. What does 'good enough' mean in Big-O terms?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 5.
Find where he discusses when O(n²) is acceptable.
What trade-off does he describe?

---

## Exercises — The Trials of the Master Crafts

### Trial 1: The Priority Scheduler
```c
/*
 * Implement a task scheduler using a priority queue.
 *
 * typedef struct {
 *     char task_name[50];
 *     int priority;      // Higher = more urgent
 *     time_t deadline;
 * } Task;
 *
 * Requirements:
 * 1. Add tasks with priorities
 * 2. Always execute highest priority task next
 * 3. Support priority updates
 * 4. Handle deadline-based priority boost
 */
```

### Trial 2: The Inventory Counter
```c
/*
 * Use Counting Sort to generate a histogram of parts by quantity.
 *
 * Requirements:
 * 1. Count parts with quantities 0-99
 * 2. Display bar chart of quantity distribution
 * 3. Support adding/removing parts and updating histogram
 */
```

### Trial 3: The Grand Sort-Off
```c
/*
 * Implement a benchmark comparing all sorting algorithms.
 *
 * Test on:
 * 1. Random data
 * 2. Nearly sorted data
 * 3. Reverse sorted data
 * 4. Many duplicates
 * 5. Already sorted data
 *
 * Measure: comparisons, swaps, time, memory
 */
```

---

*Thus ends the Khazadgarmâ of the Sorting Arts.
From the simple mining sorts to the master crafts,
we have learned to bring order from chaos.
In the next part, we shall explore the Railway Networks,
graphs and the paths that connect all things.*

---

[Continue to Part IV: The Dwarven Railways — Graph Algorithms →](./THE_SILMARILLION_OF_ALGORITHMS_PART4.md)

---
