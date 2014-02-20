/*
 ** z80.cpp for gameboy
 **
 ** Made by Guillaume "Vermeille" Sanchez
 ** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
 **
 ** Started on  mer. 25 avril 2012 13:59:09 CEST Guillaume "Vermeille" Sanchez
 ** Last update 2014-02-20 17:36 vermeille
 */


#include "z80.h"

#include <iomanip>
#include <iostream>
#include <tuple>
#include <vector>

#include "instruction.hpp"

    Z80::Z80(AddressBus& addr, Video& v)
: _sp(0xFFFE), _pc(0x100), _addr(addr), _vid(v), _interrupts(0xFF)
{
    Register<F>::Set(this, 0xB0);
    Register<A>::Set(this, 0x01);
    Register<C>::Set(this, 0x13);
    Register<B>::Set(this, 0x00);
    Register<E>::Set(this, 0xD8);
    Register<D>::Set(this, 0x00);
    Register<L>::Set(this, 0x4D);
    Register<H>::Set(this, 0x01);

    _addr.Set(0xFF05, 0x00); // TIMA
    _addr.Set(0xFF06, 0X00); // TMA
    _addr.Set(0xFF07, 0x00); // TAC
    _addr.Set(0xFF10, 0x80); // NR10
    _addr.Set(0xFF11, 0xBF); // NR11
    _addr.Set(0xFF12, 0xF3); // NR12
    _addr.Set(0xFF14, 0xBF); // NR14
    _addr.Set(0xFF16, 0x3F); // NR21
    _addr.Set(0xFF17, 0x00); // NR22
    _addr.Set(0xFF19, 0xBF); // NR24
    _addr.Set(0xFF1A, 0x7F); // NR30
    _addr.Set(0xFF1B, 0xFF); // NR31
    _addr.Set(0xFF1C, 0x9F); // NR32
    _addr.Set(0xFF1E, 0xBF); // NR33
    _addr.Set(0xFF20, 0xFF); // NR41
    _addr.Set(0xFF21, 0x00); // NR42
    _addr.Set(0xFF22, 0x00); // NR43
    _addr.Set(0xFF23, 0xBF); // NR30
    _addr.Set(0xFF24, 0x77); // NR50
    _addr.Set(0xFF25, 0xF3); // NR51
    _addr.Set(0xFF26, 0xF1); // NR52
    _addr.Set(0xFF40, 0x91); // LCDC
    _addr.Set(0xFF42, 0x00); // SCY
    _addr.Set(0xFF43, 0x00); // SCX
    _addr.Set(0xFF45, 0x00); // LYC
    _addr.Set(0xFF47, 0xFC); // BGP
    _addr.Set(0xFF48, 0xFF); // OBP0
    _addr.Set(0xFF49, 0xFF); // OBP1
    _addr.Set(0xFF4A, 0x00); // WY
    _addr.Set(0xFF4B, 0x00); // WX
    _addr.Set(0xFFFF, 0x00); // IE

    RegisterOpcode<0x00, NOP>();
    RegisterOpcode<0x01, Instr<LD, Register<BC>, NextWord>>();
    RegisterOpcode<0x02, Instr<LD,ToAddr<Register<BC>>,Register<A>>>();
    RegisterOpcode<0x03, Instr<INC, Register<BC>, void>>();
    RegisterOpcode<0x04, Instr<INC, Register<B>, void>>();
    RegisterOpcode<0x05, Instr<DEC, Register<B>, void>>();
    RegisterOpcode<0x06, Instr<LD, Register<B>, NextByte>>();
    RegisterOpcode<0x07, Instr<RLC, Register<A>, void>>();
    RegisterOpcode<0x08, Instr<LD, ToAddr<NextWord>, Register<SP>>>();
    RegisterOpcode<0x09, Instr<ADD, Register<HL>, Register<BC>>>();
    RegisterOpcode<0x0A, Instr<LD,Register<A>,ToAddr<Register<BC>>>>();
    RegisterOpcode<0x0B, Instr<DEC, Register<BC>, void>>();
    RegisterOpcode<0x0C, Instr<INC, Register<C>, void>>();
    RegisterOpcode<0x0D, Instr<DEC, Register<C>, void>>();
    RegisterOpcode<0x0E, Instr<LD, Register<C>, NextByte>>();
    RegisterOpcode<0x0F, Instr<RRC, Register<A>, void>>();
    RegisterOpcode<0x10, Instr<Stop, void, void>>();
    RegisterOpcode<0x11, Instr<LD, Register<DE>, NextWord>>();
    RegisterOpcode<0x12, Instr<LD,ToAddr<Register<DE>>,Register<A>>>();
    RegisterOpcode<0x13, Instr<INC, Register<DE>, void>>();
    RegisterOpcode<0x14, Instr<INC, Register<D>, void>>();
    RegisterOpcode<0x15, Instr<DEC, Register<D>, void>>();
    RegisterOpcode<0x16, Instr<LD, Register<D>, NextByte>>();
    RegisterOpcode<0x17, Instr<RL, Register<A>, void>>();
    RegisterOpcode<0x18, Instr<JR, NextByte, void>>();
    RegisterOpcode<0x19, Instr<ADD, Register<HL>, Register<DE>>>();
    RegisterOpcode<0x1A, Instr<LD, Register<A>, ToAddr<Register<DE>> >>();
    RegisterOpcode<0x1B, Instr<DEC, Register<DE>, void>>();
    RegisterOpcode<0x1C, Instr<INC, Register<E>, void>>();
    RegisterOpcode<0x1D, Instr<DEC, Register<E>, void>>();
    RegisterOpcode<0x1E, Instr<LD, Register<E>, NextByte>>();
    RegisterOpcode<0x1F, Instr<RR, Register<A>, void>>();
    RegisterOpcode<0x20, Instr<JRNZ, NextByte, void>>();
    RegisterOpcode<0x21, Instr<LD, Register<HL>, NextWord>>();
    RegisterOpcode<0x22, Instr<LDI, ToAddr<Register<HL>>, Register<A>>>();
    RegisterOpcode<0x23, Instr<INC, Register<HL>, void>>();
    RegisterOpcode<0x24, Instr<INC, Register<H>, void>>();
    RegisterOpcode<0x25, Instr<DEC, Register<H>, void>>();
    RegisterOpcode<0x26, Instr<LD, Register<H>, NextByte>>();
    RegisterOpcode<0x27, Instr<Nop, void, void>>(); // FIXME DAA
    RegisterOpcode<0x28, Instr<JRZ, NextByte, void>>();
    RegisterOpcode<0x29, Instr<ADD, Register<HL>, Register<HL>>>();
    RegisterOpcode<0x2A, Instr<LDI, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x2B, Instr<DEC, Register<HL>, void>>();
    RegisterOpcode<0x2C, Instr<INC, Register<L>, void>>();
    RegisterOpcode<0x2D, Instr<DEC, Register<L>, void>>();
    RegisterOpcode<0x2E, Instr<LD, Register<L>, NextByte>>();
    RegisterOpcode<0x2F, Instr<CPL, Register<A>, void>>();
    RegisterOpcode<0x30, Instr<JRC, NextByte, void>>();
    RegisterOpcode<0x31, Instr<LD, Register<SP>, NextWord>>();
    RegisterOpcode<0x32, Instr<LDD, ToAddr<Register<HL>>, Register<A>>>();
    RegisterOpcode<0x33, Instr<INC, Register<SP>, void>>();
    RegisterOpcode<0x34, Instr<INC, ToAddr<Register<HL>>, void>>();
    RegisterOpcode<0x35, Instr<DEC, ToAddr<Register<HL>>, void>>();
    RegisterOpcode<0x36, Instr<LD, ToAddr<Register<HL>>, NextByte>>();
    RegisterOpcode<0x37, Instr<SCF, void, void>>();
    RegisterOpcode<0x38, Instr<JRC, NextByte, void>>();
    RegisterOpcode<0x39, Instr<ADD, Register<HL>, Register<SP>>>();
    RegisterOpcode<0x3A, Instr<LDD, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x3B, Instr<DEC, Register<SP>, void>>();
    RegisterOpcode<0x3C, Instr<INC, Register<A>, void>>();
    RegisterOpcode<0x3D, Instr<DEC, Register<A>, void>>();
    RegisterOpcode<0x3E, Instr<LD, Register<A>, NextByte>>();
    RegisterOpcode<0x3F, Instr<CCF, void, void>>();
    RegisterOpcode<0x40, Instr<LD, Register<B>, Register<B>>>();
    RegisterOpcode<0x41, Instr<LD, Register<B>, Register<C>>>();
    RegisterOpcode<0x42, Instr<LD, Register<B>, Register<D>>>();
    RegisterOpcode<0x43, Instr<LD, Register<B>, Register<E>>>();
    RegisterOpcode<0x44, Instr<LD, Register<B>, Register<H>>>();
    RegisterOpcode<0x45, Instr<LD, Register<B>, Register<L>>>();
    RegisterOpcode<0x46, Instr<LD, Register<B>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x47, Instr<LD, Register<B>, Register<A>>>();
    RegisterOpcode<0x48, Instr<LD, Register<C>, Register<B>>>();
    RegisterOpcode<0x49, Instr<LD, Register<C>, Register<C>>>();
    RegisterOpcode<0x4A, Instr<LD, Register<C>, Register<D>>>();
    RegisterOpcode<0x4B, Instr<LD, Register<C>, Register<D>>>();
    RegisterOpcode<0x4C, Instr<LD, Register<C>, Register<H>>>();
    RegisterOpcode<0x4D, Instr<LD, Register<C>, Register<L>>>();
    RegisterOpcode<0x4E, Instr<LD, Register<C>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x4F, Instr<LD, Register<C>, Register<A>>>();
    RegisterOpcode<0x50, Instr<LD, Register<D>, Register<B>>>();
    RegisterOpcode<0x51, Instr<LD, Register<D>, Register<C>>>();
    RegisterOpcode<0x52, Instr<LD, Register<D>, Register<D>>>();
    RegisterOpcode<0x53, Instr<LD, Register<D>, Register<E>>>();
    RegisterOpcode<0x54, Instr<LD, Register<D>, Register<H>>>();
    RegisterOpcode<0x55, Instr<LD, Register<D>, Register<L>>>();
    RegisterOpcode<0x56, Instr<LD, Register<D>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x57, Instr<LD, Register<D>, Register<A>>>();
    RegisterOpcode<0x58, Instr<LD, Register<E>, Register<B>>>();
    RegisterOpcode<0x59, Instr<LD, Register<E>, Register<C>>>();
    RegisterOpcode<0x5a, Instr<LD, Register<E>, Register<D>>>();
    RegisterOpcode<0x5b, Instr<LD, Register<E>, Register<E>>>();
    RegisterOpcode<0x5c, Instr<LD, Register<E>, Register<H>>>();
    RegisterOpcode<0x5d, Instr<LD, Register<E>, Register<L>>>();
    RegisterOpcode<0x5e, Instr<LD, Register<E>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x5f, Instr<LD, Register<E>, Register<A>>>();
    RegisterOpcode<0x60, Instr<LD, Register<H>, Register<B>>>();
    RegisterOpcode<0x61, Instr<LD, Register<H>, Register<C>>>();
    RegisterOpcode<0x62, Instr<LD, Register<H>, Register<D>>>();
    RegisterOpcode<0x63, Instr<LD, Register<H>, Register<E>>>();
    RegisterOpcode<0x64, Instr<LD, Register<H>, Register<H>>>();
    RegisterOpcode<0x65, Instr<LD, Register<H>, Register<L>>>();
    RegisterOpcode<0x66, Instr<LD, Register<H>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x67, Instr<LD, Register<H>, Register<A>>>();
    RegisterOpcode<0x68, Instr<LD, Register<L>, Register<B>>>();
    RegisterOpcode<0x69, Instr<LD, Register<L>, Register<C>>>();
    RegisterOpcode<0x6a, Instr<LD, Register<L>, Register<D>>>();
    RegisterOpcode<0x6b, Instr<LD, Register<L>, Register<E>>>();
    RegisterOpcode<0x6c, Instr<LD, Register<L>, Register<H>>>();
    RegisterOpcode<0x6d, Instr<LD, Register<L>, Register<L>>>();
    RegisterOpcode<0x6e, Instr<LD, Register<L>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x6f, Instr<LD, Register<L>, Register<A>>>();
    RegisterOpcode<0x70, Instr<LD, ToAddr<Register<HL>>, Register<B>>>();
    RegisterOpcode<0x71, Instr<LD, ToAddr<Register<HL>>, Register<C>>>();
    RegisterOpcode<0x72, Instr<LD, ToAddr<Register<HL>>, Register<D>>>();
    RegisterOpcode<0x73, Instr<LD, ToAddr<Register<HL>>, Register<E>>>();
    RegisterOpcode<0x74, Instr<LD, ToAddr<Register<HL>>, Register<H>>>();
    RegisterOpcode<0x75, Instr<LD, ToAddr<Register<HL>>, Register<L>>>();
    RegisterOpcode<0x76, Instr<HALT, void, void>>();
    RegisterOpcode<0x77, Instr<LD, ToAddr<Register<HL>>, Register<A>>>();
    RegisterOpcode<0x78, Instr<LD, Register<A>, Register<B>>>();
    RegisterOpcode<0x79, Instr<LD, Register<A>, Register<C>>>();
    RegisterOpcode<0x7a, Instr<LD, Register<A>, Register<D>>>();
    RegisterOpcode<0x7b, Instr<LD, Register<A>, Register<E>>>();
    RegisterOpcode<0x7c, Instr<LD, Register<A>, Register<H>>>();
    RegisterOpcode<0x7d, Instr<LD, Register<A>, Register<L>>>();
    RegisterOpcode<0x7e, Instr<LD, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x7f, Instr<LD, Register<A>, Register<A>>>();
    RegisterOpcode<0x80, Instr<ADD, Register<A>, Register<B>>>();
    RegisterOpcode<0x81, Instr<ADD, Register<A>, Register<C>>>();
    RegisterOpcode<0x82, Instr<ADD, Register<A>, Register<D>>>();
    RegisterOpcode<0x83, Instr<ADD, Register<A>, Register<E>>>();
    RegisterOpcode<0x84, Instr<ADD, Register<A>, Register<H>>>();
    RegisterOpcode<0x85, Instr<ADD, Register<A>, Register<L>>>();
    RegisterOpcode<0x86, Instr<ADD, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x87, Instr<ADD, Register<A>, Register<A>>>();
    RegisterOpcode<0x88, Instr<ADC, Register<A>, Register<B>>>();
    RegisterOpcode<0x89, Instr<ADC, Register<A>, Register<C>>>();
    RegisterOpcode<0x8A, Instr<ADC, Register<A>, Register<D>>>();
    RegisterOpcode<0x8B, Instr<ADC, Register<A>, Register<E>>>();
    RegisterOpcode<0x8C, Instr<ADC, Register<A>, Register<H>>>();
    RegisterOpcode<0x8D, Instr<ADC, Register<A>, Register<L>>>();
    RegisterOpcode<0x8E, Instr<ADC, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x8F, Instr<ADC, Register<A>, Register<A>>>();
    RegisterOpcode<0x90, Instr<SUB, Register<A>, Register<B>>>();
    RegisterOpcode<0x91, Instr<SUB, Register<A>, Register<C>>>();
    RegisterOpcode<0x92, Instr<SUB, Register<A>, Register<D>>>();
    RegisterOpcode<0x93, Instr<SUB, Register<A>, Register<E>>>();
    RegisterOpcode<0x94, Instr<SUB, Register<A>, Register<H>>>();
    RegisterOpcode<0x95, Instr<SUB, Register<A>, Register<L>>>();
    RegisterOpcode<0x96, Instr<SUB, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x97, Instr<SUB, Register<A>, Register<A>>>();
    RegisterOpcode<0x98, Instr<SBC, Register<A>, Register<B>>>();
    RegisterOpcode<0x99, Instr<SBC, Register<A>, Register<C>>>();
    RegisterOpcode<0x9A, Instr<SBC, Register<A>, Register<D>>>();
    RegisterOpcode<0x9B, Instr<SBC, Register<A>, Register<E>>>();
    RegisterOpcode<0x9C, Instr<SBC, Register<A>, Register<H>>>();
    RegisterOpcode<0x9D, Instr<SBC, Register<A>, Register<L>>>();
    RegisterOpcode<0x9E, Instr<SBC, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x9F, Instr<SBC, Register<A>, Register<A>>>();
    RegisterOpcode<0xA0, Instr<AND, Register<A>, Register<B>>>();
    RegisterOpcode<0xA1, Instr<AND, Register<A>, Register<C>>>();
    RegisterOpcode<0xA2, Instr<AND, Register<A>, Register<D>>>();
    RegisterOpcode<0xA3, Instr<AND, Register<A>, Register<E>>>();
    RegisterOpcode<0xA4, Instr<AND, Register<A>, Register<H>>>();
    RegisterOpcode<0xA5, Instr<AND, Register<A>, Register<L>>>();
    RegisterOpcode<0xA6, Instr<AND, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0xA7, Instr<AND, Register<A>, Register<A>>>();
    RegisterOpcode<0xA8, Instr<XOR, Register<A>, Register<B>>>();
    RegisterOpcode<0xA9, Instr<XOR, Register<A>, Register<C>>>();
    RegisterOpcode<0xAA, Instr<XOR, Register<A>, Register<D>>>();
    RegisterOpcode<0xAB, Instr<XOR, Register<A>, Register<E>>>();
    RegisterOpcode<0xAC, Instr<XOR, Register<A>, Register<H>>>();
    RegisterOpcode<0xAD, Instr<XOR, Register<A>, Register<L>>>();
    RegisterOpcode<0xAE, Instr<XOR, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0xAF, Instr<XOR, Register<A>, Register<A>>>();
    RegisterOpcode<0xB0, Instr<OR, Register<A>, Register<B>>>();
    RegisterOpcode<0xB1, Instr<OR, Register<A>, Register<C>>>();
    RegisterOpcode<0xB2, Instr<OR, Register<A>, Register<D>>>();
    RegisterOpcode<0xB3, Instr<OR, Register<A>, Register<E>>>();
    RegisterOpcode<0xB4, Instr<OR, Register<A>, Register<H>>>();
    RegisterOpcode<0xB5, Instr<OR, Register<A>, Register<L>>>();
    RegisterOpcode<0xB6, Instr<OR, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0xB7, Instr<OR, Register<A>, Register<A>>>();
    RegisterOpcode<0xB8, Instr<CP, Register<A>, Register<B>>>();
    RegisterOpcode<0xB9, Instr<CP, Register<A>, Register<C>>>();
    RegisterOpcode<0xBA, Instr<CP, Register<A>, Register<D>>>();
    RegisterOpcode<0xBB, Instr<CP, Register<A>, Register<E>>>();
    RegisterOpcode<0xBC, Instr<CP, Register<A>, Register<H>>>();
    RegisterOpcode<0xBD, Instr<CP, Register<A>, Register<L>>>();
    RegisterOpcode<0xBE, Instr<CP, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0xBF, Instr<CP, Register<A>, Register<A>>>();
    RegisterOpcode<0xC0, Instr<RETNZ, void, void>>();
    RegisterOpcode<0xC1, Instr<POP, Register<BC>, void>>();
    RegisterOpcode<0xC2, Instr<JPNZ, NextWord, void>>();
    RegisterOpcode<0xC3, Instr<JP, NextWord, void>>();
    RegisterOpcode<0xC4, Instr<CALLNZ, NextWord, void>>();
    RegisterOpcode<0xC5, Instr<PUSH, Register<BC>, void>>();
    RegisterOpcode<0xC6, Instr<ADD, Register<A>, NextByte>>();
    RegisterOpcode<0xC7, Instr<RST0, void, void>>();
    RegisterOpcode<0xC8, Instr<RETZ, void, void>>();
    RegisterOpcode<0xC9, Instr<RET, void, void>>();
    RegisterOpcode<0xCA, Instr<JPZ, NextWord, void>>();
    RegisterOpcode<0xCB, Instr<Nop, void, void>>(); // FIXME extended
    RegisterOpcode<0xCC, Instr<CALLZ, NextWord, void>>();
    RegisterOpcode<0xCD, Instr<CALL, NextWord, void>>();
    RegisterOpcode<0xCE, Instr<ADC, Register<A>, NextByte>>();
    RegisterOpcode<0xCF, Instr<RST8, void, void>>();
    RegisterOpcode<0xD0, Instr<RETNC, void, void>>();
    RegisterOpcode<0xD1, Instr<POP, Register<DE>, void>>();
    RegisterOpcode<0xD2, Instr<JPNC, NextWord, void>>();
    // 0xD3 xxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xD4, Instr<CALLNC, NextWord, void>>();
    RegisterOpcode<0xD5, Instr<PUSH, Register<DE>, void>>();
    RegisterOpcode<0xD6, Instr<SUB, Register<A>, NextByte>>();
    RegisterOpcode<0xD7, Instr<RST10, void, void>>();
    RegisterOpcode<0xD8, Instr<RETC, void, void>>();
    RegisterOpcode<0xD9, Instr<RETI, void, void>>();
    RegisterOpcode<0xDA, Instr<JPC, NextWord, void>>();
    // 0xDB xxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xDC, Instr<CALLC, NextWord, void>>();
    // 0xDD xxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xDE, Instr<SBC, Register<A>, NextByte>>();
    RegisterOpcode<0xDF, Instr<RST18, void, void>>();
    RegisterOpcode<0xE0, Instr<LDH, ToAddrFF00<NextByte>, Register<A>>>();
    RegisterOpcode<0xE1, Instr<POP, Register<HL>, void>>();
    RegisterOpcode<0xE2, Instr<LDH, ToAddrFF00<Register<C>>, Register<A>>>();
    // 0xE3 xxxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xE4 xxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xE5, Instr<PUSH, Register<HL>, void>>();
    RegisterOpcode<0xE6, Instr<AND, Register<A>, NextByte>>();
    RegisterOpcode<0xE7, Instr<RST20, void, void>>();
    RegisterOpcode<0xE8, Instr<ADD, Register<SP>, NextByte>>(); // FIXME
    RegisterOpcode<0xE9, Instr<JP, ToAddr<Register<HL>>, void>>();
    RegisterOpcode<0xEA, Instr<LD, ToAddr<NextWord>, Register<A>>>();
    // 0xEB xxxxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xEC xxxxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xED xxxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xEE, Instr<XOR, Register<A>, NextByte>>();
    RegisterOpcode<0xEF, Instr<RST28, void, void>>();
    RegisterOpcode<0xF0, Instr<LDH, Register<A>, ToAddrFF00<NextByte>>>();
    RegisterOpcode<0xF1, Instr<POP, Register<AF>, void>>();
    // 0xF2 xxxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xF3, Instr<DI, void, void>>();
    // 0xF4 xxxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xF5, Instr<PUSH, Register<AF>, void>>();
    RegisterOpcode<0xF6, Instr<OR, Register<A>, NextByte>>();
    RegisterOpcode<0xF7, Instr<RST30, void, void>>();
    RegisterOpcode<0xF8, Instr<LDHL, Register<SP>, NextByte>>();
    RegisterOpcode<0xF9, Instr<LD, Register<SP>, Register<HL>>>();
    RegisterOpcode<0xFA, Instr<LD, Register<A>, ToAddr<NextWord>>>();
    RegisterOpcode<0xFB, Instr<EI, void, void>>();
    // 0xFC xxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xFD xxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xFE, Instr<CP, Register<A>, NextByte>>();
    RegisterOpcode<0xFF, Instr<RST38, void, void>>();
}

void Z80::Process()
{
#if 1
    while (true)
    {
        int c = _pc;
        std::cout << "0x"<< std::hex << _pc << "\t" << static_cast<unsigned int>(_addr.Get(_pc)) << "\t";
        _print[_addr.Get(_pc)](c, _addr);
        _instr[_addr.Get(_pc)](this);
        ++_pc;
        //std::cin.get();
        _vid.Clock();
    }
#else
    for (int i = 0 ; i < 65535 ; ++i)
    {
        try
        {
        std::cout <<std::hex << "0x" << i << "/" <<std::dec<< i << "\t" << static_cast<unsigned int>(_addr.Get(i)) << "\t";
        _print[_addr.Get(i)](i, _addr);
        } catch (std::exception) { }
    }
#endif
}

    template <unsigned char Opcode, class Inst>
void Z80::RegisterOpcode()
{
    _instr[Opcode] = std::function<void(Z80*)>(&Inst::Do);
    _print[Opcode] = std::function<void(int&, AddressBus&)>(&Inst::Print);
}

void Z80::set_interrupts(byte enable)
{
    _interrupts = enable;
}
