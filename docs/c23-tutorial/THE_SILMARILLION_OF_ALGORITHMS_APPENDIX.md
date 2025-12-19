# The Silmarillion of Algorithms - Appendix

## Baruk Khazâd! - The Dwarven Reference Tables

*"And here at the end of all things, we gather the wisdom of the ages
into tables of stone, that future craftsmen may quickly find their way."*

---

## Table of Contents

- [A. Complexity Quick Reference](#a-complexity-quick-reference)
  - [A.1 Data Structure Operations](#a1-data-structure-operations)
  - [A.2 Sorting Algorithm Comparison](#a2-sorting-algorithm-comparison)
  - [A.3 Graph Algorithm Complexity](#a3-graph-algorithm-complexity)
  - [A.4 Search Algorithm Comparison](#a4-search-algorithm-comparison)
- [B. Code Index](#b-code-index)
  - [B.1 Data Structures by Part](#b1-data-structures-by-part)
  - [B.2 Algorithms by Part](#b2-algorithms-by-part)
  - [B.3 Complete Function Reference](#b3-complete-function-reference)
- [C. Dwarven Glossary](#c-dwarven-glossary)
- [D. Scavenger Hunt Master List](#d-scavenger-hunt-master-list)
- [E. PartsDB Complete API](#e-partsdb-complete-api)
- [F. Memory Layout Diagrams](#f-memory-layout-diagrams)
- [G. Decision Trees](#g-decision-trees)

---

# A. Complexity Quick Reference

## A.1 Data Structure Operations

### Arrays and Lists

| Structure | Access | Search | Insert (End) | Insert (Mid) | Delete (End) | Delete (Mid) | Space |
|-----------|--------|--------|--------------|--------------|--------------|--------------|-------|
| Static Array | O(1) | O(n) | N/A | N/A | N/A | N/A | O(n) |
| Dynamic Array | O(1) | O(n) | O(1)* | O(n) | O(1) | O(n) | O(n) |
| Singly Linked | O(n) | O(n) | O(1)† | O(1)‡ | O(n) | O(1)‡ | O(n) |
| Doubly Linked | O(n) | O(n) | O(1) | O(1)‡ | O(1) | O(1)‡ | O(n) |
| Circular List | O(n) | O(n) | O(1) | O(1)‡ | O(1) | O(1)‡ | O(n) |
| Skip List | O(log n) | O(log n) | O(log n) | O(log n) | O(log n) | O(log n) | O(n) |

*Amortized. †With tail pointer. ‡Given node pointer.

### Trees

| Structure | Search | Insert | Delete | Min/Max | Space |
|-----------|--------|--------|--------|---------|-------|
| Binary Search Tree | O(h) | O(h) | O(h) | O(h) | O(n) |
| AVL Tree | O(log n) | O(log n) | O(log n) | O(log n) | O(n) |
| Red-Black Tree | O(log n) | O(log n) | O(log n) | O(log n) | O(n) |
| B-Tree (order m) | O(log n) | O(log n) | O(log n) | O(log n) | O(n) |
| Trie (key len k) | O(k) | O(k) | O(k) | O(k) | O(ALPHABET * n * k) |
| Binary Heap | O(n) | O(log n) | O(log n) | O(1)* | O(n) |

*Min for min-heap, Max for max-heap. h = height (worst case n for BST).

### Hash Tables

| Operation | Average | Worst Case | Notes |
|-----------|---------|------------|-------|
| Search | O(1) | O(n) | With good hash function |
| Insert | O(1) | O(n) | Amortized with resizing |
| Delete | O(1) | O(n) | With tombstones or chaining |
| Space | O(n) | O(n) | Load factor typically 0.7-0.8 |

### Graphs (V = vertices, E = edges)

| Representation | Space | Add Vertex | Add Edge | Remove Edge | Query Edge | Iterate Neighbors |
|----------------|-------|------------|----------|-------------|------------|-------------------|
| Adjacency Matrix | O(V²) | O(V²) | O(1) | O(1) | O(1) | O(V) |
| Adjacency List | O(V+E) | O(1) | O(1) | O(E) | O(V) | O(degree) |
| Edge List | O(E) | O(1) | O(1) | O(E) | O(E) | O(E) |

---

## A.2 Sorting Algorithm Comparison

### Comparison-Based Sorts

| Algorithm | Best | Average | Worst | Space | Stable | Adaptive | Method |
|-----------|------|---------|-------|-------|--------|----------|--------|
| Bubble Sort | O(n) | O(n²) | O(n²) | O(1) | Yes | Yes | Exchange |
| Selection Sort | O(n²) | O(n²) | O(n²) | O(1) | No | No | Selection |
| Insertion Sort | O(n) | O(n²) | O(n²) | O(1) | Yes | Yes | Insertion |
| Shell Sort | O(n log n) | O(n^1.3) | O(n²) | O(1) | No | Yes | Insertion |
| Merge Sort | O(n log n) | O(n log n) | O(n log n) | O(n) | Yes | No | Divide & Conquer |
| Quick Sort | O(n log n) | O(n log n) | O(n²) | O(log n) | No | No | Divide & Conquer |
| Heap Sort | O(n log n) | O(n log n) | O(n log n) | O(1) | No | No | Selection |
| Introsort | O(n log n) | O(n log n) | O(n log n) | O(log n) | No | Yes | Hybrid |
| Tim Sort | O(n) | O(n log n) | O(n log n) | O(n) | Yes | Yes | Hybrid |

### Non-Comparison Sorts

| Algorithm | Best | Average | Worst | Space | Stable | Constraint |
|-----------|------|---------|-------|-------|--------|------------|
| Counting Sort | O(n+k) | O(n+k) | O(n+k) | O(k) | Yes | k = range of values |
| Radix Sort (LSD) | O(d(n+k)) | O(d(n+k)) | O(d(n+k)) | O(n+k) | Yes | d = digits, k = base |
| Bucket Sort | O(n+k) | O(n+k) | O(n²) | O(n+k) | Yes | Uniform distribution |

### When to Use Which Sort

```
┌─────────────────────────────────────────────────────────────────┐
│                    SORTING DECISION TREE                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Is n < 50?                                                     │
│  ├─ YES → Insertion Sort (low overhead, cache-friendly)        │
│  └─ NO ↓                                                        │
│                                                                 │
│  Are values integers with small range?                          │
│  ├─ YES → Counting Sort or Radix Sort                          │
│  └─ NO ↓                                                        │
│                                                                 │
│  Is stability required?                                         │
│  ├─ YES → Merge Sort (guaranteed O(n log n), stable)           │
│  └─ NO ↓                                                        │
│                                                                 │
│  Is memory constrained (embedded/kernel)?                       │
│  ├─ YES → Heap Sort (O(1) space, guaranteed O(n log n))        │
│  └─ NO ↓                                                        │
│                                                                 │
│  Is data nearly sorted?                                         │
│  ├─ YES → Insertion Sort or Tim Sort                           │
│  └─ NO ↓                                                        │
│                                                                 │
│  General purpose → Introsort (Quick + Heap + Insertion)        │
│                    or library qsort()                           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## A.3 Graph Algorithm Complexity

### Traversal and Search

| Algorithm | Time | Space | Notes |
|-----------|------|-------|-------|
| BFS | O(V+E) | O(V) | Shortest path (unweighted) |
| DFS (recursive) | O(V+E) | O(V) | Stack depth = path length |
| DFS (iterative) | O(V+E) | O(V) | Explicit stack |
| Topological Sort | O(V+E) | O(V) | DAG only |

### Shortest Path

| Algorithm | Time | Space | Negative Weights | Notes |
|-----------|------|-------|------------------|-------|
| Dijkstra (binary heap) | O((V+E) log V) | O(V) | No | Single source |
| Dijkstra (Fibonacci heap) | O(E + V log V) | O(V) | No | Theoretical |
| Bellman-Ford | O(VE) | O(V) | Yes | Detects negative cycles |
| Floyd-Warshall | O(V³) | O(V²) | Yes | All pairs |
| A* | O(E) to O(b^d) | O(V) | No | Heuristic-based |

### Minimum Spanning Tree

| Algorithm | Time | Space | Notes |
|-----------|------|-------|-------|
| Kruskal's | O(E log E) | O(V) | Sort edges + Union-Find |
| Prim's (binary heap) | O((V+E) log V) | O(V) | Similar to Dijkstra |
| Prim's (Fibonacci heap) | O(E + V log V) | O(V) | Dense graphs |
| Borůvka's | O(E log V) | O(V) | Parallelizable |

### Network Flow

| Algorithm | Time | Space | Notes |
|-----------|------|-------|-------|
| Ford-Fulkerson (DFS) | O(E × max_flow) | O(V) | Integer capacities |
| Edmonds-Karp (BFS) | O(VE²) | O(V) | Polynomial guarantee |
| Dinic's | O(V²E) | O(V) | Unit capacity: O(E√V) |
| Push-Relabel | O(V²E) | O(V) | Often faster in practice |

---

## A.4 Search Algorithm Comparison

| Algorithm | Time (Avg) | Time (Worst) | Space | Requires |
|-----------|------------|--------------|-------|----------|
| Linear Search | O(n) | O(n) | O(1) | None |
| Binary Search | O(log n) | O(log n) | O(1) | Sorted array |
| Jump Search | O(√n) | O(√n) | O(1) | Sorted array |
| Interpolation Search | O(log log n) | O(n) | O(1) | Uniform distribution |
| Exponential Search | O(log n) | O(log n) | O(1) | Sorted, unbounded |
| Hash Table Lookup | O(1) | O(n) | O(n) | Hash function |
| BST Search | O(log n) | O(n) | O(n) | Balanced tree |
| Trie Search (key k) | O(k) | O(k) | O(n×k) | String keys |

---

# B. Code Index

## B.1 Data Structures by Part

### Part I: Foundations
*No data structures - theoretical introduction*

### Part II: Data Structures

| Structure | File | Line Reference |
|-----------|------|----------------|
| `PartArray` | PART2.md | Part II-A |
| `PartRingBuffer` | PART2.md | Part II-A |
| `PartStack` | PART2.md | Part II-A |
| `PartQueue` | PART2.md | Part II-A |
| `PartNode` (singly linked) | PART2.md | Part II-B |
| `PartList` (singly linked) | PART2.md | Part II-B |
| `DPartNode` (doubly linked) | PART2.md | Part II-B |
| `DPartList` (doubly linked) | PART2.md | Part II-B |
| `SentinelList` | PART2.md | Part II-B |
| `CircularList` | PART2.md | Part II-B |
| `IntrusiveListNode` | PART2.md | Part II-B |
| `BTreeNode` | PART2.md | Part II-C |
| `AVLNode` | PART2.md | Part II-C |
| `RBNode` | PART2.md | Part II-C |
| `TrieNode` | PART2.md | Part II-C |

### Part III: Sorting

| Structure | File | Purpose |
|-----------|------|---------|
| `PriorityQueue` | PART3.md | Heap-based priority queue |
| `BucketNode` | PART3.md | Bucket sort linked list |

### Part IV: Graphs

| Structure | File | Purpose |
|-----------|------|---------|
| `AdjacencyMatrix` | PART4.md | Dense graph representation |
| `AdjNode` | PART4.md | Adjacency list node |
| `AdjacencyList` | PART4.md | Sparse graph representation |
| `Edge` | PART4.md | Edge list element |
| `EdgeList` | PART4.md | Edge list representation |
| `WarehouseGraph` | PART4.md | PartsDB location graph |
| `BFSResult` | PART4.md | BFS output structure |
| `DFSResult` | PART4.md | DFS output structure |
| `DijkstraResult` | PART4.md | Shortest path output |
| `UnionFind` | PART4.md | Disjoint set structure |
| `MSTResult` | PART4.md | MST output structure |
| `FlowEdge` | PART4.md | Flow network edge |
| `FlowNetwork` | PART4.md | Max flow structure |

### Part V: Communication

| Structure | File | Purpose |
|-----------|------|---------|
| `PowerStation` | PART5.md | Hydroelectric generator |
| `PowerGrid` | PART5.md | Power distribution network |
| `PumpedStorage` | PART5.md | Energy storage system |
| `MorseEntry` | PART5.md | Morse code lookup entry |
| `MorseNode` | PART5.md | Morse tree node |
| `MorseSignal` | PART5.md | Signal timing structure |
| `TelegraphKey` | PART5.md | Input device |
| `TelegraphLine` | PART5.md | Transmission medium |
| `TelegraphRelay` | PART5.md | Signal repeater |
| `TelegraphSounder` | PART5.md | Output device |
| `TelegraphStation` | PART5.md | Complete station |
| `TelegraphNetwork` | PART5.md | Multi-station network |

### Part VI: Integration

| Structure | File | Purpose |
|-----------|------|---------|
| `PartsDB` | PART6.md | Complete parts database |
| `PartIndex` | PART6.md | Multi-key index structure |
| `SearchResult` | PART6.md | Query result container |

---

## B.2 Algorithms by Part

### Part III: Sorting Algorithms

| Algorithm | Function | Complexity |
|-----------|----------|------------|
| Bubble Sort | `bubble_sort()` | O(n²) |
| Optimized Bubble | `bubble_sort_optimized()` | O(n²) |
| Selection Sort | `selection_sort()` | O(n²) |
| Insertion Sort | `insertion_sort()` | O(n²) |
| Binary Insertion | `binary_insertion_sort()` | O(n²) |
| Shell Sort (Ciura) | `shell_sort_ciura()` | O(n^1.3) |
| Shell Sort (Sedgewick) | `shell_sort_sedgewick()` | O(n^4/3) |
| Merge Sort | `merge_sort()` | O(n log n) |
| Bottom-Up Merge | `merge_sort_bottom_up()` | O(n log n) |
| Natural Merge | `natural_merge_sort()` | O(n log n) |
| Quick Sort (Lomuto) | `quick_sort_lomuto()` | O(n log n) avg |
| Quick Sort (Hoare) | `quick_sort_hoare()` | O(n log n) avg |
| Quick Sort (Median) | `quick_sort_median()` | O(n log n) avg |
| Three-Way Quick | `quick_sort_3way()` | O(n log n) avg |
| Introsort | `introsort()` | O(n log n) |
| Heap Sort | `heap_sort()` | O(n log n) |
| Counting Sort | `counting_sort()` | O(n+k) |
| Radix Sort (LSD) | `radix_sort_lsd()` | O(d(n+k)) |
| Bucket Sort | `bucket_sort()` | O(n+k) avg |

### Part IV: Graph Algorithms

| Algorithm | Function | Complexity |
|-----------|----------|------------|
| BFS | `bfs()` | O(V+E) |
| DFS (recursive) | `dfs_recursive()` | O(V+E) |
| DFS (iterative) | `dfs_iterative()` | O(V+E) |
| Topological Sort | `topological_sort()` | O(V+E) |
| Cycle Detection | `has_cycle()` | O(V+E) |
| Connected Components | `find_components()` | O(V+E) |
| Dijkstra | `dijkstra()` | O((V+E) log V) |
| A* Search | `a_star()` | O(E) to O(b^d) |
| Union-Find (init) | `uf_create()` | O(V) |
| Union-Find (find) | `uf_find()` | O(α(n)) |
| Union-Find (union) | `uf_union()` | O(α(n)) |
| Kruskal's MST | `kruskal_mst()` | O(E log E) |
| Prim's MST | `prim_mst()` | O((V+E) log V) |
| Ford-Fulkerson | `ford_fulkerson()` | O(E × max_flow) |
| Edmonds-Karp | `edmonds_karp()` | O(VE²) |
| Bipartite Check | `is_bipartite()` | O(V+E) |
| Bipartite Match | `bipartite_matching()` | O(VE) |

### Part V: Communication Algorithms

| Algorithm | Function | Purpose |
|-----------|----------|---------|
| Morse Encode | `morse_encode()` | Text to Morse |
| Morse Decode | `morse_decode()` | Morse to text |
| Morse Tree Build | `morse_build_tree()` | Binary tree decoder |
| Morse Tree Decode | `morse_tree_decode()` | Tree-based decode |
| Telegraph Transmit | `telegraph_transmit()` | Send message |
| Telegraph Receive | `telegraph_receive()` | Receive message |
| Power Calculate | `power_calculate_load()` | Grid load analysis |

---

## B.3 Complete Function Reference

### Memory Management Pattern
```c
// Every structure follows this pattern:
TypeName* typename_create(...);      // Allocate and initialize
void typename_destroy(TypeName*);    // Free all memory
bool typename_operation(TypeName*);  // Perform operation
```

### Array Functions
```c
PartArray* part_array_create(size_t initial_capacity);
void part_array_destroy(PartArray* arr);
bool part_array_push(PartArray* arr, Part part);
Part* part_array_get(PartArray* arr, size_t index);
Part part_array_pop(PartArray* arr);
size_t part_array_size(PartArray* arr);
void part_array_clear(PartArray* arr);
```

### Linked List Functions
```c
PartList* part_list_create(void);
void part_list_destroy(PartList* list);
bool part_list_prepend(PartList* list, Part part);
bool part_list_append(PartList* list, Part part);
bool part_list_insert_after(PartNode* node, Part part);
Part* part_list_find(PartList* list, const char* part_number);
bool part_list_remove(PartList* list, const char* part_number);
void part_list_foreach(PartList* list, void (*fn)(Part*));
```

### Tree Functions
```c
BTreeNode* btree_insert(BTreeNode* root, Part part);
BTreeNode* btree_search(BTreeNode* root, const char* key);
BTreeNode* btree_delete(BTreeNode* root, const char* key);
void btree_inorder(BTreeNode* root, void (*fn)(Part*));
int btree_height(BTreeNode* root);
void btree_destroy(BTreeNode* root);

AVLNode* avl_insert(AVLNode* root, Part part);
AVLNode* avl_delete(AVLNode* root, const char* key);
int avl_balance_factor(AVLNode* node);
AVLNode* avl_rotate_left(AVLNode* node);
AVLNode* avl_rotate_right(AVLNode* node);
```

### Graph Functions
```c
AdjacencyList* adjlist_create(size_t vertices);
void adjlist_destroy(AdjacencyList* graph);
void adjlist_add_edge(AdjacencyList* graph, size_t from, size_t to, double weight);
void adjlist_remove_edge(AdjacencyList* graph, size_t from, size_t to);
bool adjlist_has_edge(AdjacencyList* graph, size_t from, size_t to);

BFSResult* bfs(AdjacencyList* graph, size_t start);
DFSResult* dfs(AdjacencyList* graph, size_t start);
DijkstraResult* dijkstra(AdjacencyList* graph, size_t start);
MSTResult* kruskal_mst(EdgeList* edges, size_t vertices);
int ford_fulkerson(FlowNetwork* network, size_t source, size_t sink);
```

---

# C. Dwarven Glossary

*A translation of the Khuzdul terms used throughout this work*

| Khuzdul | Translation | Algorithm Concept |
|---------|-------------|-------------------|
| Ainulindalë | Music of the Ainur | Algorithm design, the harmony of computation |
| Valaquenta | Account of the Valar | Data structure fundamentals |
| Khazad-dûm | Dwarrowdelf | Main repository, the great halls of data |
| Khazadgarmâ | Dwarven crafts | Sorting algorithms as forge work |
| Nauglamír | Necklace of the Dwarves | The treasure hunt, mastery challenge |
| Iglishmêk | Dwarven gesture-language | Extended Morse code system |
| Baruk Khazâd | Axes of the Dwarves | Testing assertion, battle cry |
| Khazâd ai-mênu | The Dwarves are upon you | Debugging assertion |
| Zirak-zigil | Silvertine | Peak performance optimization |
| Barazinbar | Redhorn | Error handling, danger zones |
| Bundushathûr | Cloudyhead | Abstraction layers |
| Moria | Black Pit | Memory corruption, the deep bugs |
| Durin | Eldest Dwarf | Core algorithms, foundational code |
| Gimli | Star | Polish, final optimization |

### Dwarven Proverbs in Code

```c
/* "Baruk Khazâd!" - Axes of the Dwarves!
 * Used as assertion prefix in Dwarven tradition */
#define BARUK_KHAZAD(cond) assert((cond) && "Khazâd ai-mênu!")

/* "The wealth of Moria was not in gold or jewels
 * but in mithril" - True value is in the algorithm */
#define MITHRIL_QUALITY(code) /* Optimized hot path */

/* "They delved too greedily and too deep" -
 * Warning against unbounded recursion */
#define DELVE_LIMIT 1000  /* Maximum recursion depth */
```

---

# D. Scavenger Hunt Master List

## Complete Clue Index

| # | Part | Location | Clue | Source Reference |
|---|------|----------|------|------------------|
| 1 | I-A | Origins | Union-Find genesis | Sedgewick Ch. 1.5 |
| 2 | I-A | Origins | Binary search basics | Grokking Ch. 1 |
| 3 | I-B | Fundamentals | Euclidean GCD | Sedgewick Ch. 1 |
| 4 | I-B | Fundamentals | Recursion patterns | Grokking Ch. 3 |
| 5 | I-C | Complexity | Big-O analysis | Sedgewick Ch. 2 / Grokking Ch. 1 |
| 6 | II-A | Arrays | ADT foundations | Sedgewick Ch. 3 |
| 7 | II-A | Arrays | Selection sort intro | Grokking Ch. 2 |
| 8 | II-B | Lists | Elementary list ops | Sedgewick Ch. 3.3 |
| 9 | II-B | Lists | Recursion call stack | Grokking Ch. 3 |
| 10 | II-C | Trees | Symbol table search | Sedgewick Ch. 4 |
| 11 | II-C | Trees | Breadth-first preview | Grokking Ch. 6 |
| 12 | III-A | Mining Sorts | Elementary sorts | Sedgewick Ch. 6 |
| 13 | III-A | Mining Sorts | Selection sort details | Grokking Ch. 2 |
| 14 | III-B | Construction | Quicksort analysis | Sedgewick Ch. 7 |
| 15 | III-B | Construction | Quicksort visualization | Grokking Ch. 4 |
| 16 | III-C | Master Crafts | Priority queues | Sedgewick Ch. 9 |
| 17 | III-C | Master Crafts | Counting sort concept | Grokking Ch. 11 |
| 18 | IV-A | Graph Basics | Graph ADT | Sedgewick Ch. 17 |
| 19 | IV-A | Graph Basics | Graph introduction | Grokking Ch. 6 |
| 20 | IV-B | Pathfinding | Shortest paths | Sedgewick Ch. 18 |
| 21 | IV-B | Pathfinding | Dijkstra's algorithm | Grokking Ch. 7 |
| 22 | IV-C | Networks | MST algorithms | Sedgewick Ch. 20 |
| 23 | V-A | Power | Bit manipulation | Sedgewick Ch. 17.6 |
| 24 | V-B | Morse | Hash tables | Grokking Ch. 5 |
| 25 | V-C | Telegraph | Network simulation | Sedgewick Ch. 17.7 |

## Challenge Summary

### Sedgewick Challenges (S1-S6)
- **S1**: Union-Find variant implementation
- **S2**: GCD call counting analysis
- **S3**: Sorting operation counting
- **S4**: Partition analysis
- **S5**: BST height analysis
- **S6**: Floyd-Warshall implementation

### Grokking Challenges (G1-G5)
- **G1**: Binary search guessing game
- **G2**: Recursive box search
- **G3**: Selection sort step visualization
- **G4**: Hash table collision analysis
- **G5**: BFS mango seller search

---

# E. PartsDB Complete API

## Header File (partsdb.h)

```c
#ifndef PARTSDB_H
#define PARTSDB_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/*============================================================
 * PART STRUCTURE
 *============================================================*/

#define PART_NUMBER_LEN 32
#define PART_NAME_LEN 64
#define PART_LOCATION_LEN 16

typedef struct Part {
    char part_number[PART_NUMBER_LEN];
    char name[PART_NAME_LEN];
    char location[PART_LOCATION_LEN];
    uint32_t quantity;
    uint32_t reorder_point;
    double unit_price;
} Part;

/*============================================================
 * DATABASE STRUCTURE
 *============================================================*/

typedef struct PartsDB PartsDB;

/* Creation and destruction */
PartsDB* partsdb_create(size_t initial_capacity);
void partsdb_destroy(PartsDB* db);

/* Basic operations */
bool partsdb_insert(PartsDB* db, const Part* part);
bool partsdb_update(PartsDB* db, const Part* part);
bool partsdb_delete(PartsDB* db, const char* part_number);

/* Search operations */
Part* partsdb_find_by_number(PartsDB* db, const char* part_number);
Part** partsdb_find_by_name(PartsDB* db, const char* name_prefix, size_t* count);
Part** partsdb_find_by_location(PartsDB* db, const char* location, size_t* count);
Part** partsdb_find_low_stock(PartsDB* db, size_t* count);

/* Traversal */
void partsdb_foreach(PartsDB* db, void (*fn)(Part*, void*), void* user_data);
size_t partsdb_count(PartsDB* db);

/* Sorting */
Part** partsdb_sorted_by_number(PartsDB* db, size_t* count);
Part** partsdb_sorted_by_name(PartsDB* db, size_t* count);
Part** partsdb_sorted_by_quantity(PartsDB* db, size_t* count);

/* Warehouse graph operations */
double partsdb_path_distance(PartsDB* db, const char* from_loc, const char* to_loc);
char** partsdb_shortest_path(PartsDB* db, const char* from, const char* to, size_t* len);
char** partsdb_optimal_pick_route(PartsDB* db, const char** locations, size_t loc_count, size_t* route_len);

/* Persistence */
bool partsdb_save(PartsDB* db, const char* filename);
PartsDB* partsdb_load(const char* filename);

/* Statistics */
typedef struct {
    size_t total_parts;
    size_t unique_locations;
    double total_value;
    size_t low_stock_count;
} PartsDBStats;

PartsDBStats partsdb_statistics(PartsDB* db);

#endif /* PARTSDB_H */
```

## Usage Example

```c
#include "partsdb.h"
#include <stdio.h>

int main(void) {
    /* Create database */
    PartsDB* db = partsdb_create(1000);
    if (!db) {
        fprintf(stderr, "Failed to create database\n");
        return 1;
    }

    /* Insert parts */
    Part resistor = {
        .part_number = "R10K-0805",
        .name = "10K Resistor 0805",
        .location = "A-01-01",
        .quantity = 5000,
        .reorder_point = 1000,
        .unit_price = 0.01
    };
    partsdb_insert(db, &resistor);

    /* Search by part number */
    Part* found = partsdb_find_by_number(db, "R10K-0805");
    if (found) {
        printf("Found: %s at %s\n", found->name, found->location);
    }

    /* Find low stock items */
    size_t low_count;
    Part** low_stock = partsdb_find_low_stock(db, &low_count);
    printf("%zu items need reordering\n", low_count);
    free(low_stock);

    /* Calculate pick route */
    const char* pick_list[] = {"A-01-01", "B-02-03", "C-01-02"};
    size_t route_len;
    char** route = partsdb_optimal_pick_route(db, pick_list, 3, &route_len);
    printf("Optimal pick route:\n");
    for (size_t i = 0; i < route_len; i++) {
        printf("  %zu. %s\n", i + 1, route[i]);
        free(route[i]);
    }
    free(route);

    /* Save and cleanup */
    partsdb_save(db, "inventory.db");
    partsdb_destroy(db);

    return 0;
}
```

---

# F. Memory Layout Diagrams

## Dynamic Array

```
PartArray structure:
┌──────────────────────────────────────────────────────────┐
│ PartArray                                                │
├──────────────┬───────────────┬───────────────────────────┤
│ data (ptr)   │ size (size_t) │ capacity (size_t)         │
│     │        │      5        │         8                 │
└─────│────────┴───────────────┴───────────────────────────┘
      │
      ▼
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│  P0 │  P1 │  P2 │  P3 │  P4 │  -  │  -  │  -  │
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
  Used elements (size=5)    Unused (capacity-size=3)
```

## Doubly Linked List

```
DPartList with sentinel nodes:
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│ SENTINEL│     │  Node A │     │  Node B │     │ SENTINEL│
│  HEAD   │────▶│  Part 1 │────▶│  Part 2 │────▶│  TAIL   │
│         │◀────│         │◀────│         │◀────│         │
└─────────┘     └─────────┘     └─────────┘     └─────────┘
     ▲                                               │
     └───────────────────────────────────────────────┘
                    (circular variant)
```

## AVL Tree Balance

```
         [30]  bf=0
        /    \
    [20]      [40]  bf=0
    bf=0      /  \
           [35] [50]
           bf=0 bf=0

After inserting 25:
         [30]  bf=-1
        /    \
    [20]      [40]  bf=0
    bf=1      /  \
      \    [35] [50]
     [25]
     bf=0

After inserting 22 (triggers rotation):
Before:              After Right Rotation at 20:
    [20]                    [22]
    bf=2                    bf=0
      \                    /    \
     [25]               [20]   [25]
     bf=-1              bf=0   bf=0
     /
   [22]
```

## Hash Table with Chaining

```
┌───┐
│ 0 │──▶ NULL
├───┤
│ 1 │──▶ [Key1:Val1] ──▶ [Key5:Val5] ──▶ NULL
├───┤
│ 2 │──▶ [Key2:Val2] ──▶ NULL
├───┤
│ 3 │──▶ NULL
├───┤
│ 4 │──▶ [Key3:Val3] ──▶ [Key7:Val7] ──▶ [Key9:Val9] ──▶ NULL
├───┤
│ 5 │──▶ [Key4:Val4] ──▶ NULL
├───┤
│ 6 │──▶ NULL
├───┤
│ 7 │──▶ [Key6:Val6] ──▶ NULL
└───┘

Load factor = 9 entries / 8 buckets = 1.125
Longest chain = 3 (bucket 4)
```

## Graph Representations

```
Graph with 4 vertices, 5 edges:
    0 ──2──▶ 1
    │        │
    3        1
    │        │
    ▼        ▼
    2 ◀──4── 3

Adjacency Matrix:        Adjacency List:
     0  1  2  3          0: [(1,2), (2,3)]
  0 [0  2  3  0]         1: [(3,1)]
  1 [0  0  0  1]         2: []
  2 [0  0  0  0]         3: [(2,4)]
  3 [0  0  4  0]

Edge List:
[(0,1,2), (0,2,3), (1,3,1), (3,2,4)]
```

---

# G. Decision Trees

## Choosing a Data Structure

```
┌─────────────────────────────────────────────────────────────────┐
│                DATA STRUCTURE SELECTION                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  What's the primary operation?                                  │
│  ├─ Random Access by Index → Array                             │
│  ├─ Key-Value Lookup → Hash Table                              │
│  ├─ Ordered Traversal → Tree (BST/AVL/RB)                      │
│  ├─ FIFO Processing → Queue                                    │
│  ├─ LIFO Processing → Stack                                    │
│  ├─ Priority Processing → Heap / Priority Queue                │
│  └─ Relationships/Connections → Graph                          │
│                                                                 │
│  Follow-up questions:                                           │
│  ├─ Need O(1) insert/delete at ends? → Linked List             │
│  ├─ Need O(1) insert/delete anywhere? → Doubly Linked + ptr    │
│  ├─ Need sorted order maintained? → AVL/RB Tree or Sorted List │
│  ├─ Need prefix matching? → Trie                               │
│  ├─ Memory is very constrained? → Array over Linked List       │
│  └─ Concurrent access needed? → Consider lock-free structures  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Choosing a Graph Algorithm

```
┌─────────────────────────────────────────────────────────────────┐
│                 GRAPH ALGORITHM SELECTION                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  What do you need to find?                                      │
│                                                                 │
│  ┌─ SHORTEST PATH                                               │
│  │  ├─ Unweighted graph → BFS                                  │
│  │  ├─ Non-negative weights → Dijkstra                         │
│  │  ├─ With heuristic → A*                                     │
│  │  ├─ Negative weights → Bellman-Ford                         │
│  │  └─ All pairs → Floyd-Warshall                              │
│  │                                                              │
│  ┌─ CONNECTIVITY                                                │
│  │  ├─ Is path possible? → BFS or DFS                          │
│  │  ├─ Connected components → Union-Find or DFS                │
│  │  ├─ Strongly connected (directed) → Tarjan's/Kosaraju's     │
│  │  └─ Articulation points/bridges → DFS with discovery times  │
│  │                                                              │
│  ┌─ SPANNING TREE                                               │
│  │  ├─ Dense graph → Prim's                                    │
│  │  ├─ Sparse graph → Kruskal's                                │
│  │  └─ Parallelizable → Borůvka's                              │
│  │                                                              │
│  ┌─ FLOW/MATCHING                                               │
│  │  ├─ Maximum flow → Ford-Fulkerson / Edmonds-Karp            │
│  │  ├─ Min-cut → Max-flow (equivalent)                         │
│  │  └─ Bipartite matching → Hopcroft-Karp or Hungarian         │
│  │                                                              │
│  ┌─ ORDERING                                                    │
│  │  ├─ Dependencies (DAG) → Topological Sort                   │
│  │  └─ Cycle detection → DFS with colors                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Final Words

*"Thus ends the Silmarillion of Algorithms, a chronicle of computational
wisdom passed down through the ages of the Dwarven kingdoms. May these
tables serve as quick reference for the journeyman programmer, and may
the indices guide the seeker to knowledge most profound.*

*In the words of Gimli son of Glóin: 'Faithless is he that says farewell
when the road darkens.' So too with algorithms—the true craftsman knows
that each challenge is but another tunnel to mine, another gem to discover.*

*The code is compiled. The tests pass. The forge is quiet.*

*Baruk Khazâd! Khazâd ai-mênu!"*

---

**Document Statistics:**
- Total Parts: 6 (with sub-parts A, B, C each)
- Total Scavenger Hunt Clues: 25
- Data Structures Covered: 20+
- Algorithms Covered: 40+
- Lines of Example Code: 5000+
- Dwarven Proverbs: Countless

---

*End of Appendix*

*The Silmarillion of Algorithms - Complete*
