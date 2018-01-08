#include "z80.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <vector>

#include "instruction.hpp"
#include "link.h"
#include "timer.h"
#include "video.h"

std::array<std::unique_ptr<Z80::InstrBase>, 256> Z80::_instr;
std::array<std::unique_ptr<Z80::InstrBase>, 256> Z80::_cb_instr;

Z80::Z80(AddressBus& addr, Video& v, LinkCable& lk, Timer& timer, Sound& snd)
    : _sp(uint16_t(0xFFFE)),
      _pc(uint16_t(0x100)),
      _addr(addr),
      _lk(lk),
      _vid(v),
      _snd(snd),
      _timer(timer),
      _interrupts(uint8_t(0xFF)),
      _halted(false),
      _power(true) {
    Register<F>::Set(this, uint8_t(0xB0));
    Register<A>::Set(this, uint8_t(0x01));
    Register<C>::Set(this, uint8_t(0x13));
    Register<B>::Set(this, uint8_t(0x00));
    Register<E>::Set(this, uint8_t(0xD8));
    Register<D>::Set(this, uint8_t(0x00));
    Register<L>::Set(this, uint8_t(0x4D));
    Register<H>::Set(this, uint8_t(0x01));

    _addr.Set(0xFF05, uint8_t(0x00));  // TIMA
    _addr.Set(0xFF06, uint8_t(0X00));  // TMA
    _addr.Set(0xFF07, uint8_t(0x00));  // TAC
    _addr.Set(0xFF10, uint8_t(0x80));  // NR10
    _addr.Set(0xFF11, uint8_t(0xBF));  // NR11
    _addr.Set(0xFF12, uint8_t(0xF3));  // NR12
    _addr.Set(0xFF14, uint8_t(0xBF));  // NR14
    _addr.Set(0xFF16, uint8_t(0x3F));  // NR21
    _addr.Set(0xFF17, uint8_t(0x00));  // NR22
    _addr.Set(0xFF19, uint8_t(0xBF));  // NR24
    _addr.Set(0xFF1A, uint8_t(0x7F));  // NR30
    _addr.Set(0xFF1B, uint8_t(0xFF));  // NR31
    _addr.Set(0xFF1C, uint8_t(0x9F));  // NR32
    _addr.Set(0xFF1E, uint8_t(0xBF));  // NR33
    _addr.Set(0xFF20, uint8_t(0xFF));  // NR41
    _addr.Set(0xFF21, uint8_t(0x00));  // NR42
    _addr.Set(0xFF22, uint8_t(0x00));  // NR43
    _addr.Set(0xFF23, uint8_t(0xBF));  // NR30
    _addr.Set(0xFF24, uint8_t(0x77));  // NR50
    _addr.Set(0xFF25, uint8_t(0xF3));  // NR51
    _addr.Set(0xFF26, uint8_t(0xF1));  // NR52
    _addr.Set(0xFF40, uint8_t(0x91));  // LCDC
    _addr.Set(0xFF42, uint8_t(0x00));  // SCY
    _addr.Set(0xFF43, uint8_t(0x00));  // SCX
    _addr.Set(0xFF45, uint8_t(0x00));  // LYC
    _addr.Set(0xFF47, uint8_t(0xFC));  // BGP
    _addr.Set(0xFF48, uint8_t(0xFF));  // OBP0
    _addr.Set(0xFF49, uint8_t(0xFF));  // OBP1
    _addr.Set(0xFF4A, uint8_t(0x00));  // WY
    _addr.Set(0xFF4B, uint8_t(0x00));  // WX
    _addr.Set(0xFFFF, uint8_t(0x00));  // IE

    RegisterOpcode<0x00, Instr<NOP, void, void>>();
    RegisterOpcode<0x01, Instr<LDw, Register<BC>, NextWord>>();
    RegisterOpcode<0x02, Instr<LD, ToAddr<Register<BC>>, Register<A>>>();
    RegisterOpcode<0x03, Instr<INCw, Register<BC>, void>>();
    RegisterOpcode<0x04, Instr<INC, Register<B>, void>>();
    RegisterOpcode<0x05, Instr<DEC, Register<B>, void>>();
    RegisterOpcode<0x06, Instr<LD, Register<B>, NextByte>>();
    RegisterOpcode<0x07, Instr<RLCA, void, void>>();
    RegisterOpcode<0x08, Instr<LDw, ToAddr<NextWord>, Register<SP>>>();
    RegisterOpcode<0x09, Instr<ADDw, Register<HL>, Register<BC>>>();
    RegisterOpcode<0x0A, Instr<LD, Register<A>, ToAddr<Register<BC>>>>();
    RegisterOpcode<0x0B, Instr<DECw, Register<BC>, void>>();
    RegisterOpcode<0x0C, Instr<INC, Register<C>, void>>();
    RegisterOpcode<0x0D, Instr<DEC, Register<C>, void>>();
    RegisterOpcode<0x0E, Instr<LD, Register<C>, NextByte>>();
    RegisterOpcode<0x0F, Instr<RRCA, void, void>>();
    RegisterOpcode<0x10, Instr<STOP, void, void>>();
    RegisterOpcode<0x11, Instr<LDw, Register<DE>, NextWord>>();
    RegisterOpcode<0x12, Instr<LD, ToAddr<Register<DE>>, Register<A>>>();
    RegisterOpcode<0x13, Instr<INCw, Register<DE>, void>>();
    RegisterOpcode<0x14, Instr<INC, Register<D>, void>>();
    RegisterOpcode<0x15, Instr<DEC, Register<D>, void>>();
    RegisterOpcode<0x16, Instr<LD, Register<D>, NextByte>>();
    RegisterOpcode<0x17, Instr<RLA, void, void>>();
    RegisterOpcode<0x18, Instr<JR, NextByte, void>>();
    RegisterOpcode<0x19, Instr<ADDw, Register<HL>, Register<DE>>>();
    RegisterOpcode<0x1A, Instr<LD, Register<A>, ToAddr<Register<DE>>>>();
    RegisterOpcode<0x1B, Instr<DECw, Register<DE>, void>>();
    RegisterOpcode<0x1C, Instr<INC, Register<E>, void>>();
    RegisterOpcode<0x1D, Instr<DEC, Register<E>, void>>();
    RegisterOpcode<0x1E, Instr<LD, Register<E>, NextByte>>();
    RegisterOpcode<0x1F, Instr<RRA, void, void>>();
    RegisterOpcode<0x20, Instr<JRNZ, NextByte, void>>();
    RegisterOpcode<0x21, Instr<LDw, Register<HL>, NextWord>>();
    RegisterOpcode<0x22, Instr<LDI, ToAddr<Register<HL>>, Register<A>>>();
    RegisterOpcode<0x23, Instr<INCw, Register<HL>, void>>();
    RegisterOpcode<0x24, Instr<INC, Register<H>, void>>();
    RegisterOpcode<0x25, Instr<DEC, Register<H>, void>>();
    RegisterOpcode<0x26, Instr<LD, Register<H>, NextByte>>();
    RegisterOpcode<0x27, Instr<DAA, void, void>>();
    RegisterOpcode<0x28, Instr<JRZ, NextByte, void>>();
    RegisterOpcode<0x29, Instr<ADDw, Register<HL>, Register<HL>>>();
    RegisterOpcode<0x2A, Instr<LDI, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x2B, Instr<DECw, Register<HL>, void>>();
    RegisterOpcode<0x2C, Instr<INC, Register<L>, void>>();
    RegisterOpcode<0x2D, Instr<DEC, Register<L>, void>>();
    RegisterOpcode<0x2E, Instr<LD, Register<L>, NextByte>>();
    RegisterOpcode<0x2F, Instr<CPL, Register<A>, void>>();
    RegisterOpcode<0x30, Instr<JRNC, NextByte, void>>();
    RegisterOpcode<0x31, Instr<LDw, Register<SP>, NextWord>>();
    RegisterOpcode<0x32, Instr<LDD, ToAddr<Register<HL>>, Register<A>>>();
    RegisterOpcode<0x33, Instr<INCw, Register<SP>, void>>();
    RegisterOpcode<0x34, Instr<INC, ToAddr<Register<HL>>, void>>();
    RegisterOpcode<0x35, Instr<DEC, ToAddr<Register<HL>>, void>>();
    RegisterOpcode<0x36, Instr<LD, ToAddr<Register<HL>>, NextByte>>();
    RegisterOpcode<0x37, Instr<SCF, void, void>>();
    RegisterOpcode<0x38, Instr<JRC, NextByte, void>>();
    RegisterOpcode<0x39, Instr<ADDw, Register<HL>, Register<SP>>>();
    RegisterOpcode<0x3A, Instr<LDD, Register<A>, ToAddr<Register<HL>>>>();
    RegisterOpcode<0x3B, Instr<DECw, Register<SP>, void>>();
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
    RegisterOpcode<0x4B, Instr<LD, Register<C>, Register<E>>>();
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
    RegisterOpcode<0xCB, Instr<EXTENDED, void, void>>();
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
    RegisterOpcode<0xE0, Instr<LD, ToAddrFF00<NextByte>, Register<A>>>();
    RegisterOpcode<0xE1, Instr<POP, Register<HL>, void>>();
    RegisterOpcode<0xE2, Instr<LD, ToAddrFF00<Register<C>>, Register<A>>>();
    // 0xE3 xxxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xE4 xxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xE5, Instr<PUSH, Register<HL>, void>>();
    RegisterOpcode<0xE6, Instr<AND, Register<A>, NextByte>>();
    RegisterOpcode<0xE7, Instr<RST20, void, void>>();
    RegisterOpcode<0xE8, Instr<ADDO, Register<SP>, NextByte>>();
    RegisterOpcode<0xE9, Instr<JP, Register<HL>, void>>();
    RegisterOpcode<0xEA, Instr<LD, ToAddr<NextWord>, Register<A>>>();
    // 0xEB xxxxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xEC xxxxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xED xxxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xEE, Instr<XOR, Register<A>, NextByte>>();
    RegisterOpcode<0xEF, Instr<RST28, void, void>>();
    RegisterOpcode<0xF0, Instr<LD, Register<A>, ToAddrFF00<NextByte>>>();
    RegisterOpcode<0xF1, Instr<POP, Register<AF>, void>>();
    RegisterOpcode<0xF2, Instr<LD, Register<A>, ToAddrFF00<Register<C>>>>();
    RegisterOpcode<0xF3, Instr<DI, void, void>>();
    // 0xF4 xxxxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xF5, Instr<PUSH, Register<AF>, void>>();
    RegisterOpcode<0xF6, Instr<OR, Register<A>, NextByte>>();
    RegisterOpcode<0xF7, Instr<RST30, void, void>>();
    RegisterOpcode<0xF8, Instr<LDHLSPN, void, void>>();
    RegisterOpcode<0xF9, Instr<LDw, Register<SP>, Register<HL>>>();
    RegisterOpcode<0xFA, Instr<LD, Register<A>, ToAddr<NextWord>>>();
    RegisterOpcode<0xFB, Instr<EI, void, void>>();
    // 0xFC xxxxxxxxxxxxxxxxxxxxxxxxx
    // 0xFD xxxxxxxxxxxxxxxxxxxxxxxxx
    RegisterOpcode<0xFE, Instr<CP, Register<A>, NextByte>>();
    RegisterOpcode<0xFF, Instr<RST38, void, void>>();

    RegisterCBOpcode<0x00, Instr<RLC, Register<B>, void>>();
    RegisterCBOpcode<0x01, Instr<RLC, Register<C>, void>>();
    RegisterCBOpcode<0x02, Instr<RLC, Register<D>, void>>();
    RegisterCBOpcode<0x03, Instr<RLC, Register<E>, void>>();
    RegisterCBOpcode<0x04, Instr<RLC, Register<H>, void>>();
    RegisterCBOpcode<0x05, Instr<RLC, Register<L>, void>>();
    RegisterCBOpcode<0x06, Instr<RLC, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x07, Instr<RLC, Register<A>, void>>();
    RegisterCBOpcode<0x08, Instr<RRC, Register<B>, void>>();
    RegisterCBOpcode<0x09, Instr<RRC, Register<C>, void>>();
    RegisterCBOpcode<0x0A, Instr<RRC, Register<D>, void>>();
    RegisterCBOpcode<0x0B, Instr<RRC, Register<E>, void>>();
    RegisterCBOpcode<0x0C, Instr<RRC, Register<H>, void>>();
    RegisterCBOpcode<0x0D, Instr<RRC, Register<L>, void>>();
    RegisterCBOpcode<0x0E, Instr<RRC, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x0F, Instr<RRC, Register<A>, void>>();
    RegisterCBOpcode<0x10, Instr<RL, Register<B>, void>>();
    RegisterCBOpcode<0x11, Instr<RL, Register<C>, void>>();
    RegisterCBOpcode<0x12, Instr<RL, Register<D>, void>>();
    RegisterCBOpcode<0x13, Instr<RL, Register<E>, void>>();
    RegisterCBOpcode<0x14, Instr<RL, Register<H>, void>>();
    RegisterCBOpcode<0x15, Instr<RL, Register<L>, void>>();
    RegisterCBOpcode<0x16, Instr<RL, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x17, Instr<RL, Register<A>, void>>();
    RegisterCBOpcode<0x18, Instr<RR, Register<B>, void>>();
    RegisterCBOpcode<0x19, Instr<RR, Register<C>, void>>();
    RegisterCBOpcode<0x1A, Instr<RR, Register<D>, void>>();
    RegisterCBOpcode<0x1B, Instr<RR, Register<E>, void>>();
    RegisterCBOpcode<0x1C, Instr<RR, Register<H>, void>>();
    RegisterCBOpcode<0x1D, Instr<RR, Register<L>, void>>();
    RegisterCBOpcode<0x1E, Instr<RR, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x1F, Instr<RR, Register<A>, void>>();
    RegisterCBOpcode<0x20, Instr<SLA, Register<B>, void>>();
    RegisterCBOpcode<0x21, Instr<SLA, Register<C>, void>>();
    RegisterCBOpcode<0x22, Instr<SLA, Register<D>, void>>();
    RegisterCBOpcode<0x23, Instr<SLA, Register<E>, void>>();
    RegisterCBOpcode<0x24, Instr<SLA, Register<H>, void>>();
    RegisterCBOpcode<0x25, Instr<SLA, Register<L>, void>>();
    RegisterCBOpcode<0x26, Instr<SLA, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x27, Instr<SLA, Register<A>, void>>();
    RegisterCBOpcode<0x28, Instr<SRA, Register<B>, void>>();
    RegisterCBOpcode<0x29, Instr<SRA, Register<C>, void>>();
    RegisterCBOpcode<0x2A, Instr<SRA, Register<D>, void>>();
    RegisterCBOpcode<0x2B, Instr<SRA, Register<E>, void>>();
    RegisterCBOpcode<0x2C, Instr<SRA, Register<H>, void>>();
    RegisterCBOpcode<0x2D, Instr<SRA, Register<L>, void>>();
    RegisterCBOpcode<0x2E, Instr<SRA, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x2F, Instr<SRA, Register<A>, void>>();
    RegisterCBOpcode<0x30, Instr<SWAP, Register<B>, void>>();
    RegisterCBOpcode<0x31, Instr<SWAP, Register<C>, void>>();
    RegisterCBOpcode<0x32, Instr<SWAP, Register<D>, void>>();
    RegisterCBOpcode<0x33, Instr<SWAP, Register<E>, void>>();
    RegisterCBOpcode<0x34, Instr<SWAP, Register<H>, void>>();
    RegisterCBOpcode<0x35, Instr<SWAP, Register<L>, void>>();
    RegisterCBOpcode<0x36, Instr<SWAP, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x37, Instr<SWAP, Register<A>, void>>();
    RegisterCBOpcode<0x38, Instr<SRL, Register<B>, void>>();
    RegisterCBOpcode<0x39, Instr<SRL, Register<C>, void>>();
    RegisterCBOpcode<0x3A, Instr<SRL, Register<D>, void>>();
    RegisterCBOpcode<0x3B, Instr<SRL, Register<E>, void>>();
    RegisterCBOpcode<0x3C, Instr<SRL, Register<H>, void>>();
    RegisterCBOpcode<0x3D, Instr<SRL, Register<L>, void>>();
    RegisterCBOpcode<0x3E, Instr<SRL, ToAddr<Register<HL>>, void>>();
    RegisterCBOpcode<0x3F, Instr<SRL, Register<A>, void>>();
    RegisterCBOpcode<0x40, Instr<BIT, I<0>, Register<B>>>();
    RegisterCBOpcode<0x41, Instr<BIT, I<0>, Register<C>>>();
    RegisterCBOpcode<0x42, Instr<BIT, I<0>, Register<D>>>();
    RegisterCBOpcode<0x43, Instr<BIT, I<0>, Register<E>>>();
    RegisterCBOpcode<0x44, Instr<BIT, I<0>, Register<H>>>();
    RegisterCBOpcode<0x45, Instr<BIT, I<0>, Register<L>>>();
    RegisterCBOpcode<0x46, Instr<BIT, I<0>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x47, Instr<BIT, I<0>, Register<A>>>();
    RegisterCBOpcode<0x48, Instr<BIT, I<1>, Register<B>>>();
    RegisterCBOpcode<0x49, Instr<BIT, I<1>, Register<C>>>();
    RegisterCBOpcode<0x4A, Instr<BIT, I<1>, Register<D>>>();
    RegisterCBOpcode<0x4B, Instr<BIT, I<1>, Register<E>>>();
    RegisterCBOpcode<0x4C, Instr<BIT, I<1>, Register<H>>>();
    RegisterCBOpcode<0x4D, Instr<BIT, I<1>, Register<L>>>();
    RegisterCBOpcode<0x4E, Instr<BIT, I<1>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x4F, Instr<BIT, I<1>, Register<A>>>();
    RegisterCBOpcode<0x50, Instr<BIT, I<2>, Register<B>>>();
    RegisterCBOpcode<0x51, Instr<BIT, I<2>, Register<C>>>();
    RegisterCBOpcode<0x52, Instr<BIT, I<2>, Register<D>>>();
    RegisterCBOpcode<0x53, Instr<BIT, I<2>, Register<E>>>();
    RegisterCBOpcode<0x54, Instr<BIT, I<2>, Register<H>>>();
    RegisterCBOpcode<0x55, Instr<BIT, I<2>, Register<L>>>();
    RegisterCBOpcode<0x56, Instr<BIT, I<2>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x57, Instr<BIT, I<2>, Register<A>>>();
    RegisterCBOpcode<0x58, Instr<BIT, I<3>, Register<B>>>();
    RegisterCBOpcode<0x59, Instr<BIT, I<3>, Register<C>>>();
    RegisterCBOpcode<0x5A, Instr<BIT, I<3>, Register<D>>>();
    RegisterCBOpcode<0x5B, Instr<BIT, I<3>, Register<E>>>();
    RegisterCBOpcode<0x5C, Instr<BIT, I<3>, Register<H>>>();
    RegisterCBOpcode<0x5D, Instr<BIT, I<3>, Register<L>>>();
    RegisterCBOpcode<0x5E, Instr<BIT, I<3>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x5F, Instr<BIT, I<3>, Register<A>>>();
    RegisterCBOpcode<0x60, Instr<BIT, I<4>, Register<B>>>();
    RegisterCBOpcode<0x61, Instr<BIT, I<4>, Register<C>>>();
    RegisterCBOpcode<0x62, Instr<BIT, I<4>, Register<D>>>();
    RegisterCBOpcode<0x63, Instr<BIT, I<4>, Register<E>>>();
    RegisterCBOpcode<0x64, Instr<BIT, I<4>, Register<H>>>();
    RegisterCBOpcode<0x65, Instr<BIT, I<4>, Register<L>>>();
    RegisterCBOpcode<0x66, Instr<BIT, I<4>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x67, Instr<BIT, I<4>, Register<A>>>();
    RegisterCBOpcode<0x68, Instr<BIT, I<5>, Register<B>>>();
    RegisterCBOpcode<0x69, Instr<BIT, I<5>, Register<C>>>();
    RegisterCBOpcode<0x6A, Instr<BIT, I<5>, Register<D>>>();
    RegisterCBOpcode<0x6B, Instr<BIT, I<5>, Register<E>>>();
    RegisterCBOpcode<0x6C, Instr<BIT, I<5>, Register<H>>>();
    RegisterCBOpcode<0x6D, Instr<BIT, I<5>, Register<L>>>();
    RegisterCBOpcode<0x6E, Instr<BIT, I<5>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x6F, Instr<BIT, I<5>, Register<A>>>();
    RegisterCBOpcode<0x70, Instr<BIT, I<6>, Register<B>>>();
    RegisterCBOpcode<0x71, Instr<BIT, I<6>, Register<C>>>();
    RegisterCBOpcode<0x72, Instr<BIT, I<6>, Register<D>>>();
    RegisterCBOpcode<0x73, Instr<BIT, I<6>, Register<E>>>();
    RegisterCBOpcode<0x74, Instr<BIT, I<6>, Register<H>>>();
    RegisterCBOpcode<0x75, Instr<BIT, I<6>, Register<L>>>();
    RegisterCBOpcode<0x76, Instr<BIT, I<6>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x77, Instr<BIT, I<6>, Register<A>>>();
    RegisterCBOpcode<0x78, Instr<BIT, I<7>, Register<B>>>();
    RegisterCBOpcode<0x79, Instr<BIT, I<7>, Register<C>>>();
    RegisterCBOpcode<0x7A, Instr<BIT, I<7>, Register<D>>>();
    RegisterCBOpcode<0x7B, Instr<BIT, I<7>, Register<E>>>();
    RegisterCBOpcode<0x7C, Instr<BIT, I<7>, Register<H>>>();
    RegisterCBOpcode<0x7D, Instr<BIT, I<7>, Register<L>>>();
    RegisterCBOpcode<0x7E, Instr<BIT, I<7>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x7F, Instr<BIT, I<7>, Register<A>>>();
    RegisterCBOpcode<0x80, Instr<RES, I<0>, Register<B>>>();
    RegisterCBOpcode<0x81, Instr<RES, I<0>, Register<C>>>();
    RegisterCBOpcode<0x82, Instr<RES, I<0>, Register<D>>>();
    RegisterCBOpcode<0x83, Instr<RES, I<0>, Register<E>>>();
    RegisterCBOpcode<0x84, Instr<RES, I<0>, Register<H>>>();
    RegisterCBOpcode<0x85, Instr<RES, I<0>, Register<L>>>();
    RegisterCBOpcode<0x86, Instr<RES, I<0>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x87, Instr<RES, I<0>, Register<A>>>();
    RegisterCBOpcode<0x88, Instr<RES, I<1>, Register<B>>>();
    RegisterCBOpcode<0x89, Instr<RES, I<1>, Register<C>>>();
    RegisterCBOpcode<0x8A, Instr<RES, I<1>, Register<D>>>();
    RegisterCBOpcode<0x8B, Instr<RES, I<1>, Register<E>>>();
    RegisterCBOpcode<0x8C, Instr<RES, I<1>, Register<H>>>();
    RegisterCBOpcode<0x8D, Instr<RES, I<1>, Register<L>>>();
    RegisterCBOpcode<0x8E, Instr<RES, I<1>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x8F, Instr<RES, I<1>, Register<A>>>();
    RegisterCBOpcode<0x90, Instr<RES, I<2>, Register<B>>>();
    RegisterCBOpcode<0x91, Instr<RES, I<2>, Register<C>>>();
    RegisterCBOpcode<0x92, Instr<RES, I<2>, Register<D>>>();
    RegisterCBOpcode<0x93, Instr<RES, I<2>, Register<E>>>();
    RegisterCBOpcode<0x94, Instr<RES, I<2>, Register<H>>>();
    RegisterCBOpcode<0x95, Instr<RES, I<2>, Register<L>>>();
    RegisterCBOpcode<0x96, Instr<RES, I<2>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x97, Instr<RES, I<2>, Register<A>>>();
    RegisterCBOpcode<0x98, Instr<RES, I<3>, Register<B>>>();
    RegisterCBOpcode<0x99, Instr<RES, I<3>, Register<C>>>();
    RegisterCBOpcode<0x9A, Instr<RES, I<3>, Register<D>>>();
    RegisterCBOpcode<0x9B, Instr<RES, I<3>, Register<E>>>();
    RegisterCBOpcode<0x9C, Instr<RES, I<3>, Register<H>>>();
    RegisterCBOpcode<0x9D, Instr<RES, I<3>, Register<L>>>();
    RegisterCBOpcode<0x9E, Instr<RES, I<3>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0x9F, Instr<RES, I<3>, Register<A>>>();
    RegisterCBOpcode<0xA0, Instr<RES, I<4>, Register<B>>>();
    RegisterCBOpcode<0xA1, Instr<RES, I<4>, Register<C>>>();
    RegisterCBOpcode<0xA2, Instr<RES, I<4>, Register<D>>>();
    RegisterCBOpcode<0xA3, Instr<RES, I<4>, Register<E>>>();
    RegisterCBOpcode<0xA4, Instr<RES, I<4>, Register<H>>>();
    RegisterCBOpcode<0xA5, Instr<RES, I<4>, Register<L>>>();
    RegisterCBOpcode<0xA6, Instr<RES, I<4>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xA7, Instr<RES, I<4>, Register<A>>>();
    RegisterCBOpcode<0xA8, Instr<RES, I<5>, Register<B>>>();
    RegisterCBOpcode<0xA9, Instr<RES, I<5>, Register<C>>>();
    RegisterCBOpcode<0xAA, Instr<RES, I<5>, Register<D>>>();
    RegisterCBOpcode<0xAB, Instr<RES, I<5>, Register<E>>>();
    RegisterCBOpcode<0xAC, Instr<RES, I<5>, Register<H>>>();
    RegisterCBOpcode<0xAD, Instr<RES, I<5>, Register<L>>>();
    RegisterCBOpcode<0xAE, Instr<RES, I<5>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xAF, Instr<RES, I<5>, Register<A>>>();
    RegisterCBOpcode<0xB0, Instr<RES, I<6>, Register<B>>>();
    RegisterCBOpcode<0xB1, Instr<RES, I<6>, Register<C>>>();
    RegisterCBOpcode<0xB2, Instr<RES, I<6>, Register<D>>>();
    RegisterCBOpcode<0xB3, Instr<RES, I<6>, Register<E>>>();
    RegisterCBOpcode<0xB4, Instr<RES, I<6>, Register<H>>>();
    RegisterCBOpcode<0xB5, Instr<RES, I<6>, Register<L>>>();
    RegisterCBOpcode<0xB6, Instr<RES, I<6>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xB7, Instr<RES, I<6>, Register<A>>>();
    RegisterCBOpcode<0xB8, Instr<RES, I<7>, Register<B>>>();
    RegisterCBOpcode<0xB9, Instr<RES, I<7>, Register<C>>>();
    RegisterCBOpcode<0xBA, Instr<RES, I<7>, Register<D>>>();
    RegisterCBOpcode<0xBB, Instr<RES, I<7>, Register<E>>>();
    RegisterCBOpcode<0xBC, Instr<RES, I<7>, Register<H>>>();
    RegisterCBOpcode<0xBD, Instr<RES, I<7>, Register<L>>>();
    RegisterCBOpcode<0xBE, Instr<RES, I<7>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xBF, Instr<RES, I<7>, Register<A>>>();

    RegisterCBOpcode<0xC0, Instr<SET, I<0>, Register<B>>>();
    RegisterCBOpcode<0xC1, Instr<SET, I<0>, Register<C>>>();
    RegisterCBOpcode<0xC2, Instr<SET, I<0>, Register<D>>>();
    RegisterCBOpcode<0xC3, Instr<SET, I<0>, Register<E>>>();
    RegisterCBOpcode<0xC4, Instr<SET, I<0>, Register<H>>>();
    RegisterCBOpcode<0xC5, Instr<SET, I<0>, Register<L>>>();
    RegisterCBOpcode<0xC6, Instr<SET, I<0>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xC7, Instr<SET, I<0>, Register<A>>>();
    RegisterCBOpcode<0xC8, Instr<SET, I<1>, Register<B>>>();
    RegisterCBOpcode<0xC9, Instr<SET, I<1>, Register<C>>>();
    RegisterCBOpcode<0xCA, Instr<SET, I<1>, Register<D>>>();
    RegisterCBOpcode<0xCB, Instr<SET, I<1>, Register<E>>>();
    RegisterCBOpcode<0xCC, Instr<SET, I<1>, Register<H>>>();
    RegisterCBOpcode<0xCD, Instr<SET, I<1>, Register<L>>>();
    RegisterCBOpcode<0xCE, Instr<SET, I<1>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xCF, Instr<SET, I<1>, Register<A>>>();
    RegisterCBOpcode<0xD0, Instr<SET, I<2>, Register<B>>>();
    RegisterCBOpcode<0xD1, Instr<SET, I<2>, Register<C>>>();
    RegisterCBOpcode<0xD2, Instr<SET, I<2>, Register<D>>>();
    RegisterCBOpcode<0xD3, Instr<SET, I<2>, Register<E>>>();
    RegisterCBOpcode<0xD4, Instr<SET, I<2>, Register<H>>>();
    RegisterCBOpcode<0xD5, Instr<SET, I<2>, Register<L>>>();
    RegisterCBOpcode<0xD6, Instr<SET, I<2>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xD7, Instr<SET, I<2>, Register<A>>>();
    RegisterCBOpcode<0xD8, Instr<SET, I<3>, Register<B>>>();
    RegisterCBOpcode<0xD9, Instr<SET, I<3>, Register<C>>>();
    RegisterCBOpcode<0xDA, Instr<SET, I<3>, Register<D>>>();
    RegisterCBOpcode<0xDB, Instr<SET, I<3>, Register<E>>>();
    RegisterCBOpcode<0xDC, Instr<SET, I<3>, Register<H>>>();
    RegisterCBOpcode<0xDD, Instr<SET, I<3>, Register<L>>>();
    RegisterCBOpcode<0xDE, Instr<SET, I<3>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xDF, Instr<SET, I<3>, Register<A>>>();
    RegisterCBOpcode<0xE0, Instr<SET, I<4>, Register<B>>>();
    RegisterCBOpcode<0xE1, Instr<SET, I<4>, Register<C>>>();
    RegisterCBOpcode<0xE2, Instr<SET, I<4>, Register<D>>>();
    RegisterCBOpcode<0xE3, Instr<SET, I<4>, Register<E>>>();
    RegisterCBOpcode<0xE4, Instr<SET, I<4>, Register<H>>>();
    RegisterCBOpcode<0xE5, Instr<SET, I<4>, Register<L>>>();
    RegisterCBOpcode<0xE6, Instr<SET, I<4>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xE7, Instr<SET, I<4>, Register<A>>>();
    RegisterCBOpcode<0xE8, Instr<SET, I<5>, Register<B>>>();
    RegisterCBOpcode<0xE9, Instr<SET, I<5>, Register<C>>>();
    RegisterCBOpcode<0xEA, Instr<SET, I<5>, Register<D>>>();
    RegisterCBOpcode<0xEB, Instr<SET, I<5>, Register<E>>>();
    RegisterCBOpcode<0xEC, Instr<SET, I<5>, Register<H>>>();
    RegisterCBOpcode<0xED, Instr<SET, I<5>, Register<L>>>();
    RegisterCBOpcode<0xEE, Instr<SET, I<5>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xEF, Instr<SET, I<5>, Register<A>>>();
    RegisterCBOpcode<0xF0, Instr<SET, I<6>, Register<B>>>();
    RegisterCBOpcode<0xF1, Instr<SET, I<6>, Register<C>>>();
    RegisterCBOpcode<0xF2, Instr<SET, I<6>, Register<D>>>();
    RegisterCBOpcode<0xF3, Instr<SET, I<6>, Register<E>>>();
    RegisterCBOpcode<0xF4, Instr<SET, I<6>, Register<H>>>();
    RegisterCBOpcode<0xF5, Instr<SET, I<6>, Register<L>>>();
    RegisterCBOpcode<0xF6, Instr<SET, I<6>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xF7, Instr<SET, I<6>, Register<A>>>();
    RegisterCBOpcode<0xF8, Instr<SET, I<7>, Register<B>>>();
    RegisterCBOpcode<0xF9, Instr<SET, I<7>, Register<C>>>();
    RegisterCBOpcode<0xFA, Instr<SET, I<7>, Register<D>>>();
    RegisterCBOpcode<0xFB, Instr<SET, I<7>, Register<E>>>();
    RegisterCBOpcode<0xFC, Instr<SET, I<7>, Register<H>>>();
    RegisterCBOpcode<0xFD, Instr<SET, I<7>, Register<L>>>();
    RegisterCBOpcode<0xFE, Instr<SET, I<7>, ToAddr<Register<HL>>>>();
    RegisterCBOpcode<0xFF, Instr<SET, I<7>, Register<A>>>();
}

