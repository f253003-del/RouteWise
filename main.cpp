// main.cpp - entry point.
//   RouteWise.exe            -> starts the interactive delivery system
//   RouteWise.exe --test     -> runs the built-in self-tests (no user input needed)
#include <iostream>
#include <string>
#include "RouteWiseSystem.h"
#include "Tests.h"

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--test") return runAllTests();

    RouteWiseSystem system;
    system.run();
    return 0;
}
