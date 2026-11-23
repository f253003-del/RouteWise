// Models.h
// The "records" of the system. Customer and Parcel objects are created with `new`
// (dynamic memory) and owned by RouteWiseSystem, which deletes them in its destructor.
// Every other structure (hash table, BST, AVL, queue, heap) only stores pointers / IDs.
#pragma once
#include <string>

enum class Status { REGISTERED, PENDING, DISPATCHED, DELIVERED };
enum class RequestType { NONE, NORMAL, URGENT };

inline const char* statusToString(Status s) {
    switch (s) {
    case Status::REGISTERED: return "Registered";
    case Status::PENDING:    return "Pending";
    case Status::DISPATCHED: return "Dispatched";
    case Status::DELIVERED:  return "Delivered";
    }
    return "?";
}

inline const char* requestTypeToString(RequestType t) {
    switch (t) {
    case RequestType::NONE:   return "-";
    case RequestType::NORMAL: return "Normal";
    case RequestType::URGENT: return "Urgent";
    }
    return "?";
}

inline const char* priorityToString(int p) {
    switch (p) {
    case 1: return "Low";
    case 2: return "Medium";
    case 3: return "High";
    case 4: return "Very High";
    case 5: return "Critical";
    }
    return "?";
}

struct Customer {
    int id;
    std::string name;
    std::string phone;
    std::string address;
    Customer() : id(0) {}
    Customer(int i, const std::string& n, const std::string& p, const std::string& a)
        : id(i), name(n), phone(p), address(a) {
    }
};

struct Parcel {
    int trackingId;
    int customerId;
    std::string source;
    std::string destination;
    double weight;          // kg
    int priority;           // 1 (low) .. 5 (critical)
    Status status;
    RequestType requestType;
    long arrivalSeq;        // arrival order number of its delivery request (0 = not requested)

    Parcel()
        : trackingId(0), customerId(0), weight(0), priority(1),
        status(Status::REGISTERED), requestType(RequestType::NONE), arrivalSeq(0) {
    }
    Parcel(int t, int c, const std::string& s, const std::string& d, double w, int p)
        : trackingId(t), customerId(c), source(s), destination(d), weight(w), priority(p),
        status(Status::REGISTERED), requestType(RequestType::NONE), arrivalSeq(0) {
    }
};

// What sits inside the normal queue and the urgent heap.
struct DeliveryRequest {
    int trackingId;
    int priority;
    long seq;               // global arrival counter: smaller = arrived earlier
    DeliveryRequest() : trackingId(0), priority(0), seq(0) {}
    DeliveryRequest(int t, int p, long s) : trackingId(t), priority(p), seq(s) {}
};

// Heap rule (Module 2): higher priority first; equal priority -> earlier arrival first.
struct UrgentOrder {
    bool operator()(const DeliveryRequest& a, const DeliveryRequest& b) const {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.seq < b.seq;
    }
};

// One line of the delivery history (doubly linked list).
struct HistoryEntry {
    long number;
    int trackingId;
    std::string event;      // "DISPATCHED" or "DELIVERED"
    HistoryEntry() : number(0), trackingId(0) {}
    HistoryEntry(long n, int t, const std::string& e) : number(n), trackingId(t), event(e) {}
};

// One entry of the undo stack: enough information to reverse the operation.
enum class OpType { ADD_NORMAL, ADD_URGENT, PROCESS, DELIVER };

struct UndoRecord {
    OpType type;
    int trackingId;
    Status prevStatus;          // parcel status BEFORE the operation
    DeliveryRequest request;    // the request involved (needed to put it back)
    bool fromUrgent;            // PROCESS: did it come from the heap (true) or the queue (false)?
    UndoRecord() : type(OpType::ADD_NORMAL), trackingId(0), prevStatus(Status::REGISTERED), fromUrgent(false) {}
};
