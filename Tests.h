// Tests.h - self-tests. Run with:  RouteWise.exe --test
// A tiny test framework: CHECK(condition) counts passes/failures.
#pragma once
#include <iostream>
#include <string>
#include "DynamicArray.h"
#include "SinglyLinkedList.h"
#include "DoublyLinkedList.h"
#include "Stack.h"
#include "Queue.h"
#include "Heap.h"
#include "HashTable.h"
#include "BST.h"
#include "AVL.h"
#include "Graph.h"
#include "GraphAlgorithms.h"
#include "RouteWiseSystem.h"
#include "Utils.h"

static int g_checks = 0;
static int g_failed = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++g_checks;                                                                   \
        if (!(cond)) { ++g_failed; std::cout << "    FAILED: " #cond " (line " << __LINE__ << ")\n"; } \
    } while (0)

inline void testDynamicArray() {
    std::cout << "DynamicArray\n";
    DynamicArray<int> a(2);
    for (int i = 0; i < 100; ++i) a.pushBack(i);
    CHECK(a.size() == 100);
    CHECK(a[99] == 99);
    a.removeAt(0);
    CHECK(a.size() == 99 && a[0] == 1);
    CHECK(a.popBack() == 99);
    a.assign(5, 7);
    CHECK(a.size() == 5 && a[4] == 7);
}

inline void testLists() {
    std::cout << "Linked lists\n";
    SinglyLinkedList<int> s;
    s.pushBack(2); s.pushBack(3); s.pushFront(1);
    CHECK(s.size() == 3);
    int x = 0;
    CHECK(s.removeFirstIf([](int v) { return v == 2; }, x) && x == 2);
    CHECK(s.removeFirstIf([](int v) { return v == 3; }, x));   // removing the tail
    s.pushBack(9);                                             // tail pointer must still be valid
    CHECK(s.popFront(x) && x == 1);
    CHECK(s.popFront(x) && x == 9);
    CHECK(!s.popFront(x) && s.empty());

    DoublyLinkedList<int> d;
    d.pushBack(1); d.pushBack(2); d.pushBack(3); d.pushFront(0);
    CHECK(d.size() == 4 && d.head()->data == 0 && d.tail()->data == 3);
    CHECK(d.popBack(x) && x == 3);
    CHECK(d.tail()->data == 2 && d.tail()->prev->data == 1);
    CHECK(d.popFront(x) && x == 0);
}

inline void testStackQueue() {
    std::cout << "Stack and Queue\n";
    Stack<int> st;
    for (int i = 1; i <= 50; ++i) st.push(i);
    int x = 0;
    CHECK(st.peek(x) && x == 50);
    CHECK(st.pop(x) && x == 50);
    CHECK(st.pop(x) && x == 49);
    CHECK(st.size() == 48);
    st.clear();
    CHECK(!st.pop(x));

    Queue<int> q;
    q.enqueue(1); q.enqueue(2); q.enqueue(3);
    CHECK(q.dequeue(x) && x == 1);
    q.enqueueFront(1);
    CHECK(q.dequeue(x) && x == 1);
    CHECK(q.removeFirstIf([](int v) { return v == 3; }, x));
    CHECK(q.dequeue(x) && x == 2);
    CHECK(!q.dequeue(x));
}

