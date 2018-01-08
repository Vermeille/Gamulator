#pragma once

#include <functional>
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

class RenderZone {
   public:
    template <class Px, class Z>
    class Pixel {
       public:
        Pixel(Px c, Z z) : _color(c), _z(z) {}

        template <class Px2, class Z2>
        void operator=(const Pixel<Px2, Z2>& p) {
            _color = p.color();
            _z = p.z();
        }

        template <class Px2, class Z2>
        void Render(const Pixel<Px2, Z2>& p) {
            if (p.z() >= _z) {
                _z = p.z();
                _color = p.color();
            }
        }

        Px color() const { return _color; }
        void set_color(Px p) { _color = p; }

        byte z() const { return _z; }
        void set_z(byte z) { _z = z; }

       private:
        Px _color;
        Z _z;
    };

    class PixelIterator {
       public:
        PixelIterator(sf::Color* color, byte* prio) : _color(color), _z(prio) {}

        void operator++() {
            ++_color;
            ++_z;
        }

        bool operator==(const PixelIterator& it) const {
            return it._color == _color && it._z == _z;
        }

        bool operator!=(const PixelIterator& it) const {
            return !operator==(it);
        }

        auto operator*() const {
            return Pixel<decltype(std::ref(*_color)), decltype(std::ref(*_z))>(
                std::ref(*_color), std::ref(*_z));
        }

        auto operator[](int i) const {
            return Pixel<sf::Color&, byte&>(_color[i], _z[i]);
        }

       private:
        sf::Color* _color;
        byte* _z;
    };

    RenderZone()
        : _window(sf::VideoMode(160 * 4, 144 * 4), "Gameboy"),
          _pixels(160 * 144),
          _z(160 * 144) {
        _texture.create(160, 144);
        _window.setFramerateLimit(60);
        std::fill(_z.begin(), _z.end(), 0);
    }

    void Render();

    PixelIterator pixs(int line = 0) {
        return PixelIterator(&_pixels[160 * line], &_z[160 * line]);
    }

   private:
    sf::RenderWindow _window;
    std::vector<sf::Color> _pixels;
    std::vector<byte> _z;
    sf::Texture _texture;
};

template <class P, class Z>
RenderZone::Pixel<P, Z> make_pixel(P p, Z z) {
    return RenderZone::Pixel<P, Z>(p, z);
}

class Video {
   public:
    Video() : _clock(0), _sprites(*this) {
        _oam.fill(uint8_t(0));
        _vram.fill(uint8_t(0));
    }

    void set_lcdc(byte b) {
        _ctrl.Set(b);
        cevent << "LCDC: " << std::hex << int(b)
               << "BG MAP: " << _ctrl.bg_tile_map_mode()
               << " DATA: " << _ctrl.tile_data_mode() << "\n";
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
        byte mode = _state.mode();
        return (_state.lyc_interrupt() && _state.coincidence()) ||
               (_phase_changed &&  // just on phase change
                ((mode == LCDStatus::HBLANK && _state.hblank()) ||
                 ((mode == LCDStatus::VBLANK && _state.vblank()) ||
                  // we want OAM in line 144 too
                  (_line == 144 && _state.oam_interrupt())) ||
                 (mode == LCDStatus::SEARCH_OAM && _state.oam_interrupt())));
    }

    byte bg_palette() const { return _bg_palette.Get(); }
    void set_bg_palette(byte x) { _bg_palette.Set(x); }

    byte obj0_palette() const { return _sprites.obj0_palette(); }
    void set_obj0_palette(byte x) { _sprites.set_obj0_palette(x); }

    byte obj1_palette() const { return _sprites.obj1_palette(); }
    void set_obj1_palette(byte x) { _sprites.set_obj1_palette(x); }

    void Clock();

    RenderZone& render_zone() { return _render; }

   private:
    Data8 bg_tilemap(int tile_nbr) const {
        return _vram[tile_nbr +
                     ((_ctrl.bg_tile_map_mode() ? 0x9C00 : 0x9800) - 0x8000)];
    }

    Data8 win_tilemap(int tile_nbr) const {
        return _vram[tile_nbr +
                     ((_ctrl.win_tile_map() ? 0x9C00 : 0x9800) - 0x8000)];
    }

    int8_t GetTilePix(Data8 tile, int32_t y, int32_t x);

    void Render(int line);

    void RenderBg(int line);
    void RenderWindow(int line);

    int32_t _clock = 0;
    int32_t _line = 0;
    int32_t _ly_comp;
    LCDCtrl _ctrl;
    LCDStatus _state;
    std::array<Data8, 0xA000 - 0x8000> _vram;
    std::array<Data8, 0xFEA0 - 0xFE00> _oam;
    byte _scroll_x;
    byte _scroll_y;
    int _wy;
    int _wx;
    Palette _bg_palette;
    SpritesTable _sprites;
    bool _phase_changed;

    byte _vblank_int;
    RenderZone _render;
};
