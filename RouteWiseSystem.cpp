#include "RouteWiseSystem.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using std::cout;
using std::string;

// ====================================================================== construction
RouteWiseSystem::RouteWiseSystem()
    : parcelHash_(HashTable<Parcel*>::QUADRATIC, 11),
    nextCustomerId_(101), nextTrackingId_(1001), seqCounter_(0), historyCounter_(0) {
}

// The system OWNS all Customer and Parcel objects, so it must delete them.
RouteWiseSystem::~RouteWiseSystem() {
    Customer* c = nullptr;
    while (customers_.popFront(c)) delete c;
    Parcel* p = nullptr;
    while (parcels_.popFront(p)) delete p;
    // The other structures (hash, BST, AVL, queue, heap...) only hold pointers/IDs and clean their own nodes.
}

// ====================================================================== helpers
Customer* RouteWiseSystem::findCustomer(int id) const {
    SinglyLinkedList<Customer*>::Node* n = customers_.findIf([id](Customer* c) { return c->id == id; });
    return n ? n->data : nullptr;
}

Parcel* RouteWiseSystem::findParcel(int trackingId) {
    Parcel* p = nullptr;
    int probes = 0, slot = 0;
    if (parcelHash_.find(trackingId, p, probes, slot, false)) return p;
    return nullptr;
}

void RouteWiseSystem::logHistory(int trackingId, const string& event) {
    history_.pushBack(HistoryEntry(++historyCounter_, trackingId, event));
}

// ====================================================================== core API
Customer* RouteWiseSystem::createCustomer(int id, const string& name, const string& phone, const string& address) {
    if (id <= 0) id = nextCustomerId_;
    if (findCustomer(id) != nullptr) return nullptr;            // duplicate ID
    if (id >= nextCustomerId_) nextCustomerId_ = id + 1;
    Customer* c = new Customer(id, name, phone, address);       // dynamic allocation
    customers_.pushBack(c);
    return c;
}

Parcel* RouteWiseSystem::createParcel(int trackingId, int customerId, const string& source,
    const string& destination, double weight, int priority, string& error) {
    if (findCustomer(customerId) == nullptr) { error = "Customer " + std::to_string(customerId) + " does not exist."; return nullptr; }
    int s = graph_.indexOf(source);
    int d = graph_.indexOf(destination);
    if (s < 0) { error = "Unknown source location '" + source + "'."; return nullptr; }
    if (d < 0) { error = "Unknown destination location '" + destination + "'."; return nullptr; }
    if (weight <= 0) { error = "Weight must be positive."; return nullptr; }
    if (priority < 1 || priority > 5) { error = "Priority must be 1-5."; return nullptr; }
    if (trackingId <= 0) trackingId = nextTrackingId_;
    if (findParcel(trackingId) != nullptr) { error = "Tracking ID " + std::to_string(trackingId) + " already exists."; return nullptr; }

    Parcel* p = new Parcel(trackingId, customerId, graph_.name(s), graph_.name(d), weight, priority);
    parcelHash_.insert(trackingId, p);      // fast lookup by tracking ID
    bstIndex_.insert(trackingId, p);        // plain BST index
    avlIndex_.insert(trackingId, p);        // balanced AVL index
    parcels_.pushBack(p);                   // master list (ownership)
    if (trackingId >= nextTrackingId_) nextTrackingId_ = trackingId + 1;
    return p;
}

bool RouteWiseSystem::submitRequest(int trackingId, bool urgent, bool recordUndo, string& message) {
    Parcel* p = findParcel(trackingId);
    if (p == nullptr) { message = "No parcel with tracking ID " + std::to_string(trackingId) + "."; return false; }
    if (p->status != Status::REGISTERED) {
        message = "Parcel " + std::to_string(trackingId) + " is already '" + statusToString(p->status) + "' - it cannot be requested again.";
        return false;
    }
    DeliveryRequest req(trackingId, p->priority, ++seqCounter_);
    UndoRecord u;
    u.type = urgent ? OpType::ADD_URGENT : OpType::ADD_NORMAL;
    u.trackingId = trackingId;
    u.prevStatus = p->status;
    u.request = req;

    p->status = Status::PENDING;
    p->requestType = urgent ? RequestType::URGENT : RequestType::NORMAL;
    p->arrivalSeq = req.seq;
    if (urgent) urgentHeap_.push(req);
    else normalQueue_.enqueue(req);
    if (recordUndo) undoStack_.push(u);

    message = string(urgent ? "URGENT" : "NORMAL") + " request added for parcel " + std::to_string(trackingId) +
        " (priority " + std::to_string(p->priority) + ", arrival #" + std::to_string(req.seq) + ").";
    return true;
}