inline void testHeap() {
    std::cout << "Heap (urgent order)\n";
    Heap<DeliveryRequest, UrgentOrder> h;
    h.push(DeliveryRequest(1, 3, 1));
    h.push(DeliveryRequest(2, 5, 2));
    h.push(DeliveryRequest(3, 5, 3));   // same priority as #2 but arrived later
    h.push(DeliveryRequest(4, 1, 4));
    h.push(DeliveryRequest(5, 4, 5));
    DeliveryRequest r;
    CHECK(h.pop(r) && r.trackingId == 2);
    CHECK(h.pop(r) && r.trackingId == 3);   // tie -> earlier arrival first
    CHECK(h.pop(r) && r.trackingId == 5);
    h.push(DeliveryRequest(6, 2, 6));
    CHECK(h.removeFirstIf([](const DeliveryRequest& q) { return q.trackingId == 1; }, r));
    CHECK(h.pop(r) && r.trackingId == 6);
    CHECK(h.pop(r) && r.trackingId == 4);
    CHECK(!h.pop(r));

    // 500 random-ish priorities must come out sorted
    Heap<DeliveryRequest, UrgentOrder> big;
    for (int i = 0; i < 500; ++i) big.push(DeliveryRequest(i, (i * 7919) % 5 + 1, i));
    int lastPri = 6;
    bool sorted = true;
    while (big.pop(r)) { if (r.priority > lastPri) sorted = false; lastPri = r.priority; }
    CHECK(sorted);
}

inline void testHash(HashTable<int>::Mode mode) {
    HashTable<int> t(mode, 11);
    int v = 0, probes = 0, slot = 0;
    CHECK(t.insert(1, 100));
    CHECK(t.insert(12, 200));    // 12 % 11 == 1 -> collision with key 1
    CHECK(t.insert(23, 300));    // collides again
    CHECK(!t.insert(12, 999));   // duplicate
    CHECK(t.find(23, v, probes, slot) && v == 300 && probes >= 3);
    CHECK(t.remove(12));         // leaves a tombstone
    CHECK(t.find(23, v, probes, slot) && v == 300);   // probe chain must NOT be broken
    CHECK(!t.find(12, v, probes, slot));
    CHECK(t.insert(12, 222));    // reuses the tombstone
    CHECK(t.find(12, v, probes, slot) && v == 222);
    for (int i = 1000; i < 1300; ++i) CHECK(t.insert(i, i * 2));   // forces several rehashes
    bool all = true;
    for (int i = 1000; i < 1300; ++i)
        if (!t.find(i, v, probes, slot) || v != i * 2) all = false;
    CHECK(all);
    CHECK(t.loadFactor() <= 0.5);
    CHECK(t.find(1, v, probes, slot) && v == 100);
}

inline void testTrees() {
    std::cout << "BST and AVL\n";
    BST<int> b;
    AVL<int> a;
    for (int k = 1; k <= 100; ++k) { b.insert(k, k); a.insert(k, k); }
    CHECK(b.height() == 100);
    CHECK(a.height() <= 8);                     // 1.44*log2(102) ~ 9.6; sequential input gives 7
    CHECK(a.size() == 100 && b.size() == 100);
    CHECK(!b.insert(50, 0) && !a.insert(50, 0));  // duplicates rejected
    int v = 0, c = 0;
    CHECK(a.search(77, v, c) && v == 77 && c <= a.height());
    CHECK(b.search(77, v, c) && c == 77);
    CHECK(!a.search(500, v, c));

    bool sortedA = true, sortedB = true;
    int prev = 0;
    a.inorder([&](int k, int) { if (k <= prev) sortedA = false; prev = k; });
    prev = 0;
    b.inorder([&](int k, int) { if (k <= prev) sortedB = false; prev = k; });
    CHECK(sortedA && sortedB);

    // all four rotation cases
    AVL<int> ll, rr, lr, rl;
    ll.insert(3, 0); ll.insert(2, 0); ll.insert(1, 0);
    rr.insert(1, 0); rr.insert(2, 0); rr.insert(3, 0);
    lr.insert(3, 0); lr.insert(1, 0); lr.insert(2, 0);
    rl.insert(1, 0); rl.insert(3, 0); rl.insert(2, 0);
    CHECK(ll.height() == 2 && rr.height() == 2 && lr.height() == 2 && rl.height() == 2);
    CHECK(lr.doubleRotationCount() == 1 && rl.doubleRotationCount() == 1);

    // traversals on a known BST: root 4, left child 2, right child 6
    BST<int> t;
    t.insert(4, 0); t.insert(2, 0); t.insert(6, 0);
    std::string pre, post, lvl;
    t.preorder([&](int k, int) { pre += std::to_string(k); });
    t.postorder([&](int k, int) { post += std::to_string(k); });
    t.levelOrder([&](int k, int) { lvl += std::to_string(k); });
    CHECK(pre == "426" && post == "264" && lvl == "426");
}

