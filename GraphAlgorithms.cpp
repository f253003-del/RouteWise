#include "GraphAlgorithms.h"
#include "Queue.h"
#include "Stack.h"
#include "Utils.h"

// ---------------------------------------------------------------- BFS
// Visit vertices level by level. A vertex is marked visited when it is ENQUEUED,
// so it can never be queued twice.
void bfs(const Graph& g, int source, DynamicArray<int>& order, DynamicArray<int>& hops) {
    int n = g.vertexCount();
    DynamicArray<bool> visited;
    visited.assign(n, false);
    hops.assign(n, -1);
    order.clear();

    Queue<int> q;
    visited[source] = true;
    hops[source] = 0;
    q.enqueue(source);

    int u = 0;
    while (q.dequeue(u)) {
        order.pushBack(u);
        for (Graph::EdgeNode* e = g.neighbors(u); e != nullptr; e = e->next) {
            int v = e->data.to;
            if (!visited[v]) {
                visited[v] = true;
                hops[v] = hops[u] + 1;
                q.enqueue(v);
            }
        }
    }
}

// ---------------------------------------------------------------- DFS
// Iterative DFS with an explicit stack. Neighbours are pushed in REVERSE so the
// first neighbour in the adjacency list is explored first (same order as recursion).
void dfs(const Graph& g, int source, DynamicArray<int>& order) {
    int n = g.vertexCount();
    DynamicArray<bool> visited;
    visited.assign(n, false);
    order.clear();

    Stack<int> st;
    st.push(source);
    int u = 0;
    while (st.pop(u)) {
        if (visited[u]) continue;           // may have been pushed several times
        visited[u] = true;
        order.pushBack(u);

        DynamicArray<int> fresh;
        for (Graph::EdgeNode* e = g.neighbors(u); e != nullptr; e = e->next)
            if (!visited[e->data.to]) fresh.pushBack(e->data.to);
        for (int i = fresh.size() - 1; i >= 0; --i) st.push(fresh[i]);
    }
}

// ---------------------------------------------------------------- Dijkstra
// Greedy: always finalise the closest unfinished vertex. Requires NON-NEGATIVE weights.
// "Lazy deletion": instead of decrease-key we push a new item and skip stale ones.
void dijkstra(const Graph& g, int source, DynamicArray<int>& dist, DynamicArray<int>& parent) {
    int n = g.vertexCount();
    dist.assign(n, INF);
    parent.assign(n, -1);

    MinHeap pq;
    dist[source] = 0;
    pq.push(PQItem(0, source, -1));

    PQItem cur;
    while (pq.pop(cur)) {
        int u = cur.vertex;
        if (cur.key > dist[u]) continue;     // stale entry: a shorter path was already found
        for (Graph::EdgeNode* e = g.neighbors(u); e != nullptr; e = e->next) {
            int v = e->data.to;
            int candidate = dist[u] + e->data.weight;
            if (candidate < dist[v]) {       // "relaxation"
                dist[v] = candidate;
                parent[v] = u;
                pq.push(PQItem(candidate, v, u));
            }
        }
    }
}

// Walk parent[] from target back to source (gives the path BACKWARDS) and use a
// stack to flip it into source -> target order.
bool buildPath(const DynamicArray<int>& parent, int source, int target, DynamicArray<int>& path) {
    path.clear();
    Stack<int> st;
    int v = target;
    int guard = 0;
    while (v != -1 && guard <= parent.size()) {
        st.push(v);
        if (v == source) break;
        v = parent[v];
        ++guard;
    }
    int top = 0;
    if (!st.peek(top) || top != source) return false;   // never reached the source: unreachable
    while (st.pop(top)) path.pushBack(top);
    return true;
}

// ---------------------------------------------------------------- Prim
// Grow one tree: repeatedly take the cheapest edge that connects the tree to a new vertex.
// If the graph is disconnected we restart from an unvisited vertex -> minimum spanning FOREST.
int primMST(const Graph& g, DynamicArray<MstEdge>& edges, int& components) {
    int n = g.vertexCount();
    DynamicArray<bool> inTree;
    inTree.assign(n, false);
    edges.clear();
    components = 0;
    int total = 0;

    for (int start = 0; start < n; ++start) {
        if (inTree[start]) continue;
        ++components;
        MinHeap pq;
        pq.push(PQItem(0, start, -1));
        PQItem cur;
        while (pq.pop(cur)) {
            int u = cur.vertex;
            if (inTree[u]) continue;
            inTree[u] = true;
            if (cur.from != -1) {
                edges.pushBack(MstEdge(cur.from, u, cur.key));
                total += cur.key;
            }
            for (Graph::EdgeNode* e = g.neighbors(u); e != nullptr; e = e->next)
                if (!inTree[e->data.to]) pq.push(PQItem(e->data.weight, e->data.to, u));
        }
    }
    return total;
}
