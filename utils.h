#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>

using byte = uint8_t;

union Data8 {
   public:
    uint8_t u;
    int8_t s;

    Data8() : u(0) {}
    Data8(uint8_t x) : u(x) {}
    Data8(int8_t x) : s(x) {}

    Data8(const Data8& o) : u(o.u) {}
};

union Data16 {
   public:
    uint16_t u;
    int16_t s;
    struct {
        Data8 l;
        Data8 h;
    } bytes;

    Data16() : u(0) {}
    Data16(uint16_t x) : u(x) {}
    Data16(int16_t x) : s(x) {}
    Data16(const Data16& o) : u(o.u) {}
};

template <class T>
std::string hex(T x) {
    std::ostringstream oss;
    oss << std::hex << x;
    return oss.str();
}

template <class T>
T SetBit(T x, int idx, bool v) {
    x = (x & ~(1 << idx)) | (v << idx);
    return x;
}

template <class T>
bool GetBit(T x, int idx) {
    return (x >> idx) & 1;
}

template <class T>
T SetBit(T x, int idx) {
    x |= (1 << idx);
    return x;
}

template <class T>
T ClearBit(T x, int idx) {
    x &= ~(1 << idx);
    return x;
}

template <class T>
T WriteBit(T x, int idx, bool val) {
    return ClearBit(x, idx) | (val << idx);
}

struct Logger : public std::basic_ostream<char> {
    Logger() : enabled(false) {}
    bool enabled;
};

template <class T>
Logger& operator<<(Logger& l, const T& x) {
    if (!l.enabled) {
        return l;
    }
    std::cout << x;
    return l;
}

inline Logger& operator<<(Logger& l, Data8 x) {
    if (!l.enabled) {
        return l;
    }
    std::cout << "0x" << std::hex << int(x.u) << "/" << std::dec << int(x.u)
              << "u/" << int(x.s);
    return l;
}

inline Logger& operator<<(Logger& l, Data16 x) {
    if (!l.enabled) {
        return l;
    }
    std::cout << "0x" << std::hex << x.u << "/" << std::dec << x.u << "u/"
              << x.s;
    return l;
}

inline Logger& operator<<(Logger& l, std::ostream& (*f)(std::ostream&)) {
    if (!l.enabled) {
        return l;
    }
    f(std::cout);
    return l;
}

inline Logger& operator<<(Logger& l, std::ostream& (*f)(std::ios&)) {
    if (!l.enabled) {
        return l;
    }
    f(std::cout);
    return l;
}

inline Logger& operator<<(Logger& l, std::ostream& (*f)(std::ios_base&)) {
    if (!l.enabled) {
        return l;
    }
    f(std::cout);
    return l;
}

extern Logger cinstr;
extern Logger cdebug;
extern Logger cevent;
extern Logger cerror;
extern Logger serial;
