#pragma once

#include <array>
#include <cassert>
#include <limits>
#include <vector>

#include "utils.h"

class WaveReader {
   public:
    WaveReader() : _active(true), _freq(440), _cache(1024) {}

    int16_t* GenSamples();

    int nb_samples() const { return _cache.size(); }

    void Write(uint16_t addr, byte x) { _data[addr - 0xFF30] = x; }
    byte Read(uint16_t addr) const { return _data[addr - 0xFF30]; }

    void set_freq(int f) { _freq = f; }

    void set_level(byte lvl) { _level = lvl & 3; }
    void set_active(bool b) { _active = b; }
    bool active() const { return _active; }

   private:
    byte NthNibble(int16_t n) const {
        byte b = _data[n / 2];
        if (n % 2) {
            return b & 0xf;
        } else {
            return b >> 4;
        }
    }

    int16_t NibbleToInt16(byte x) const {
        return std::numeric_limits<int16_t>::min() + x * (65535 / 16);
    }

    int16_t AdjustLevel(int16_t x) const {
        switch (_level) {
            default:
                assert(false);
            case 0:
                return 0;
            case 1:
                return x;
            case 2:
                return x / 2;
            case 3:
                return x / 4;
        }
    }

    float _cursor;
    bool _active;
    int _freq;
    byte _level;
    std::vector<int16_t> _cache;
    std::array<byte, 0xFF40 - 0xFF30> _data;
};
