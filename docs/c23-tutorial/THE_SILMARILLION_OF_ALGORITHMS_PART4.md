# The Silmarillion of Algorithms
## Part IV: The Dwarven Railways — Graph Algorithms

*"And the Dwarves, being the greatest tunnelers in all Arda,
carved a vast network of passages beneath the mountains.
From Belegost to Nogrod, from Khazad-dûm to Erebor,
the tunnels connected all the great kingdoms,
and the Dwarves learned the arts of navigating these networks."*

---

# Part IV-A: Tunnel Networks — The Fundamentals of Graphs

*In which we learn the language of connections:
vertices and edges, the building blocks of all networks.*

## The Philosophy of Graphs

When the Dwarves carved their great tunnel networks, they faced
a fundamental question: how do we represent and reason about
connections between places? A map of tunnels is not merely a list
of caverns — it is the relationships between them that matter.

A **Graph** G = (V, E) consists of:
- **Vertices (V)**: The nodes, stations, or locations (caverns)
- **Edges (E)**: The connections between vertices (tunnels)

```
The Dwarven Railway Network:

        [Erebor]
           │
           │ 450 leagues
           │
      [Iron Hills]──────200──────[Grey Mountains]
           │                            │
           │ 180                        │ 300
           │                            │
    [Khazad-dûm]────────350────────[Gundabad]
         /   \                          │
       /       \                        │ 250
     320        280                     │
     /           \                [Misty Pass]
[Belegost]    [Nogrod]

Vertices: 7 great halls
Edges: 8 connecting tunnels (with distances in leagues)
```

### Types of Graphs

```
┌────────────────────────────────────────────────────────────────────┐
│                    TAXONOMY OF GRAPHS                              │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  DIRECTED vs UNDIRECTED:                                           │
│                                                                    │
│  Undirected:        Directed:                                      │
│  A ─── B            A ───► B     (one-way tunnel)                 │
│  (two-way tunnel)   A ◄─── B     (another one-way tunnel)         │
│                     A ◄───► B    (two separate edges)              │
│                                                                    │
│  WEIGHTED vs UNWEIGHTED:                                           │
│                                                                    │
│  Unweighted:        Weighted:                                      │
│  A ─── B            A ──5── B    (tunnel is 5 leagues long)       │
│  (just connected)                                                  │
│                                                                    │
│  CYCLIC vs ACYCLIC:                                                │
│                                                                    │
│  Cyclic:            Acyclic (DAG):                                 │
│  A ─── B            A ───► B                                       │
│  │     │                   │                                       │
│  └──C──┘            C ◄────┘     (no way to return to start)      │
│  (can return)                                                      │
│                                                                    │
│  CONNECTED vs DISCONNECTED:                                        │
│                                                                    │
│  Connected:         Disconnected:                                  │
│  A ─ B ─ C          A ─ B    C ─ D  (two separate networks)       │
│                                                                    │
│  DENSE vs SPARSE:                                                  │
│                                                                    │
│  Dense: |E| ≈ |V|²  (almost everyone connected to everyone)       │
│  Sparse: |E| ≈ |V|  (few connections relative to vertices)        │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

---

## Graph Representations

### Adjacency Matrix — The Grand Map Table

*"In the Hall of Records, the Dwarves kept a great stone table,
where each row and column represented a cavern, and the cells
showed the distance of direct tunnels between them."*

```c
/*
 * Adjacency Matrix Representation
 *
 * A 2D array where matrix[i][j] represents the edge from i to j.
 * For weighted graphs: matrix[i][j] = weight (0 or INF = no edge)
 * For unweighted: matrix[i][j] = 1 (connected) or 0 (not connected)
 *
 * Space: O(V²)
 * Edge lookup: O(1)
 * All neighbors: O(V)
 * Add edge: O(1)
 * Add vertex: O(V²) — must resize matrix
 */

#define MAX_VERTICES 100
#define INF INT_MAX

typedef struct {
    char name[50];        /* Station name */
    int id;               /* Vertex ID */
} Station;

typedef struct {
    int matrix[MAX_VERTICES][MAX_VERTICES];
    Station stations[MAX_VERTICES];
    int vertex_count;
    bool is_directed;
} AdjacencyMatrix;

/*
 * Initialize an empty graph
 */
void adj_matrix_init(AdjacencyMatrix *g, bool directed) {
    g->vertex_count = 0;
    g->is_directed = directed;

    /* Initialize all edges to "no connection" */
    for (int i = 0; i < MAX_VERTICES; i++) {
        for (int j = 0; j < MAX_VERTICES; j++) {
            g->matrix[i][j] = (i == j) ? 0 : INF;
        }
    }
}

/*
 * Add a vertex (station)
 */
int adj_matrix_add_vertex(AdjacencyMatrix *g, const char *name) {
    if (g->vertex_count >= MAX_VERTICES) return -1;

    int id = g->vertex_count;
    strncpy(g->stations[id].name, name, sizeof(g->stations[id].name) - 1);
    g->stations[id].id = id;
    g->vertex_count++;

    return id;
}

/*
 * Add an edge (tunnel) with weight (distance)
 */
void adj_matrix_add_edge(AdjacencyMatrix *g, int from, int to, int weight) {
    if (from < 0 || from >= g->vertex_count) return;
    if (to < 0 || to >= g->vertex_count) return;

    g->matrix[from][to] = weight;

    if (!g->is_directed) {
        g->matrix[to][from] = weight;  /* Undirected: add reverse */
    }
}

/*
 * Check if edge exists
 */
bool adj_matrix_has_edge(AdjacencyMatrix *g, int from, int to) {
    if (from < 0 || from >= g->vertex_count) return false;
    if (to < 0 || to >= g->vertex_count) return false;

    return g->matrix[from][to] != INF && g->matrix[from][to] != 0;
}

/*
 * Get edge weight
 */
int adj_matrix_get_weight(AdjacencyMatrix *g, int from, int to) {
    if (from < 0 || from >= g->vertex_count) return INF;
    if (to < 0 || to >= g->vertex_count) return INF;

    return g->matrix[from][to];
}

/*
 * Get all neighbors of a vertex
 */
int adj_matrix_neighbors(AdjacencyMatrix *g, int vertex,
                         int *neighbors, int *weights) {
    int count = 0;

    for (int i = 0; i < g->vertex_count; i++) {
        if (g->matrix[vertex][i] != INF && g->matrix[vertex][i] != 0) {
            neighbors[count] = i;
            weights[count] = g->matrix[vertex][i];
            count++;
        }
    }

    return count;
}

/*
 * Adjacency Matrix Visualization:
 *
 *           Erebor  IronH  GreyM  Khazad  Gunda  MPass  Beleg  Nogr
 * Erebor   [  0      450    INF    INF    INF    INF    INF    INF ]
 * IronH    [ 450      0     200    180    INF    INF    INF    INF ]
 * GreyM    [ INF     200      0    INF    300    INF    INF    INF ]
 * Khazad   [ INF     180    INF      0    350    INF    320    280 ]
 * Gunda    [ INF     INF    300    350      0    250    INF    INF ]
 * MPass    [ INF     INF    INF    INF    250      0    INF    INF ]
 * Beleg    [ INF     INF    INF    320    INF    INF      0    INF ]
 * Nogr     [ INF     INF    INF    280    INF    INF    INF      0 ]
 */
```

### Adjacency List — The Tunnel Ledgers

*"Each Hall-Master kept a ledger of tunnels leading from their hall,
listing destinations and distances. This was more practical than
consulting the Grand Table for every journey."*

```c
/*
 * Adjacency List Representation
 *
 * Each vertex has a list of its neighbors.
 * More space-efficient for sparse graphs.
 *
 * Space: O(V + E)
 * Edge lookup: O(degree) — check neighbor list
 * All neighbors: O(degree)
 * Add edge: O(1)
 * Add vertex: O(1)
 */

