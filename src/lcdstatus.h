#pragma once

#include "utils.h"

class LCDStatus {
   public:
    LCDStatus() : _state(0) {}
    void Set(byte v) { _state = (v & ~0b111) | (_state & 0b111); }
    byte Get() const { return _state; }

    void set_lyc_interrupt(bool b) { SetStateBit(6, b); }
    bool lyc_interrupt() const { return GetStateBit(6); }

    void set_oam_interrupt(bool b) { SetStateBit(5, b); }
    bool oam_interrupt() const { return GetStateBit(5); }

    void set_vblank(bool b) {
        std::cout << "vblank interrupt " << b << std::endl;
        SetStateBit(4, b);
    }
    bool vblank() const { return GetStateBit(4); }

    void set_hblank(bool b) { SetStateBit(3, b); }
    bool hblank() const { return GetStateBit(3); }

    bool coincidence() const { return GetStateBit(2); }

    enum Mode { HBLANK = 0, VBLANK = 1, SEARCH_OAM = 2, TRANSFER = 3 };

    void set_mode(Mode mode) { _state = (_state & ~0b11) | mode; }
    Mode mode() const { return Mode(_state % 4); }

   private:
    void SetStateBit(int b, bool val) {
        if (val)
            _state |= (1 << b);
        else
            _state &= ~(1 << b);
    }

    bool GetStateBit(int b) const { return _state & (1 << b); }

    byte _state;
};
