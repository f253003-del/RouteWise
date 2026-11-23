// HashTable.h
// Open-addressing hash table (integer keys) with LINEAR or QUADRATIC probing.
//  - hash function  : key mod capacity   (capacity is always PRIME)
//  - deletion       : "tombstones" (DELETED slots) so probe chains do not break
//  - resizing       : when load factor would exceed 0.5, capacity -> next prime >= 2x, rehash all
//    (load <= 0.5 with a prime size guarantees quadratic probing always finds a free slot)
//  - statistics     : probes per lookup, collisions - shown in the System Statistics screen
// Used for: fast parcel lookup by tracking ID (Module 1), average O(1).
#pragma once

template <typename V>
class HashTable {
public:
    enum Mode { LINEAR, QUADRATIC };

private:
    enum State { EMPTY = 0, OCCUPIED = 1, DELETED = 2 };

    struct Slot {
        int key;
        V value;
        State state;
        Slot() : key(0), value(), state(EMPTY) {}
    };

    Slot* table_;
    int capacity_;
    int count_;        // OCCUPIED slots
    int tombstones_;   // DELETED slots
    Mode mode_;
    long totalProbes_;
    long totalSearches_;
    long collisions_;

    static bool isPrime(int n) {
        if (n < 2) return false;
        for (int i = 2; (long long)i * i <= n; ++i)
            if (n % i == 0) return false;
        return true;
    }
    static int nextPrime(int n) {
        while (!isPrime(n)) ++n;
        return n;
    }

    int hash(int key) const {
        int h = key % capacity_;
        return h < 0 ? h + capacity_ : h;
    }

    // i-th slot of the probe sequence that starts at home slot h.
    int probe(int h, int i) const {
        long long idx = (mode_ == LINEAR) ? (long long)h + i
            : (long long)h + (long long)i * i;
        return (int)(idx % capacity_);
    }

    // Insert without any resizing check (used by insert and rehash).
    bool place(int key, const V& value, bool countStats) {
        int h = hash(key);
        int firstDeleted = -1;
        for (int i = 0; i < capacity_; ++i) {
            int idx = probe(h, i);
            Slot& s = table_[idx];
            if (s.state == EMPTY) {
                int target = (firstDeleted >= 0) ? firstDeleted : idx;
                if (firstDeleted >= 0) --tombstones_;
                table_[target].key = key;
                table_[target].value = value;
                table_[target].state = OCCUPIED;
                ++count_;
                if (countStats && i > 0) ++collisions_;
                return true;
            }
            if (s.state == DELETED) {
                if (firstDeleted < 0) firstDeleted = idx;
            }
            else if (s.key == key) {
                return false;                       // duplicate key
            }
        }
        if (firstDeleted >= 0) {                    // whole table walked, reuse a tombstone
            --tombstones_;
            table_[firstDeleted].key = key;
            table_[firstDeleted].value = value;
            table_[firstDeleted].state = OCCUPIED;
            ++count_;
            return true;
        }
        return false;
    }

    void rehash(int newCapacity) {
        Slot* old = table_;
        int oldCapacity = capacity_;
        table_ = new Slot[newCapacity];
        capacity_ = newCapacity;
        count_ = 0;
        tombstones_ = 0;
        for (int i = 0; i < oldCapacity; ++i)
            if (old[i].state == OCCUPIED) place(old[i].key, old[i].value, false);
        delete[] old;
    }

public:
    explicit HashTable(Mode mode = QUADRATIC, int initialCapacity = 11)
        : table_(nullptr), capacity_(nextPrime(initialCapacity)), count_(0), tombstones_(0),
        mode_(mode), totalProbes_(0), totalSearches_(0), collisions_(0) {
        table_ = new Slot[capacity_];
    }
    ~HashTable() { delete[] table_; }

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Average O(1). Returns false if the key already exists.
    bool insert(int key, const V& value) {
        if ((count_ + tombstones_ + 1) * 2 > capacity_) rehash(nextPrime(capacity_ * 2));
        return place(key, value, true);
    }

    // Average O(1). probes = number of slots examined, slot = index where it was found.
    bool find(int key, V& out, int& probes, int& slot, bool track = true) {
        int h = hash(key);
        probes = 0;
        slot = -1;
        bool found = false;
        for (int i = 0; i < capacity_; ++i) {
            int idx = probe(h, i);
            ++probes;
            Slot& s = table_[idx];
            if (s.state == EMPTY) break;
            if (s.state == OCCUPIED && s.key == key) {
                out = s.value;
                slot = idx;
                found = true;
                break;
            }
        }
        if (track) {
            ++totalSearches_;
            totalProbes_ += probes;
        }
        return found;
    }

    // Average O(1). Leaves a tombstone.
    bool remove(int key) {
        int h = hash(key);
        for (int i = 0; i < capacity_; ++i) {
            int idx = probe(h, i);
            Slot& s = table_[idx];
            if (s.state == EMPTY) return false;
            if (s.state == OCCUPIED && s.key == key) {
                s.state = DELETED;
                s.value = V();
                --count_;
                ++tombstones_;
                return true;
            }
        }
        return false;
    }

    // Visit every stored entry: f(slotIndex, key, value)
    template <typename F>
    void forEach(F f) const {
        for (int i = 0; i < capacity_; ++i)
            if (table_[i].state == OCCUPIED) f(i, table_[i].key, table_[i].value);
    }

    int size() const { return count_; }
    int capacity() const { return capacity_; }
    double loadFactor() const { return (double)count_ / capacity_; }
    long collisions() const { return collisions_; }
    long searches() const { return totalSearches_; }
    double averageProbes() const {
        return totalSearches_ == 0 ? 0.0 : (double)totalProbes_ / totalSearches_;
    }
    const char* modeName() const { return mode_ == LINEAR ? "Linear probing" : "Quadratic probing"; }
};
