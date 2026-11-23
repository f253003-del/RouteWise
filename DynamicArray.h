// DynamicArray.h
// Array-based ADT: a resizable array (our own replacement for std::vector).
// Used inside Heap, Stack, Graph (location names) and many algorithms.
#pragma once
#include <stdexcept>

template <typename T>
class DynamicArray {
private:
    T* data_;
    int size_;
    int capacity_;

    // Allocate a bigger block, copy the old elements, free the old block.
    void grow(int newCapacity) {
        T* bigger = new T[newCapacity];
        for (int i = 0; i < size_; ++i) bigger[i] = data_[i];
        delete[] data_;
        data_ = bigger;
        capacity_ = newCapacity;
    }

public:
    explicit DynamicArray(int capacity = 8)
        : data_(nullptr), size_(0), capacity_(capacity > 0 ? capacity : 1) {
        data_ = new T[capacity_];
    }
    ~DynamicArray() { delete[] data_; }

    // Copying is disabled: it would copy the raw pointer and cause a double delete.
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    // O(1) amortized: the array doubles when full.
    void pushBack(const T& value) {
        if (size_ == capacity_) grow(capacity_ * 2);
        data_[size_++] = value;
    }

    // O(1)
    T popBack() {
        if (size_ == 0) throw std::out_of_range("DynamicArray::popBack on empty array");
        return data_[--size_];
    }

    // O(n): shifts everything after index to the left.
    void removeAt(int index) {
        if (index < 0 || index >= size_) throw std::out_of_range("DynamicArray::removeAt");
        for (int i = index; i < size_ - 1; ++i) data_[i] = data_[i + 1];
        --size_;
    }

    // Replace contents with n copies of value (handy for dist[] = INF, visited[] = false).
    void assign(int n, const T& value) {
        size_ = 0;
        for (int i = 0; i < n; ++i) pushBack(value);
    }

    T& operator[](int index) {
        if (index < 0 || index >= size_) throw std::out_of_range("DynamicArray index");
        return data_[index];
    }
    const T& operator[](int index) const {
        if (index < 0 || index >= size_) throw std::out_of_range("DynamicArray index");
        return data_[index];
    }

    int size() const { return size_; }
    int capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    void clear() { size_ = 0; }
};
