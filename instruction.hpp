#pragma once

#include "registers.hpp"
#include "utils.h"

#include <iostream>
#include <type_traits>

struct NextWord {
    static Data16 GetW(Z80* p) {
        Data16 w;
        p->next_opcode();
        w.bytes.l = p->_addr.Get(p->_pc.u);
        p->next_opcode();
        w.bytes.h = p->_addr.Get(p->_pc.u);
        return w;
    }

    static void Print(Z80* p) {
        Data16 w;
        w.bytes.l = p->_addr.Get(Z80::Register<Z80::PC>::GetW(p).u + 1);
        w.bytes.h = p->_addr.Get(Z80::Register<Z80::PC>::GetW(p).u + 2);
        cinstr << w;
    }
};

struct NextByte {
    static Data8 Get(Z80* p) {
        p->next_opcode();
        return p->_addr.Get(p->_pc.u);
    }
    static void Print(Z80* p) {
        Data8 b = p->_addr.Get(Z80::Register<Z80::PC>::GetW(p).u + 1);
        cinstr << b;
    }
};

template <class Addr>
struct ToAddr {
    static inline Data16 GetW(Z80* p) {
        auto addr = Addr::GetW(p).u;
        Data16 w;
        w.bytes.l = p->_addr.Get(addr);
        w.bytes.h = p->_addr.Get(addr + 1);
        cinstr << std::hex << "read word " << w << " at " << std::hex << addr
               << "(" << p->_addr.Print(addr) << ")" << std::endl;
        return w;
    }

    static inline void SetW(Z80* p, Data16 val) {
        auto addr = Addr::GetW(p).u;
        p->_addr.Set(addr, uint8_t(val.u & 0xFF));
        p->_addr.Set(addr + 1, uint8_t(val.u >> 8));
        cinstr << std::hex << "set word " << val << " at " << std::hex << addr
               << "(" << p->_addr.Print(addr) << ")" << std::endl;
    }

    static inline Data8 Get(Z80* p) {
        word addr = Addr::GetW(p).u;
        auto b = p->_addr.Get(addr);
        cinstr << std::hex << "  read byte " << b << " at " << std::hex << addr
               << "(" << p->_addr.Print(addr) << ")" << std::endl;
        return b;
    }

    static inline void Set(Z80* p, Data8 val) {
        auto addr = Addr::GetW(p).u;
        p->_addr.Set(addr, val);
        cinstr << "  set byte " << std::hex << val << " at " << std::hex << addr
               << "(" << p->_addr.Print(addr) << ")" << std::endl;
    }

    static void Print(Z80* p) {
        cinstr << "(";
        Addr::Print(p);
        cinstr << ")";
    }
};

template <class Addr>
struct ToAddrFF00 {
    static inline Data8 Get(Z80* p) {
        Data8 offset = Addr::Get(p);
        auto addr = 0xFF00 + offset.u;
        auto b = p->_addr.Get(addr);

        cinstr << std::hex << "  read byte " << int(b.u) << " at " << addr
               << "(" << p->_addr.Print(addr) << ")" << std::endl;

        return b;
    }

    static inline void Set(Z80* p, Data8 val) {
        Data8 offset = Addr::Get(p);
        auto addr = 0xFF00 + offset.u;
        p->_addr.Set(addr, val);
        cinstr << std::hex << "  set byte " << int(val.u) << " at " << addr
               << "(" << p->_addr.Print(addr) << ")" << std::endl;
    }

    static void Print(Z80* p) {
        cinstr << "(0xFF00 + ";
        Addr::Print(p);
        cinstr << ")";
    }
};

template <uint16_t X>
struct I {
    static inline Data8 Get() { return uint8_t(X); }
    static inline Data8 Get(Z80*) { return uint8_t(X); }
    static inline Data16 GetW() { return uint16_t(X); }
    static inline Data16 GetW(Z80*) { return uint16_t(X); }
};

