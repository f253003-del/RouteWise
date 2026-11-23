// AVL.h
// Self-balancing Binary Search Tree (Module 4). After every insert, each node's
// balance factor  bf = height(left) - height(right)  is kept in {-1, 0, +1}
// using rotations. Guarantees height <= 1.44 * log2(n+2), so search is O(log n).
//   LL case -> rotate right      RR case -> rotate left
//   LR case -> rotate left child, then rotate right
//   RL case -> rotate right child, then rotate left
#pragma once
#include <iostream>
#include <string>

template <typename V>
class AVL {
public:
    struct Node {
        int key;
        V value;
        Node* left;
        Node* right;
        int height;           // height of the subtree rooted here (leaf = 1)
        Node(int k, const V& v) : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
    };

private:
    Node* root_;
    int size_;
    int rotations_;           // single rotations performed (a double rotation = 2 of them)
    int doubleRotations_;     // LR + RL cases

    static int h(Node* n) { return n ? n->height : 0; }
    static void update(Node* n) {
        int l = h(n->left), r = h(n->right);
        n->height = 1 + (l > r ? l : r);
    }
    static int balanceOf(Node* n) { return n ? h(n->left) - h(n->right) : 0; }

    static void destroy(Node* n) {
        if (n == nullptr) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    // Right rotation (fixes the LL case):
    //        y                 x
    //       / (left)          / (right)
    //      x        ==>      ...  y
    //   x's right subtree (t2) moves to become y's left subtree.
    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* t2 = x->right;
        x->right = y;
        y->left = t2;
        update(y);          // y is now lower, so update it FIRST
        update(x);
        ++rotations_;
        return x;           // x is the new root of this subtree
    }

    // Left rotation (fixes the RR case) - the mirror image:
    // y's left subtree (t2) moves to become x's right subtree, y becomes the new root.
    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* t2 = y->left;
        y->left = x;
        x->right = t2;
        update(x);
        update(y);
        ++rotations_;
        return y;
    }

    Node* insertRec(Node* n, int key, const V& value, bool& inserted) {
        if (n == nullptr) {
            inserted = true;
            return new Node(key, value);
        }
        if (key < n->key) n->left = insertRec(n->left, key, value, inserted);
        else if (key > n->key) n->right = insertRec(n->right, key, value, inserted);
        else { inserted = false; return n; }

        update(n);
        int bf = balanceOf(n);

        if (bf > 1 && key < n->left->key) return rotateRight(n);          // LL
        if (bf < -1 && key > n->right->key) return rotateLeft(n);         // RR
        if (bf > 1 && key > n->left->key) {                               // LR
            n->left = rotateLeft(n->left);
            ++doubleRotations_;
            return rotateRight(n);
        }
        if (bf < -1 && key < n->right->key) {                             // RL
            n->right = rotateRight(n->right);
            ++doubleRotations_;
            return rotateLeft(n);
        }
        return n;
    }

    template <typename F> static void inorderRec(Node* n, F& f) {
        if (n == nullptr) return;
        inorderRec(n->left, f);
        f(n->key, n->value);
        inorderRec(n->right, f);
    }
    static void printRec(Node* n, int depth, std::ostream& os) {
        if (n == nullptr) return;
        printRec(n->right, depth + 1, os);
        os << std::string(depth * 6, ' ') << n->key << " (h=" << n->height << ")\n";
        printRec(n->left, depth + 1, os);
    }

public:
    AVL() : root_(nullptr), size_(0), rotations_(0), doubleRotations_(0) {}
    ~AVL() { destroy(root_); }
    AVL(const AVL&) = delete;
    AVL& operator=(const AVL&) = delete;

    // O(log n)
    bool insert(int key, const V& value) {
        bool inserted = false;
        root_ = insertRec(root_, key, value, inserted);
        if (inserted) ++size_;
        return inserted;
    }

    // O(log n)
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

    int height() const { return h(root_); }
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    int rotationCount() const { return rotations_; }
    int doubleRotationCount() const { return doubleRotations_; }

    template <typename F> void inorder(F f) const { inorderRec(root_, f); }
    void printSideways(std::ostream& os) const { printRec(root_, 0, os); }
};