typedef struct EdgeNode {
    int dest;              /* Destination vertex */
    int weight;            /* Edge weight */
    struct EdgeNode *next; /* Next edge in list */
} EdgeNode;

typedef struct {
    char name[50];
    EdgeNode *edges;       /* Head of edge list */
    int edge_count;
} VertexNode;

typedef struct {
    VertexNode *vertices;
    int vertex_count;
    int capacity;
    bool is_directed;
} AdjacencyList;

/*
 * Create an adjacency list graph
 */
AdjacencyList *adj_list_create(int initial_capacity, bool directed) {
    AdjacencyList *g = malloc(sizeof(AdjacencyList));
    if (!g) return NULL;

    g->vertices = calloc(initial_capacity, sizeof(VertexNode));
    if (!g->vertices) {
        free(g);
        return NULL;
    }

    g->vertex_count = 0;
    g->capacity = initial_capacity;
    g->is_directed = directed;

    return g;
}

/*
 * Add a vertex
 */
int adj_list_add_vertex(AdjacencyList *g, const char *name) {
    /* Grow if needed */
    if (g->vertex_count >= g->capacity) {
        int new_cap = g->capacity * 2;
        VertexNode *new_verts = realloc(g->vertices,
                                         new_cap * sizeof(VertexNode));
        if (!new_verts) return -1;
        g->vertices = new_verts;
        g->capacity = new_cap;

        /* Initialize new vertices */
        for (int i = g->vertex_count; i < new_cap; i++) {
            g->vertices[i].edges = NULL;
            g->vertices[i].edge_count = 0;
        }
    }

    int id = g->vertex_count;
    strncpy(g->vertices[id].name, name, sizeof(g->vertices[id].name) - 1);
    g->vertices[id].edges = NULL;
    g->vertices[id].edge_count = 0;
    g->vertex_count++;

    return id;
}

/*
 * Add an edge
 */
bool adj_list_add_edge(AdjacencyList *g, int from, int to, int weight) {
    if (from < 0 || from >= g->vertex_count) return false;
    if (to < 0 || to >= g->vertex_count) return false;

    /* Create new edge node */
    EdgeNode *edge = malloc(sizeof(EdgeNode));
    if (!edge) return false;

    edge->dest = to;
    edge->weight = weight;
    edge->next = g->vertices[from].edges;
    g->vertices[from].edges = edge;
    g->vertices[from].edge_count++;

    /* For undirected graphs, add reverse edge */
    if (!g->is_directed) {
        EdgeNode *reverse = malloc(sizeof(EdgeNode));
        if (!reverse) return false;

        reverse->dest = from;
        reverse->weight = weight;
        reverse->next = g->vertices[to].edges;
        g->vertices[to].edges = reverse;
        g->vertices[to].edge_count++;
    }

    return true;
}

/*
 * Check if edge exists
 */
bool adj_list_has_edge(AdjacencyList *g, int from, int to) {
    if (from < 0 || from >= g->vertex_count) return false;

    for (EdgeNode *e = g->vertices[from].edges; e != NULL; e = e->next) {
        if (e->dest == to) return true;
    }

    return false;
}

/*
 * Iterate over neighbors
 */
typedef void (*NeighborVisitor)(int vertex, int neighbor, int weight, void *ctx);

void adj_list_for_each_neighbor(AdjacencyList *g, int vertex,
                                 NeighborVisitor visitor, void *ctx) {
    if (vertex < 0 || vertex >= g->vertex_count) return;

    for (EdgeNode *e = g->vertices[vertex].edges; e != NULL; e = e->next) {
        visitor(vertex, e->dest, e->weight, ctx);
    }
}

/*
 * Destroy the graph
 */
void adj_list_destroy(AdjacencyList *g) {
    if (!g) return;

    for (int i = 0; i < g->vertex_count; i++) {
        EdgeNode *e = g->vertices[i].edges;
        while (e) {
            EdgeNode *next = e->next;
            free(e);
            e = next;
        }
    }

    free(g->vertices);
    free(g);
}

/*
 * Adjacency List Visualization:
 *
 * Erebor   → [IronHills, 450] → NULL
 * IronHills→ [Erebor, 450] → [GreyMts, 200] → [Khazad, 180] → NULL
 * GreyMts  → [IronHills, 200] → [Gundabad, 300] → NULL
 * Khazad   → [IronHills, 180] → [Gundabad, 350] → [Belegost, 320] → [Nogrod, 280] → NULL
 * ...
 */
```

### Edge List — The Master Registry

*"For the great engineers who planned new tunnels,
a simple list of all connections sufficed."*

```c
/*
 * Edge List Representation
 *
 * Simply a list of all edges. Simplest representation.
 * Good for algorithms that process all edges (Kruskal's MST).
 *
 * Space: O(E)
 * Edge lookup: O(E)
 * All edges: O(E)
 * Add edge: O(1)
 */

typedef struct {
    int from;
    int to;
    int weight;
} Edge;

typedef struct {
    Edge *edges;
    int edge_count;
    int edge_capacity;
    int vertex_count;
    bool is_directed;
} EdgeList;

EdgeList *edge_list_create(int initial_edge_capacity, bool directed) {
    EdgeList *g = malloc(sizeof(EdgeList));
    if (!g) return NULL;

    g->edges = malloc(initial_edge_capacity * sizeof(Edge));
    if (!g->edges) {
        free(g);
        return NULL;
    }

    g->edge_count = 0;
    g->edge_capacity = initial_edge_capacity;
    g->vertex_count = 0;
    g->is_directed = directed;

    return g;
}

void edge_list_add_edge(EdgeList *g, int from, int to, int weight) {
    /* Grow if needed */
    if (g->edge_count >= g->edge_capacity) {
        int new_cap = g->edge_capacity * 2;
        Edge *new_edges = realloc(g->edges, new_cap * sizeof(Edge));
        if (!new_edges) return;
        g->edges = new_edges;
        g->edge_capacity = new_cap;
    }

    g->edges[g->edge_count].from = from;
    g->edges[g->edge_count].to = to;
    g->edges[g->edge_count].weight = weight;
    g->edge_count++;

    /* Update vertex count */
    if (from >= g->vertex_count) g->vertex_count = from + 1;
    if (to >= g->vertex_count) g->vertex_count = to + 1;
}

/*
 * Sort edges by weight (for Kruskal's algorithm)
 */
int compare_edges(const void *a, const void *b) {
    return ((Edge *)a)->weight - ((Edge *)b)->weight;
}

void edge_list_sort_by_weight(EdgeList *g) {
    qsort(g->edges, g->edge_count, sizeof(Edge), compare_edges);
}
```

---

## Choosing a Representation

```
╔══════════════════════════════════════════════════════════════════════════╗
║              THE TUNNEL LEDGER — CHOOSING A REPRESENTATION               ║
╠══════════════════════════════════════════════════════════════════════════╣
║  Operation          │ Adj Matrix │ Adj List  │ Edge List │ Best For     ║
╠═════════════════════╪════════════╪═══════════╪═══════════╪══════════════╣
║  Space              │ O(V²)      │ O(V + E)  │ O(E)      │ Sparse→List  ║
║  Add Vertex         │ O(V²)      │ O(1)*     │ O(1)      │ Dynamic→List ║
║  Add Edge           │ O(1)       │ O(1)      │ O(1)      │ Any          ║
║  Remove Edge        │ O(1)       │ O(E)      │ O(E)      │ Matrix       ║
║  Has Edge (u,v)?    │ O(1)       │ O(deg u)  │ O(E)      │ Matrix       ║
║  All neighbors of u │ O(V)       │ O(deg u)  │ O(E)      │ List         ║
║  Iterate all edges  │ O(V²)      │ O(V + E)  │ O(E)      │ Edge List    ║
╠═════════════════════╧════════════╧═══════════╧═══════════╧══════════════╣
║  Dense graph (E ≈ V²): Adjacency Matrix is often better                 ║
║  Sparse graph (E ≈ V): Adjacency List saves memory                      ║
║  MST algorithms: Edge List is convenient                                 ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Graph Properties and Terminology

