
#pragma once
#include "DynamicArray.h"

template <typename T, typename Cmp>
class Heap {
private:
    DynamicArray<T> a_;
    Cmp cmp_;

    void swapAt(int i, int j) {
        T temp = a_[i];
        a_[i] = a_[j];
        a_[j] = temp;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;

            if (cmp_(a_[i], a_[parent])) {
                swapAt(i, parent);
                i = parent;
            }
            else {
                break;
            }
        }
    }

    void siftDown(int i) {
        int n = a_.size();

        while (true) {
            int left = 2 * i + 1;
            int right = left + 1;
            int best = i;

            if (left < n && cmp_(a_[left], a_[best]))
                best = left;

            if (right < n && cmp_(a_[right], a_[best]))
                best = right;

            if (best == i)
                break;

            swapAt(i, best);
            i = best;
        }
    }

public:
    Heap() = default;

    void push(const T& value) {
        a_.pushBack(value);
        siftUp(a_.size() - 1);
    }

    bool pop(T& out) {
        if (a_.empty())
            return false;

        out = a_[0];

        T last = a_.popBack();

        if (!a_.empty()) {
            a_[0] = last;
            siftDown(0);
        }

        return true;
    }

    bool top(T& out) const {
        if (a_.empty())
            return false;

        out = a_[0];
        return true;
    }

    template <typename Pred>
    bool removeFirstIf(Pred pred, T& out) {
        for (int i = 0; i < a_.size(); i++) {
            if (pred(a_[i])) {
                out = a_[i];

                T last = a_.popBack();

                if (i < a_.size()) {
                    a_[i] = last;

                    if (i > 0 && cmp_(a_[i], a_[(i - 1) / 2]))
                        siftUp(i);
                    else
                        siftDown(i);
                }

                return true;
            }
        }

        return false;
    }

    void sortedSnapshot(DynamicArray<T>& out) const {
        Heap<T, Cmp> copy;

        for (int i = 0; i < a_.size(); i++)
            copy.a_.pushBack(a_[i]);

        out.clear();

        T item;

        while (copy.pop(item))
            out.pushBack(item);
    }

    bool empty() const {
        return a_.empty();
    }

    int size() const {
        return a_.size();
    }

    void clear() {
        a_.clear();
    }
};
