// RouteWiseSystem.h
// The integrated application. It OWNS every Customer and Parcel record and connects
// all the data structures together:
//
//   Customer list ........ SinglyLinkedList<Customer*>   (Module 1)
//   Parcel master list ... SinglyLinkedList<Parcel*>      (owns the memory)
//   Tracking-ID lookup ... HashTable<Parcel*>             (Module 1, quadratic probing)
//   Normal deliveries .... Queue<DeliveryRequest>         (Module 2, FIFO)
//   Urgent deliveries .... Heap<DeliveryRequest>          (Module 2, priority + arrival)
//   Delivery history ..... DoublyLinkedList<HistoryEntry> (Module 3)
//   Undo ................. Stack<UndoRecord>              (Module 3)
//   Ordered indexes ...... BST<Parcel*> and AVL<Parcel*>  (Module 4)
//   City network ......... Graph (list + matrix)          (Modules 5-8)
#pragma once
#include <string>
#include "Models.h"
#include "SinglyLinkedList.h"
#include "DoublyLinkedList.h"
#include "Stack.h"
#include "Queue.h"
#include "Heap.h"
#include "HashTable.h"
#include "BST.h"
#include "AVL.h"
#include "Graph.h"
#include "GraphAlgorithms.h"

typedef Heap<DeliveryRequest, UrgentOrder> UrgentHeap;

class RouteWiseSystem {
private:
    SinglyLinkedList<Customer*> customers_;
    SinglyLinkedList<Parcel*> parcels_;
    HashTable<Parcel*> parcelHash_;
    BST<Parcel*> bstIndex_;
    AVL<Parcel*> avlIndex_;
    Queue<DeliveryRequest> normalQueue_;
    UrgentHeap urgentHeap_;
    DoublyLinkedList<HistoryEntry> history_;
    Stack<UndoRecord> undoStack_;
    Graph graph_;

    int nextCustomerId_;
    int nextTrackingId_;
    long seqCounter_;       // arrival order of delivery requests
    long historyCounter_;

    // ---- helpers
    Customer* findCustomer(int id) const;
    Parcel* findParcel(int trackingId);               // hash lookup (not counted in statistics)
    void logHistory(int trackingId, const std::string& event);

    // ---- interactive screens (one per menu item)
    void registerCustomerUI();
    void registerParcelUI();
    void searchParcelUI();
    void addRequestUI(bool urgent);
    void processNextUI();
    void markDeliveredUI();
    void undoUI();
    void displayPendingUI();
    void displayParcelsUI();
    void networkMenuUI();
    void bfsUI();
    void dfsUI();
    void shortestRouteUI();
    void mstUI();
    void compareTreesUI();
    void statisticsUI();

    // ---- printing helpers
    void printMenu() const;
    void printParcelHeader() const;
    void printParcelRow(const Parcel* p) const;
    void printLocationList() const;
    int askLocation(const std::string& prompt) const;   // number or name; blank = -1
    void printShortestRoute(int src, int dst) const;
    void exportHistory(const std::string& path) const;

public:
    RouteWiseSystem();
    ~RouteWiseSystem();
    RouteWiseSystem(const RouteWiseSystem&) = delete;
    RouteWiseSystem& operator=(const RouteWiseSystem&) = delete;

    // ---- core (non-interactive) API: used by the menu, the file loaders and the self-tests
    Customer* createCustomer(int id, const std::string& name, const std::string& phone, const std::string& address);
    Parcel* createParcel(int trackingId, int customerId, const std::string& source,
        const std::string& destination, double weight, int priority, std::string& error);
    bool submitRequest(int trackingId, bool urgent, bool recordUndo, std::string& message);
    Parcel* dispatchNext(bool& wasUrgent);               // Process Next Delivery
    bool deliver(int trackingId, std::string& message);  // Mark as Delivered
    bool undoLast(std::string& message);                 // Undo Last Operation

    Graph& graph() { return graph_; }
    const Graph& graph() const { return graph_; }
    Parcel* lookup(int trackingId) { return findParcel(trackingId); }
    int pendingNormal() const { return normalQueue_.size(); }
    int pendingUrgent() const { return urgentHeap_.size(); }
    int historySize() const { return history_.size(); }
    int undoDepth() const { return undoStack_.size(); }
    int parcelCount() const { return parcels_.size(); }
    int customerCount() const { return customers_.size(); }
    int avlHeight() const { return avlIndex_.height(); }
    int bstHeight() const { return bstIndex_.height(); }

    // ---- file loading ('|' separated text files, see data/ folder)
    int loadNetworkFile(const std::string& path);     // returns lines loaded, -1 if file missing
    int loadCustomersFile(const std::string& path);
    int loadParcelsFile(const std::string& path);
    int loadRequestsFile(const std::string& path);
    bool loadSampleData(const std::string& folder);

    void run();                                        // the main menu loop
};
