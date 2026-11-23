// Stack.h
// Array-based LIFO stack built on our DynamicArray.
// Used for: UNDO (Module 3), iterative DFS, and rebuilding a shortest path in order.
#pragma once
#include "DynamicArray.h"

template <typename T>
class Stack {
private:
    DynamicArray<T> items_;

public:
    Stack() {}

    void push(const T& value) { items_.pushBack(value); }          // O(1) amortized

    bool pop(T& out) {                                              // O(1)
        if (items_.empty()) return false;
        out = items_.popBack();
        return true;
    }

    bool peek(T& out) const {                                       // O(1)
        if (items_.empty()) return false;
        out = items_[items_.size() - 1];
        return true;
    }

    bool empty() const { return items_.empty(); }
    int size() const { return items_.size(); }
    void clear() { items_.clear(); }
};