```c
/*
 * Calculate the degree of a vertex
 * (number of edges connected to it)
 */
int adj_list_degree(AdjacencyList *g, int vertex) {
    if (vertex < 0 || vertex >= g->vertex_count) return 0;
    return g->vertices[vertex].edge_count;
}

/*
 * In-degree and Out-degree (for directed graphs)
 */
int adj_list_out_degree(AdjacencyList *g, int vertex) {
    return adj_list_degree(g, vertex);  /* Same as degree */
}

int adj_list_in_degree(AdjacencyList *g, int vertex) {
    int in_deg = 0;

    for (int i = 0; i < g->vertex_count; i++) {
        for (EdgeNode *e = g->vertices[i].edges; e; e = e->next) {
            if (e->dest == vertex) in_deg++;
        }
    }

    return in_deg;
}

/*
 * Check if graph is connected (for undirected)
 * Uses DFS from vertex 0 and checks if all vertices reached
 */
bool adj_list_is_connected(AdjacencyList *g) {
    if (g->vertex_count == 0) return true;

    bool *visited = calloc(g->vertex_count, sizeof(bool));
    if (!visited) return false;

    /* DFS from vertex 0 */
    int *stack = malloc(g->vertex_count * sizeof(int));
    int top = 0;
    stack[top++] = 0;
    int visited_count = 0;

    while (top > 0) {
        int v = stack[--top];
        if (visited[v]) continue;

        visited[v] = true;
        visited_count++;

        for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
            if (!visited[e->dest]) {
                stack[top++] = e->dest;
            }
        }
    }

    free(stack);
    free(visited);

    return visited_count == g->vertex_count;
}
```

---

## The PartsDB Warehouse Graph

Let's model a warehouse as a graph for the PartsDB system:

```c
/*
 * Warehouse Graph for PartsDB
 *
 * Vertices: Storage locations (shelves, bins, areas)
 * Edges: Paths between locations with travel time (seconds)
 */

typedef enum {
    LOC_ENTRANCE,
    LOC_RECEIVING,
    LOC_SHELF_A,
    LOC_SHELF_B,
    LOC_SHELF_C,
    LOC_BIN_1,
    LOC_BIN_2,
    LOC_BIN_3,
    LOC_SHIPPING,
    LOC_COUNT
} WarehouseLocation;

const char *location_names[] = {
    "Entrance", "Receiving", "Shelf-A", "Shelf-B", "Shelf-C",
    "Bin-1", "Bin-2", "Bin-3", "Shipping"
};

/*
 * Create the warehouse graph
 */
AdjacencyList *create_warehouse_graph(void) {
    AdjacencyList *g = adj_list_create(LOC_COUNT, false);

    /* Add all locations */
    for (int i = 0; i < LOC_COUNT; i++) {
        adj_list_add_vertex(g, location_names[i]);
    }

    /* Add paths with travel times (seconds) */
    adj_list_add_edge(g, LOC_ENTRANCE, LOC_RECEIVING, 10);
    adj_list_add_edge(g, LOC_RECEIVING, LOC_SHELF_A, 15);
    adj_list_add_edge(g, LOC_RECEIVING, LOC_SHELF_B, 20);
    adj_list_add_edge(g, LOC_SHELF_A, LOC_SHELF_B, 8);
    adj_list_add_edge(g, LOC_SHELF_A, LOC_SHELF_C, 12);
    adj_list_add_edge(g, LOC_SHELF_B, LOC_SHELF_C, 10);
    adj_list_add_edge(g, LOC_SHELF_A, LOC_BIN_1, 5);
    adj_list_add_edge(g, LOC_SHELF_B, LOC_BIN_2, 5);
    adj_list_add_edge(g, LOC_SHELF_C, LOC_BIN_3, 5);
    adj_list_add_edge(g, LOC_BIN_1, LOC_BIN_2, 6);
    adj_list_add_edge(g, LOC_BIN_2, LOC_BIN_3, 6);
    adj_list_add_edge(g, LOC_BIN_3, LOC_SHIPPING, 15);
    adj_list_add_edge(g, LOC_SHELF_C, LOC_SHIPPING, 18);

    return g;
}

/*
 * Warehouse layout:
 *
 *  [Entrance]───10───[Receiving]
 *                      /      \
 *                    15        20
 *                   /            \
 *             [Shelf-A]───8───[Shelf-B]
 *                |   \          |
 *                5    12       5  10
 *                |     \      |   /
 *             [Bin-1]  [Shelf-C]──18──[Shipping]
 *                |        |  \          /
 *                6        5   ──────────
 *                |        |
 *             [Bin-2]──6──[Bin-3]───15───┘
 */
```

---

## Scavenger Hunt — Clue #18

*"In Sedgewick's Chapter 17, he speaks of graph representations.
He reveals a crucial insight about when to use adjacency matrices
versus adjacency lists. What is the 'crossover point' in terms of
edges and vertices where one becomes more efficient than the other?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 17.
Find the space complexity comparison. When is |E| > |V|²/C
for some constant C, which representation wins?

---

## Exercises — The Trials of the Tunnel Networks

### Trial 1: The Railway Map
```c
/*
 * Create a graph representing the Dwarven railway network:
 * - 7 major stations (see diagram above)
 * - Add realistic travel times
 * - Support bidirectional travel
 *
 * Print the adjacency list for each station.
 */
```

### Trial 2: The Warehouse Navigator
```c
/*
 * Extend the warehouse graph:
 * - Add 5 more storage locations
 * - Track which locations store which part categories
 * - Find all locations adjacent to a given location
 *
 * typedef struct {
 *     int location_id;
 *     PartCategory category;
 *     int capacity;
 * } StorageLocation;
 */
```

---

*Thus ends the first chapter of the Railway Arts.
We have learned to represent the networks of tunnels.
In the next chapter, we shall learn to navigate them —
the arts of pathfinding through the depths.*

---

# Part IV-B: The Pathfinding Arts — BFS, DFS, and Dijkstra

*In which we learn to navigate the tunnel networks:
Breadth-First Search for shortest hops,
Depth-First Search for exploration,
and Dijkstra's Algorithm for shortest weighted paths.*

## Breadth-First Search — The Expanding Wave

*"When news must spread through the tunnels, it travels like
a wave, reaching all adjacent halls before moving deeper.
This is the way of Breadth-First Search."*

BFS explores all vertices at distance 1, then distance 2, etc.
It uses a **queue** to maintain the frontier of exploration.

```
BFS Visualization from Erebor:

Wave 0: [Erebor]                    ← Start
           |
Wave 1: [Iron Hills]                ← All neighbors of Wave 0
           |    \
Wave 2: [Grey Mts] [Khazad-dûm]     ← All neighbors of Wave 1
              |        |    \
Wave 3: [Gundabad] [Belegost] [Nogrod]  ← All neighbors of Wave 2
              |
Wave 4: [Misty Pass]                ← All neighbors of Wave 3
```

