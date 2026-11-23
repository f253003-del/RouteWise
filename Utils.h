// Utils.h
// Small helpers: safe console input (line based), string tools, constants.
// Every input is read with getline, so a wrong key can never leave the program stuck.
#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include "DynamicArray.h"

const int INF = 1000000000;   // "infinite" distance

namespace util {

    inline std::string trim(const std::string& s) {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace((unsigned char)s[a])) ++a;
        while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }

    inline std::string toLower(std::string s) {
        for (size_t i = 0; i < s.size(); ++i) s[i] = (char)std::tolower((unsigned char)s[i]);
        return s;
    }

    inline bool equalsIgnoreCase(const std::string& a, const std::string& b) {
        return toLower(a) == toLower(b);
    }

    // Cut a string to at most w characters so table columns stay aligned.
    inline std::string fit(const std::string& s, size_t w) {
        return s.size() <= w ? s : s.substr(0, w - 1) + ".";
    }

    inline bool parseInt(const std::string& s, int& out) {
        try {
            size_t pos = 0;
            long v = std::stol(s, &pos);
            if (pos != s.size()) return false;
            out = (int)v;
            return true;
        }
        catch (...) { return false; }
    }

    inline bool parseDouble(const std::string& s, double& out) {
        try {
            size_t pos = 0;
            double v = std::stod(s, &pos);
            if (pos != s.size()) return false;
            out = v;
            return true;
        }
        catch (...) { return false; }
    }

    // Split "a|b|c" into pieces.
    inline void split(const std::string& s, char delim, DynamicArray<std::string>& out) {
        out.clear();
        std::string cur;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == delim) { out.pushBack(trim(cur)); cur.clear(); }
            else cur += s[i];
        }
        out.pushBack(trim(cur));
    }

    inline std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {          // Ctrl+Z / end of piped input
            std::cout << "\n[Input closed - exiting]\n";
            std::exit(0);
        }
        return trim(line);
    }

    inline int readInt(const std::string& prompt, int lo, int hi) {
        while (true) {
            std::string line = readLine(prompt);
            int v = 0;
            if (parseInt(line, v) && v >= lo && v <= hi) return v;
            std::cout << "  Invalid input. Please enter a whole number from " << lo << " to " << hi << ".\n";
        }
    }

    inline double readDouble(const std::string& prompt, double lo, double hi) {
        while (true) {
            std::string line = readLine(prompt);
            double v = 0;
            if (parseDouble(line, v) && v >= lo && v <= hi) return v;
            std::cout << "  Invalid input. Please enter a number from " << lo << " to " << hi << ".\n";
        }
    }

    inline bool readYesNo(const std::string& prompt) {
        while (true) {
            std::string line = toLower(readLine(prompt));
            if (line == "y" || line == "yes") return true;
            if (line == "n" || line == "no") return false;
            std::cout << "  Please type y or n.\n";
        }
    }

    inline void printLine(char c = '=', int n = 70) { std::cout << std::string(n, c) << "\n"; }

} // namespace util