// Urgent heap has priority over the normal queue.
Parcel* RouteWiseSystem::dispatchNext(bool& wasUrgent) {
    DeliveryRequest req;
    if (urgentHeap_.pop(req)) wasUrgent = true;
    else if (normalQueue_.dequeue(req)) wasUrgent = false;
    else return nullptr;

    Parcel* p = findParcel(req.trackingId);
    UndoRecord u;
    u.type = OpType::PROCESS;
    u.trackingId = req.trackingId;
    u.prevStatus = p->status;
    u.request = req;
    u.fromUrgent = wasUrgent;

    p->status = Status::DISPATCHED;
    logHistory(p->trackingId, "DISPATCHED");
    undoStack_.push(u);
    return p;
}

bool RouteWiseSystem::deliver(int trackingId, string& message) {
    Parcel* p = findParcel(trackingId);
    if (p == nullptr) { message = "No parcel with tracking ID " + std::to_string(trackingId) + "."; return false; }
    if (p->status != Status::DISPATCHED) {
        message = "Parcel " + std::to_string(trackingId) + " is '" + statusToString(p->status) + "'. Only DISPATCHED parcels can be delivered.";
        return false;
    }
    UndoRecord u;
    u.type = OpType::DELIVER;
    u.trackingId = trackingId;
    u.prevStatus = p->status;

    p->status = Status::DELIVERED;
    logHistory(trackingId, "DELIVERED");
    undoStack_.push(u);
    message = "Parcel " + std::to_string(trackingId) + " marked as DELIVERED.";
    return true;
}

bool RouteWiseSystem::undoLast(string& message) {
    UndoRecord u;
    if (!undoStack_.pop(u)) { message = "Nothing to undo."; return false; }
    Parcel* p = findParcel(u.trackingId);
    DeliveryRequest removed;
    HistoryEntry entry;
    int id = u.trackingId;

    switch (u.type) {
    case OpType::ADD_NORMAL:
        normalQueue_.removeFirstIf([id](const DeliveryRequest& r) { return r.trackingId == id; }, removed);
        p->requestType = RequestType::NONE;
        p->arrivalSeq = 0;
        message = "Undone: normal request for parcel " + std::to_string(id) + " removed from the queue.";
        break;
    case OpType::ADD_URGENT:
        urgentHeap_.removeFirstIf([id](const DeliveryRequest& r) { return r.trackingId == id; }, removed);
        p->requestType = RequestType::NONE;
        p->arrivalSeq = 0;
        message = "Undone: urgent request for parcel " + std::to_string(id) + " removed from the heap.";
        break;
    case OpType::PROCESS:
        history_.popBack(entry);
        if (u.fromUrgent) urgentHeap_.push(u.request);          // same seq -> same place in line
        else normalQueue_.enqueueFront(u.request);              // back to the FRONT of the queue
        message = "Undone: dispatch of parcel " + std::to_string(id) + " - it is back in the " +
            (u.fromUrgent ? "urgent heap." : "normal queue.");
        break;
    case OpType::DELIVER:
        history_.popBack(entry);
        message = "Undone: delivery of parcel " + std::to_string(id) + " - it is DISPATCHED again.";
        break;
    }
    p->status = u.prevStatus;
    return true;
}

// ====================================================================== file loading
int RouteWiseSystem::loadNetworkFile(const string& path) {
    std::ifstream in(path.c_str());
    if (!in) return -1;
    string line;
    DynamicArray<string> f;
    int count = 0, lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = util::trim(line);
        if (line.empty() || line[0] == '#') continue;
        util::split(line, '|', f);
        if (util::equalsIgnoreCase(f[0], "LOCATION") && f.size() >= 2) {
            if (graph_.addLocation(f[1]) >= 0) ++count;
            else cout << "  [network] line " << lineNo << ": duplicate location '" << f[1] << "' skipped.\n";
        }
        else if (util::equalsIgnoreCase(f[0], "ROAD") && f.size() >= 4) {
            int a = graph_.indexOf(f[1]), b = graph_.indexOf(f[2]), w = 0;
            if (util::parseInt(f[3], w) && graph_.isValidRoad(a, b, w)) { graph_.addRoad(a, b, w); ++count; }
            else cout << "  [network] line " << lineNo << ": invalid road skipped.\n";
        }
        else {
            cout << "  [network] line " << lineNo << ": not understood, skipped.\n";
        }
    }
    return count;
}

int RouteWiseSystem::loadCustomersFile(const string& path) {
    std::ifstream in(path.c_str());
    if (!in) return -1;
    string line;
    DynamicArray<string> f;
    int count = 0, lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = util::trim(line);
        if (line.empty() || line[0] == '#') continue;
        util::split(line, '|', f);
        int id = 0;
        if (f.size() >= 4 && util::parseInt(f[0], id) && id > 0 && createCustomer(id, f[1], f[2], f[3]) != nullptr) ++count;
        else cout << "  [customers] line " << lineNo << ": skipped.\n";
    }
    return count;
}