```c
/*
 * Breadth-First Search — O(V + E)
 *
 * Explores level by level, like ripples in a pond.
 * Finds shortest path in unweighted graphs.
 *
 * Properties:
 * - Completeness: Finds a path if one exists
 * - Optimality: Finds shortest path (fewest edges)
 * - Space: O(V) for queue and visited array
 */

typedef struct {
    int *distance;    /* Distance from source to each vertex */
    int *parent;      /* Parent in BFS tree (-1 = no parent) */
    bool *visited;    /* Has vertex been visited? */
    int source;       /* Starting vertex */
} BFSResult;

BFSResult *bfs(AdjacencyList *g, int source) {
    BFSResult *result = malloc(sizeof(BFSResult));
    if (!result) return NULL;

    int n = g->vertex_count;
    result->distance = malloc(n * sizeof(int));
    result->parent = malloc(n * sizeof(int));
    result->visited = malloc(n * sizeof(bool));
    result->source = source;

    if (!result->distance || !result->parent || !result->visited) {
        free(result->distance);
        free(result->parent);
        free(result->visited);
        free(result);
        return NULL;
    }

    /* Initialize */
    for (int i = 0; i < n; i++) {
        result->distance[i] = -1;  /* -1 = unreachable */
        result->parent[i] = -1;
        result->visited[i] = false;
    }

    /* BFS uses a queue */
    int *queue = malloc(n * sizeof(int));
    int front = 0, back = 0;

    /* Start from source */
    queue[back++] = source;
    result->visited[source] = true;
    result->distance[source] = 0;

    while (front < back) {
        int current = queue[front++];

        /* Visit all neighbors */
        for (EdgeNode *e = g->vertices[current].edges; e; e = e->next) {
            if (!result->visited[e->dest]) {
                result->visited[e->dest] = true;
                result->distance[e->dest] = result->distance[current] + 1;
                result->parent[e->dest] = current;
                queue[back++] = e->dest;
            }
        }
    }

    free(queue);
    return result;
}

/*
 * Reconstruct path from source to destination
 */
int bfs_get_path(BFSResult *result, int dest, int *path) {
    if (result->distance[dest] == -1) {
        return 0;  /* No path exists */
    }

    /* Build path backwards from destination */
    int length = result->distance[dest] + 1;
    int current = dest;

    for (int i = length - 1; i >= 0; i--) {
        path[i] = current;
        current = result->parent[current];
    }

    return length;
}

/*
 * Print shortest path
 */
void bfs_print_path(AdjacencyList *g, BFSResult *result, int dest) {
    if (result->distance[dest] == -1) {
        printf("No path from %s to %s\n",
               g->vertices[result->source].name,
               g->vertices[dest].name);
        return;
    }

    int *path = malloc((result->distance[dest] + 1) * sizeof(int));
    int len = bfs_get_path(result, dest, path);

    printf("Path (%d hops): ", len - 1);
    for (int i = 0; i < len; i++) {
        printf("%s", g->vertices[path[i]].name);
        if (i < len - 1) printf(" -> ");
    }
    printf("\n");

    free(path);
}

void bfs_result_destroy(BFSResult *result) {
    if (result) {
        free(result->distance);
        free(result->parent);
        free(result->visited);
        free(result);
    }
}
```

### Applications of BFS

```c
/*
 * Find all vertices within k hops
 */
int bfs_within_distance(AdjacencyList *g, int source, int k,
                         int *result_vertices) {
    BFSResult *bfs_result = bfs(g, source);
    int count = 0;

    for (int i = 0; i < g->vertex_count; i++) {
        if (bfs_result->distance[i] >= 0 &&
            bfs_result->distance[i] <= k) {
            result_vertices[count++] = i;
        }
    }

    bfs_result_destroy(bfs_result);
    return count;
}

/*
 * Check if graph is bipartite (two-colorable)
 * A graph is bipartite if we can color vertices with 2 colors
 * such that no adjacent vertices share a color.
 */
bool bfs_is_bipartite(AdjacencyList *g) {
    int n = g->vertex_count;
    int *color = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        color[i] = -1;  /* Uncolored */
    }

    /* BFS from each unvisited vertex (handles disconnected graphs) */
    for (int start = 0; start < n; start++) {
        if (color[start] != -1) continue;

        int *queue = malloc(n * sizeof(int));
        int front = 0, back = 0;

        queue[back++] = start;
        color[start] = 0;

        while (front < back) {
            int v = queue[front++];

            for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
                if (color[e->dest] == -1) {
                    color[e->dest] = 1 - color[v];  /* Opposite color */
                    queue[back++] = e->dest;
                } else if (color[e->dest] == color[v]) {
                    /* Adjacent vertices same color — not bipartite */
                    free(queue);
                    free(color);
                    return false;
                }
            }
        }

        free(queue);
    }

    free(color);
    return true;
}
```

---

## Depth-First Search — The Explorer's Path

*"The bold Dwarf explorer ventures as deep as possible
into one tunnel before backtracking to try another.
This is the way of Depth-First Search."*

DFS explores as far as possible along each branch before backtracking.
It uses a **stack** (or recursion) to track the path.

```
DFS Visualization from Erebor:

Step 1: Visit Erebor
Step 2: Visit Iron Hills (neighbor of Erebor)
Step 3: Visit Grey Mountains (neighbor of Iron Hills)
Step 4: Visit Gundabad (neighbor of Grey Mountains)
Step 5: Visit Misty Pass (neighbor of Gundabad)
Step 6: Backtrack to Gundabad (no unvisited neighbors)
Step 7: Backtrack to Grey Mountains (no unvisited neighbors)
Step 8: Backtrack to Iron Hills
Step 9: Visit Khazad-dûm (another neighbor of Iron Hills)
... and so on
```

```c
/*
 * Depth-First Search — O(V + E)
 *
 * Explores deeply before broadly.
 * Useful for: cycle detection, topological sort, connectivity.
 *
 * Properties:
 * - Uses stack (recursive call stack or explicit)
 * - Space: O(V) for recursion depth in worst case
 */

typedef struct {
    int *discovery;   /* Discovery time of each vertex */
    int *finish;      /* Finish time of each vertex */
    int *parent;      /* Parent in DFS tree */
    bool *visited;
    int time;         /* Global timestamp */
} DFSResult;

/*
 * Recursive DFS helper
 */
static void dfs_visit(AdjacencyList *g, int v, DFSResult *result) {
    result->visited[v] = true;
    result->discovery[v] = result->time++;

    for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
        if (!result->visited[e->dest]) {
            result->parent[e->dest] = v;
            dfs_visit(g, e->dest, result);
        }
    }

    result->finish[v] = result->time++;
}

/*
 * DFS from all vertices (visits entire graph)
 */
DFSResult *dfs(AdjacencyList *g) {
    int n = g->vertex_count;

    DFSResult *result = malloc(sizeof(DFSResult));
    result->discovery = malloc(n * sizeof(int));
    result->finish = malloc(n * sizeof(int));
    result->parent = malloc(n * sizeof(int));
    result->visited = calloc(n, sizeof(bool));
    result->time = 0;

    for (int i = 0; i < n; i++) {
        result->parent[i] = -1;
    }

    /* DFS from each unvisited vertex */
    for (int i = 0; i < n; i++) {
        if (!result->visited[i]) {
            dfs_visit(g, i, result);
        }
    }

    return result;
}

/*
 * Iterative DFS (avoids stack overflow on deep graphs)
 */
void dfs_iterative(AdjacencyList *g, int source,
                    void (*visit)(int v, void *ctx), void *ctx) {
    int n = g->vertex_count;
    bool *visited = calloc(n, sizeof(bool));
    int *stack = malloc(n * sizeof(int));
    int top = 0;

    stack[top++] = source;

    while (top > 0) {
        int v = stack[--top];

        if (visited[v]) continue;
        visited[v] = true;
        visit(v, ctx);

        /* Push neighbors (in reverse order for consistent ordering) */
        for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
            if (!visited[e->dest]) {
                stack[top++] = e->dest;
            }
        }
    }

    free(stack);
    free(visited);
}
```

### Applications of DFS

