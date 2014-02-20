#pragma once
#include "registers.hpp"
#include <type_traits>
#include <iostream>

struct NextWord
{
    static word Get(Z80* p)
    {
        ++p->_pc;
        int w = p->_addr.Get(p->_pc);
        ++p->_pc;
        return w + (p->_addr.Get(p->_pc) << 8);
    }

    static void Print(int& c, AddressBus& code)
    {
        ++c;
        int w = code.Get(c);
        ++c;
        w += code.Get(c) << 8;
        std::cout << "0x" << std::hex << reinterpret_cast<uint16_t&>(w);
    }
};

struct NextByte
{
    static byte Get(Z80* p)
    {
        ++p->_pc;
        return p->_addr.Get(p->_pc);
    }
    static void Print(int& c, AddressBus& code)
    {
        ++c;
        byte b = code.Get(c);
        std::cout << "0x" << std::hex << static_cast<int>(b)
            << std::dec << "/" << static_cast<int>(reinterpret_cast<char&>(b));
    }
};

template <class Addr>
struct ToAddr
{
    static inline word GetW(Z80* p)
    {
        word val = (p->_addr.Get(Addr::Get(p)) << 8)
            + p->_addr.Get(Addr::Get(p) + 1);
        std::cout << std::hex << "read word " << val << " at " << Addr::Get(p) << std::endl;
        return val;
    }

    static inline void SetW(Z80* p, word val)
    {
        p->_addr.Set(Addr::Get(p), val >> 8);
        p->_addr.Set(Addr::Get(p) + 1, val & 0xFF);
        std::cout << std::hex << "set word " << val << " at " << Addr::Get(p) << std::endl;
    }

    static inline byte Get(Z80* p)
    {
        return p->_addr.Get(Addr::Get(p));
    }

    static inline void Set(Z80* p, byte val)
    {
        p->_addr.Set(Addr::Get(p), val);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "(";
        Addr::Print(c, code);
        std::cout << ")";
    }
};

template <class Addr>
struct ToAddrFF00
{
    static inline byte Get(Z80* p)
    {
        byte offset = Addr::Get(p);
        return p->_addr.Get(0xFF00+reinterpret_cast<unsigned char&>(offset));
    }

    static inline void Set(Z80* p, byte val)
    {
        byte offset = Addr::Get(p);
        p->_addr.Set(0xFF00+reinterpret_cast<unsigned char&>(offset), val);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "(0xFF00 + ";
        Addr::Print(c, code);
        std::cout << ")";
    }
};

template <class Val, class sdfs>
struct RLC
{
    static void Do(Z80* p)
    {
        int res = Val::Get(p) << 1;
        res += res >> 8;
        word flags = (res >> 8) << 4;
        res &= 0xFF;
        Val::Set(p, res);
        Z80::Register<Z80::F>::Set(p, flags);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "rlc ";
        Val::Print(c, code);
        std::cout << std::endl;
    }
};

template <class Val, class sdfs>
struct RRC
{
    static void Do(Z80* p)
    {
        int res = Val::Get(p);
        word flags = (res % 2) << 4;
        res = (res >> 1) + ((res % 2) << 7);
        Val::Set(p, res);
        Z80::Register<Z80::F>::Set(p, flags);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "rrc ";
        Val::Print(c, code);
        std::cout << std::endl;
    }
};

template <class Val, class sdfs>
struct RL
{
    static void Do(Z80* p)
    {
        int res = Val::Get(p) << 1;
        word flags = (res >> 8) << 4;
        res += (Z80::Register<Z80::F>::Get(p) >> 4);
        Val::Set(p, res & 0xFF);
        Z80::Register<Z80::F>::Set(p, flags);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "rl ";
        Val::Print(c, code);
        std::cout << std::endl;
    }
};