int RouteWiseSystem::loadParcelsFile(const string& path) {
    std::ifstream in(path.c_str());
    if (!in) return -1;
    string line, error;
    DynamicArray<string> f;
    int count = 0, lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = util::trim(line);
        if (line.empty() || line[0] == '#') continue;
        util::split(line, '|', f);
        int tid = 0, cid = 0, pri = 0;
        double w = 0;
        if (f.size() >= 6 && util::parseInt(f[0], tid) && util::parseInt(f[1], cid) &&
            util::parseDouble(f[4], w) && util::parseInt(f[5], pri)) {
            if (createParcel(tid, cid, f[2], f[3], w, pri, error) != nullptr) ++count;
            else cout << "  [parcels] line " << lineNo << ": " << error << "\n";
        }
        else {
            cout << "  [parcels] line " << lineNo << ": malformed, skipped.\n";
        }
    }
    return count;
}

int RouteWiseSystem::loadRequestsFile(const string& path) {
    std::ifstream in(path.c_str());
    if (!in) return -1;
    string line, msg;
    DynamicArray<string> f;
    int count = 0, lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = util::trim(line);
        if (line.empty() || line[0] == '#') continue;
        util::split(line, '|', f);
        int tid = 0;
        bool urgent = f.size() >= 2 && util::equalsIgnoreCase(f[0], "URGENT");
        bool normal = f.size() >= 2 && util::equalsIgnoreCase(f[0], "NORMAL");
        if ((urgent || normal) && util::parseInt(f[1], tid) && submitRequest(tid, urgent, false, msg)) ++count;
        else cout << "  [requests] line " << lineNo << ": skipped (" << msg << ")\n";
    }
    return count;
}

bool RouteWiseSystem::loadSampleData(const string& folder) {
    int nw = loadNetworkFile(folder + "network.txt");
    if (nw < 0) return false;
    int cu = loadCustomersFile(folder + "customers.txt");
    int pa = loadParcelsFile(folder + "parcels.txt");
    int rq = loadRequestsFile(folder + "requests.txt");
    cout << "  Loaded from '" << folder << "': " << graph_.vertexCount() << " locations, "
        << graph_.edgeCount() << " roads, " << (cu < 0 ? 0 : cu) << " customers, "
        << (pa < 0 ? 0 : pa) << " parcels, " << (rq < 0 ? 0 : rq) << " delivery requests.\n";
    (void)nw;
    return true;
}

// ====================================================================== printing helpers
void RouteWiseSystem::printMenu() const {
    util::printLine('=', 41);
    cout << "ROUTEWISE DELIVERY SYSTEM\n";
    util::printLine('=', 41);
    cout << " 1. Register Customer\n"
        " 2. Register Parcel\n"
        " 3. Search Parcel\n"
        " 4. Add Normal Delivery Request\n"
        " 5. Add Urgent Delivery Request\n"
        " 6. Process Next Delivery\n"
        " 7. Mark Parcel as Delivered\n"
        " 8. Undo Last Operation\n"
        " 9. Display Pending Deliveries\n"
        "10. Display Parcel Records\n"
        "11. Display City Network\n"
        "12. BFS Network Traversal\n"
        "13. DFS Network Traversal\n"
        "14. Find Shortest Delivery Route\n"
        "15. Generate Minimum Delivery Network\n"
        "16. Compare BST and AVL Performance\n"
        "17. Display System Statistics\n"
        "18. Exit\n";
}

void RouteWiseSystem::printParcelHeader() const {
    cout << std::left << std::setw(7) << "Track" << std::setw(6) << "Cust" << std::setw(18) << "Source"
        << std::setw(18) << "Destination" << std::setw(8) << "Wt(kg)" << std::setw(13) << "Priority"
        << std::setw(12) << "Status" << "Request\n";
    util::printLine('-', 90);
}

void RouteWiseSystem::printParcelRow(const Parcel* p) const {
    std::ostringstream w;
    w << std::fixed << std::setprecision(1) << p->weight;
    string pri = std::to_string(p->priority) + "-" + priorityToString(p->priority);
    cout << std::left << std::setw(7) << p->trackingId << std::setw(6) << p->customerId
        << std::setw(18) << util::fit(p->source, 17) << std::setw(18) << util::fit(p->destination, 17)
        << std::setw(8) << w.str() << std::setw(13) << pri << std::setw(12) << statusToString(p->status)
        << requestTypeToString(p->requestType) << "\n";
}

void RouteWiseSystem::printLocationList() const {
    cout << "  Locations:\n";
    for (int i = 0; i < graph_.vertexCount(); ++i) {
        cout << "   " << std::right << std::setw(2) << (i + 1) << ". " << std::left << std::setw(18) << graph_.name(i);
        if ((i + 1) % 3 == 0) cout << "\n";
    }
    if (graph_.vertexCount() % 3 != 0) cout << "\n";
    cout << std::right;
}

