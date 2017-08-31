#pragma once

#include <cstdint>
#include <functional>
#include "addressbus.h"
#include "video.h"

typedef unsigned char byte;
typedef uint16_t word;

class Z80 {
   public:
    Z80(AddressBus& addr, Video& v, LinkCable& lk);

    void Process();

    template <template <class, class> class Action, class Op1, class Op2>
    class Instr {
       public:
        static void Do(Z80* p) { Action<Op1, Op2>::Do(p); }

        static void Print(Z80* p) { Action<Op1, Op2>::Print(p); }
    };

    enum RegName { A, B, C, D, E, F, H, L, AF, BC, DE, HL, SP, PC };
    template <RegName>
    struct Register;
    void set_interrupts(byte);

    static void PrintInstr(uint8_t pc, Z80* p);
    static void PrintCBInstr(uint8_t pc, Z80* p);
    void Dump() const;
    void RunOpcode(byte opcode);
    void RunCBOpcode(byte opcode);

    bool zero_f() const { return GetBit(_regs[6].u, 6); }
    void set_zero_f(bool v) {
        _regs[6].u = (_regs[6].u & ~(1 << 6)) | (v << 6);
    }

    bool hcarry_f() const { return (_regs[6].u >> 4) & 1; }
    void set_hcarry_f(bool v) {
        _regs[6].u = (_regs[6].u & ~(1 << 4)) | (v << 4);
    }

    bool sub_f() const { return (_regs[6].u >> 1) & 1; }
    void set_sub_f(bool v) { _regs[6].u = (_regs[6].u & ~(1 << 1)) | (v << 1); }

    bool carry_f() const { return (_regs[6].u >> 0) & 1; }
    void set_carry_f(bool v) {
        _regs[6].u = (_regs[6].u & ~(1 << 0)) | (v << 0);
    }

    void next_opcode() { ++_pc.u; }

   private:
    template <unsigned char Opcode, class Instr>
    void RegisterOpcode();

    template <unsigned char Opcode, class Inst>
    void RegisterCBOpcode();

    void ProcessInterrupts();

    friend struct NextWord;
    friend struct NextByte;
    template <class>
    friend struct ToAddr;
    template <class>
    friend struct ToAddrFF00;

    template <class, class>
    friend struct HALT;

   public:
    Data8 _regs[8];
    Data16 _sp;
    Data16 _pc;
    AddressBus& _addr;
    LinkCable _lk;
    Video& _vid;
    static std::array<std::function<void(Z80*)>, 256> _instr;
    static std::array<std::function<void(Z80*)>, 256> _cb_instr;
    static std::array<std::function<void(Z80*)>, 256> _print;
    static std::array<std::function<void(Z80*)>, 256> _cb_print;
    byte _interrupts;
};