template <class, class>
struct DAA {
    static void Do(Z80* p) {
        int16_t a = Z80::Register<Z80::A>::Get(p).u;
        if (!p->sub_f()) {
            if ((a & 0x0F) > 9 || p->hcarry_f()) {
                a += 6;
            }
            if (a > 0x9F || p->carry_f()) {
                a += 0x60;
                p->set_carry_f(true);
            }
        } else {
            if (p->hcarry_f()) {
                a = (a - 6) & 0xFF;
            }
            if (p->carry_f()) {
                a -= 0x60;
            }
        }
        p->set_hcarry_f(false);
        p->set_zero_f((a & 0xff) == 0);
        if (a >> 8) {
            p->set_carry_f(true);
        }
        Z80::Register<Z80::A>::Set(p, int8_t(a & 0xFF));
        p->next_opcode();
    }

    static void Print(Z80*) { cinstr << "daa\n"; }
};

template <class Val, class sdfs>
struct RLC {
    static void Do(Z80* p) {
        Data8 res = Val::Get(p);

        res.u = (res.u << 1) | ((res.u >> 7) & 1);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(res.u & 1);

        Val::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "rlc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class sdfs>
struct RRC {
    static void Do(Z80* p) {
        Data8 res = Val::Get(p);

        res.u = (res.u >> 1) | ((res.u & 1) << 7);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(res.u >> 7);

        Val::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "rrc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class sdfs>
struct RL {
    static void Do(Z80* p) {
        Data8 res = Val::Get(p);
        int c = p->carry_f();

        p->set_carry_f(res.u >> 7);

        res.u = res.u << 1 | c;

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        Val::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "rl ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct RR {
    static void Do(Z80* p) {
        Data8 res = Val::Get(p);
        int c = p->carry_f() ? 1 : 0;

        p->set_carry_f(GetBit(res.u, 0));

        res.u = res.u >> 1;
        res.u = WriteBit(res.u, 7, c);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        Val::Set(p, res);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "rr ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct RRA {
    static void Do(Z80* p) {
        Data8 res = Z80::Register<Z80::A>::Get(p);
        int c = p->carry_f();

        p->set_carry_f(GetBit(res.u, 0));

        res.u = res.u >> 1;
        res.u = WriteBit(res.u, 7, c);
        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        Z80::Register<Z80::A>::Set(p, res);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "rra ";
        Z80::Register<Z80::A>::Print(p);
        cinstr << std::endl;
    }
};

template <class Bit, class A>
struct RES {
    static void Do(Z80* p) {
        Data8 res = A::Get(p);
        res.u = res.u & ~(1 << Bit::Get().u);
        A::Set(p, res);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "res " << Bit::Get() << ", ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class Bit, class A>
struct SET {
    static void Do(Z80* p) {
        A::Set(p, A::Get(p).u | (1 << Bit::Get().u));
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "set " << Bit::Get() << ", ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class Bit, class R>
struct BIT {
    static void Do(Z80* p) {
        p->set_zero_f(!(R::Get(p).u & (1 << Bit::Get().u)));
        p->set_sub_f(0);
        p->set_hcarry_f(1);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "bit " << Bit::Get() << ", ";
        R::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct CPL {
    static inline void Do(Z80* p) {
        uint8_t r = 0xFF ^ A::Get(p).u;
        A::Set(p, r);

        p->set_zero_f(r == 0);
        p->set_sub_f(1);
        p->set_hcarry_f(1);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "cpl ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SCF {
    static inline void Do(Z80* p) {
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(true);
        p->next_opcode();
    }
    static void Print(Z80*) {
        cinstr << "scf ";
        cinstr << std::endl;
    }
};

template <class A, class>
struct CCF {
    static inline void Do(Z80* p) {
        p->set_sub_f(false);
        p->set_hcarry_f(!p->hcarry_f());
        p->set_carry_f(!p->carry_f());
        p->next_opcode();
    }
    static void Print(Z80*) {
        cinstr << "ccf ";
        cinstr << std::endl;
    }
};

template <class Val, class>
struct INC {
    static inline void Do(Z80* p) {
        Data8 res = Val::Get(p);
        res.u += 1;
        Val::Set(p, res);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "inc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct INCw {
    static inline void Do(Z80* p) {
        Data16 res = Val::GetW(p);
        res.u += 1;
        Val::SetW(p, res);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "inc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct DEC {
    static inline void Do(Z80* p) {
        Data8 res = Val::Get(p);
        res.u -= 1;
        Val::Set(p, res);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(1);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "dec ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct DECw {
    static inline void Do(Z80* p) {
        Data16 r = Val::GetW(p);
        r.u -= 1;
        Val::SetW(p, r);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "dec ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct ADD {
    static void Do(Z80* p) {
        uint16_t res = A::Get(p).u + B::Get(p).u;

        p->set_zero_f((res & 0xFF) == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);  // FIXME
        p->set_carry_f(res >> 8);

        A::Set(p, uint8_t(res & 0xFF));
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "add ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct ADDw {
    static void Do(Z80* p) {
        uint32_t res = A::GetW(p).u + B::GetW(p).u;

        p->set_zero_f((res & 0xFFFF) == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);  // FIXME
        p->set_carry_f(res >> 16);

        A::SetW(p, uint16_t(res & 0xFFFF));
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "add ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct ADDO {
    static void Do(Z80* p) {
        Data16 res = A::GetW(p);
        res.s = res.s + B::Get(p).s;

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);  // FIXME
        p->set_carry_f(res.u >> 16);

        A::SetW(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "add ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct ADC {
    static void Do(Z80* p) {
        uint16_t res = B::Get(p).u + A::Get(p).u + p->carry_f();

        p->set_zero_f((res & 0xFF) == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);  // FIXME
        p->set_carry_f(res >> 8);

        A::Set(p, uint8_t(res & 0xFF));
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "adc ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct SUB {
    static void Do(Z80* p) {
        Data8 a = A::Get(p);
        Data8 b = B::Get(p);
        Data8 res;
        res.s = a.s - b.s;

        p->set_zero_f(res.u == 0);
        p->set_hcarry_f(false);
        p->set_sub_f(true);
        p->set_carry_f(a.s < b.s);

        A::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "sub ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct SBC {
    static void Do(Z80* p) {
        Data8 a = A::Get(p);
        Data8 b = B::Get(p);
        Data8 res;
        res.s = a.s - b.s - p->carry_f();

        p->set_zero_f(res.u == 0);
        p->set_hcarry_f(false);
        p->set_sub_f(true);
        p->set_carry_f(a.s < b.s + p->carry_f());

        A::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "sbc ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct AND {
    static void Do(Z80* p) {
        uint8_t res = A::Get(p).u & B::Get(p).u;

        p->set_zero_f(res == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(true);
        p->set_carry_f(false);

        A::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "and ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct XOR {
    static void Do(Z80* p) {
        uint8_t res = A::Get(p).u ^ B::Get(p).u;

        p->set_zero_f(res == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(false);

        A::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "xor ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct OR {
    static void Do(Z80* p) {
        uint8_t res = A::Get(p).u | B::Get(p).u;

        p->set_zero_f(res == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(false);

        A::Set(p, res);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "or ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SRL {
    static void Do(Z80* p) {
        Data8 a = A::Get(p);

        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(a.u & 1);

        a.u = (a.u >> 1);
        a.u = ClearBit(a.u, 7);
        p->set_zero_f(a.u == 0);

        A::Set(p, a);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "srl ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SLA {
    static void Do(Z80* p) {
        Data8 a = A::Get(p);

        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(GetBit(a.u, 7));

        a.u = a.u << 1;
        p->set_zero_f(a.u);

        A::Set(p, a);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "sla ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct CP {
    static void Do(Z80* p) {
        auto a = A::Get(p).u;
        auto b = B::Get(p).u;

        p->set_zero_f(a == b);
        p->set_sub_f(true);
        p->set_hcarry_f(false);
        p->set_carry_f(a < b);
        p->next_opcode();
    }

    static void Print(Z80* p) {
        cinstr << "cp ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SWAP {
    static void Do(Z80* p) {
        Data8 res = A::Get(p);
        res.u = ((res.u & 0x0F) << 4) | ((res.u & 0xF0) >> 4);
        A::Set(p, res);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(true);
        p->set_carry_f(false);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "swap ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct POP {
    static void Do(Z80* p) {
        A::SetW(p, ToAddr<Z80::Register<Z80::SP>>::GetW(p));
        Data16 sp = Z80::Register<Z80::SP>::GetW(p);
        sp.u += 2;
        Z80::Register<Z80::SP>::SetW(p, sp);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "pop ";
        A::Print(p);
        cinstr << std::endl;
    }
};

struct True {
    static inline bool Do(Z80*) { return true; }

    static void Print(Z80*) {}
};

struct IfC {
    static inline bool Do(Z80* p) { return p->carry_f(); }

    static void Print(Z80* p) { cinstr << "C (" << p->carry_f() << ")"; }
};

struct NC {
    static inline bool Do(Z80* p) { return !p->carry_f(); }
    static void Print(Z80*) { cinstr << "NC"; }
};

struct IfZ {
    static inline bool Do(Z80* p) { return p->zero_f(); }
    static void Print(Z80* p) { cinstr << "Z (" << p->zero_f() << ")"; }
};

struct NZ {
    static inline bool Do(Z80* p) { return p->zero_f() == false; }
    static void Print(Z80*) { cinstr << "NZ"; }
};

template <class Test, class A, class>
struct JR_Impl {
    static void Do(Z80* p) {
        Data16 pc = Z80::Register<Z80::PC>::GetW(p);
        Data8 jmp = A::Get(p);
        pc.u += jmp.s + 2;
        if (Test::Do(p)) {
            Z80::Register<Z80::PC>::SetW(p, pc);
        } else {
            p->next_opcode();
        }
    }
    static void Print(Z80* p) {
        cinstr << "jr";
        Test::Print(p);
        cinstr << " ";
        A::Print(p);
        cinstr << std::endl;
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
struct RET_Impl {
    static void Do(Z80* p) {
        if (Test::Do(p)) {
            POP<Z80::Register<Z80::PC>, void>::Do(p);
            --p->_pc.u;
        } else {
            p->next_opcode();
        }
    }
    static void Print(Z80* p) {
        cinstr << "ret ";
        Test::Print(p);
        cinstr << std::endl;
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
struct JP_Impl {
    static void Do(Z80* p) {
        Data16 jmp = A::GetW(p);
        if (Test::Do(p)) {
            Z80::Register<Z80::PC>::SetW(p, jmp);
        } else {
            p->next_opcode();
        }
    }
    static void Print(Z80* p) {
        cinstr << "jp";
        Test::Print(p);
        cinstr << " ";
        A::Print(p);
        cinstr << std::endl;
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

template <class A, class>
struct PUSH {
    static void Do(Z80* p) {
        Data16 sp = Z80::Register<Z80::SP>::GetW(p);
        sp.u -= 2;
        Z80::Register<Z80::SP>::SetW(p, sp);
        ToAddr<Z80::Register<Z80::SP>>::SetW(p, A::GetW(p));
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "push ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class Test, class A, class>
struct CALL_Impl {
    static void Do(Z80* p) {
        Data16 addr = A::GetW(p);
        if (Test::Do(p)) {
            p->next_opcode();  // move PAST the CALL so that we save the
                               // instruction right after it
            PUSH<Z80::Register<Z80::PC>, void>::Do(p);

            Z80::Register<Z80::PC>::SetW(p, addr);
        } else {
            p->next_opcode();
        }
    }
    static void Print(Z80* p) {
        cinstr << "call";
        Test::Print(p);
        cinstr << " ";
        A::Print(p);
        cinstr << std::endl;
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

template <class, class>
struct Nop {};

template <>
void Z80::Instr<Nop, void, void>::Do(Z80* p) {
    p->next_opcode();
}
template <>
void Z80::Instr<Nop, void, void>::Print(Z80*) {
    cinstr << "nop" << std::endl;
}

template <class A, class B>
struct LD {
    static inline void Do(Z80* proc) {
        A::Set(proc, B::Get(proc));
        proc->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ld ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDw {
    static inline void Do(Z80* proc) {
        A::SetW(proc, B::GetW(proc));
        proc->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ld ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDHL {
    static inline void Do(Z80* p) {
        Data16 a = A::GetW(p);
        Data8 offset = B::Get(p);
        int res = a.s + offset.s;
        a.s = res;
        Z80::Register<Z80::HL>::SetW(p, a);
        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(res > 0xFFFF);
        p->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ldhl ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDD;

template <class A, class B>
struct LDD<ToAddr<A>, B> {
    static inline void Do(Z80* proc) {
        Data16 addr = A::GetW(proc);
        ToAddr<A>::Set(proc, B::Get(proc));
        --addr.u;
        A::SetW(proc, addr);
        proc->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ldd (";
        A::Print(p);
        cinstr << "), ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDD<A, ToAddr<B>> {
    static inline void Do(Z80* proc) {
        Data16 addr = B::GetW(proc);
        A::Set(proc, proc->_addr.Get(addr.u));
        --addr.u;
        B::SetW(proc, addr);
        proc->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ldd ";
        A::Print(p);
        cinstr << ", ";
        ToAddr<B>::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDI;

template <class A, class B>
struct LDI<ToAddr<A>, B> {
    static inline void Do(Z80* proc) {
        Data16 addr = A::GetW(proc);
        proc->_addr.Set(addr.u, B::Get(proc));
        ++addr.u;
        A::SetW(proc, addr);
        proc->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ldi ";
        ToAddr<A>::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDI<A, ToAddr<B>> {
    static inline void Do(Z80* proc) {
        Data16 addr = B::GetW(proc);
        A::Set(proc, proc->_addr.Get(addr.u));
        ++addr.u;
        B::SetW(proc, addr);
        proc->next_opcode();
    }
    static void Print(Z80* p) {
        cinstr << "ldi ";
        A::Print(p);
        cinstr << ", ";
        ToAddr<B>::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct EI {
    static inline void Do(Z80* p) {
        p->set_interrupts(0xFF);
        p->next_opcode();
    }
    static void Print(Z80*) { cinstr << "ei" << std::endl; }
};
template <class, class>
struct DI {
    static inline void Do(Z80* p) {
        p->set_interrupts(0);
        p->next_opcode();
    }
    static void Print(Z80*) { cinstr << "di" << std::endl; }
};

typedef Z80::Instr<Nop, void, void> NOP;

template <class, class>
struct HALT {
    static inline void Do(Z80* p) {
        // stupidly loop on the halt / do nothing
        p->set_interrupts(0xFF);
        if (p->_addr.Get(0xFF0F).u != 0) {
            p->next_opcode();
        }
    }
    static void Print(Z80*) { cinstr << "halt" << std::endl; }
};

template <uint16_t Addr, class, class>
struct RST_Impl {
    static void Do(Z80* p) {
        p->set_interrupts(0x00);
        --p->_pc.u;
        CALL<I<Addr>, void>::Do(p);
        cinstr << "interrupt caught by " << std::hex << Addr << std::endl;
    }
    static void Print(Z80*) {
        cinstr << "rst 0x" << std::hex << Addr << std::endl;
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
struct RETI {
    static inline void Do(Z80* p) {
        p->set_interrupts(0xFF);
        RET<void, void>::Do(p);
    }
    static void Print(Z80*) { cinstr << "reti" << std::endl; }
};

template <class, class>
struct STOP {
    static inline void Do(Z80* p) { HALT<void, void>::Do(p); }
    static void Print(Z80*) { cinstr << "stop" << std::endl; }
};

template <class, class>
struct EXTENDED {
    static inline void Do(Z80* p) {
        p->next_opcode();
        p->RunCBOpcode(p->_addr.Get(Z80::Register<Z80::PC>::GetW(p).u).u);
    }
    static void Print(Z80* p) {
        cinstr << std::hex << int(p->_addr.Get(p->_pc.u + 1).u) << " ";
        Z80::PrintCBInstr(p->_addr.Get(p->_pc.u + 1).u, p);
        cinstr << "\n";
    }
};