// Accepts the list number or the location name. Blank input = cancel (returns -1).
int RouteWiseSystem::askLocation(const string& prompt) const {
    if (graph_.vertexCount() == 0) {
        cout << "  The network has no locations yet (menu 11 can add some).\n";
        return -1;
    }
    printLocationList();
    while (true) {
        string in = util::readLine(prompt + " (number or name, blank = cancel): ");
        if (in.empty()) return -1;
        int num = 0;
        if (util::parseInt(in, num) && num >= 1 && num <= graph_.vertexCount()) return num - 1;
        int idx = graph_.indexOf(in);
        if (idx >= 0) return idx;
        cout << "  Unknown location. Try again.\n";
    }
}

void RouteWiseSystem::printShortestRoute(int src, int dst) const {
    DynamicArray<int> dist, parent, path;
    dijkstra(graph_, src, dist, parent);                  // Dijkstra + min-heap
    cout << "  Source:      " << graph_.name(src) << "\n";
    cout << "  Destination: " << graph_.name(dst) << "\n";
    if (dist[dst] >= INF || !buildPath(parent, src, dst, path)) {
        cout << "  No route exists: '" << graph_.name(dst) << "' is not connected to '" << graph_.name(src) << "'.\n";
        return;
    }
    cout << "  Recommended Route:\n    ";
    for (int i = 0; i < path.size(); ++i) {
        cout << graph_.name(path[i]);
        if (i + 1 < path.size()) cout << " -> ";
    }
    cout << "\n  Total Distance: " << dist[dst] << " km  (" << (path.size() - 1) << " road segment"
        << (path.size() - 1 == 1 ? "" : "s") << ")\n";
}

void RouteWiseSystem::exportHistory(const string& path) const {
    std::ofstream out(path.c_str());
    if (!out) return;
    out << "# RouteWise delivery history (oldest -> newest)\n# No|TrackingID|Event\n";
    for (DoublyLinkedList<HistoryEntry>::Node* n = history_.head(); n; n = n->next)
        out << n->data.number << "|" << n->data.trackingId << "|" << n->data.event << "\n";
}

// ====================================================================== menu screens
void RouteWiseSystem::registerCustomerUI() {
    cout << "\n--- Register Customer ---\n";
    string name = util::readLine("  Name: ");
    string phone = util::readLine("  Phone: ");
    string address = util::readLine("  Address: ");
    if (name.empty() || phone.empty() || address.empty()) { cout << "  All fields are required. Nothing saved.\n"; return; }
    Customer* c = createCustomer(0, name, phone, address);
    cout << "  Customer registered with ID " << c->id << ".\n";
}

void RouteWiseSystem::registerParcelUI() {
    cout << "\n--- Register Parcel ---\n";
    if (customers_.empty()) { cout << "  Register a customer first (menu 1).\n"; return; }
    if (graph_.vertexCount() < 2) { cout << "  The network needs at least 2 locations (menu 11).\n"; return; }
    int cid = util::readInt("  Customer ID: ", 1, 2000000000);
    if (findCustomer(cid) == nullptr) { cout << "  No such customer.\n"; return; }
    int src = askLocation("  Source");
    if (src < 0) { cout << "  Cancelled.\n"; return; }
    int dst = askLocation("  Destination");
    if (dst < 0) { cout << "  Cancelled.\n"; return; }
    if (src == dst) { cout << "  Source and destination must be different.\n"; return; }
    double w = util::readDouble("  Weight in kg (0.1 - 1000): ", 0.1, 1000);
    int pri = util::readInt("  Priority (1=Low 2=Medium 3=High 4=Very High 5=Critical): ", 1, 5);
    string err;
    Parcel* p = createParcel(0, cid, graph_.name(src), graph_.name(dst), w, pri, err);
    if (p == nullptr) { cout << "  Error: " << err << "\n"; return; }
    cout << "  Parcel registered. Tracking ID = " << p->trackingId << "\n";
    printParcelHeader();
    printParcelRow(p);
}

