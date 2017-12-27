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

    void set_win_tile_map(int mode) { SetCtrlBit(6, mode); }
    int win_tile_map() const { return GetCtrlBit(6); }

    void set_win_display_enable(bool b) { SetCtrlBit(5, b); }
    bool win_display_enable() const { return GetCtrlBit(5); }

    void set_tile_data_mode(bool b) { SetCtrlBit(4, b); }
    bool tile_data_mode() const { return GetCtrlBit(4); }

    void set_bg_tile_map_mode(bool b) { SetCtrlBit(3, b); }
    bool bg_tile_map_mode() const { return GetCtrlBit(3); }

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
        _disp.create(160 * 4, 144 * 4);
        _vram.fill(uint8_t(0));
        _oam.fill(uint8_t(0));
        _window.setFramerateLimit(2);
    }

    void set_lcdc(byte b) { _ctrl.Set(b); }
    byte lcdc() const { return _ctrl.Get(); }

    void set_lcd_status(byte v) { _state.Set(v); }
    byte lcd_status() const { return _state.Get(); }

    byte y_coord() const { return _line; }
    void reset_y_coord() { _line = 0; }

    byte vram(uint16_t idx) const { return _vram[idx - 0x8000u].u; }
    void set_vram(uint16_t idx, byte val) { _vram[idx - 0x8000u].u = val; }

    byte scroll_x() const { return _scroll_x; }
    void set_scroll_x(byte val) { _scroll_x = val; }

    byte scroll_y() const { return _scroll_y; }
    void set_scroll_y(byte val) { _scroll_y = val; }

    byte ly_compare() const { return _ly_comp; }
    void set_ly_compare(byte v) { _ly_comp = v; }

    byte oam(uint16_t idx) const { return _oam[idx - 0xFE00u].u; }
    void set_oam(uint16_t idx, byte v) { _oam[idx - 0xFE00u].u = v; }

    byte win_y_pos() const { return _wy; }
    void set_win_y_pos(byte x) { _wy = x; }

    byte win_x_pos() const { return _wx + 7; }
    void set_win_x_pos(byte x) { _wx = x - 7; }

    bool vblank_int() const { return _vblank_int; }
    bool stat_int() const {
        return ((_ly_comp == _line) & _state.coincidence()) |
               (_hblank_int & _state.hblank());
    }

    void Clock();

   private:
    const Data8* bg_tilemap() const {
        return &_vram[(_ctrl.bg_tile_map_mode() ? 0x9C00u : 0x9800u) - 0x8000u];
    }

    const Data8* win_tilemap() const {
        return &_vram[(_ctrl.win_tile_map() ? 0x9C00u : 0x9800u) - 0x8000u];
    }
    int8_t GetTilePix(Data8 tile, int32_t y, int32_t x) {
        uint32_t addr = 0;
        if (_ctrl.tile_data_mode()) {
            addr = 0x8000 + tile.u * 16 + y * 2;
        } else {
            addr = 0x9000 + tile.s * 16 + y * 2;
        }
        cdebug << addr << "\n";

        int8_t l = (_vram[addr - 0x8000].u >> (7 - x)) & 1;
        int8_t h = (_vram[addr + 1 - 0x8000].u >> (7 - x)) & 1;
        return (h << 1) | l;
    }

    void NewFrame();
    void Render(int line);

    int32_t _clock = 0;
    int32_t _line = 0;
    int32_t _ly_comp;
    LCDCtrl _ctrl;
    LCDStatus _state;
    std::array<Data8, 0xA000 - 0x8000> _vram;
    std::array<Data8, 0xFEA0 - 0xFE00> _oam;
    byte _scroll_x;
    byte _scroll_y;
    byte _wy;
    byte _wx;

    byte _vblank_int;
    byte _hblank_int;
    sf::RenderWindow _window;
    sf::RenderTexture _disp;
};
