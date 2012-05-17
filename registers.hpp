#pragma once

template <>
struct Z80::Register<Z80::A>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[7];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[7] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "A";
    }
};

template <>
struct Z80::Register<Z80::B>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[0];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[0] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "B";
    }
};

template <>
struct Z80::Register<Z80::C>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[1];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[1] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "C";
    }
};

template <>
struct Z80::Register<Z80::D>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[2];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[2] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "D";
    }
};

template <>
struct Z80::Register<Z80::E>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[3];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[3] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "E";
    }
};

template <>
struct Z80::Register<Z80::F>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[6];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[6] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "F";
    }
};

template <>
struct Z80::Register<Z80::H>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[4];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[4] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "H";
    }
};

template <>
struct Z80::Register<Z80::L>
{
    static inline byte Get(Z80* proc)
    {
        return proc->_regs[5];
    }

    static inline void Set(Z80* proc, byte val)
    {
        proc->_regs[5] = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "L";
    }
};

template <>
struct Z80::Register<Z80::AF>
{
    static inline word Get(Z80* proc)
    {
        return (Z80::Register<A>::Get(proc) << 8) + Z80::Register<F>::Get(proc);
    }

    static inline void Set(Z80* proc, word val)
    {
        Z80::Register<A>::Set(proc, val >> 8);
        Z80::Register<F>::Set(proc, val & 0xFF);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "AF";
    }
};

template <>
struct Z80::Register<Z80::BC>
{
    static inline word Get(Z80* proc)
    {
        return (Z80::Register<B>::Get(proc) << 8) + Z80::Register<C>::Get(proc);
    }

    static inline void Set(Z80* proc, word val)
    {
        Z80::Register<B>::Set(proc, val >> 8);
        Z80::Register<C>::Set(proc, val & 0xFF);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "BC";
    }
};

template <>
struct Z80::Register<Z80::DE>
{
    static inline word Get(Z80* proc)
    {
        return (Z80::Register<D>::Get(proc) << 8) + Z80::Register<E>::Get(proc);
    }

    static inline void Set(Z80* proc, word val)
    {
        Z80::Register<D>::Set(proc, val >> 8);
        Z80::Register<E>::Set(proc, val & 0xFF);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "DE";
    }
};

template <>
struct Z80::Register<Z80::HL>
{
    static inline word Get(Z80* proc)
    {
        return (Z80::Register<H>::Get(proc)<<8) + Z80::Register<L>::Get(proc);
    }

    static inline void Set(Z80* proc, word val)
    {
        Z80::Register<H>::Set(proc, val >> 8);
        Z80::Register<L>::Set(proc, val & 0xFF);
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "HL";
    }
};

template <>
struct Z80::Register<Z80::SP>
{
    static inline word Get(Z80* proc)
    {
        return proc->_sp;
    }

    static inline void Set(Z80* proc, word val)
    {
        proc->_sp = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "SP";
    }
};

template <>
struct Z80::Register<Z80::PC>
{
    static inline word Get(Z80* proc)
    {
        return proc->_pc;
    }

    static inline void Set(Z80* proc, word val)
    {
        proc->_pc = val;
    }
    static void Print(int&, AddressBus&)
    {
        std::cout << "PC";
    }
};
