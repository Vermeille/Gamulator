#pragma once

#include "utils.h"

template <>
struct Z80::Register<Z80::B> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[0]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[0] = val; }
    static void Print(Z80* p) { cinstr << "B(" << p->_regs[0] << ")"; }
};

template <>
struct Z80::Register<Z80::C> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[1]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[1] = val; }
    static void Print(Z80* p) { cinstr << "C(" << p->_regs[1] << ")"; }
};

template <>
struct Z80::Register<Z80::D> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[2]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[2] = val; }
    static void Print(Z80* p) { cinstr << "D(" << p->_regs[2] << ")"; }
};

template <>
struct Z80::Register<Z80::E> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[3]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[3] = val; }
    static void Print(Z80* p) { cinstr << "E(" << p->_regs[3] << ")"; }
};

template <>
struct Z80::Register<Z80::H> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[4]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[4] = val; }
    static void Print(Z80* p) { cinstr << "H(" << p->_regs[4] << ")"; }
};

template <>
struct Z80::Register<Z80::L> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[5]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[5] = val; }
    static void Print(Z80* p) { cinstr << "L(" << p->_regs[5] << ")"; }
};

template <>
struct Z80::Register<Z80::A> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[7]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[7] = val; }
    static void Print(Z80* p) { cinstr << "A(" << p->_regs[7] << ")"; }
};

template <>
struct Z80::Register<Z80::F> {
    static const int cycles = 0;
    static inline Data8 Get(const Z80* proc) { return proc->_regs[6]; }

    static inline void Set(Z80* proc, Data8 val) { proc->_regs[6].u = val.u; }
    static void Print(Z80* p) { cinstr << "F(" << p->_regs[6] << ")"; }
};

template <>
struct Z80::Register<Z80::AF> {
    static const int cycles = 0;
    static inline Data16 GetW(const Z80* proc) {
        Data16 x;
        x.bytes.h = Z80::Register<A>::Get(proc);
        x.bytes.l = Z80::Register<F>::Get(proc);
        return x;
    }

    static inline void SetW(Z80* proc, Data16 val) {
        Z80::Register<A>::Set(proc, val.bytes.h);
        Z80::Register<F>::Set(proc, uint8_t(val.bytes.l.u & 0xF0));
    }
    static void Print(Z80* p) { cinstr << "AF(" << GetW(p) << ")"; }
};

template <>
struct Z80::Register<Z80::BC> {
    static const int cycles = 0;
    static inline Data16 GetW(const Z80* proc) {
        Data16 x;
        x.bytes.h = Z80::Register<B>::Get(proc);
        x.bytes.l = Z80::Register<C>::Get(proc);
        return x;
    }

    static inline void SetW(Z80* proc, Data16 val) {
        Z80::Register<B>::Set(proc, val.bytes.h);
        Z80::Register<C>::Set(proc, val.bytes.l);
    }
    static void Print(Z80* p) { cinstr << "BC(" << GetW(p) << ")"; }
};

template <>
struct Z80::Register<Z80::DE> {
    static const int cycles = 0;
    static inline Data16 GetW(const Z80* proc) {
        Data16 x;
        x.bytes.h = Z80::Register<D>::Get(proc);
        x.bytes.l = Z80::Register<E>::Get(proc);
        return x;
    }

    static inline void SetW(Z80* proc, Data16 val) {
        Z80::Register<D>::Set(proc, val.bytes.h);
        Z80::Register<E>::Set(proc, val.bytes.l);
    }
    static void Print(Z80* p) { cinstr << "DE(" << GetW(p) << ")"; }
};

template <>
struct Z80::Register<Z80::HL> {
    static const int cycles = 0;
    static inline Data16 GetW(const Z80* proc) {
        Data16 x;
        x.bytes.h = Z80::Register<H>::Get(proc);
        x.bytes.l = Z80::Register<L>::Get(proc);
        return x;
    }

    static inline void SetW(Z80* proc, Data16 val) {
        Z80::Register<H>::Set(proc, val.bytes.h);
        Z80::Register<L>::Set(proc, val.bytes.l);
    }
    static void Print(Z80* p) { cinstr << "HL(" << GetW(p) << ")"; }
};

template <>
struct Z80::Register<Z80::SP> {
    static const int cycles = 0;
    static inline Data16 GetW(const Z80* proc) { return proc->_sp; }

    static inline void SetW(Z80* proc, Data16 val) { proc->_sp = val; }
    static void Print(Z80* p) { cinstr << "SP(" << GetW(p) << ")"; }
};

template <>
struct Z80::Register<Z80::PC> {
    static const int cycles = 0;
    static inline Data16 GetW(const Z80* proc) { return proc->_pc; }

    static inline void SetW(Z80* proc, Data16 val) { proc->_pc = val; }
    static void Print(Z80* p) { cinstr << "PC(" << GetW(p) << ")"; }
};