void RouteWiseSystem::searchParcelUI() {
    cout << "\n--- Search Parcel (hash table lookup) ---\n";
    if (parcels_.empty()) { cout << "  No parcels registered yet.\n"; return; }
    int id = util::readInt("  Tracking ID: ", 1, 2000000000);
    Parcel* p = nullptr;
    int probes = 0, slot = -1;
    bool found = parcelHash_.find(id, p, probes, slot);
    if (!found) {
        cout << "  Not found (hash table examined " << probes << " slot(s)).\n";
        return;
    }
    cout << "  FOUND via hash table: slot " << slot << " of " << parcelHash_.capacity() << ", "
        << probes << " probe(s) [" << parcelHash_.modeName() << "]\n";
    Parcel* q = nullptr;
    int cb = 0, ca = 0;
    bstIndex_.search(id, q, cb);
    avlIndex_.search(id, q, ca);
    cout << "  For comparison: BST needed " << cb << " comparison(s), AVL needed " << ca << ".\n\n";
    printParcelHeader();
    printParcelRow(p);
    Customer* c = findCustomer(p->customerId);
    if (c) cout << "\n  Customer: " << c->name << " | " << c->phone << " | " << c->address << "\n";
    if (p->requestType != RequestType::NONE) cout << "  Arrival order number of its request: #" << p->arrivalSeq << "\n";
}

void RouteWiseSystem::addRequestUI(bool urgent) {
    cout << "\n--- Add " << (urgent ? "Urgent" : "Normal") << " Delivery Request ---\n";
    bool any = false;
    for (SinglyLinkedList<Parcel*>::Node* n = parcels_.head(); n; n = n->next) {
        if (n->data->status != Status::REGISTERED) continue;
        if (!any) { cout << "  Parcels waiting for a request: "; any = true; }
        cout << n->data->trackingId << "(p" << n->data->priority << ") ";
    }
    if (!any) { cout << "  No registered parcels are waiting for a request.\n"; return; }
    cout << "\n";
    int id = util::readInt("  Tracking ID: ", 1, 2000000000);
    string msg;
    submitRequest(id, urgent, true, msg);
    cout << "  " << msg << "\n";
}

void RouteWiseSystem::processNextUI() {
    cout << "\n--- Process Next Delivery ---\n";
    bool urgent = false;
    Parcel* p = dispatchNext(urgent);
    if (p == nullptr) { cout << "  No pending deliveries.\n"; return; }
    cout << "  Dispatched " << (urgent ? "URGENT" : "NORMAL") << " parcel " << p->trackingId
        << " (priority " << p->priority << "-" << priorityToString(p->priority) << ", arrival #" << p->arrivalSeq << ")\n";
    cout << "  (urgent heap is served before the normal queue)\n\n";
    int s = graph_.indexOf(p->source), d = graph_.indexOf(p->destination);
    if (s >= 0 && d >= 0) printShortestRoute(s, d);
    cout << "\n  Remaining: " << urgentHeap_.size() << " urgent, " << normalQueue_.size() << " normal.\n";
}

void RouteWiseSystem::markDeliveredUI() {
    cout << "\n--- Mark Parcel as Delivered ---\n";
    bool any = false;
    for (SinglyLinkedList<Parcel*>::Node* n = parcels_.head(); n; n = n->next) {
        if (n->data->status != Status::DISPATCHED) continue;
        if (!any) { cout << "  Dispatched parcels: "; any = true; }
        cout << n->data->trackingId << " ";
    }
    if (!any) { cout << "  There are no dispatched parcels. Use menu 6 first.\n"; return; }
    cout << "\n";
    int id = util::readInt("  Tracking ID: ", 1, 2000000000);
    string msg;
    deliver(id, msg);
    cout << "  " << msg << "\n";
}

void RouteWiseSystem::undoUI() {
    cout << "\n--- Undo Last Operation ---\n";
    UndoRecord top;
    if (undoStack_.peek(top)) cout << "  Undo stack depth: " << undoStack_.size() << "\n";
    string msg;
    undoLast(msg);
    cout << "  " << msg << "\n";
}

void RouteWiseSystem::displayPendingUI() {
    cout << "\n--- Pending Deliveries ---\n";
    DynamicArray<DeliveryRequest> urgent;
    urgentHeap_.sortedSnapshot(urgent);
    cout << "URGENT (heap) - dispatch order: highest priority first, ties by arrival\n";
    if (urgent.empty()) cout << "  (none)\n";
    for (int i = 0; i < urgent.size(); ++i) {
        Parcel* p = findParcel(urgent[i].trackingId);
        cout << "  " << (i + 1) << ". Parcel " << urgent[i].trackingId << "  priority " << urgent[i].priority
            << "  arrival #" << urgent[i].seq << "  -> " << (p ? p->destination : "?") << "\n";
    }
    cout << "NORMAL (queue) - dispatch order: first come, first served\n";
    if (normalQueue_.empty()) cout << "  (none)\n";
    int k = 1;
    for (Queue<DeliveryRequest>::Node* n = normalQueue_.head(); n; n = n->next, ++k) {
        Parcel* p = findParcel(n->data.trackingId);
        cout << "  " << k << ". Parcel " << n->data.trackingId << "  priority " << n->data.priority
            << "  arrival #" << n->data.seq << "  -> " << (p ? p->destination : "?") << "\n";
    }
    cout << "DELIVERY HISTORY (doubly linked list, newest first, last 10 of " << history_.size() << ")\n";
    if (history_.empty()) cout << "  (empty)\n";
    int shown = 0;
    for (DoublyLinkedList<HistoryEntry>::Node* n = history_.tail(); n && shown < 10; n = n->prev, ++shown)
        cout << "  #" << n->data.number << "  parcel " << n->data.trackingId << "  " << n->data.event << "\n";
}