// Test graph:  A-B 4, A-C 1, C-B 2, B-D 5, D-E 3, and F isolated
inline void testGraph() {
    std::cout << "Graph, BFS, DFS, Dijkstra, Prim\n";
    Graph g;
    const char* names[] = { "A", "B", "C", "D", "E", "F" };
    for (int i = 0; i < 6; ++i) g.addLocation(names[i]);
    CHECK(g.addLocation("a") == -1);      // duplicate, case-insensitive
    CHECK(g.addRoad(0, 1, 4));
    g.addRoad(0, 2, 1);
    g.addRoad(2, 1, 2);
    g.addRoad(1, 3, 5);
    g.addRoad(3, 4, 3);
    CHECK(g.edgeCount() == 5);
    CHECK(!g.addRoad(1, 0, 9));           // updates existing road
    CHECK(g.matrixAt(0, 1) == 9 && g.matrixAt(1, 0) == 9 && g.edgeCount() == 5);
    g.addRoad(0, 1, 4);
    CHECK(!g.isValidRoad(0, 0, 3) && !g.isValidRoad(0, 1, 0));

    DynamicArray<int> order, hops, dist, parent, path;
    bfs(g, 0, order, hops);
    CHECK(order.size() == 5 && order[0] == 0 && order[1] == 1 && order[2] == 2);   // A, B, C, D, E
    CHECK(hops[3] == 2 && hops[5] == -1);
    dfs(g, 0, order);
    CHECK(order.size() == 5 && order[0] == 0 && order[1] == 1);                     // A, B, then deep
    dijkstra(g, 0, dist, parent);
    CHECK(dist[3] == 8);                  // A -> C -> B -> D = 1 + 2 + 5
    CHECK(dist[5] == INF);
    CHECK(buildPath(parent, 0, 3, path) && path.size() == 4 && path[1] == 2 && path[2] == 1);
    CHECK(!buildPath(parent, 0, 5, path));

    DynamicArray<MstEdge> mst;
    int comps = 0;
    int total = primMST(g, mst, comps);
    CHECK(total == 11 && mst.size() == 4 && comps == 2);   // 1 + 2 + 5 + 3 ; F is its own component

    // grow the matrix past its initial capacity of 8
    Graph big;
    for (int i = 0; i < 30; ++i) big.addLocation("L" + std::to_string(i));
    for (int i = 0; i < 29; ++i) big.addRoad(i, i + 1, i + 1);
    CHECK(big.matrixAt(28, 29) == 29 && big.matrixAt(29, 28) == 29 && big.hasRoad(0, 1));
    dijkstra(big, 0, dist, parent);
    CHECK(dist[29] == 29 * 30 / 2);
}