```c
/*
 * Cycle Detection (Undirected Graph)
 * If we find an edge to a visited vertex that isn't our parent, it's a cycle.
 */
static bool has_cycle_helper(AdjacencyList *g, int v, int parent,
                              bool *visited) {
    visited[v] = true;

    for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
        if (!visited[e->dest]) {
            if (has_cycle_helper(g, e->dest, v, visited)) {
                return true;
            }
        } else if (e->dest != parent) {
            return true;  /* Back edge found — cycle! */
        }
    }

    return false;
}

bool dfs_has_cycle(AdjacencyList *g) {
    bool *visited = calloc(g->vertex_count, sizeof(bool));

    for (int i = 0; i < g->vertex_count; i++) {
        if (!visited[i]) {
            if (has_cycle_helper(g, i, -1, visited)) {
                free(visited);
                return true;
            }
        }
    }

    free(visited);
    return false;
}

/*
 * Topological Sort (for DAGs only)
 * Linear ordering where u comes before v for every edge u → v.
 */
static void topo_dfs(AdjacencyList *g, int v, bool *visited,
                      int *result, int *idx) {
    visited[v] = true;

    for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
        if (!visited[e->dest]) {
            topo_dfs(g, e->dest, visited, result, idx);
        }
    }

    result[(*idx)--] = v;  /* Add to result after visiting all descendants */
}

bool topological_sort(AdjacencyList *g, int *result) {
    /* Only valid for directed acyclic graphs */
    if (!g->is_directed) return false;

    int n = g->vertex_count;
    bool *visited = calloc(n, sizeof(bool));
    int idx = n - 1;

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            topo_dfs(g, i, visited, result, &idx);
        }
    }

    free(visited);
    return true;
}

/*
 * Connected Components
 * Find all groups of connected vertices.
 */
int find_connected_components(AdjacencyList *g, int *component_id) {
    int n = g->vertex_count;
    bool *visited = calloc(n, sizeof(bool));
    int num_components = 0;

    for (int i = 0; i < n; i++) {
        component_id[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            /* DFS to mark all vertices in this component */
            int *stack = malloc(n * sizeof(int));
            int top = 0;
            stack[top++] = i;

            while (top > 0) {
                int v = stack[--top];
                if (visited[v]) continue;

                visited[v] = true;
                component_id[v] = num_components;

                for (EdgeNode *e = g->vertices[v].edges; e; e = e->next) {
                    if (!visited[e->dest]) {
                        stack[top++] = e->dest;
                    }
                }
            }

            free(stack);
            num_components++;
        }
    }

    free(visited);
    return num_components;
}
```

---

## Dijkstra's Algorithm — The Shortest Weighted Path

*"The wise Dwarf Edsger knew that not all tunnels are equal.
Some are longer, some are treacherous. To find the truly
shortest path, one must account for the weight of each tunnel."*

Dijkstra's Algorithm finds the shortest path in a weighted graph
with non-negative edge weights.

```
Dijkstra's Algorithm Visualization:

Graph:
    A --4-- B
    |      /|
    2    1  3
    |  /    |
    C --5-- D

Finding shortest paths from A:

Step 0: dist = {A:0, B:∞, C:∞, D:∞}
        Process A (dist 0)
        Update: B = 4, C = 2

Step 1: dist = {A:0, B:4, C:2, D:∞}
        Process C (dist 2, smallest unvisited)
        Update: B = min(4, 2+1) = 3, D = 2+5 = 7

Step 2: dist = {A:0, B:3, C:2, D:7}
        Process B (dist 3)
        Update: D = min(7, 3+3) = 6

Step 3: dist = {A:0, B:3, C:2, D:6}
        Process D (dist 6)
        Done!

Shortest paths from A:
  A → A: 0
  A → C: 2
  A → C → B: 3
  A → C → B → D: 6
```

```c
/*
 * Dijkstra's Algorithm — O((V + E) log V) with priority queue
 *
 * Finds shortest paths from source to all other vertices.
 * REQUIRES: All edge weights must be non-negative!
 */

typedef struct {
    int *distance;    /* Shortest distance from source */
    int *parent;      /* Parent in shortest path tree */
    bool *visited;    /* Has vertex been finalized? */
    int source;
} DijkstraResult;

/*
 * Simple O(V²) implementation using linear search for minimum
 */
DijkstraResult *dijkstra_simple(AdjacencyList *g, int source) {
    int n = g->vertex_count;

    DijkstraResult *result = malloc(sizeof(DijkstraResult));
    result->distance = malloc(n * sizeof(int));
    result->parent = malloc(n * sizeof(int));
    result->visited = calloc(n, sizeof(bool));
    result->source = source;

    /* Initialize distances to infinity */
    for (int i = 0; i < n; i++) {
        result->distance[i] = INF;
        result->parent[i] = -1;
    }
    result->distance[source] = 0;

    /* Process all vertices */
    for (int count = 0; count < n; count++) {
        /* Find unvisited vertex with minimum distance */
        int u = -1;
        int min_dist = INF;

        for (int i = 0; i < n; i++) {
            if (!result->visited[i] && result->distance[i] < min_dist) {
                min_dist = result->distance[i];
                u = i;
            }
        }

        if (u == -1) break;  /* All remaining vertices unreachable */

        result->visited[u] = true;

        /* Update distances to neighbors */
        for (EdgeNode *e = g->vertices[u].edges; e; e = e->next) {
            int v = e->dest;
            int weight = e->weight;

            if (!result->visited[v] &&
                result->distance[u] != INF &&
                result->distance[u] + weight < result->distance[v]) {
                result->distance[v] = result->distance[u] + weight;
                result->parent[v] = u;
            }
        }
    }

    return result;
}

/*
 * Priority Queue entry for Dijkstra
 */
typedef struct {
    int vertex;
    int distance;
} PQEntry;

/*
 * Min-heap for Dijkstra's priority queue
 */
typedef struct {
    PQEntry *heap;
    int *position;  /* Position of each vertex in heap */
    int size;
    int capacity;
} DijkstraHeap;

static void dijkstra_heap_swap(DijkstraHeap *h, int i, int j) {
    h->position[h->heap[i].vertex] = j;
    h->position[h->heap[j].vertex] = i;

    PQEntry temp = h->heap[i];
    h->heap[i] = h->heap[j];
    h->heap[j] = temp;
}

static void dijkstra_heap_decrease_key(DijkstraHeap *h, int v, int dist) {
    int i = h->position[v];
    h->heap[i].distance = dist;

    /* Bubble up */
    while (i > 0 && h->heap[(i-1)/2].distance > h->heap[i].distance) {
        dijkstra_heap_swap(h, i, (i-1)/2);
        i = (i-1)/2;
    }
}

static PQEntry dijkstra_heap_extract_min(DijkstraHeap *h) {
    PQEntry min = h->heap[0];
    h->heap[0] = h->heap[--h->size];
    h->position[h->heap[0].vertex] = 0;

    /* Heapify down */
    int i = 0;
    while (true) {
        int smallest = i;
        int left = 2*i + 1;
        int right = 2*i + 2;

        if (left < h->size &&
            h->heap[left].distance < h->heap[smallest].distance) {
            smallest = left;
        }
        if (right < h->size &&
            h->heap[right].distance < h->heap[smallest].distance) {
            smallest = right;
        }

        if (smallest == i) break;

        dijkstra_heap_swap(h, i, smallest);
        i = smallest;
    }

    return min;
}

/*
 * Dijkstra with Priority Queue — O((V + E) log V)
 */
DijkstraResult *dijkstra(AdjacencyList *g, int source) {
    int n = g->vertex_count;

    DijkstraResult *result = malloc(sizeof(DijkstraResult));
    result->distance = malloc(n * sizeof(int));
    result->parent = malloc(n * sizeof(int));
    result->visited = calloc(n, sizeof(bool));
    result->source = source;

    /* Initialize heap */
    DijkstraHeap heap;
    heap.heap = malloc(n * sizeof(PQEntry));
    heap.position = malloc(n * sizeof(int));
    heap.size = n;
    heap.capacity = n;

    for (int i = 0; i < n; i++) {
        result->distance[i] = (i == source) ? 0 : INF;
        result->parent[i] = -1;
        heap.heap[i].vertex = i;
        heap.heap[i].distance = result->distance[i];
        heap.position[i] = i;
    }

    /* Move source to front */
    dijkstra_heap_swap(&heap, 0, source);

    while (heap.size > 0) {
        PQEntry min_entry = dijkstra_heap_extract_min(&heap);
        int u = min_entry.vertex;

        if (result->distance[u] == INF) break;

        result->visited[u] = true;

        /* Relax edges */
        for (EdgeNode *e = g->vertices[u].edges; e; e = e->next) {
            int v = e->dest;
            int weight = e->weight;

            if (!result->visited[v] &&
                result->distance[u] + weight < result->distance[v]) {
                result->distance[v] = result->distance[u] + weight;
                result->parent[v] = u;
                dijkstra_heap_decrease_key(&heap, v, result->distance[v]);
            }
        }
    }

    free(heap.heap);
    free(heap.position);

    return result;
}

/*
 * Get shortest path from source to dest
 */
int dijkstra_get_path(DijkstraResult *result, int dest, int *path) {
    if (result->distance[dest] == INF) return 0;

    /* Count path length */
    int length = 0;
    for (int v = dest; v != -1; v = result->parent[v]) {
        length++;
    }

    /* Fill path backwards */
    int idx = length - 1;
    for (int v = dest; v != -1; v = result->parent[v]) {
        path[idx--] = v;
    }

    return length;
}

void dijkstra_result_destroy(DijkstraResult *result) {
    if (result) {
        free(result->distance);
        free(result->parent);
        free(result->visited);
        free(result);
    }
}
```

