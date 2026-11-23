// Queue.h
// Linked-list-based FIFO queue built on SinglyLinkedList.
// Used for: NORMAL deliveries (Module 2) and BFS (Module 6).
#pragma once
#include "SinglyLinkedList.h"

template <typename T>
class Queue {
private:
    SinglyLinkedList<T> list_;

public:
    typedef typename SinglyLinkedList<T>::Node Node;

    Queue() {}

    void enqueue(const T& value) { list_.pushBack(value); }          // O(1)
    bool dequeue(T& out) { return list_.popFront(out); }             // O(1)

    // Only used by UNDO: puts an element back at the FRONT so the original order is restored.
    void enqueueFront(const T& value) { list_.pushFront(value); }    // O(1)

    // Only used by UNDO of "add request": removes a specific element from the middle.
    template <typename Pred>
    bool removeFirstIf(Pred pred, T& out) { return list_.removeFirstIf(pred, out); }  // O(n)

    bool front(T& out) const {
        if (list_.empty()) return false;
        out = list_.head()->data;
        return true;
    }

    Node* head() const { return list_.head(); }   // read-only walking for display
    bool empty() const { return list_.empty(); }
    int size() const { return list_.size(); }
    void clear() { list_.clear(); }
};
