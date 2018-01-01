#pragma once

#include <iomanip>
#include <iostream>

#include <stdint.h>
#include <SFML/Graphics.hpp>
#include <array>

#include "lcdc.h"
#include "lcdstatus.h"
#include "palette.h"
#include "palette.h"
#include "spritestable.h"
#include "utils.h"

class Video {
   public:
    Video()
        : _pixels(new sf::Uint8[160 * 144 * 4]),
          _clock(0),
          _sprites(_pixels.get(), *this),
          _window(sf::VideoMode(160 * 4, 144 * 4), "Gameboy") {
        _texture.create(160, 144);
        _oam.fill(uint8_t(0));
        _vram.fill(uint8_t(0));
        _window.setFramerateLimit(60);
    }

    void set_lcdc(byte b) {
        cevent << "LCDC: " << std::hex << int(b) << "\n";
        _ctrl.Set(b);
    }
    LCDCtrl lcdc() const { return _ctrl; }

    void set_lcd_status(byte v) { _state.Set(v); }
    byte lcd_status() const { return _state.Get(); }

    byte y_coord() const { return _line; }
    void reset_y_coord() {
        _line = 0;
        _state.set_mode(LCDStatus::SEARCH_OAM);
        _clock = 0;
    }

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
    const Data8* oam_ptr() const { return &_oam[0]; }

    byte win_y_pos() const { return _wy; }
    void set_win_y_pos(byte x) { _wy = x; }

    byte win_x_pos() const { return _wx + 7; }
    void set_win_x_pos(byte x) { _wx = x - 7; }

    bool vblank_int() const { return _vblank_int; }
    bool stat_int() const {
        return ((_ly_comp == _line) & _state.coincidence()) |
               (_hblank_int & _state.hblank());
    }

    byte bg_palette() const { return _bg_palette.Get(); }
    void set_bg_palette(byte x) { _bg_palette.Set(x); }

    byte obj0_palette() const { return _sprites.obj0_palette(); }
    void set_obj0_palette(byte x) { _sprites.set_obj0_palette(x); }

    byte obj1_palette() const { return _sprites.obj1_palette(); }
    void set_obj1_palette(byte x) { _sprites.set_obj1_palette(x); }

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

        int8_t l = (_vram[addr - 0x8000].u >> (7 - x)) & 1;
        int8_t h = (_vram[addr + 1 - 0x8000].u >> (7 - x)) & 1;
        return (h << 1) | l;
    }

    void NewFrame();
    void Render(int line);

    void RenderBg(int line);
    void RenderWindow(int line);

    std::unique_ptr<sf::Uint8[]> _pixels;
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
    Palette _bg_palette;
    SpritesTable _sprites;

    byte _vblank_int;
    byte _hblank_int;
    sf::RenderWindow _window;
    sf::Texture _texture;
};