template <class Val, class sdfs>
struct RR
{
    static void Do(Z80* p)
    {
        int res = Val::Get(p);
        word flags = res % 2;;
        res += (Z80::Register<Z80::F>::Get(p) >> 4) << 7;
        Val::Set(p, res);
        Z80::Register<Z80::F>::Set(p, flags);
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "rr ";
        Val::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class>
struct CPL
{
    static inline void Do(Z80* p)
    {
        A::Set(p, ~A::Get(p));
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "cpl ";
        A::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class>
struct SCF
{
    static inline void Do(Z80*p)
    {
        Z80::Register<Z80::F>::Set(p, Z80::Register<Z80::F>::Get(p)
                | 00010000_b);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "scf ";
        std::cout << std::endl;
    }
};

template <class A, class>
struct CCF
{
    static inline void Do(Z80*p)
    {
        Z80::Register<Z80::F>::Set(p, Z80::Register<Z80::F>::Get(p)
                & ~00010000_b);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "ccf ";
        std::cout << std::endl;
    }
};

template <class Val, class Garbage>
struct INC
{
    static inline void Do(Z80* p)
    {
        Val::Set(p, Val::Get(p)+1);
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "inc ";
        Val::Print(c, code);
        std::cout << std::endl;
    }
};

template <class Val, class Garbage>
struct DEC
{
    static inline void Do(Z80* p)
    {
        Val::Set(p, Val::Get(p)-1);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "dec ";
        Val::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct ADD
{
    static void Do(Z80*p)
    {
        int res = B::Get(p) + A::Get(p);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        flag += ((res >> 4) ? 1 << 5 : 0);
        if (std::is_same<decltype(A::Get(p)), byte>::value)
            flag += ((res >> 8) ? 1 << 4 : 0);
        else
            flag += ((res >> 16) ? 1 << 4 : 0);
        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "add ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct ADC
{
    static void Do(Z80*p)
    {
        int res = B::Get(p) + A::Get(p)
            + ((Z80::Register<Z80::F>::Get(p) & 00010000_b) >> 4);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        flag += ((res >> 4) ? 1 << 5 : 0);
        if (std::is_same<decltype(A::Get(p)), byte>::value)
            flag += ((res >> 8) ? 1 << 4 : 0);
        else
            flag += ((res >> 16) ? 1 << 4 : 0);

        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "adc ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct SUB
{
    static void Do(Z80*p)
    {
        int res = A::Get(p) - B::Get(p);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        flag += 1 << 6;
        flag += ((res >> 4) ? 1 << 5 : 0);
        flag += ((res < 0) ? 1 << 4 : 0);
        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "sub ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct SBC
{
    static void Do(Z80*p)
    {
        int res = A::Get(p) - B::Get(p)
            - (Z80::Register<Z80::F>::Get(p) & 00010000_b);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        flag += 1 << 6;
        flag += ((res >> 4) ? 1 << 5 : 0);
        flag += ((res < 0) ? 1 << 4 : 0);
        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "sbc ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct AND
{
    static void Do(Z80*p)
    {
        decltype(A::Get(p)) res = A::Get(p) & B::Get(p);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        flag += 1 << 5;
        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "and ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct XOR
{
    static void Do(Z80*p)
    {
        decltype(A::Get(p)) res = A::Get(p) ^ B::Get(p);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "xor ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct OR
{
    static void Do(Z80*p)
    {
        decltype(A::Get(p)) res = A::Get(p) | B::Get(p);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        Z80::Register<Z80::F>::Set(p, flag);
        A::Set(p, res);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "or ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct CP
{
    static void Do(Z80*p)
    {
        int res = A::Get(p) - B::Get(p);

        int flag = 0;
        flag = (res == 0 ? (1 << 7) : 0);
        flag += 1 << 6;
        flag += ((res >> 4) ? 1 << 5 : 0);
        flag += ((res < 0) ? 1 << 4 : 0);
        Z80::Register<Z80::F>::Set(p, flag);
    }

    static void Print(int& c, AddressBus& code)
    {
        std::cout << "cp ";
        A::Print(c, code);
        std::cout <<", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

struct True
{
    static inline bool Do(Z80*)
    {
        return true;
    }

    static void Print(int&, AddressBus&)
    {
    }
};

struct IfC
{
    static inline bool Do(Z80* p)
    {
        return Z80::Register<Z80::F>::Get(p) & 00010000_b;
    }

    static void Print(int&, AddressBus&)
    {
        std::cout << "C";
    }
};


struct NC
{
    static inline bool Do(Z80* p)
    {
        return !(Z80::Register<Z80::F>::Get(p) & 00010000_b);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "NC";
    }
};

struct IfZ
{
    static inline bool Do(Z80* p)
    {
        return Z80::Register<Z80::F>::Get(p) & 10000000_b;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "Z";
    }
};


struct NZ
{
    static inline bool Do(Z80* p)
    {
        return !(Z80::Register<Z80::F>::Get(p) & 10000000_b);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "NZ";
    }
};

template <class Test, class A, class fghdl>
struct JR_Impl
{
    static void Do(Z80* p)
    {
        byte jmp = A::Get(p);
        if (Test::Do(p))
            Z80::Register<Z80::PC>::Set(p,
                Z80::Register<Z80::PC>::Get(p) + reinterpret_cast<char&>(jmp));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "jr";
        Test::Print(c, code);
        std::cout << " ";
        A::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
using JR = JR_Impl<True, A, B>;

template <class A, class B>
using JRZ = JR_Impl<IfZ, A, B>;

template <class A, class B>
using JRNZ = JR_Impl<NZ, A, B>;

template <class A, class B>
using JRC = JR_Impl<IfC, A, B>;

template <class A, class B>
using JRNC = JR_Impl<NC, A, B>;

template <class Test, class A, class fghdl>
struct RET_Impl
{
    static void Do(Z80* p)
    {
        if (Test::Do(p))
        {
            Z80::Register<Z80::PC>::Set(p, ToAddr<Z80::Register<Z80::SP>>::GetW(p));
            Z80::Register<Z80::SP>::Set(p, Z80::Register<Z80::SP>::Get(p)+2);
        }
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ret ";
        Test::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
using RET = RET_Impl<True, A, B>;

template <class A, class B>
using RETZ = RET_Impl<IfZ, A, B>;

template <class A, class B>
using RETNZ = RET_Impl<NZ, A, B>;

template <class A, class B>
using RETC = RET_Impl<IfC, A, B>;

template <class A, class B>
using RETNC = RET_Impl<NC, A, B>;

template <class Test, class A, class fghdl>
struct JP_Impl
{
    static void Do(Z80* p)
    {
        word jmp = A::Get(p);
        if (Test::Do(p))
            Z80::Register<Z80::PC>::Set(p, jmp-1);
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "jp";
        Test::Print(c, code);
        std::cout << " ";
        A::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
using JP = JP_Impl<True, A, B>;

template <class A, class B>
using JPZ = JP_Impl<IfZ, A, B>;

template <class A, class B>
using JPNZ = JP_Impl<NZ, A, B>;

template <class A, class B>
using JPC = JP_Impl<IfC, A, B>;

template <class A, class B>
using JPNC = JP_Impl<NC, A, B>;

template <class Test, class A, class fghdl>
struct CALL_Impl
{
    static void Do(Z80* p)
    {
        if (Test::Do(p))
        {
            Z80::Register<Z80::SP>::Set(p, Z80::Register<Z80::SP>::Get(p) - 2);
            ToAddr<Z80::Register<Z80::SP>>::SetW(p, Z80::Register<Z80::PC>::Get(p) + 2);
            Z80::Register<Z80::PC>::Set(p, A::Get(p) - 1);
        }
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "call";
        Test::Print(c, code);
        std::cout << " ";
        A::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
using CALL = CALL_Impl<True, A, B>;

template <class A, class B>
using CALLZ = CALL_Impl<IfZ, A, B>;

template <class A, class B>
using CALLNZ = CALL_Impl<NZ, A, B>;

template <class A, class B>
using CALLC = CALL_Impl<IfC, A, B>;

template <class A, class B>
using CALLNC = CALL_Impl<NC, A, B>;

template <class A, class>
struct POP
{
    static void Do(Z80* p)
    {
        A::Set(p, ToAddr<Z80::Register<Z80::SP>>::GetW(p));
        Z80::Register<Z80::SP>::Set(p, Z80::Register<Z80::SP>::Get(p)+2);
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "pop ";
        A::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class>
struct PUSH
{
    static void Do(Z80* p)
    {
        Z80::Register<Z80::SP>::Set(p, Z80::Register<Z80::SP>::Get(p)-2);
        ToAddr<Z80::Register<Z80::SP>>::SetW(p, A::Get(p));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "push ";
        A::Print(c, code);
        std::cout << std::endl;
    }
};

template <class, class>
struct Nop {};

template <>
void Z80::Instr<Nop, void, void>::Do(Z80*)
{
}
template <>
void Z80::Instr<Nop, void, void>::Print(int&, AddressBus&)
{
    std::cout << "nop" << std::endl;
}


template <class A, class B>
struct LD
{
    static inline void Do(Z80* proc)
    {
        A::Set(proc, B::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ld ";
        A::Print(c, code);
        std::cout << ", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct LDH
{
    static inline void Do(Z80* proc)
    {
        A::Set(proc, B::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ldh ";
        A::Print(c, code);
        std::cout << ", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct LDHL
{
    static inline void Do(Z80* proc)
    {
        Z80::Register<Z80::HL>::Set(proc, A::Get(proc) + B::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ldhl ";
        A::Print(c, code);
        std::cout << ", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct LDD;

template <class A, class B>
struct LDD<ToAddr<A>, B>
{
    static inline void Do(Z80* proc)
    {
        ToAddr<A>::Set(proc, B::Get(proc));
        A::Set(proc, 1 - A::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ldd ";
        A::Print(c, code);
        std::cout << ", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct LDD<A, ToAddr<B>>
{
    static inline void Do(Z80* proc)
    {
        A::Set(proc, ToAddr<B>::Get(proc));
        B::Set(proc, 1 - B::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ldd ";
        A::Print(c, code);
        std::cout << ", ";
        ToAddr<B>::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct LDI;

template <class A, class B>
struct LDI<ToAddr<A>, B>
{
    static inline void Do(Z80* proc)
    {
        ToAddr<A>::Set(proc, B::Get(proc));
        A::Set(proc, 1 + A::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ldi ";
        ToAddr<A>::Print(c, code);
        std::cout << ", ";
        B::Print(c, code);
        std::cout << std::endl;
    }
};

template <class A, class B>
struct LDI<A, ToAddr<B>>
{
    static inline void Do(Z80* proc)
    {
        A::Set(proc, ToAddr<B>::Get(proc));
        B::Set(proc, 1 + B::Get(proc));
    }
    static void Print(int& c, AddressBus& code)
    {
        std::cout << "ldi ";
        A::Print(c, code);
        std::cout << ", ";
        ToAddr<B>::Print(c, code);
        std::cout << std::endl;
    }
};

template <class, class>
struct EI
{
    static inline void Do(Z80* p)
    {
        p->set_interrupts(0xFF);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "ei" << std::endl;
    }
};
template <class, class>
struct DI
{
    static inline void Do(Z80* p)
    {
        p->set_interrupts(0);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "di" << std::endl;
    }
};

typedef Z80::Instr<Nop, void, void> NOP;

template <class, class>
struct HALT
{
    static inline void Do(Z80* p)
    {
        //stupidly loop on the halt / do nothing
        EI<void, void>::Do(p);
        if (!p->_addr.Get(0xFF0F))
            Z80::Register<Z80::PC>::Set(p, Z80::Register<Z80::PC>::Get(p) - 1);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "halt" << std::endl;
    }
};

template <int Addr, class, class>
struct RST_Impl
{
    static void Do(Z80*p)
    {
        Z80::Register<Z80::SP>::Set(p, Z80::Register<Z80::SP>::Get(p) - 2);
        ToAddr<Z80::Register<Z80::SP>>::SetW(p, Z80::Register<Z80::PC>::Get(p) + 2);
        Z80::Register<Z80::PC>::Set(p, Addr - 1);
        DI<void, void>::Do(p);
        std::cout << "interrupt caught by " << std::hex << Addr << std::endl;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "rst 0x" << std::hex << Addr << std::endl;
    }
};

template <class A, class B>
using RST0 = RST_Impl<0x0, A, B>;

template <class A, class B>
using RST8 = RST_Impl<0x8, A, B>;

template <class A, class B>
using RST10 = RST_Impl<0x10, A, B>;

template <class A, class B>
using RST18 = RST_Impl<0x18, A, B>;

template <class A, class B>
using RST20 = RST_Impl<0x20, A, B>;

template <class A, class B>
using RST28 = RST_Impl<0x28, A, B>;

template <class A, class B>
using RST30 = RST_Impl<0x30, A, B>;

template <class A, class B>
using RST38 = RST_Impl<0x38, A, B>;

template <class A, class B>
using RST40 = RST_Impl<0x40, A, B>;

template <class A, class B>
using RST48 = RST_Impl<0x48, A, B>;

template <class A, class B>
using RST50 = RST_Impl<0x50, A, B>;

template <class A, class B>
using RST58 = RST_Impl<0x58, A, B>;

template <class A, class B>
using RST60 = RST_Impl<0x60, A, B>;

template <class, class>
struct RETI
{
    static inline void Do(Z80* p)
    {
        RET<void, void>::Do(p);
        EI<void, void>::Do(p);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "reti" << std::endl;
    }
};

template <class, class>
struct Stop
{
    static inline void Do(Z80* p)
    {
        HALT<void, void>::Do(p);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "stop"<< std::endl;
    }
};