inline void testSystem() {
    std::cout << "Integrated system (dispatch rules and undo)\n";
    RouteWiseSystem sys;
    Graph& g = sys.graph();
    g.addLocation("Warehouse"); g.addLocation("Airport"); g.addLocation("Hospital");
    g.addRoad(0, 1, 10); g.addRoad(0, 2, 5);
    CHECK(sys.createCustomer(0, "Ali", "0300", "Lahore") != nullptr);
    std::string err, msg;
    CHECK(sys.createParcel(0, 999, "Warehouse", "Airport", 1, 1, err) == nullptr);       // unknown customer
    CHECK(sys.createParcel(0, 101, "Nowhere", "Airport", 1, 1, err) == nullptr);         // unknown location
    CHECK(sys.createParcel(0, 101, "Warehouse", "Airport", 1, 9, err) == nullptr);       // bad priority
    Parcel* p1 = sys.createParcel(0, 101, "warehouse", "AIRPORT", 2.0, 3, err);          // case-insensitive
    Parcel* p2 = sys.createParcel(0, 101, "Warehouse", "Hospital", 1.0, 5, err);
    Parcel* p3 = sys.createParcel(0, 101, "Warehouse", "Hospital", 1.0, 5, err);
    Parcel* p4 = sys.createParcel(0, 101, "Warehouse", "Airport", 1.0, 1, err);
    CHECK(p1 && p2 && p3 && p4);
    CHECK(p1->source == "Warehouse" && p1->trackingId == 1001 && p4->trackingId == 1004);
    CHECK(sys.lookup(1003) == p3 && sys.lookup(5555) == nullptr);

    // normal: p1, p4 ; urgent: p2 then p3 (both priority 5)
    CHECK(sys.submitRequest(1001, false, true, msg));
    CHECK(sys.submitRequest(1004, false, true, msg));
    CHECK(sys.submitRequest(1003, true, true, msg));
    CHECK(sys.submitRequest(1002, true, true, msg));
    CHECK(!sys.submitRequest(1002, true, true, msg));      // already pending
    CHECK(sys.pendingNormal() == 2 && sys.pendingUrgent() == 2);

    bool urgent = false;
    Parcel* d = sys.dispatchNext(urgent);
    CHECK(d == p3 && urgent);                              // p3 arrived BEFORE p2 -> first
    d = sys.dispatchNext(urgent);
    CHECK(d == p2 && urgent);
    d = sys.dispatchNext(urgent);
    CHECK(d == p1 && !urgent);                             // urgent heap empty -> queue in FIFO order
    CHECK(p1->status == Status::DISPATCHED);

    // undo the last dispatch: p1 must return to the FRONT of the normal queue
    CHECK(sys.undoLast(msg));
    CHECK(p1->status == Status::PENDING && sys.pendingNormal() == 2 && sys.historySize() == 2);
    d = sys.dispatchNext(urgent);
    CHECK(d == p1);

    CHECK(!sys.deliver(1004, msg));                        // p4 is still pending
    CHECK(sys.deliver(1001, msg) && p1->status == Status::DELIVERED);
    CHECK(sys.undoLast(msg) && p1->status == Status::DISPATCHED);

    // undo everything back to the start
    int guard = 0;
    while (sys.undoDepth() > 0 && guard++ < 100) sys.undoLast(msg);
    CHECK(p1->status == Status::REGISTERED && p2->status == Status::REGISTERED && p3->status == Status::REGISTERED);
    CHECK(sys.pendingNormal() == 0 && sys.pendingUrgent() == 0 && sys.historySize() == 0);
    CHECK(!sys.undoLast(msg));

    // many parcels: hash table + AVL must stay correct, AVL must stay shallow
    for (int i = 0; i < 200; ++i) sys.createParcel(0, 101, "Warehouse", "Airport", 1, 2, err);
    CHECK(sys.parcelCount() == 204);
    CHECK(sys.lookup(1204) != nullptr);
    CHECK(sys.avlHeight() <= 10 && sys.bstHeight() == 204);
}

inline int runAllTests() {
    std::cout << "=== RouteWise self-tests ===\n";
    testDynamicArray();
    testLists();
    testStackQueue();
    testHeap();
    std::cout << "Hash table (linear probing)\n";
    testHash(HashTable<int>::LINEAR);
    std::cout << "Hash table (quadratic probing)\n";
    testHash(HashTable<int>::QUADRATIC);
    testTrees();
    testGraph();
    testSystem();
    std::cout << "\n" << (g_checks - g_failed) << " / " << g_checks << " checks passed";
    if (g_failed == 0) std::cout << "  -> ALL TESTS PASSED\n";
    else std::cout << "  -> " << g_failed << " FAILED\n";
    return g_failed == 0 ? 0 : 1;
}
