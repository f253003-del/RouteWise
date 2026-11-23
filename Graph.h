// Graph.h
// Undirected weighted graph = the city road network (Module 5).
// TWO representations are kept in sync:
//   1) Adjacency LIST   : array of singly linked lists of Edge{to, weight}  -> used by all algorithms
//                         space O(V+E), iterate neighbours in O(deg)
//   2) Adjacency MATRIX : V x V ints (0 = no road)                          -> O(1) "is there a road?"
//                         space O(V^2)
// Vertices are numbered 0..V-1; names_[i] is the location name of vertex i.
#pragma once
#include <iostream>
#include <string>
#include "DynamicArray.h"
#include "SinglyLinkedList.h"

struct Edge {
    int to;
    int weight;
    Edge() : to(0), weight(0) {}
    Edge(int t, int w) : to(t), weight(w) {}
};

class Graph {
public:
    typedef SinglyLinkedList<Edge>::Node EdgeNode;

private:
    DynamicArray<std::string> names_;
    DynamicArray<SinglyLinkedList<Edge>*> adj_;
    int** matrix_;
    int matrixCapacity_;
    int edgeCount_;

    void growMatrix();

public:
    Graph();
    ~Graph();
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    // Returns the new vertex index, or -1 if a location with that name already exists.
    int addLocation(const std::string& name);

    // Case-insensitive lookup. O(V). Returns -1 if not found.
    int indexOf(const std::string& name) const;

    // Adds an undirected road. If it already exists its weight is updated.
    // Returns true if a NEW road was created, false if an existing one was updated
    // (or the arguments were invalid - check with isValidRoad first).
    bool addRoad(int u, int v, int weight);
    bool isValidRoad(int u, int v, int weight) const;

    int vertexCount() const { return names_.size(); }
    int edgeCount() const { return edgeCount_; }
    const std::string& name(int v) const { return names_[v]; }
    EdgeNode* neighbors(int v) const { return adj_[v]->head(); }
    int matrixAt(int u, int v) const { return matrix_[u][v]; }
    bool hasRoad(int u, int v) const { return matrix_[u][v] != 0; }   // O(1) via the matrix

    void displayAdjacencyList(std::ostream& os) const;
    void displayAdjacencyMatrix(std::ostream& os) const;
};
