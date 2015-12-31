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
    friend Coords operator*(const Coords& coords, double d);
    friend double maxX(Coords& first, Coords& second);
    friend double maxY(Coords& first, Coords& second);
    friend double minX(Coords& first, Coords& second);
    friend double minY(Coords& first, Coords& second);
};

inline double
maxX(Coords& first, Coords& second) {
    return ((first.x > second.x) ? first.x : second.x);
}

inline double
maxY(Coords& first, Coords& second) {
    return ((first.y > second.y) ? first.y : second.y);
}

inline double
minX(Coords& first, Coords& second) {
    return ((first.x < second.x) ? first.x : second.x);
}

inline double
minY(Coords& first, Coords& second) {
    return ((first.y < second.y) ? first.y : second.y);
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

#define VSPRINTF(format)                    \
    va_list args;                           \
    va_start(args,format);                  \
    char buf[256];                          \
    std::vsprintf(buf, format, args);       \
    va_end(args);

inline void info(const char* format, ...) {
    VSPRINTF(format);
    std::cout << std::string(buf) << std::endl;
}

inline void warning(const char* format, ...) {
    VSPRINTF(format);
    std::cout << "WARNING: " << std::string(buf) << std::endl;
}

inline void error(const char* format, ...) {
    VSPRINTF(format);
    throw std::string(buf).c_str();
}


/*
#define MAX_SYMBOL_CODE_LEN 8
#define MAX_SYMBOLS_NUM 200

typedef enum _bool{false=0, true=1} bool;

#define WARNING(...)     \
    printf("WARNING: "); \
    printf(__VA_ARGS__); \
    printf("\n");

#define ERROR(...)        \
    printf("ERROR: ");    \
    printf(__VA_ARGS__);  \
    printf("\n");         \
    exit(1);                            

typedef struct _SymbolCode {
    char * code;
    bool isText;
} SymbolCode;
*/

#endif // COMMON_H_INCLUDED