void RouteWiseSystem::displayParcelsUI() {
    cout << "\n--- Parcel Records (in-order traversal of the AVL index = sorted by tracking ID) ---\n";
    if (avlIndex_.empty()) { cout << "  No parcels registered yet.\n"; return; }
    printParcelHeader();
    avlIndex_.inorder([this](int, Parcel* p) { printParcelRow(p); });
    cout << "  Total: " << avlIndex_.size() << " parcel(s), " << customers_.size() << " customer(s).\n";
}

void RouteWiseSystem::networkMenuUI() {
    while (true) {
        cout << "\n--- City Network (" << graph_.vertexCount() << " locations, " << graph_.edgeCount() << " roads) ---\n"
            "  1. Display adjacency list\n"
            "  2. Display adjacency matrix\n"
            "  3. Add location\n"
            "  4. Add / update road\n"
            "  5. Back to main menu\n";
        int c = util::readInt("  Choice: ", 1, 5);
        if (c == 1) { cout << "\n"; graph_.displayAdjacencyList(cout); }
        else if (c == 2) { cout << "\n"; graph_.displayAdjacencyMatrix(cout); }
        else if (c == 3) {
            string name = util::readLine("  New location name: ");
            if (name.empty() || name.find('|') != string::npos) cout << "  Invalid name.\n";
            else if (graph_.addLocation(name) < 0) cout << "  That location already exists.\n";
            else cout << "  Location '" << name << "' added (vertex " << graph_.vertexCount() - 1 << ").\n";
        }
        else if (c == 4) {
            if (graph_.vertexCount() < 2) { cout << "  Add at least two locations first.\n"; continue; }
            int a = askLocation("  Road from");
            if (a < 0) continue;
            int b = askLocation("  Road to");
            if (b < 0) continue;
            if (a == b) { cout << "  A road must connect two different locations.\n"; continue; }
            int w = util::readInt("  Length in km (1 - 10000): ", 1, 10000);
            bool isNew = graph_.addRoad(a, b, w);
            cout << "  Road " << graph_.name(a) << " <-> " << graph_.name(b) << " (" << w << " km) "
                << (isNew ? "added." : "already existed: length updated.") << "\n";
        }
        else return;
    }
}

static void showTraversal(const Graph& g, int src, int dst, const DynamicArray<int>& order, const DynamicArray<int>* hops) {
    cout << "  Visit order:\n    ";
    for (int i = 0; i < order.size(); ++i) {
        cout << g.name(order[i]);
        if (i + 1 < order.size()) cout << " -> ";
    }
    cout << "\n";
    if (hops != nullptr) {
        cout << "  Hops from " << g.name(src) << " (BFS level):\n";
        for (int i = 0; i < order.size(); ++i)
            cout << "    " << std::left << std::setw(18) << g.name(order[i]) << std::right << (*hops)[order[i]] << "\n";
    }
    DynamicArray<bool> seen;
    seen.assign(g.vertexCount(), false);
    for (int i = 0; i < order.size(); ++i) seen[order[i]] = true;
    bool any = false;
    for (int v = 0; v < g.vertexCount(); ++v) {
        if (seen[v]) continue;
        if (!any) { cout << "  NOT reachable from " << g.name(src) << ": "; any = true; }
        else cout << ", ";
        cout << g.name(v);
    }
    if (any) cout << "\n";
    else cout << "  Every location is reachable from " << g.name(src) << " (network is connected).\n";
    if (dst >= 0) {
        if (seen[dst]) {
            cout << "  Destination check: " << g.name(dst) << " IS reachable from " << g.name(src);
            if (hops != nullptr) cout << " in " << (*hops)[dst] << " hop(s)";
            cout << ".\n";
        }
        else {
            cout << "  Destination check: " << g.name(dst) << " is NOT reachable from " << g.name(src) << ".\n";
        }
    }
}

void RouteWiseSystem::bfsUI() {
    cout << "\n--- BFS Traversal (queue-based, level by level) ---\n";
    int src = askLocation("  Start location");
    if (src < 0) { cout << "  Cancelled.\n"; return; }
    int dst = askLocation("  Destination to test (optional)");
    DynamicArray<int> order, hops;
    bfs(graph_, src, order, hops);
    showTraversal(graph_, src, dst, order, &hops);
}

