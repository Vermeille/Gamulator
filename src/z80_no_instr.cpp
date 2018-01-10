#include "z80.h"

#include <iomanip>
#include <iostream>

#include "gpu/video.h"
#include "instruction.hpp"
#include "keypad.h"
#include "link.h"
#include "timer.h"

void Z80::Process() {
    int cycles = 0;
    while (_power && !_keypad.poweroff()) {
        if (!halted()) {
            if (cycles == 0) {
                cinstr << "0x" << std::hex << _pc.u << "\t"
                       << int(_addr.Get(_pc.u).u) << "\t";
                PrintInstr(_addr.Get(_pc.u).u, this);
                cycles = RunOpcode(_addr.Get(_pc.u).u);
            }
            --cycles;
        }

        _vid.set_maxspeed(_keypad.max_speed());
        _vid.Clock();
        _lk.Clock();
        _timer.Clock();

        if (_vid.vblank_int()) {
            _addr.Set(0xFF0F, SetBit(_addr.Get(0xFF0F).u, 0));
            cevent << "VBlank INT SET\n";
        }
        if (_vid.stat_int()) {
            _addr.Set(0xFF0F, SetBit(_addr.Get(0xFF0F).u, 1));
            cevent << "STAT INT SET\n";
        }
        if (_timer.tima_int()) {
            cevent << "TIMA INT\n";
            _addr.Set(0xFF0F, SetBit(_addr.Get(0xFF0F).u, 2));
        }
        if (_lk.transferred()) {
            _addr.Set(0xFF0F, SetBit(_addr.Get(0xFF0F).u, 3));
        }
        if (_keypad.pressed()) {
            _addr.Set(0xFF0F, SetBit(_addr.Get(0xFF0F).u, 4));
        }
        cycles += ProcessInterrupts();
    }
}

void Z80::PrintInstr(uint8_t op, Z80* p) { _instr[op]->Print(p); }
void Z80::PrintCBInstr(uint8_t op, Z80* p) { _cb_instr[op]->Print(p); }

int Z80::ProcessInterrupts() {
    byte ints = _addr.Get(0xFFFF).u & _addr.Get(0xFF0F).u & _interrupts;

    set_halt(halted() && !ints);
    if (ints) {
        --_pc.u;
    }
    if (ints & 1) {
        cevent << "VBlank int!\n";
        _addr.Set(0xFF0F, ClearBit(_addr.Get(0xFF0F).u, 0));
        return RST40<void, void>::Do(this);
    } else if (ints & 0b10) {
        cevent << "STAT int!\n";
        _addr.Set(0xFF0F, ClearBit(_addr.Get(0xFF0F).u, 1));
        return RST48<void, void>::Do(this);
    } else if (ints & 0b100) {
        _addr.Set(0xFF0F, ClearBit(_addr.Get(0xFF0F).u, 2));
        return RST50<void, void>::Do(this);
    } else if (ints & 0b1000) {
        _addr.Set(0xFF0F, ClearBit(_addr.Get(0xFF0F).u, 3));
        return RST58<void, void>::Do(this);
    } else if (ints & 0b10000) {
        _addr.Set(0xFF0F, ClearBit(_addr.Get(0xFF0F).u, 4));
        return RST60<void, void>::Do(this);
    }
    return 0;
}
