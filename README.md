# RouteWise – Smart Delivery and Route Optimization System

## 1. Project Overview

RouteWise is a C++ based Smart Delivery and Route Optimization System developed for the CL2001 Data Structures Lab Project.

The system manages customers, parcels, delivery requests, priority-based dispatching, parcel tracking, and route optimization through a city road network.

It demonstrates the practical implementation and integration of fundamental and advanced data structures and algorithms.

## 2. Problem Statement

Delivery systems need efficient ways to manage customer records, parcel information, delivery requests, urgent orders, and transportation routes.

RouteWise addresses these challenges by integrating multiple data structures and graph algorithms into one application.

## 3. Project Objectives

- Manage customer and parcel records.
- Support normal and urgent delivery requests.
- Process urgent deliveries according to priority.
- Provide efficient parcel tracking.
- Maintain delivery history and undo operations.
- Find shortest routes between locations.
- Analyze city network connectivity.
- Compare Binary Search Trees and AVL Trees.

## 4. Main Features

- Customer registration and management.
- Parcel registration and tracking.
- Priority-based urgent delivery management.
- Normal FIFO delivery processing.
- Delivery history and undo functionality.
- BFS and DFS graph traversal.
- Dijkstra shortest-path algorithm.
- Prim minimum spanning forest.
- Hash table collision handling.
- BST and AVL comparison.
- Sample dataset loading.
- Automated testing.

## 5. Data Structures Used

| Data Structure | Application |
|---|---|
| Dynamic Array | Resizable storage |
| Singly Linked List | Customer, parcel and adjacency storage |
| Doubly Linked List | Delivery history |
| Stack | Undo operations and DFS |
| Queue | Normal delivery processing and BFS |
| Binary Heap | Urgent delivery priority |
| Hash Table | Parcel tracking |
| Binary Search Tree | Ordered parcel lookup |
| AVL Tree | Balanced parcel lookup |
| Graph | City locations and roads |

## 6. Algorithms Used

- Linear probing and quadratic probing for hash table collision resolution.
- Binary heap insertion and deletion.
- BST insertion and searching.
- AVL rotations and balancing.
- Breadth First Search (BFS).
- Depth First Search (DFS).
- Dijkstra's shortest path algorithm.
- Prim's minimum spanning forest algorithm.

## 7. Time Complexity Analysis

| Operation | Time Complexity |
|---|---|
| Stack push / pop | O(1) |
| Queue enqueue / dequeue | O(1) |
| Heap push / pop | O(log n) |
| Hash insert / find (average) | O(1) |
| Hash insert / find (worst) | O(n) |
| BST insert / search (average) | O(log n) |
| BST insert / search (worst) | O(n) |
| AVL insert / search | O(log n) |
| BFS / DFS | O(V + E) |
| Dijkstra with heap | O((V + E) log V) |
| Prim with heap | O(E log V) |

## 8. Project File Organization

The project is organized into header files, implementation files, and sample data files.

### Header Files
- DynamicArray.h
- SinglyLinkedList.h
- DoublyLinkedList.h
- Stack.h
- Queue.h
- Heap.h
- HashTable.h
- BST.h
- AVL.h
- Models.h
- Utils.h
- Graph.h
- GraphAlgorithms.h
- RouteWiseSystem.h
- Tests.h

### Source Files
- Graph.cpp
- GraphAlgorithms.cpp
- RouteWiseSystem.cpp
- main.cpp

### Data Files
- network.txt
- customers.txt
- parcels.txt
- requests.txt

## 9. Requirements

- Microsoft Visual Studio 2022 or compatible Visual Studio version.
- C++ compiler with C++14 support.
- Windows operating system.

## 10. Build Instructions

1. Open the RouteWise Visual Studio project.
2. Select the Debug configuration.
3. Select the appropriate platform, such as x64.
4. Build the solution using Ctrl + Shift + B.
5. Ensure the build completes successfully.

## 11. Execution Instructions

1. Open RouteWise in Visual Studio.
2. Remove `--test` from Project Properties → Debugging → Command Arguments.
3. Run using Ctrl + F5.
4. Enter `y` when prompted to load the sample dataset.
5. Use the displayed menu to access the system features.

## 12. Automated Testing

The project includes an automated test suite.

**Testing Result: 715 / 715 checks passed.**

## 13. Sample Dataset

The project includes four sample data files:

- network.txt – City locations and road connections.
- customers.txt – Customer records.
- parcels.txt – Parcel records.
- requests.txt – Normal and urgent delivery requests.

These files are stored in the `data` folder.

## 14. Screenshots

Project demonstration screenshots are available in the `docs` folder.

They cover:

1. Automated test results.
2. Urgent delivery priority order.
3. Delivery dispatching.
4. Parcel tracking.
5. Parcel delivery.
6. Undo delivery.
7. BFS traversal.
8. DFS traversal.
9. Shortest route optimization.
10. Minimum spanning network.
11. BST vs AVL comparison.
12. Hash table layout.

## 15. Conclusion

RouteWise demonstrates how multiple data structures and algorithms can be integrated into a practical delivery management and route optimization application.

The project emphasizes efficient data management, priority scheduling, searching, traversal, and shortest-path computation.

## 16. Team Members



- Name:
- Roll Number:
- Name:
- Roll Number:

## 17. Course Information

**Course:** CL2001 – Data Structures Lab  
**Project:** RouteWise – Smart Delivery and Route Optimization System