#pragma once

#include <iomanip>
#include <sstream>

template <class T>
std::string hex(T x)
{
    std::ostringstream oss;
    oss << std::hex << x;
    return oss.str();
}
