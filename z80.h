#pragma once


#include <cstdint>
#include <functional>
#include "addressbus.h"
#include "video.h"

typedef unsigned char byte;
typedef uint16_t word;

constexpr int operator "" _b(unsigned long long int N)
{
    return (N ? (N % 10 + 2* operator "" _b(N/10)) : 0);
}

class Z80
{
    public:
        Z80(AddressBus& addr, Video& v);

        void Process();

        template <template <class, class> class Action, class Op1, class Op2>
        class Instr
        {
            public:
                static void Do(Z80* p)
                {
                    Action<Op1, Op2>::Do(p);
                }

                static void Print(int& c, AddressBus& code)
                {
                    Action<Op1, Op2>::Print(c, code);
                }
        };

        enum RegName { A, B, C, D, E, F, H, L, AF, BC, DE, HL, SP, PC };
        template <RegName>
        struct Register;
        void set_interrupts(byte);
        void Dump() const;

    private:
        template <unsigned char Opcode, class Instr>
        void RegisterOpcode();

        friend struct NextWord;
        friend struct NextByte;
        template <class>
            friend struct ToAddr;
        template <class>
            friend struct ToAddrFF00;

        template <class,class>
            friend struct HALT;

        byte _regs[8];
        byte _flags;
        word _sp;
        word _pc;
        AddressBus& _addr;
        Video& _vid;
        std::function<void(Z80*)> _instr[256];
        std::function<void(int&, AddressBus&)> _print[256];
        byte _interrupts;
};
