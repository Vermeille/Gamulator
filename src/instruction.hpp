#pragma once

#include "registers.hpp"
#include "utils.h"

#include <iostream>
#include <type_traits>

struct NextWord {
    static const int cycles = 8;
    static Data16 GetW(Z80* p) {
        Data16 w;
        p->next_opcode();
        w.bytes.l = p->addr().Get(p->pc().u);
        p->next_opcode();
        w.bytes.h = p->addr().Get(p->pc().u);
        return w;
    }

    static void Print(Z80* p) {
        Data16 w;
        w.bytes.l = p->addr().Get(Z80::Register<Z80::PC>::GetW(p).u + 1);
        w.bytes.h = p->addr().Get(Z80::Register<Z80::PC>::GetW(p).u + 2);
        cinstr << w;
    }
};

struct NextByte {
    static const int cycles = 4;
    static Data8 Get(Z80* p) {
        p->next_opcode();
        return p->addr().Get(p->pc().u);
    }
    static void Print(Z80* p) {
        Data8 b = p->addr().Get(Z80::Register<Z80::PC>::GetW(p).u + 1);
        cinstr << b;
    }
};

template <class Addr>
struct ToAddr {
    static const int cycles = 4 + Addr::cycles;
    static inline Data16 GetW(Z80* p) {
        auto addr = Addr::GetW(p).u;
        Data16 w;
        w.bytes.l = p->addr().Get(addr);
        w.bytes.h = p->addr().Get(addr + 1);
        cinstr << std::hex << "read word " << w << " at " << std::hex << addr
               << "(" << p->addr().Print(addr) << ")" << std::endl;
        return w;
    }

    static inline void SetW(Z80* p, Data16 val) {
        auto addr = Addr::GetW(p).u;
        p->addr().Set(addr, uint8_t(val.u & 0xFF));
        p->addr().Set(addr + 1, uint8_t(val.u >> 8));
        cinstr << std::hex << "set word " << val << " at " << std::hex << addr
               << "(" << p->addr().Print(addr) << ")" << std::endl;
    }

    static inline Data8 Get(Z80* p) {
        word addr = Addr::GetW(p).u;
        auto b = p->addr().Get(addr);
        cinstr << std::hex << "  read byte " << b << " at " << std::hex << addr
               << "(" << p->addr().Print(addr) << ")" << std::endl;
        return b;
    }

    static inline void Set(Z80* p, Data8 val) {
        auto addr = Addr::GetW(p).u;
        p->addr().Set(addr, val);
        cinstr << "  set byte " << std::hex << val << " at " << std::hex << addr
               << "(" << p->addr().Print(addr) << ")" << std::endl;
    }

    static void Print(Z80* p) {
        cinstr << "(";
        Addr::Print(p);
        cinstr << ")";
    }
};

template <class Addr>
struct ToAddrFF00 {
    static const int cycles = 4 + Addr::cycles;
    static inline Data8 Get(Z80* p) {
        Data8 offset = Addr::Get(p);
        auto addr = 0xFF00 + offset.u;
        auto b = p->addr().Get(addr);

        cinstr << std::hex << "  read byte " << int(b.u) << " at " << addr
               << "(" << p->addr().Print(addr) << ")" << std::endl;

        return b;
    }

    static inline void Set(Z80* p, Data8 val) {
        Data8 offset = Addr::Get(p);
        auto addr = 0xFF00 + offset.u;
        p->addr().Set(addr, val);
        cinstr << std::hex << "  set byte " << int(val.u) << " at " << addr
               << "(" << p->addr().Print(addr) << ")" << std::endl;
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
    static int Do(Z80* p) {
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
        return 4;
    }

    static void Print(Z80*) { cinstr << "daa\n"; }
};

template <class Val, class>
struct RLC {
    static int Do(Z80* p) {
        Data8 res = Val::Get(p);

        res.u = (res.u << 1) | ((res.u >> 7) & 1);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(res.u & 1);

        Val::Set(p, res);
        p->next_opcode();
        return 8 + 2 * Val::cycles;
    }