void RouteWiseSystem::dfsUI() {
    cout << "\n--- DFS Traversal (stack-based, deep first) ---\n";
    int src = askLocation("  Start location");
    if (src < 0) { cout << "  Cancelled.\n"; return; }
    int dst = askLocation("  Destination to test (optional)");
    DynamicArray<int> order;
    dfs(graph_, src, order);
    showTraversal(graph_, src, dst, order, nullptr);
}

void RouteWiseSystem::shortestRouteUI() {
    cout << "\n--- Shortest Delivery Route (Dijkstra with a min-heap) ---\n";
    int src = askLocation("  Source");
    if (src < 0) { cout << "  Cancelled.\n"; return; }
    int dst = askLocation("  Destination");
    if (dst < 0) { cout << "  Cancelled.\n"; return; }
    cout << "\n";
    printShortestRoute(src, dst);
}

void RouteWiseSystem::mstUI() {
    cout << "\n--- Minimum Delivery Network (Prim's algorithm with a min-heap) ---\n";
    if (graph_.vertexCount() == 0) { cout << "  The network is empty.\n"; return; }
    DynamicArray<MstEdge> edges;
    int components = 0;
    int total = primMST(graph_, edges, components);
    cout << "  Selected connections:\n";
    for (int i = 0; i < edges.size(); ++i)
        cout << "   " << std::right << std::setw(2) << (i + 1) << ". " << std::left << std::setw(18) << graph_.name(edges[i].u)
        << " <-> " << std::setw(18) << graph_.name(edges[i].v) << std::right << std::setw(4) << edges[i].weight << " km\n";
    cout << "  Total network cost: " << total << " km using " << edges.size() << " of " << graph_.edgeCount() << " roads.\n";
    if (components > 1)
        cout << "  Note: the map has " << components << " disconnected parts, so this is a minimum spanning FOREST.\n";
}

void RouteWiseSystem::compareTreesUI() {
    cout << "\n--- BST vs AVL ---\n";
    cout << "A) Your current parcel dataset (" << avlIndex_.size() << " parcels, IDs inserted in registration order)\n";
    if (avlIndex_.empty()) {
        cout << "   (no parcels yet)\n";
    }
    else {
        long cb = 0, ca = 0;
        for (SinglyLinkedList<Parcel*>::Node* n = parcels_.head(); n; n = n->next) {
            Parcel* q = nullptr;
            int x = 0, y = 0;
            bstIndex_.search(n->data->trackingId, q, x);
            avlIndex_.search(n->data->trackingId, q, y);
            cb += x;
            ca += y;
        }
        double nn = (double)parcels_.size();
        cout << std::fixed << std::setprecision(2);
        cout << "   BST height: " << bstIndex_.height() << "   avg comparisons per search: " << cb / nn << "\n";
        cout << "   AVL height: " << avlIndex_.height() << "   avg comparisons per search: " << ca / nn
            << "   rotations: " << avlIndex_.rotationCount() << "\n";
        cout << std::defaultfloat;
    }

    cout << "\nB) Stress test: insert keys 1..N in SORTED order (the worst case for a plain BST)\n";
    int n = util::readInt("   N (1 - 3000, use <= 15 to also draw the trees): ", 1, 3000);
    BST<int> b;
    AVL<int> a;
    for (int k = 1; k <= n; ++k) { b.insert(k, 0); a.insert(k, 0); }
    long tb = 0, ta = 0;
    for (int k = 1; k <= n; ++k) {
        int v = 0, x = 0, y = 0;
        b.search(k, v, x);
        a.search(k, v, y);
        tb += x;
        ta += y;
    }
    int minH = 0;
    while (((1 << minH) - 1) < n) ++minH;      // smallest possible height of any binary tree with n nodes
    cout << std::fixed << std::setprecision(2);
    cout << "   ---------------------------------------------------------\n";
    cout << "   " << std::left << std::setw(26) << "" << std::setw(14) << "BST" << std::setw(14) << "AVL" << "\n";
    cout << "   " << std::setw(26) << "Height" << std::setw(14) << b.height() << std::setw(14) << a.height() << "\n";
    cout << "   " << std::setw(26) << "Best possible height" << std::setw(14) << minH << std::setw(14) << minH << "\n";
    cout << "   " << std::setw(26) << "Avg comparisons / search" << std::setw(14) << (double)tb / n << std::setw(14) << (double)ta / n << "\n";
    cout << "   " << std::setw(26) << "Rotations performed" << std::setw(14) << 0 << std::setw(14) << a.rotationCount() << "\n";
    cout << "   ---------------------------------------------------------\n";
    cout << std::defaultfloat << std::right;
    if (n >= 2) cout << "   The BST became a " << b.height() << "-level chain; the AVL stayed balanced by rotating "
        << a.rotationCount() << " time(s).\n";
    if (n <= 15) {
        cout << "\n   BST shape (root at left, right subtree on top):\n";
        b.printSideways(cout);
        cout << "\n   AVL shape (h = subtree height):\n";
        a.printSideways(cout);
    }
}