---

## The A* Algorithm — Informed Search

*"The cleverest Dwarves used their knowledge of geography
to guide their search, heading toward the destination
rather than exploring blindly."*

A* uses a heuristic to guide the search toward the goal.

```c
/*
 * A* Algorithm — O((V + E) log V) with good heuristic
 *
 * Like Dijkstra but uses f(n) = g(n) + h(n) where:
 * - g(n) = actual cost from start to n
 * - h(n) = estimated cost from n to goal (heuristic)
 *
 * Heuristic must be admissible (never overestimates).
 */

typedef int (*Heuristic)(int vertex, int goal, void *ctx);

/* For grid-based movement: Manhattan distance */
int manhattan_heuristic(int v, int goal, void *ctx) {
    /* Assumes ctx contains grid coordinates */
    typedef struct { int x, y; } Coord;
    Coord *coords = (Coord *)ctx;

    return abs(coords[v].x - coords[goal].x) +
           abs(coords[v].y - coords[goal].y);
}

DijkstraResult *a_star(AdjacencyList *g, int source, int goal,
                        Heuristic h, void *h_ctx) {
    int n = g->vertex_count;

    DijkstraResult *result = malloc(sizeof(DijkstraResult));
    result->distance = malloc(n * sizeof(int));  /* g(n) */
    result->parent = malloc(n * sizeof(int));
    result->visited = calloc(n, sizeof(bool));
    result->source = source;

    int *f_score = malloc(n * sizeof(int));  /* f(n) = g(n) + h(n) */

    for (int i = 0; i < n; i++) {
        result->distance[i] = INF;
        f_score[i] = INF;
        result->parent[i] = -1;
    }

    result->distance[source] = 0;
    f_score[source] = h(source, goal, h_ctx);

    /* Priority queue ordered by f_score */
    DijkstraHeap heap;
    heap.heap = malloc(n * sizeof(PQEntry));
    heap.position = malloc(n * sizeof(int));
    heap.size = 0;
    heap.capacity = n;

    /* Initialize positions */
    for (int i = 0; i < n; i++) {
        heap.position[i] = -1;
    }

    /* Add source */
    heap.heap[heap.size].vertex = source;
    heap.heap[heap.size].distance = f_score[source];
    heap.position[source] = heap.size;
    heap.size++;

    while (heap.size > 0) {
        PQEntry current = dijkstra_heap_extract_min(&heap);
        int u = current.vertex;

        if (u == goal) break;  /* Found it! */

        result->visited[u] = true;

        for (EdgeNode *e = g->vertices[u].edges; e; e = e->next) {
            int v = e->dest;
            if (result->visited[v]) continue;

            int tentative_g = result->distance[u] + e->weight;

            if (tentative_g < result->distance[v]) {
                result->distance[v] = tentative_g;
                result->parent[v] = u;
                f_score[v] = tentative_g + h(v, goal, h_ctx);

                if (heap.position[v] == -1) {
                    /* Add to heap */
                    heap.heap[heap.size].vertex = v;
                    heap.heap[heap.size].distance = f_score[v];
                    heap.position[v] = heap.size;
                    heap.size++;
                    /* Bubble up */
                    int i = heap.size - 1;
                    while (i > 0 &&
                           heap.heap[(i-1)/2].distance > heap.heap[i].distance) {
                        dijkstra_heap_swap(&heap, i, (i-1)/2);
                        i = (i-1)/2;
                    }
                } else {
                    dijkstra_heap_decrease_key(&heap, v, f_score[v]);
                }
            }
        }
    }

    free(f_score);
    free(heap.heap);
    free(heap.position);

    return result;
}
```

---

## Comparison of Pathfinding Algorithms

```
╔════════════════════════════════════════════════════════════════════════════╗
║                THE PATHFINDER'S TABLET                                     ║
╠════════════════════════════════════════════════════════════════════════════╣
║  Algorithm  │ Time         │ Space  │ Weighted │ Optimal │ Complete      ║
╠═════════════╪══════════════╪════════╪══════════╪═════════╪═══════════════╣
║  BFS        │ O(V + E)     │ O(V)   │ No       │ Yes*    │ Yes           ║
║  DFS        │ O(V + E)     │ O(V)   │ No       │ No      │ Yes           ║
║  Dijkstra   │ O((V+E)log V)│ O(V)   │ Yes      │ Yes     │ Yes**         ║
║  A*         │ O((V+E)log V)│ O(V)   │ Yes      │ Yes***  │ Yes**         ║
╠═════════════╧══════════════╧════════╧══════════╧═════════╧═══════════════╣
║  * BFS is optimal for unweighted graphs (fewest edges)                    ║
║  ** Requires non-negative edge weights                                    ║
║  *** With admissible heuristic                                            ║
╚════════════════════════════════════════════════════════════════════════════╝
```

---

## Scavenger Hunt — Clue #19

*"Bhargava dedicates Chapter 6 to Breadth-First Search.
He uses the tale of finding a 'mango seller' in your network
of friends. He reveals that BFS answers two types of questions.
What are these two questions?"*

🔍 **Your Quest**: Read Grokking Algorithms Chapter 6.
Find the two questions BFS answers. Why does the queue
data structure matter for shortest path?

---

## Scavenger Hunt — Clue #20

*"In Sedgewick's Chapter 21, he presents Dijkstra's algorithm
with a crucial insight about relaxation. He shows that the key
operation is checking whether a shorter path exists through
a newly discovered vertex. What is the mathematical condition
for 'relaxation' of an edge (u, v)?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 21.
Find the relaxation condition. When do we update the distance
to vertex v?

---

## Exercises — The Trials of the Pathfinders

### Trial 1: The Warehouse Router
```c
/*
 * Using the warehouse graph, find:
 * 1. Shortest path (fewest locations) from Entrance to Shipping
 * 2. Fastest path (least time) from Entrance to Shipping
 * 3. All locations reachable within 30 seconds of Receiving
 */
```

### Trial 2: The Delivery Optimizer
```c
/*
 * Given a list of parts to pick and their locations,
 * find an efficient order to visit all locations.
 * (This is related to the Traveling Salesman Problem — just find a good route)
 */
```

---

