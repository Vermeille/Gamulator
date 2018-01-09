#pragma once

#include <SFML/Graphics.hpp>

#include "color.h"
#include "utils.h"

class Palette {
   public:
    Palette() : _colors{0, 1, 2, 3} {}
    byte Get() const {
        return _colors[0] | (_colors[1] << 2) | (_colors[2] << 4) |
               (_colors[3] << 6);
    }

    void Set(byte b) {
        _colors[0] = b & 0b11;
        _colors[1] = (b >> 2) & 0b11;
        _colors[2] = (b >> 4) & 0b11;
        _colors[3] = (b >> 6) & 0b11;
    }

    Color GetColor(int idx) const { return kColors[_colors[idx]]; }

   private:
    byte _colors[4];
    static const Color kColors[];
};
