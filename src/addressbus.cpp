/*
 ** addressbus.cpp for gameboy
 **
 ** Made by Guillaume "Vermeille" Sanchez
 ** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
 **
 ** Started on  lun. 30 avril 2012 03:27:30 CEST Guillaume "Vermeille" Sanchez
 ** Last update mer. 02 mai 2012 13:14:28 CEST Guillaume "Vermeille" Sanchez
 */

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "addressbus.h"
#include "utils.h"
#include "z80.h"

byte NotImplementedGet(uint16_t idx) {
    cerror << "/!\\ " << std::hex << idx << " is not implemented yet\n";
    return 0xFF;
}

void NotImplementedSet(uint16_t idx, byte) {
    cerror << "/!\\ " << std::hex << idx << " is not implemented yet\n";
}

AddressBus::AddressBus(Cartridge& card,
                       Video& v,
                       LinkCable& lk,
                       Keypad& kp,
                       Timer& timer,
                       Sound& snd)
    : _card(card), _vid(v), _lk(lk), _kp(kp), _timer(timer), _snd(snd) {
    using namespace std::placeholders;
    _mem_map = {
        {"cartridge_rom_bank_0",
         0x0000,
         0x3FFF,
         std::bind(&Cartridge::Read, &_card, _1),
         std::bind(&Cartridge::Write, &_card, _1, _2)},
        {"cartridge_rom_bank_switchable",
         0x4000,
         0x7FFF,
         std::bind(&Cartridge::Read, &_card, _1),
         std::bind(&Cartridge::Write, &_card, _1, _2)},
        {"vram",
         0x8000,
         0x97FF,
         std::bind(&Video::vram, &_vid, _1),
         std::bind(&Video::set_vram, &_vid, _1, _2)},
        {"vram_bg1",
         0x9800,
         0x9BFF,
         std::bind(&Video::vram, &_vid, _1),
         std::bind(&Video::set_vram, &_vid, _1, _2)},
        {"vram_bg2",
         0x9C00,
         0x9FFF,
         std::bind(&Video::vram, &_vid, _1),
         std::bind(&Video::set_vram, &_vid, _1, _2)},
        {"cartridge_ram",
         0xA000,
         0xBFFF,
         std::bind(&Cartridge::Read, &_card, _1),
         std::bind(&Cartridge::Write, &_card, _1, _2)},
        {"internal_ram_bank0",
         0xC000,
         0xCFFF,
         [&](uint16_t index) { return _wram0[index - 0xC000u].u; },
         [&](uint16_t index, byte v) { _wram0[index - 0xC000u].u = v; }},
        {"internal_ram_bank1",
         0xD000,
         0xDFFF,
         [&](uint16_t index) { return _wram0[index - 0xC000u].u; },
         [&](uint16_t index, byte v) { _wram0[index - 0xC000u].u = v; }},
        {"echo_ram",
         0xE000,
         0xFDFF,
         [&](uint16_t index) { return _wram0[index - 0xE000u].u; },
         [&](uint16_t index, byte v) { _wram0[index - 0xE000u].u = v; }},
        {"oam",
         0xFE00,
         0xFE9F,
         std::bind(&Video::oam, &_vid, _1),
         std::bind(&Video::set_oam, &_vid, _1, _2)},
        {"unusable", 0xFEA0, 0xFEFF, NotImplementedGet, NotImplementedSet},
        // IO PORTS
        {"joyp",
         0xFF00,
         0xFF00,
         std::bind(&Keypad::joyp, &_kp),
         std::bind(&Keypad::set_joyp, &_kp, _2)},
        {"serial_transfer",
         0xFF01,
         0xFF01,
         std::bind(&LinkCable::Read, &_lk),
         std::bind(&LinkCable::Send, &_lk, _2)},
        {"serial_ctrl",
         0xFF02,
         0xFF02,
         std::bind(&LinkCable::serial_control, &_lk),
         std::bind(&LinkCable::set_serial_control, &_lk, _2)},
        {"unused", 0xFF03, 0xFF03, NotImplementedGet, NotImplementedSet},
        {"timer_divider",
         0xFF04,
         0xFF04,
         [&](uint16_t) { return _timer.div().u; },
         [&](uint16_t, byte) { _timer.Reset(); }},
        {"timer_counter",
         0xFF05,
         0xFF05,
         [&](uint16_t) { return _timer.tima().u; },
         [&](uint16_t, byte x) { _timer.set_tima(x); }},
        {"timer_modulo",
         0xFF06,
         0xFF06,
         [&](uint16_t) { return _timer.tma().u; },
         [&](uint16_t, byte x) { _timer.set_tma(x); }},
        {"timer_control",
         0xFF07,
         0xFF07,
         [&](uint16_t) { return _timer.tac().u; },
         [&](uint16_t, byte x) { _timer.set_tac(x); }},
        {"io_ports", 0xFF08, 0xFF0E, NotImplementedGet, NotImplementedSet},
        {"int_flag",
         0xFF0F,
         0xFF0F,
         [&](uint16_t) { return _interrupts; },
         [&](uint16_t, byte v) { _interrupts = v; }},
        {"io_ports", 0xFF10, 0xFF10, NotImplementedGet, NotImplementedSet},
        {"nr11_pattern",
         0xFF11,
         0xFF11,
         [&](uint16_t) { return _snd.channel_1().len_pattern(); },
         [&](uint16_t, byte x) { _snd.channel_1().set_len_pattern(x); }},
        {"nr12_enveloppe",
         0xFF12,
         0xFF12,
         NotImplementedGet,
         NotImplementedSet},
        {"nr13_freq_lo",
         0xFF13,
         0xFF13,
         NotImplementedGet,
         [&](uint16_t, byte x) { _snd.channel_1().set_freq_lo(x); }},
        {"nr14_freq_lo",
         0xFF14,
         0xFF14,
         NotImplementedGet,
         [&](uint16_t, byte x) { _snd.channel_1().set_freq_hi(x); }},
        {"io_ports", 0xFF15, 0xFF15, NotImplementedGet, NotImplementedSet},
        {"nr21_pattern",
         0xFF16,
         0xFF16,
         [&](uint16_t) { return _snd.channel_2().len_pattern(); },
         [&](uint16_t, byte x) { _snd.channel_2().set_len_pattern(x); }},
        {"nr22_enveloppe",
         0xFF17,
         0xFF17,
         NotImplementedGet,
         NotImplementedSet},
        {"nr23_freq_lo",
         0xFF18,
         0xFF18,
         NotImplementedGet,
         [&](uint16_t, byte x) { _snd.channel_2().set_freq_lo(x); }},
        {"nr24_freq_lo",
         0xFF19,
         0xFF19,
         NotImplementedGet,
         [&](uint16_t, byte x) { _snd.channel_2().set_freq_hi(x); }},
        {"nr30_on_off",
         0xFF1A,
         0xFF1A,
         [&](uint16_t) { return _snd.wave().active(); },
         [&](uint16_t, byte x) { _snd.wave().set_active(x); }},
        {"nr31_length",
         0xFF1B,
         0xFF1B,
         NotImplementedGet,
         [&](uint16_t, byte x) { _snd.wave().set_length(x); }},
        {"nr32_level",
         0xFF1C,
         0xFF1C,
         [&](uint16_t) { return _snd.wave().level(); },
         [&](uint16_t, byte x) { _snd.wave().set_level(x); }},
        {"nr33_freq_lo",
         0xFF1D,
         0xFF1D,
         NotImplementedGet,
         [&](uint16_t, byte x) { _snd.wave().set_freq_lo(x); }},
        {"nr34_freq_hi",
         0xFF1E,
         0xFF1E,
         [&](uint16_t) { return _snd.wave().freq_hi(); },
         [&](uint16_t, byte x) { _snd.wave().set_freq_hi(x); }},
        {"io_ports", 0xFF1F, 0xFF2F, NotImplementedGet, NotImplementedSet},
        {"wave_pattern",
         0xFF30,
         0xFF3F,
         [&](uint16_t x) { return _snd.wave().Read(x); },
         [&](uint16_t addr, byte x) { _snd.wave().Write(addr, x); }},
        {"lcdc",
         0xFF40,
         0xFF40,
         [&](uint16_t) { return _vid.lcdc().Get(); },
         std::bind(&Video::set_lcdc, &_vid, _2)},
        {"lcd_status",
         0xFF41,
         0xFF41,
         std::bind(&Video::lcd_status, &_vid),
         std::bind(&Video::set_lcd_status, &_vid, _2)},
        {"scroll_y",
         0xFF42,
         0xFF42,
         std::bind(&Video::scroll_y, &_vid),
         std::bind(&Video::set_scroll_y, &_vid, _2)},
        {"scroll_x",
         0xFF43,
         0xFF43,
         std::bind(&Video::scroll_x, &_vid),
         std::bind(&Video::set_scroll_x, &_vid, _2)},
        {"y_coord",
         0xFF44,
         0xFF44,
         std::bind(&Video::y_coord, &_vid),
         std::bind(&Video::reset_y_coord, &_vid)},
        {"ly_compare",
         0xFF45,
         0xFF45,
         std::bind(&Video::ly_compare, &_vid),
         std::bind(&Video::set_ly_compare, &_vid, _2)},
        {"dma",
         0xFF46,
         0xFF46,
         NotImplementedGet,
         [&](int, byte v) {
             for (int i = 0; i < 0x9F; ++i) {
                 _vid.set_oam(0xFE00 + i, Get((v << 8) + i).u);
             }
         }},
        {"bg_palette",
         0xFF47,
         0xFF47,
         std::bind(&Video::bg_palette, &_vid),
         std::bind(&Video::set_bg_palette, &_vid, _2)},
        {"obj0_palette",
         0xFF48,
         0xFF48,
         std::bind(&Video::obj0_palette, &_vid),
         std::bind(&Video::set_obj0_palette, &_vid, _2)},
        {"obj1_palette",
         0xFF49,
         0xFF49,
         std::bind(&Video::obj1_palette, &_vid),
         std::bind(&Video::set_obj1_palette, &_vid, _2)},
        {"win_y_pos",
         0xFF4A,
         0xFF4A,
         std::bind(&Video::win_y_pos, &_vid),
         std::bind(&Video::set_win_y_pos, &_vid, _2)},
        {"win_x_pos",
         0xFF4B,
         0xFF4B,
         std::bind(&Video::win_x_pos, &_vid),
         std::bind(&Video::set_win_x_pos, &_vid, _2)},
        {"io_ports", 0xFF4C, 0xFF7F, NotImplementedGet, NotImplementedSet},
        // HRAM
        {"hram",
         0xFF80,
         0xFFFE,
         [&](uint16_t index) { return _hram[index - 0xFF80u].u; },
         [&](uint16_t index, byte v) { _hram[index - 0xFF80u].u = v; }},
        {"interrupt_master_enable",
         0xFFFF,
         0xFFFF,
         [&](uint16_t) { return _int_mask; },
         [&](uint16_t, byte v) { _int_mask = v; }}};
}

const AddressBus::Addr& AddressBus::FindAddr(uint16_t addr) const {
    uint32_t b = 0;
    uint32_t e = _mem_map.size();

    while (true) {
        if (b == e) {
            assert(false);
        }

        uint32_t m = (b + e) / 2;

        if (_mem_map[m]._begin <= addr && addr <= _mem_map[m]._end) {
            return _mem_map[m];
        }

        if (addr < _mem_map[m]._begin) {
            e = m;
        } else {
            b = m + 1;
        }
    }
}

void AddressBus::Set(uint16_t index, Data8 val) {
    return FindAddr(index)._set(index, val.u);
}

Data8 AddressBus::Get(uint16_t index) const {
    return FindAddr(index)._get(index);
}

std::string AddressBus::Print(uint16_t index) const {
    return FindAddr(index)._name;
}