*Thus ends the second chapter of the Railway Arts.
We have learned to navigate the depths with BFS, DFS, and Dijkstra.
In the final chapter, we shall learn the grand designs:
spanning trees and network flow.*

---

# Part IV-C: The Grand Design — MST and Network Flow

*In which we learn to build efficient railway networks
and manage the flow of resources through tunnels.*

## Minimum Spanning Tree — Connecting All Halls

*"When the Dwarves planned a new railway connecting all halls,
they sought the design using the least total tunnel length.
This is the Minimum Spanning Tree."*

A **Spanning Tree** of a graph is a tree that includes all vertices
with exactly V-1 edges. The **Minimum Spanning Tree (MST)** is the
spanning tree with the smallest total edge weight.

```
Original Graph:                  Minimum Spanning Tree:

    A --4-- B                        A      B
    |\     /|                        |     /
    | 2   1 3                        2    1
    |  \ /  |                        |  /
    C --5-- D                        C      D

Total edges: 5                   Edges: A-C, C-B, B-D
Total weight: 15                 Total weight: 2+1+3 = 6
```

### Kruskal's Algorithm — Sorting the Tunnels

*"Sort all potential tunnels by length. Build the network by
adding the shortest tunnels, but skip any that would create a loop."*

```c
/*
 * Kruskal's Algorithm — O(E log E)
 *
 * 1. Sort all edges by weight
 * 2. For each edge (in order):
 *    - If adding it doesn't create a cycle, add it to MST
 * 3. Stop when we have V-1 edges
 *
 * Uses Union-Find for efficient cycle detection.
 */

/*
 * Union-Find (Disjoint Set Union) data structure
 */
typedef struct {
    int *parent;
    int *rank;
    int count;
} UnionFind;

UnionFind *union_find_create(int n) {
    UnionFind *uf = malloc(sizeof(UnionFind));
    uf->parent = malloc(n * sizeof(int));
    uf->rank = malloc(n * sizeof(int));
    uf->count = n;

    for (int i = 0; i < n; i++) {
        uf->parent[i] = i;  /* Each vertex is its own set */
        uf->rank[i] = 0;
    }

    return uf;
}

/* Find with path compression */
int union_find_find(UnionFind *uf, int x) {
    if (uf->parent[x] != x) {
        uf->parent[x] = union_find_find(uf, uf->parent[x]);
    }
    return uf->parent[x];
}

/* Union by rank */
bool union_find_union(UnionFind *uf, int x, int y) {
    int root_x = union_find_find(uf, x);
    int root_y = union_find_find(uf, y);

    if (root_x == root_y) return false;  /* Already in same set */

    if (uf->rank[root_x] < uf->rank[root_y]) {
        uf->parent[root_x] = root_y;
    } else if (uf->rank[root_x] > uf->rank[root_y]) {
        uf->parent[root_y] = root_x;
    } else {
        uf->parent[root_y] = root_x;
        uf->rank[root_x]++;
    }

    uf->count--;
    return true;
}

void union_find_destroy(UnionFind *uf) {
    free(uf->parent);
    free(uf->rank);
    free(uf);
}

/*
 * Kruskal's MST
 */
typedef struct {
    Edge *edges;
    int edge_count;
    int total_weight;
} MSTResult;

MSTResult *kruskal_mst(EdgeList *g) {
    /* Sort edges by weight */
    edge_list_sort_by_weight(g);

    MSTResult *result = malloc(sizeof(MSTResult));
    result->edges = malloc((g->vertex_count - 1) * sizeof(Edge));
    result->edge_count = 0;
    result->total_weight = 0;

    UnionFind *uf = union_find_create(g->vertex_count);

    for (int i = 0; i < g->edge_count && result->edge_count < g->vertex_count - 1; i++) {
        Edge *e = &g->edges[i];

        /* If this edge connects two different components */
        if (union_find_find(uf, e->from) != union_find_find(uf, e->to)) {
            /* Add to MST */
            result->edges[result->edge_count++] = *e;
            result->total_weight += e->weight;

            union_find_union(uf, e->from, e->to);
        }
    }

    union_find_destroy(uf);
    return result;
}

/*
 * Kruskal's Visualization:
 *
 * Sorted edges: (C-B,1), (A-C,2), (B-D,3), (A-B,4), (C-D,5)
 *
 * Process (C-B,1): C and B in different sets → ADD
 *                  MST = {(C-B,1)}, weight = 1
 *
 * Process (A-C,2): A and C in different sets → ADD
 *                  MST = {(C-B,1), (A-C,2)}, weight = 3
 *
 * Process (B-D,3): B and D in different sets → ADD
 *                  MST = {(C-B,1), (A-C,2), (B-D,3)}, weight = 6
 *
 * Process (A-B,4): A and B in SAME set (via A-C-B) → SKIP
 *
 * V-1 = 3 edges, done!
 */
```

### Prim's Algorithm — Growing the Network

*"Start from any hall. Repeatedly add the shortest tunnel
that connects the network to a hall not yet connected."*

```c
/*
 * Prim's Algorithm — O((V + E) log V) with priority queue
 *
 * 1. Start with any vertex in MST
 * 2. Repeatedly add the minimum weight edge connecting MST to non-MST vertex
 * 3. Stop when all vertices are in MST
 */

MSTResult *prim_mst(AdjacencyList *g) {
    int n = g->vertex_count;

    MSTResult *result = malloc(sizeof(MSTResult));
    result->edges = malloc((n - 1) * sizeof(Edge));
    result->edge_count = 0;
    result->total_weight = 0;

    bool *in_mst = calloc(n, sizeof(bool));
    int *key = malloc(n * sizeof(int));    /* Minimum weight edge to vertex */
    int *parent = malloc(n * sizeof(int)); /* Parent in MST */

    for (int i = 0; i < n; i++) {
        key[i] = INF;
        parent[i] = -1;
    }

    /* Start with vertex 0 */
    key[0] = 0;

    /* Simple O(V²) implementation */
    for (int count = 0; count < n; count++) {
        /* Find minimum key vertex not in MST */
        int u = -1;
        int min_key = INF;

        for (int v = 0; v < n; v++) {
            if (!in_mst[v] && key[v] < min_key) {
                min_key = key[v];
                u = v;
            }
        }

        if (u == -1) break;

        in_mst[u] = true;

        /* Add edge to MST (except for starting vertex) */
        if (parent[u] != -1) {
            result->edges[result->edge_count].from = parent[u];
            result->edges[result->edge_count].to = u;
            result->edges[result->edge_count].weight = key[u];
            result->edge_count++;
            result->total_weight += key[u];
        }

        /* Update keys of adjacent vertices */
        for (EdgeNode *e = g->vertices[u].edges; e; e = e->next) {
            int v = e->dest;
            if (!in_mst[v] && e->weight < key[v]) {
                key[v] = e->weight;
                parent[v] = u;
            }
        }
    }

    free(in_mst);
    free(key);
    free(parent);

    return result;
}
```

---

## Network Flow — Resources Through Tunnels

*"The Dwarves needed to transport ore from the mines to the forges.
Each tunnel had a maximum capacity. How much ore could flow
from source to sink?"*

The **Maximum Flow Problem** asks: given a network with capacities,
what is the maximum flow from source to sink?

```
Flow Network:

        ┌──10──►[B]──8──┐
        │       │ ↓     │
       [S]     ↓ 2      ↓[T]
        │      │ ↓      │
        └──5───►[C]──7──┘

Source S, Sink T
Edge labels are capacities

Maximum flow = 15
(10 through S→B→T, 5 through S→C→T)
```

