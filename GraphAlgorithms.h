// GraphAlgorithms.h
// Algorithms that run on the Graph using ONLY our own data structures.
//   BFS      (Module 6) - Queue                      O(V + E)
//   DFS      (Module 6) - Stack (iterative)          O(V + E)
//   Dijkstra (Module 7) - min-Heap (lazy deletion)   O((V + E) log V)
//   Prim MST (Module 8) - min-Heap                   O(E log V)
#pragma once
#include "Graph.h"
#include "DynamicArray.h"
#include "Heap.h"

struct MstEdge {
    int u, v, weight;
    MstEdge() : u(0), v(0), weight(0) {}
    MstEdge(int a, int b, int w) : u(a), v(b), weight(w) {}
};

// Item stored in the min-heap used by Dijkstra and Prim.
struct PQItem {
    int key;      // distance (Dijkstra) or edge weight (Prim)
    int vertex;
    int from;     // Prim: the tree vertex this edge starts from
    PQItem() : key(0), vertex(0), from(-1) {}
    PQItem(int k, int v, int f) : key(k), vertex(v), from(f) {}
};
struct PQItemLess {
    bool operator()(const PQItem& a, const PQItem& b) const { return a.key < b.key; }
};
typedef Heap<PQItem, PQItemLess> MinHeap;

// order = vertices in the order they were visited, hops[v] = BFS level (-1 = unreachable)
void bfs(const Graph& g, int source, DynamicArray<int>& order, DynamicArray<int>& hops);

// order = vertices in DFS visiting order
void dfs(const Graph& g, int source, DynamicArray<int>& order);

// dist[v] = shortest distance from source (INF if unreachable), parent[v] = previous vertex on that path
void dijkstra(const Graph& g, int source, DynamicArray<int>& dist, DynamicArray<int>& parent);

// Rebuilds source->target path (in order) from the parent array. Returns false if unreachable.
bool buildPath(const DynamicArray<int>& parent, int source, int target, DynamicArray<int>& path);

// Minimum spanning tree (forest if the graph is disconnected). Returns total cost.
int primMST(const Graph& g, DynamicArray<MstEdge>& edges, int& components);
