#include "Graph.h"
#include "Utils.h"
#include <iomanip>

Graph::Graph() : names_(), adj_(), matrix_(nullptr), matrixCapacity_(8), edgeCount_(0) {
    matrix_ = new int* [matrixCapacity_];
    for (int i = 0; i < matrixCapacity_; ++i) matrix_[i] = new int[matrixCapacity_]();  // () = zero-fill
}

Graph::~Graph() {
    for (int i = 0; i < adj_.size(); ++i) delete adj_[i];
    for (int i = 0; i < matrixCapacity_; ++i) delete[] matrix_[i];
    delete[] matrix_;
}

// Double the matrix size and copy the old V x V block. O(V^2), happens rarely.
void Graph::growMatrix() {
    int newCap = matrixCapacity_ * 2;
    int** bigger = new int* [newCap];
    for (int i = 0; i < newCap; ++i) bigger[i] = new int[newCap]();
    int n = names_.size() - 1;  // the newest vertex has no edges yet, nothing to copy for it
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) bigger[i][j] = matrix_[i][j];
    for (int i = 0; i < matrixCapacity_; ++i) delete[] matrix_[i];
    delete[] matrix_;
    matrix_ = bigger;
    matrixCapacity_ = newCap;
}

int Graph::addLocation(const std::string& name) {
    if (indexOf(name) != -1) return -1;
    names_.pushBack(name);
    adj_.pushBack(new SinglyLinkedList<Edge>());
    if (names_.size() > matrixCapacity_) growMatrix();
    return names_.size() - 1;
}

int Graph::indexOf(const std::string& name) const {
    for (int i = 0; i < names_.size(); ++i)
        if (util::equalsIgnoreCase(names_[i], name)) return i;
    return -1;
}

bool Graph::isValidRoad(int u, int v, int weight) const {
    int n = names_.size();
    return u >= 0 && v >= 0 && u < n && v < n && u != v && weight > 0;
}

bool Graph::addRoad(int u, int v, int weight) {
    if (!isValidRoad(u, v, weight)) return false;
    if (matrix_[u][v] != 0) {
        // Road exists: update the weight in the matrix and in both adjacency lists.
        matrix_[u][v] = matrix_[v][u] = weight;
        for (EdgeNode* e = adj_[u]->head(); e != nullptr; e = e->next)
            if (e->data.to == v) e->data.weight = weight;
        for (EdgeNode* e = adj_[v]->head(); e != nullptr; e = e->next)
            if (e->data.to == u) e->data.weight = weight;
        return false;
    }
    adj_[u]->pushBack(Edge(v, weight));
    adj_[v]->pushBack(Edge(u, weight));
    matrix_[u][v] = matrix_[v][u] = weight;
    ++edgeCount_;
    return true;
}

void Graph::displayAdjacencyList(std::ostream& os) const {
    os << std::right;   // other screens may have left std::left active
    os << "ADJACENCY LIST  (location -> [neighbour, km] ...)\n";
    for (int i = 0; i < names_.size(); ++i) {
        os << "  " << std::setw(2) << i << ". " << std::left << std::setw(18) << names_[i] << std::right << " -> ";
        if (adj_[i]->empty()) os << "(no roads)";
        for (EdgeNode* e = adj_[i]->head(); e != nullptr; e = e->next) {
            os << "[" << names_[e->data.to] << ", " << e->data.weight << "]";
            if (e->next) os << " -> ";
        }
        os << "\n";
    }
}

void Graph::displayAdjacencyMatrix(std::ostream& os) const {
    int n = names_.size();
    os << std::right;
    os << "ADJACENCY MATRIX  (road length in km, '-' = no direct road)\n";
    os << "      ";
    for (int j = 0; j < n; ++j) os << std::setw(4) << j;
    os << "\n";
    for (int i = 0; i < n; ++i) {
        os << "  " << std::setw(2) << i << "  ";
        for (int j = 0; j < n; ++j) {
            if (matrix_[i][j] == 0) os << std::setw(4) << "-";
            else os << std::setw(4) << matrix_[i][j];
        }
        os << "   " << names_[i] << "\n";
    }
}
