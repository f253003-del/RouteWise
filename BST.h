// BST.h
// Plain (unbalanced) Binary Search Tree keyed by an int (tracking ID).
// Module 4: ordered parcel index. Search counts key COMPARISONS so we can
// compare against the AVL tree. Height = number of NODES on the longest path.
// Worst case (sorted input) it degenerates into a linked list: height = n.
#pragma once
#include <iostream>
#include <string>
#include "Queue.h"

template <typename V>
class BST {
public:
    struct Node {
        int key;
        V value;
        Node* left;
        Node* right;
        Node(int k, const V& v) : key(k), value(v), left(nullptr), right(nullptr) {}
    };

private:
    Node* root_;
    int size_;

    static void destroy(Node* n) {
        if (n == nullptr) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }
    static int heightOf(Node* n) {
        if (n == nullptr) return 0;
        int l = heightOf(n->left), r = heightOf(n->right);
        return 1 + (l > r ? l : r);
    }
    template <typename F> static void inorderRec(Node* n, F& f) {
        if (n == nullptr) return;
        inorderRec(n->left, f);
        f(n->key, n->value);
        inorderRec(n->right, f);
    }
    template <typename F> static void preorderRec(Node* n, F& f) {
        if (n == nullptr) return;
        f(n->key, n->value);
        preorderRec(n->left, f);
        preorderRec(n->right, f);
    }
    template <typename F> static void postorderRec(Node* n, F& f) {
        if (n == nullptr) return;
        postorderRec(n->left, f);
        postorderRec(n->right, f);
        f(n->key, n->value);
    }
    static void printRec(Node* n, int depth, std::ostream& os) {
        if (n == nullptr) return;
        printRec(n->right, depth + 1, os);
        os << std::string(depth * 6, ' ') << n->key << "\n";
        printRec(n->left, depth + 1, os);
    }

public:
    BST() : root_(nullptr), size_(0) {}
    ~BST() { destroy(root_); }
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    // O(h): iterative insert. Returns false for a duplicate key.
    bool insert(int key, const V& value) {
        Node* parent = nullptr;
        Node* cur = root_;
        while (cur != nullptr) {
            parent = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else return false;
        }
        Node* n = new Node(key, value);
        if (parent == nullptr) root_ = n;
        else if (key < parent->key) parent->left = n;
        else parent->right = n;
        ++size_;
        return true;
    }

    // O(h). comparisons = how many nodes were compared against the key.
    bool search(int key, V& out, int& comparisons) const {
        comparisons = 0;
        Node* cur = root_;
        while (cur != nullptr) {
            ++comparisons;
            if (key == cur->key) { out = cur->value; return true; }
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }

    int height() const { return heightOf(root_); }
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }

    template <typename F> void inorder(F f) const { inorderRec(root_, f); }
    template <typename F> void preorder(F f) const { preorderRec(root_, f); }
    template <typename F> void postorder(F f) const { postorderRec(root_, f); }

    // Level-order (breadth-first) uses our own Queue.
    template <typename F> void levelOrder(F f) const {
        if (root_ == nullptr) return;
        Queue<Node*> q;
        q.enqueue(root_);
        Node* n = nullptr;
        while (q.dequeue(n)) {
            f(n->key, n->value);
            if (n->left) q.enqueue(n->left);
            if (n->right) q.enqueue(n->right);
        }
    }

    // Draw the tree sideways (root on the left). Only sensible for small trees.
    void printSideways(std::ostream& os) const { printRec(root_, 0, os); }
};