    static void Print(Z80* p) {
        cinstr << "rlc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct RLCA {
    static int Do(Z80* p) {
        Data8 res = Z80::Register<Z80::A>::Get(p);

        res.u = (res.u << 1) | ((res.u >> 7) & 1);

        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(res.u & 1);

        Z80::Register<Z80::A>::Set(p, res);
        p->next_opcode();
        return 4;
    }

    static void Print(Z80*) { cinstr << "rlca\n"; }
};

template <class Val, class sdfs>
struct RRC {
    static int Do(Z80* p) {
        Data8 res = Val::Get(p);

        res.u = (res.u >> 1) | ((res.u & 1) << 7);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(res.u >> 7);

        Val::Set(p, res);
        p->next_opcode();
        return 8 + 2 * Val::cycles;
    }

    static void Print(Z80* p) {
        cinstr << "rrc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct RRCA {
    static int Do(Z80* p) {
        Data8 res = Z80::Register<Z80::A>::Get(p);

        int c = res.u & 1;
        res.u = (res.u >> 1) | (c << 7);

        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(c);

        Z80::Register<Z80::A>::Set(p, res);
        p->next_opcode();
        return 4;
    }

    static void Print(Z80*) { cinstr << "rrca\n"; }
};

template <class Val, class>
struct RL {
    static int Do(Z80* p) {
        Data8 res = Val::Get(p);
        int c = p->carry_f();

        p->set_carry_f(res.u >> 7);

        res.u = res.u << 1 | c;

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        Val::Set(p, res);
        p->next_opcode();
        return 8 + 2 * Val::cycles;
    }

