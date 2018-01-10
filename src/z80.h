#pragma once

#include <cstdint>
#include <functional>
#include "addressbus.h"

typedef unsigned char byte;
typedef uint16_t word;

class Sound;
class Video;
class LinkCable;
class Timer;
class Keypad;

class Z80 {
   public:
    Z80(AddressBus& addr,
        Video& v,
        LinkCable& lk,
        Timer& timer,
        Sound& s,
        Keypad& k);

    void Process();

    class InstrBase {
       public:
        virtual int Do(Z80*) = 0;
        virtual void Print(Z80*) = 0;
    };

    template <template <class, class> class Action, class Op1, class Op2>
    class Instr : public InstrBase {
       public:
        int Do(Z80* p) override { return Action<Op1, Op2>::Do(p); }
        void Print(Z80* p) override { Action<Op1, Op2>::Print(p); }
    };

    enum RegName { A, B, C, D, E, F, H, L, AF, BC, DE, HL, SP, PC };
    template <RegName>
    struct Register;
    void set_interrupts(byte);

    static void PrintInstr(uint8_t pc, Z80* p);
    static void PrintCBInstr(uint8_t pc, Z80* p);
    void Dump() const;
    int RunOpcode(byte opcode);
    int RunCBOpcode(byte opcode);

    bool zero_f() const { return GetBit(_regs[6].u, 7); }
    void set_zero_f(bool v) { _regs[6].u = WriteBit(_regs[6].u, 7, v); }

    bool sub_f() const { return GetBit(_regs[6].u, 6); }
    void set_sub_f(bool v) { _regs[6].u = WriteBit(_regs[6].u, 6, v); }

    bool hcarry_f() const { return GetBit(_regs[6].u, 5); }
    void set_hcarry_f(bool v) { _regs[6].u = WriteBit(_regs[6].u, 5, v); }

    bool carry_f() const { return GetBit(_regs[6].u, 4); }
    void set_carry_f(bool v) { _regs[6].u = WriteBit(_regs[6].u, 4, v); }

    void next_opcode() { ++_pc.u; }

    bool halted() const { return _halted; }
    void set_halt(bool x) { _halted = x; }

    void poweroff() { _power = false; }

    Data16 pc() const { return _pc; }
    Data16& pc() { return _pc; }
    AddressBus& addr() { return _addr; }

   private:
    template <unsigned char Opcode, class Instr>
    void RegisterOpcode();

    template <unsigned char Opcode, class Inst>
    void RegisterCBOpcode();

    int ProcessInterrupts();

    friend struct NextWord;
    friend struct NextByte;
    template <class>
    friend struct ToAddr;
    template <class>
    friend struct ToAddrFF00;

    template <class, class>
    friend struct HALT;

    Data8 _regs[8];
    Data16 _sp;
    Data16 _pc;
    AddressBus& _addr;
    LinkCable& _lk;
    Video& _vid;
    Sound& _snd;
    Timer& _timer;
    Keypad& _keypad;
    static std::array<std::unique_ptr<InstrBase>, 256> _instr;
    static std::array<std::unique_ptr<InstrBase>, 256> _cb_instr;
    byte _interrupts;
    bool _halted;
    bool _power;
};
