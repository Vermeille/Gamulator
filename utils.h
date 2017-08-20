#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>

using byte = uint8_t;

template <class T>
std::string hex(T x) {
    std::ostringstream oss;
    oss << std::hex << x;
    return oss.str();
}