```c
/*
 * Ford-Fulkerson with BFS (Edmonds-Karp) — O(V × E²)
 *
 * 1. While there exists an augmenting path from s to t:
 *    a. Find path using BFS
 *    b. Find minimum capacity along path (bottleneck)
 *    c. Augment flow along path
 *    d. Update residual capacities
 */

typedef struct {
    int **capacity;    /* capacity[u][v] = capacity of edge u→v */
    int **flow;        /* flow[u][v] = current flow u→v */
    int vertex_count;
} FlowNetwork;

FlowNetwork *flow_network_create(int n) {
    FlowNetwork *net = malloc(sizeof(FlowNetwork));
    net->vertex_count = n;

    net->capacity = malloc(n * sizeof(int *));
    net->flow = malloc(n * sizeof(int *));

    for (int i = 0; i < n; i++) {
        net->capacity[i] = calloc(n, sizeof(int));
        net->flow[i] = calloc(n, sizeof(int));
    }

    return net;
}

void flow_network_add_edge(FlowNetwork *net, int from, int to, int cap) {
    net->capacity[from][to] = cap;
}

/*
 * BFS to find augmenting path in residual graph
 */
static bool bfs_augmenting_path(FlowNetwork *net, int source, int sink,
                                 int *parent) {
    int n = net->vertex_count;
    bool *visited = calloc(n, sizeof(bool));
    int *queue = malloc(n * sizeof(int));
    int front = 0, back = 0;

    queue[back++] = source;
    visited[source] = true;
    parent[source] = -1;

    while (front < back) {
        int u = queue[front++];

        for (int v = 0; v < n; v++) {
            /* Residual capacity = capacity - current flow */
            int residual = net->capacity[u][v] - net->flow[u][v];

            if (!visited[v] && residual > 0) {
                visited[v] = true;
                parent[v] = u;
                queue[back++] = v;

                if (v == sink) {
                    free(visited);
                    free(queue);
                    return true;
                }
            }
        }
    }

    free(visited);
    free(queue);
    return false;
}

/*
 * Edmonds-Karp (Ford-Fulkerson with BFS)
 */
int max_flow(FlowNetwork *net, int source, int sink) {
    int n = net->vertex_count;
    int *parent = malloc(n * sizeof(int));
    int total_flow = 0;

    /* While augmenting path exists */
    while (bfs_augmenting_path(net, source, sink, parent)) {
        /* Find minimum residual capacity along path */
        int path_flow = INF;

        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            int residual = net->capacity[u][v] - net->flow[u][v];
            if (residual < path_flow) {
                path_flow = residual;
            }
        }

        /* Augment flow along path */
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            net->flow[u][v] += path_flow;
            net->flow[v][u] -= path_flow;  /* Reverse flow for residual graph */
        }

        total_flow += path_flow;
    }

    free(parent);
    return total_flow;
}

void flow_network_destroy(FlowNetwork *net) {
    for (int i = 0; i < net->vertex_count; i++) {
        free(net->capacity[i]);
        free(net->flow[i]);
    }
    free(net->capacity);
    free(net->flow);
    free(net);
}
```

---

## Applications of Network Flow

```c
/*
 * Bipartite Matching using Max Flow
 *
 * Given workers and jobs, find maximum matching.
 * Model as flow network:
 * - Source connects to all workers (capacity 1)
 * - Workers connect to compatible jobs (capacity 1)
 * - All jobs connect to sink (capacity 1)
 */

int bipartite_matching(int num_workers, int num_jobs,
                        bool compatible[num_workers][num_jobs]) {
    /* Create flow network:
     * Vertex 0 = source
     * Vertices 1..num_workers = workers
     * Vertices num_workers+1..num_workers+num_jobs = jobs
     * Last vertex = sink
     */
    int n = 1 + num_workers + num_jobs + 1;
    int source = 0;
    int sink = n - 1;

    FlowNetwork *net = flow_network_create(n);

    /* Source to workers */
    for (int w = 0; w < num_workers; w++) {
        flow_network_add_edge(net, source, 1 + w, 1);
    }

    /* Workers to compatible jobs */
    for (int w = 0; w < num_workers; w++) {
        for (int j = 0; j < num_jobs; j++) {
            if (compatible[w][j]) {
                flow_network_add_edge(net, 1 + w,
                                       1 + num_workers + j, 1);
            }
        }
    }

    /* Jobs to sink */
    for (int j = 0; j < num_jobs; j++) {
        flow_network_add_edge(net, 1 + num_workers + j, sink, 1);
    }

    int matching = max_flow(net, source, sink);

    flow_network_destroy(net);
    return matching;
}
```

---

## Scavenger Hunt — Clue #21

*"In Sedgewick's Chapter 20, he presents Kruskal's algorithm
with a key insight about the cut property. He shows that the
minimum weight edge crossing any cut must be in the MST.
What is a 'cut' in graph theory, and why does this property
guarantee Kruskal's correctness?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 20.
Find the cut property theorem. What makes Kruskal's greedy
approach provably optimal?

---

## Scavenger Hunt — Clue #22

*"Bhargava touches on graph theory in Chapter 6 with BFS,
but in the 'Where to go from here' section at the end of the book,
he mentions network flow and the 'max-flow min-cut theorem.'
What does this theorem state, and why is it profound?"*

🔍 **Your Quest**: Explore Grokking Algorithms' final chapter.
The max-flow min-cut theorem connects two seemingly different
problems. How are they equivalent?

---

## The Complete Graph Algorithm Toolkit

```
╔════════════════════════════════════════════════════════════════════════════╗
║                   THE GRAND RAILWAY COMPENDIUM                             ║
╠════════════════════════════════════════════════════════════════════════════╣
║  Problem                   │ Algorithm          │ Time Complexity         ║
╠════════════════════════════╪════════════════════╪═════════════════════════╣
║  Shortest path (unweighted)│ BFS                │ O(V + E)                ║
║  Shortest path (weighted)  │ Dijkstra           │ O((V+E) log V)          ║
║  Shortest path (neg edges) │ Bellman-Ford       │ O(V × E)                ║
║  All-pairs shortest path   │ Floyd-Warshall     │ O(V³)                   ║
║  Connectivity              │ DFS/BFS            │ O(V + E)                ║
║  Cycle detection           │ DFS                │ O(V + E)                ║
║  Topological sort          │ DFS                │ O(V + E)                ║
║  Minimum spanning tree     │ Kruskal            │ O(E log E)              ║
║  Minimum spanning tree     │ Prim               │ O((V+E) log V)          ║
║  Maximum flow              │ Edmonds-Karp       │ O(V × E²)               ║
║  Bipartite matching        │ Max Flow           │ O(V × E²)               ║
║  Strongly connected comp.  │ Kosaraju/Tarjan    │ O(V + E)                ║
╚════════════════════════════════════════════════════════════════════════════╝
```

---

## Exercises — The Trials of the Grand Design

### Trial 1: The Warehouse Cable Network
```c
/*
 * Design the minimum total cabling to connect all warehouse
 * locations with network cables. Each connection has an
 * installation cost based on distance.
 *
 * Use Kruskal's or Prim's algorithm on the warehouse graph.
 */
```

### Trial 2: The Parts Distribution Network
```c
/*
 * Model the parts distribution as a flow network:
 * - Source: Receiving dock
 * - Intermediate: Storage areas with limited capacity
 * - Sink: Shipping dock
 *
 * Find maximum parts throughput per hour.
 */
```

### Trial 3: The Worker Assignment
```c
/*
 * Match warehouse workers to picking zones.
 * Each worker has training for certain zones.
 * Find maximum number of zones that can be staffed.
 *
 * Model as bipartite matching problem.
 */
```

---

*Thus ends the Railway Arts of Khazad-dûm.
From simple representations to grand network designs,
we have learned to model and traverse the connections
that bind all things together.

In the next part, we shall learn the Palantír Protocol —
the arts of communication across vast distances,
powered by water and transmitted by electrical impulse.*

---

[Continue to Part V: The Palantír Protocol — Communication Systems →](./THE_SILMARILLION_OF_ALGORITHMS_PART5.md)

---
