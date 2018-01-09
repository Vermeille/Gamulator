#pragma once

#include <cstdint>

struct Color {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;

    Color() = default;
    Color(uint8_t red, uint8_t green, uint8_t blue)
        : a(255), r(red), g(green), b(blue) {}
};