    static void Print(Z80* p) {
        cinstr << "rl ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct RLA {
    static int Do(Z80* p) {
        Data8 res = Z80::Register<Z80::A>::Get(p);
        int c = p->carry_f();

        p->set_carry_f(res.u >> 7);

        res.u = res.u << 1 | c;

        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        Z80::Register<Z80::A>::Set(p, res);
        p->next_opcode();
        return 4;
    }

    static void Print(Z80*) { cinstr << "rla\n"; }
};

template <class Val, class>
struct RR {
    static int Do(Z80* p) {
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
        return 8 + 2 * Val::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "rr ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct RRA {
    static int Do(Z80* p) {
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
        return 4;
    }
    static void Print(Z80* p) {
        cinstr << "rra ";
        Z80::Register<Z80::A>::Print(p);
        cinstr << std::endl;
    }
};

template <class Bit, class A>
struct RES {
    static int Do(Z80* p) {
        Data8 res = A::Get(p);
        res.u = res.u & ~(1 << Bit::Get().u);
        A::Set(p, res);
        p->next_opcode();
        return 8 + 2 * A::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "res " << Bit::Get() << ", ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class Bit, class A>
struct SET {
    static int Do(Z80* p) {
        A::Set(p, uint8_t(A::Get(p).u | (1 << Bit::Get().u)));
        p->next_opcode();
        return 8 + 2 * A::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "set " << Bit::Get() << ", ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class Bit, class R>
struct BIT {
    static int Do(Z80* p) {
        p->set_zero_f(!(R::Get(p).u & (1 << Bit::Get().u)));
        p->set_sub_f(0);
        p->set_hcarry_f(1);
        p->next_opcode();
        return 8 + R::cycles;
    }

    static void Print(Z80* p) {
        cinstr << "bit " << Bit::Get() << ", ";
        R::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct CPL {
    static inline int Do(Z80* p) {
        uint8_t r = 0xFF ^ A::Get(p).u;
        A::Set(p, r);

        p->set_sub_f(1);
        p->set_hcarry_f(1);
        p->next_opcode();
        return 4;
    }

    static void Print(Z80* p) {
        cinstr << "cpl ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SCF {
    static inline int Do(Z80* p) {
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(true);
        p->next_opcode();
        return 4;
    }
    static void Print(Z80*) {
        cinstr << "scf ";
        cinstr << std::endl;
    }
};

template <class A, class>
struct CCF {
    static inline int Do(Z80* p) {
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(!p->carry_f());
        p->next_opcode();
        return 4;
    }
    static void Print(Z80*) {
        cinstr << "ccf ";
        cinstr << std::endl;
    }
};

template <class Val, class>
struct INC {
    static inline int Do(Z80* p) {
        Data8 res = Val::Get(p);
        p->set_hcarry_f((res.u & 0xf) == 0xf);
        ++res.u;
        Val::Set(p, res);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->next_opcode();
        return 4 + 2 * Val::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "inc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct INCw {
    static inline int Do(Z80* p) {
        Data16 res = Val::GetW(p);
        res.u += 1;
        Val::SetW(p, res);
        p->next_opcode();
        return 8;
    }
    static void Print(Z80* p) {
        cinstr << "inc ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct DEC {
    static inline int Do(Z80* p) {
        Data8 res = Val::Get(p);
        p->set_hcarry_f((res.u & 0xf) == 0);
        --res.u;
        Val::Set(p, res);

        p->set_zero_f(res.u == 0);
        p->set_sub_f(1);
        p->next_opcode();
        return 4 + 2 * Val::cycles;
    }

    static void Print(Z80* p) {
        cinstr << "dec ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class Val, class>
struct DECw {
    static inline int Do(Z80* p) {
        Data16 r = Val::GetW(p);
        r.u -= 1;
        Val::SetW(p, r);
        p->next_opcode();
        return 8;
    }

    static void Print(Z80* p) {
        cinstr << "dec ";
        Val::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct ADD {
    static int Do(Z80* p) {
        Data8 a = A::Get(p);
        Data8 b = B::Get(p);
        uint16_t res = a.u + b.u;

        p->set_zero_f((res & 0xFF) == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(((a.u & 0xf) + (b.u & 0xf)) >> 4);  // FIXME
        p->set_carry_f(res >> 8);

        A::Set(p, uint8_t(res & 0xFF));
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        Data16 a = A::GetW(p);
        Data16 b = B::GetW(p);
        uint32_t res = a.u + b.u;

        p->set_sub_f(false);
        p->set_hcarry_f(((a.u & 0xFFF) + (b.u & 0xFFF)) >> 12);
        p->set_carry_f(res >> 16);

        A::SetW(p, uint16_t(res & 0xFFFF));
        p->next_opcode();
        return 8;
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
    static int Do(Z80* p) {
        Data16 a = A::GetW(p);
        Data8 b = B::Get(p);
        Data16 res;
        int r = a.s + b.s;
        res.u = r;

        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f(((a.u & 0xf) + (b.u & 0xf)) >> 4);
        p->set_carry_f(((a.u & 0xff) + (b.u & 0xff)) >> 8);

        A::SetW(p, res);
        p->next_opcode();
        return 16;
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
    static int Do(Z80* p) {
        Data8 a = A::Get(p);
        Data8 b = B::Get(p);
        uint16_t res = a.u + b.u + p->carry_f();

        p->set_zero_f((res & 0xFF) == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(((a.u & 0xf) + (b.u & 0xf) + p->carry_f()) >> 4);
        p->set_carry_f(res >> 8);

        A::Set(p, uint8_t(res & 0xFF));
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        Data8 a = A::Get(p);
        Data8 b = B::Get(p);
        Data8 res;
        int r = a.u - b.u;
        res.s = r;

        p->set_zero_f(res.u == 0);
        p->set_hcarry_f((a.s ^ b.s ^ res.s) & 0x10);
        p->set_sub_f(true);
        p->set_carry_f(r & 0x100);

        A::Set(p, res);
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        Data8 a = A::Get(p);
        Data8 b = B::Get(p);
        Data8 res;
        int r = a.u - b.u - p->carry_f();
        res.u = r;

        p->set_zero_f(res.u == 0);
        p->set_hcarry_f((a.u ^ b.u ^ r) & 0x10);
        p->set_sub_f(true);
        p->set_carry_f(r & 0x100);

        A::Set(p, res);
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        uint8_t res = A::Get(p).u & B::Get(p).u;

        p->set_zero_f(res == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(true);
        p->set_carry_f(false);

        A::Set(p, res);
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        uint8_t res = A::Get(p).u ^ B::Get(p).u;

        p->set_zero_f(res == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(false);

        A::Set(p, res);
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        uint8_t res = A::Get(p).u | B::Get(p).u;

        p->set_zero_f(res == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(false);

        A::Set(p, res);
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        Data8 a = A::Get(p);

        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(a.u & 1);

        a.u = (a.u >> 1);
        a.u = ClearBit(a.u, 7);
        p->set_zero_f(a.u == 0);

        A::Set(p, a);
        p->next_opcode();
        return 8 + 2 * A::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "srl ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SLA {
    static int Do(Z80* p) {
        Data8 a = A::Get(p);

        p->set_carry_f(GetBit(a.u, 7));

        a.u = a.u << 1;
        p->set_zero_f(a.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        A::Set(p, a);
        p->next_opcode();
        return 8 + 2 * A::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "sla ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct SRA {
    static int Do(Z80* p) {
        Data8 a = A::Get(p);

        p->set_carry_f(GetBit(a.u, 0));

        a.u = a.u >> 1;
        a.u = SetBit(a.u, 7, GetBit(a.u, 6));
        p->set_zero_f(a.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);

        A::Set(p, a);
        p->next_opcode();
        return 8 + 2 * A::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "sla ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct CP {
    static int Do(Z80* p) {
        auto a = A::Get(p).u;
        auto b = B::Get(p).u;

        p->set_zero_f(a == b);
        p->set_sub_f(true);
        p->set_hcarry_f((a & 0xf) < (b & 0xf));
        p->set_carry_f(a < b);
        p->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static int Do(Z80* p) {
        Data8 res = A::Get(p);

        res.u = (res.u << 4) | (res.u >> 4);
        A::Set(p, res);
        p->set_zero_f(res.u == 0);
        p->set_sub_f(false);
        p->set_hcarry_f(false);
        p->set_carry_f(false);
        p->next_opcode();
        return 8 + 2 * A::cycles;
    }
    static void Print(Z80* p) {
        cinstr << "swap ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class>
struct POP {
    static int Do(Z80* p) {
        A::SetW(p, ToAddr<Z80::Register<Z80::SP>>::GetW(p));
        Data16 sp = Z80::Register<Z80::SP>::GetW(p);
        sp.u += 2;
        Z80::Register<Z80::SP>::SetW(p, sp);
        p->next_opcode();
        return 12;
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
    static int Do(Z80* p) {
        Data16 pc = Z80::Register<Z80::PC>::GetW(p);
        Data8 jmp = A::Get(p);
        pc.u += jmp.s + 2;
        if (Test::Do(p)) {
            Z80::Register<Z80::PC>::SetW(p, pc);
            return 12;
        } else {
            p->next_opcode();
            return 8;
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
    static int Do(Z80* p) {
        if (Test::Do(p)) {
            POP<Z80::Register<Z80::PC>, void>::Do(p);
            --p->pc().u;
            return std::conditional_t<std::is_same<Test, True>::value,
                                      I<16>,
                                      I<20>>::Get()
                .u;
        } else {
            p->next_opcode();
            return 8;
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

template <class Test, class A, class>
struct JP_Impl {
    static int Do(Z80* p) {
        Data16 jmp = A::GetW(p);
        if (Test::Do(p)) {
            Z80::Register<Z80::PC>::SetW(p, jmp);
            return 8 + A::cycles;  // FIXME: should be 4 for jp HL
        } else {
            p->next_opcode();
            return 12;
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
    static int Do(Z80* p) {
        Data16 sp = Z80::Register<Z80::SP>::GetW(p);
        sp.u -= 2;
        Z80::Register<Z80::SP>::SetW(p, sp);
        ToAddr<Z80::Register<Z80::SP>>::SetW(p, A::GetW(p));
        p->next_opcode();
        return 16;
    }
    static void Print(Z80* p) {
        cinstr << "push ";
        A::Print(p);
        cinstr << std::endl;
    }
};

template <class Test, class A, class>
struct CALL_Impl {
    static int Do(Z80* p) {
        Data16 addr = A::GetW(p);
        if (Test::Do(p)) {
            p->next_opcode();  // move PAST the CALL so that we save the
                               // instruction right after it
            PUSH<Z80::Register<Z80::PC>, void>::Do(p);

            Z80::Register<Z80::PC>::SetW(p, addr);
            return 24;
        } else {
            p->next_opcode();
            return 12;
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
struct Nop {
    static int Do(Z80* p) {
        p->next_opcode();
        return 4;
    }
    void Print(Z80*) { cinstr << "nop" << std::endl; }
};

template <class A, class B>
struct LD {
    static inline int Do(Z80* proc) {
        A::Set(proc, B::Get(proc));
        proc->next_opcode();
        return 4 + A::cycles + B::cycles;
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
    static inline int Do(Z80* proc) {
        A::SetW(proc, B::GetW(proc));
        proc->next_opcode();
        return 12;
    }
    static void Print(Z80* p) {
        cinstr << "ld ";
        A::Print(p);
        cinstr << ", ";
        B::Print(p);
        cinstr << std::endl;
    }
};

template <class, class>
struct LDHLSPN {
    static inline int Do(Z80* p) {
        Data16 a = Z80::Register<Z80::SP>::GetW(p);
        Data8 offset = NextByte::Get(p);
        int res = a.u + offset.s;
        p->set_zero_f(false);
        p->set_sub_f(false);
        p->set_hcarry_f((a.u ^ offset.s ^ res) & 0x10);
        p->set_carry_f((a.u ^ offset.s ^ res) & 0x100);
        a.u = res;
        Z80::Register<Z80::HL>::SetW(p, a);
        p->next_opcode();
        return 12;
    }
    static void Print(Z80* p) {
        cinstr << "ldhl ";
        Z80::Register<Z80::SP>::Print(p);
        cinstr << ", ";
        NextByte::Print(p);
        cinstr << std::endl;
    }
};

template <class A, class B>
struct LDD;

template <class A, class B>
struct LDD<ToAddr<A>, B> {
    static inline int Do(Z80* proc) {
        Data16 addr = A::GetW(proc);
        ToAddr<A>::Set(proc, B::Get(proc));
        --addr.u;
        A::SetW(proc, addr);
        proc->next_opcode();
        return 8;
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
    static inline int Do(Z80* proc) {
        Data16 addr = B::GetW(proc);
        A::Set(proc, proc->addr().Get(addr.u));
        --addr.u;
        B::SetW(proc, addr);
        proc->next_opcode();
        return 8;
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
    static inline int Do(Z80* proc) {
        Data16 addr = A::GetW(proc);
        proc->addr().Set(addr.u, B::Get(proc));
        ++addr.u;
        A::SetW(proc, addr);
        proc->next_opcode();
        return 8;
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
    static inline int Do(Z80* proc) {
        Data16 addr = B::GetW(proc);
        A::Set(proc, proc->addr().Get(addr.u));
        ++addr.u;
        B::SetW(proc, addr);
        proc->next_opcode();
        return 8;
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
    static inline int Do(Z80* p) {
        p->set_interrupts(0xFF);
        p->next_opcode();
        return 4;
    }
    static void Print(Z80*) { cinstr << "ei" << std::endl; }
};
template <class, class>
struct DI {
    static inline int Do(Z80* p) {
        p->set_interrupts(0);
        p->next_opcode();
        return 4;
    }
    static void Print(Z80*) { cinstr << "di" << std::endl; }
};

template <class, class>
struct NOP {
    static inline int Do(Z80* p) {
        p->next_opcode();
        return 4;
    }
    static void Print(Z80*) { cinstr << "nop" << std::endl; }
};

template <class, class>
struct HALT {
    static inline int Do(Z80* p) {
        p->set_halt(true);
        p->next_opcode();
        return 4;
    }
    static void Print(Z80*) { cinstr << "halt" << std::endl; }
};

template <uint16_t Addr, class, class>
struct RST_Impl {
    static int Do(Z80* p) {
        p->set_interrupts(0x00);
        CALL<I<Addr>, void>::Do(p);
        cinstr << "interrupt caught by " << std::hex << Addr << std::endl;
        return 16;
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
    static const int cycles = 16;
    static inline int Do(Z80* p) {
        p->set_interrupts(0xFF);
        RET<void, void>::Do(p);
        return 16;
    }
    static void Print(Z80*) { cinstr << "reti" << std::endl; }
};

template <class, class>
struct STOP {
    static inline int Do(Z80* p) {
        HALT<void, void>::Do(p);
        return 4;
    }
    static void Print(Z80*) { cinstr << "stop" << std::endl; }
};

template <class, class>
struct EXTENDED {
    static inline int Do(Z80* p) {
        p->next_opcode();
        return p->RunCBOpcode(
            p->addr().Get(Z80::Register<Z80::PC>::GetW(p).u).u);
    }
    static void Print(Z80* p) {
        cinstr << std::hex << int(p->addr().Get(p->pc().u + 1).u) << " ";
        Z80::PrintCBInstr(p->addr().Get(p->pc().u + 1).u, p);
        cinstr << "\n";
    }
};