int Z80::RunOpcode(byte op) { return _instr[op]->Do(this); }
int Z80::RunCBOpcode(byte op) { return _cb_instr[op]->Do(this); }

void Z80::Process() {
    int cycles = 0;
    while (_power) {
        if (!halted()) {
            if (cycles == 0) {
                cinstr << "0x" << std::hex << _pc.u << "\t"
                       << int(_addr.Get(_pc.u).u) << "\t";
                PrintInstr(_addr.Get(_pc.u).u, this);
                cycles = RunOpcode(_addr.Get(_pc.u).u);
            }
            --cycles;
        }

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

template <unsigned char Opcode, class Inst>
void Z80::RegisterOpcode() {
    _instr[Opcode] = std::make_unique<Inst>();
}

template <unsigned char Opcode, class Inst>
void Z80::RegisterCBOpcode() {
    _cb_instr[Opcode] = std::make_unique<Inst>();
}

void Z80::set_interrupts(byte enable) { _interrupts = enable; }

void Z80::Dump() const {
    cerror << std::hex << "A = " << Register<A>::Get(this) << "\n"
           << "B = " << Register<B>::Get(this) << "\n"
           << "C = " << Register<C>::Get(this) << "\n"
           << "D = " << Register<D>::Get(this) << "\n"
           << "E = " << Register<E>::Get(this) << "\n"
           << "F = " << Register<F>::Get(this) << "\n"
           << "H = " << Register<H>::Get(this) << "\n"
           << "L = " << Register<L>::Get(this) << "\n"
           << "AF = " << Register<AF>::GetW(this) << "\n"
           << "BC = " << Register<BC>::GetW(this) << "\n"
           << "DE = " << Register<DE>::GetW(this) << "\n"
           << "HL = " << Register<HL>::GetW(this) << "\n"
           << "SP = " << Register<SP>::GetW(this) << "\n"
           << "PC = " << Register<PC>::GetW(this) << "\n";
}