void RouteWiseSystem::statisticsUI() {
    int byStatus[4] = { 0, 0, 0, 0 };
    for (SinglyLinkedList<Parcel*>::Node* n = parcels_.head(); n; n = n->next) byStatus[(int)n->data->status]++;
    DynamicArray<int> order, hops;
    int components = 0;
    if (graph_.vertexCount() > 0) {
        DynamicArray<bool> seen;
        seen.assign(graph_.vertexCount(), false);
        for (int v = 0; v < graph_.vertexCount(); ++v) {
            if (seen[v]) continue;
            ++components;
            bfs(graph_, v, order, hops);
            for (int i = 0; i < order.size(); ++i) seen[order[i]] = true;
        }
    }
    cout << "\n--- System Statistics ---\n";
    cout << "Records\n";
    cout << "  Customers: " << customers_.size() << "    Parcels: " << parcels_.size() << "\n";
    cout << "  Registered: " << byStatus[0] << "  Pending: " << byStatus[1]
        << "  Dispatched: " << byStatus[2] << "  Delivered: " << byStatus[3] << "\n";
    cout << "Dispatch\n";
    cout << "  Urgent heap: " << urgentHeap_.size() << "   Normal queue: " << normalQueue_.size()
        << "   History entries: " << history_.size() << "   Undo stack depth: " << undoStack_.size() << "\n";
    cout << "Hash table (" << parcelHash_.modeName() << ")\n";
    cout << "  Capacity: " << parcelHash_.capacity() << "   Stored: " << parcelHash_.size()
        << "   Load factor: " << std::fixed << std::setprecision(2) << parcelHash_.loadFactor() << "\n";
    cout << "  Collisions while inserting: " << parcelHash_.collisions() << "   Lookups: " << parcelHash_.searches()
        << "   Avg probes per lookup: " << parcelHash_.averageProbes() << "\n";
    cout << std::defaultfloat;
    cout << "Search trees\n";
    cout << "  BST height: " << bstIndex_.height() << "   AVL height: " << avlIndex_.height()
        << "   AVL rotations: " << avlIndex_.rotationCount() << "\n";
    cout << "Road network\n";
    cout << "  Locations: " << graph_.vertexCount() << "   Roads: " << graph_.edgeCount()
        << "   Connected parts: " << components << "\n";

    if (!parcels_.empty() && util::readYesNo("\nShow the hash table layout (how probing placed each parcel)? (y/n): ")) {
        int cap = parcelHash_.capacity();
        cout << "  slot  tracking-ID  home-slot  note\n";
        parcelHash_.forEach([cap](int slot, int key, Parcel*) {
            int home = key % cap;
            cout << "  " << std::right << std::setw(4) << slot << "  " << std::setw(11) << key << "  " << std::setw(9) << home
                << "  " << (slot == home ? "" : "<- collision: moved here by probing") << "\n";
            });
    }
}

// ====================================================================== main loop
void RouteWiseSystem::run() {
    util::printLine('=', 41);
    cout << "  ROUTEWISE - Smart Delivery & Route Optimizer\n";
    util::printLine('=', 41);
    if (util::readYesNo("Load the sample dataset from the 'data' folder? (y/n): ")) {
        const char* folders[] = { "data/", "../data/", "../../data/", "./" };
        bool ok = false;
        for (int i = 0; i < 4 && !ok; ++i) ok = loadSampleData(folders[i]);
        if (!ok) cout << "  Could not find data/network.txt. Starting with an empty system.\n";
    }

    while (true) {
        cout << "\n";
        printMenu();
        int choice = util::readInt("Enter your choice (1-18): ", 1, 18);
        switch (choice) {
        case 1:  registerCustomerUI(); break;
        case 2:  registerParcelUI(); break;
        case 3:  searchParcelUI(); break;
        case 4:  addRequestUI(false); break;
        case 5:  addRequestUI(true); break;
        case 6:  processNextUI(); break;
        case 7:  markDeliveredUI(); break;
        case 8:  undoUI(); break;
        case 9:  displayPendingUI(); break;
        case 10: displayParcelsUI(); break;
        case 11: networkMenuUI(); break;
        case 12: bfsUI(); break;
        case 13: dfsUI(); break;
        case 14: shortestRouteUI(); break;
        case 15: mstUI(); break;
        case 16: compareTreesUI(); break;
        case 17: statisticsUI(); break;
        case 18:
            if (!history_.empty()) {
                exportHistory("delivery_history.txt");
                cout << "Delivery history exported to delivery_history.txt\n";
            }
            cout << "Thank you for using RouteWise. Goodbye!\n";
            return;
        }
    }
}
