// DoublyLinkedList.h
// Custom doubly linked list. Used for the DELIVERY HISTORY (Module 3):
//  - pushBack  : record a new event at the end (O(1))
//  - popBack   : undo removes the newest event (O(1)) - impossible in O(1) on a singly list
//  - traversal in BOTH directions (oldest->newest and newest->oldest)
#pragma once

template <typename T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
    };

private:
    Node* head_;
    Node* tail_;
    int size_;

public:
    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}
    ~DoublyLinkedList() { clear(); }

    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    void pushBack(const T& value) {
        Node* n = new Node(value);
        n->prev = tail_;
        if (tail_ != nullptr) tail_->next = n;
        else head_ = n;
        tail_ = n;
        ++size_;
    }

    void pushFront(const T& value) {
        Node* n = new Node(value);
        n->next = head_;
        if (head_ != nullptr) head_->prev = n;
        else tail_ = n;
        head_ = n;
        ++size_;
    }

    bool popBack(T& out) {
        if (tail_ == nullptr) return false;
        Node* old = tail_;
        out = old->data;
        tail_ = old->prev;
        if (tail_ != nullptr) tail_->next = nullptr;
        else head_ = nullptr;
        delete old;
        --size_;
        return true;
    }

    bool popFront(T& out) {
        if (head_ == nullptr) return false;
        Node* old = head_;
        out = old->data;
        head_ = old->next;
        if (head_ != nullptr) head_->prev = nullptr;
        else tail_ = nullptr;
        delete old;
        --size_;
        return true;
    }

    Node* head() const { return head_; }
    Node* tail() const { return tail_; }
    bool empty() const { return size_ == 0; }
    int size() const { return size_; }

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
