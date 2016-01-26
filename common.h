#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>
#include <iostream>
#include <cstdio>
#include <cstdarg>

#define DEBUG

class Coords{
protected:
    double x;
    double y;
public:
    Coords() : x(0), y(0) {};
    Coords(double _x, double _y) : x(_x), y(_y) {};
    double X() const { return x; };
    double Y() const { return y; };
    bool operator==(const Coords& coords) const { return (x == coords.x && y == coords.y); };
    void operator*=(double d) {
        x *= d;
        y *= d;
    };
    std::string getAsString() {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
    friend Coords operator*(const Coords& coords, double d);
    friend double maxX(const Coords& first, const Coords& second);
    friend double maxY(const Coords& first, const Coords& second);
    friend double minX(const Coords& first, const Coords& second);
    friend double minY(const Coords& first, const Coords& second);
};

inline double
maxX(const Coords& first, const Coords& second) {
    return std::max<double>(first.x, second.x);
}

inline double
maxY(const Coords& first, const Coords& second) {
    return std::max<double>(first.y, second.y);
}

inline double
minX(const Coords& first, const Coords& second) {
    return std::min<double>(first.x, second.x);
}

inline double
minY(const Coords& first, const Coords& second) {
    return std::min<double>(first.y, second.y);
}

inline Coords
operator*(const Coords& coords, double d) {
    return Coords(coords.X() * d,
                  coords.Y() * d);
}

class TrueInit {
    bool inited;
public:
    TrueInit(bool _inited=false) : inited(_inited) {};
    bool isInited() const { return inited; };
};

inline void
info(const std::string& str) {
    std::cout << str << std::endl;
}

inline void
warning(const std::string& str) {
    std::cerr << "WARNING: " << str << std::endl;
}

class Error {
    std::string* msg;
public:
    Error(const std::string& str) {
        msg = new std::string;
        *msg = str;
    };
    ~Error() {
        delete msg;
    };
    void print() const {
        std::cerr << "ERROR: " << *msg << std::endl;
    }
};

#endif // COMMON_H_INCLUDED

