// SinglyLinkedList.h
// Custom singly linked list with head and tail pointers.
// Reused by: Queue (normal deliveries), Graph (adjacency lists),
// and the system's customer / parcel record lists.
#pragma once

template <typename T>
class SinglyLinkedList {
public:
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& value) : data(value), next(nullptr) {}
    };

private:
    Node* head_;
    Node* tail_;
    int size_;

public:
    SinglyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}
    ~SinglyLinkedList() { clear(); }

    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    // O(1)
    void pushFront(const T& value) {
        Node* n = new Node(value);
        n->next = head_;
        head_ = n;
        if (tail_ == nullptr) tail_ = n;
        ++size_;
    }

    // O(1) thanks to the tail pointer
    void pushBack(const T& value) {
        Node* n = new Node(value);
        if (tail_ != nullptr) tail_->next = n;
        else head_ = n;
        tail_ = n;
        ++size_;
    }

    // O(1). Returns false if the list is empty.
    bool popFront(T& out) {
        if (head_ == nullptr) return false;
        Node* old = head_;
        out = old->data;
        head_ = old->next;
        if (head_ == nullptr) tail_ = nullptr;
        delete old;
        --size_;
        return true;
    }

    // O(n): removes the first element for which pred(element) is true.
    template <typename Pred>
    bool removeFirstIf(Pred pred, T& out) {
        Node* prev = nullptr;
        Node* cur = head_;
        while (cur != nullptr) {
            if (pred(cur->data)) {
                out = cur->data;
                if (prev != nullptr) prev->next = cur->next;
                else head_ = cur->next;
                if (cur == tail_) tail_ = prev;
                delete cur;
                --size_;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    // O(n): returns the first node whose data satisfies pred, or nullptr.
    template <typename Pred>
    Node* findIf(Pred pred) const {
        for (Node* cur = head_; cur != nullptr; cur = cur->next)
            if (pred(cur->data)) return cur;
        return nullptr;
    }

    Node* head() const { return head_; }
    bool empty() const { return size_ == 0; }
    int size() const { return size_; }

    // Frees every node (only the nodes; if T is a pointer, the caller owns what it points to).
    void clear() {
        while (head_ != nullptr) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
        size_ = 0;
    }
};
