#pragma once

#include <stdint.h>
#include <SFML/Graphics.hpp>
#include <array>

#include "utils.h"

class LCDCtrl {
   public:
    void Set(byte v) { _ctrl = v; }
    byte Get() const { return _ctrl; }

    void set_lcd_display_enable(bool d) { SetCtrlBit(7, d); }
    bool lcd_display_enable() const { return GetCtrlBit(7); }

    void set_tile_map(int mode) { SetCtrlBit(6, mode); }
    int tile_map() const { return GetCtrlBit(6); }

    void set_win_display_enable(bool b) { SetCtrlBit(5, b); }
    bool win_display_enable() const { return GetCtrlBit(5); }

    void set_tile_data_mode(bool b) { SetCtrlBit(4, b); }
    bool tile_data_mode() const { return GetCtrlBit(4); }

    void set_tile_map_mode(bool b) { SetCtrlBit(3, b); }
    bool tile_map_mode() const { return GetCtrlBit(3); }

    void set_sprite_size(int mode) { SetCtrlBit(2, mode); }
    bool sprite_size() const { return GetCtrlBit(2); }

    void set_sprite_display_enable(bool b) { SetCtrlBit(1, b); }
    bool sprite_display_enable() const { return GetCtrlBit(1); }

    void set_bg_display(bool b) { SetCtrlBit(0, b); }
    bool bg_display() const { return GetCtrlBit(0); }

   private:
    void SetCtrlBit(int b, bool val) {
        if (val)
            _ctrl |= (1 << b);
        else
            _ctrl &= ~(1 << b);
    }

    bool GetCtrlBit(int b) const { return _ctrl & (1 << b); }

    byte _ctrl;
};

class LCDStatus {
   public:
    void Set(byte v) { _state = v; }
    byte Get() const { return _state; }

    void set_lyc_interrupt(bool b) { SetStateBit(6, b); }
    bool lyc_interrupt() const { return GetStateBit(6); }

    void set_oam_interrupt(bool b) { SetStateBit(5, b); }
    bool oam_interrupt() const { return GetStateBit(5); }

    void set_vblank(bool b) {
        std::cout << "vblank interrupt " << b << std::endl;
        SetStateBit(4, b);
    }
    bool vblank() const { return GetStateBit(4); }

    void set_hblank(bool b) { SetStateBit(3, b); }
    bool hblank() const { return GetStateBit(3); }

    bool coincidence() const { return GetStateBit(2); }

    enum Mode { HBLANK = 0, VBLANK = 1, SEARCH_OAM = 2, TRANSFER = 3 };

    void set_mode(Mode mode) { _state = (_state & ~0b11) | mode; }
    Mode mode() const { return Mode(_state % 4); }

   private:
    void SetStateBit(int b, bool val) {
        if (val)
            _state |= (1 << b);
        else
            _state &= ~(1 << b);
    }

    bool GetStateBit(int b) const { return _state & (1 << b); }

    byte _state;
};

class Video {
   public:
    Video() : _window(sf::VideoMode(160 * 4, 144 * 4), "Gameboy") {
        _vram.fill(uint8_t(0));
        _oam.fill(uint8_t(0));
        _window.setFramerateLimit(60);
    }

    void set_lcdc(byte b) { _ctrl.Set(b); }
    byte lcdc() const { return _ctrl.Get(); }

    void set_lcd_status(byte v) { _state.Set(v); }
    byte lcd_status() const { return _state.Get(); }

    byte y_coord() { return _line; }

    byte vram(uint16_t idx) const { return _vram[idx - 0x8000].u; }
    void set_vram(uint16_t idx, byte val) { _vram[idx - 0x8000].u = val; }

    byte scroll_x() const { return _scroll_x; }
    void set_scroll_x(byte val) { _scroll_x = val; }

    byte scroll_y() const { return _scroll_y; }
    void set_scroll_y(byte val) { _scroll_y = val; }

    byte ly_compare() const { return _ly_comp; }
    void set_ly_compare(byte v) { _ly_comp = v; }

    const Data8* tilemap() const {
        return &_vram[(_ctrl.tile_map_mode() ? 0x9C00 : 0x9800) - 0x8000];
    }

    int GetTilePix(Data8 tile, int y, int x) {
        int addr = 0;
        if (_ctrl.tile_data_mode()) {
            addr = tile.u * 16 + y * 2;
        } else {
            addr = 0x800 + tile.s * 16 + y * 2;
        }
        cdebug << addr << "\n";

        auto l = (_vram[addr].u >> (7 - x)) & 1;
        auto h = (_vram[addr + 1].u >> (7 - x)) & 1;
        return (h << 1) | l;
    }

    void Clock();

    void Render();

    byte oam(int idx) const { return _oam[idx - 0xFE00].u; }
    void set_oam(int idx, byte v) { _oam[idx - 0xFE00].u = v; }

    bool vblank_int() const { return _vblank_int; }
    bool stat_int() const {
        return ((_ly_comp == _line) & _state.coincidence()) |
               (_hblank_int & _state.hblank());
    }

   private:
    int _clock = 0;
    int _line = 0;
    int _ly_comp;
    LCDCtrl _ctrl;
    LCDStatus _state;
    std::array<Data8, 0xA000 - 0x8000> _vram;
    std::array<Data8, 0xFEA0 - 0xFE00> _oam;
    byte _scroll_x;
    byte _scroll_y;

    byte _vblank_int;
    byte _hblank_int;
    sf::RenderWindow _window;
};
