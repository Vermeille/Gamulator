#pragma once

#include <stdint.h>
#include <SFML/Graphics.hpp>

#include "utils.h"

class LCDCtrl {
    public:
        void Set(byte v) { _ctrl = v; }
        byte Get() const { return _ctrl; }

        void set_lcd_display_enable(bool d) { SetCtrlBit(7, d); }
        bool lcd_display_enable() { return GetCtrlBit(7); }

        void set_tile_map(int mode) { SetCtrlBit(6, mode); }
        int tile_map() { return GetCtrlBit(6); }

        void set_win_display_enable(bool b) { SetCtrlBit(5, b); }
        bool win_display_enable() { return GetCtrlBit(5); }

        void set_tile_data_mode(bool b) { SetCtrlBit(4, b); }
        bool tile_data_mode() { return GetCtrlBit(4); }

        void set_tile_map_mode(bool b) { SetCtrlBit(3, b); }
        bool tile_map_mode() { return GetCtrlBit(3); }

        void set_sprite_size(int mode) { SetCtrlBit(2, mode); }
        bool sprite_size() { return GetCtrlBit(2); }

        void set_sprite_display_enable(bool b) { SetCtrlBit(1,b); }
        bool sprite_display_enable() { return GetCtrlBit(0); }

        void set_bg_display(bool b) { SetCtrlBit(0, b); }
        bool bg_display() { return GetCtrlBit(0); }

    private:
        void SetCtrlBit(int b, bool val)
        {
            if (val)
                _ctrl |= (1 << b);
            else
                _ctrl &= ~(1 << b);
        }

        bool GetCtrlBit(int b) { return _ctrl & (1 << b); }

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

        bool coincidence() { return GetStateBit(2); }

        enum Mode {
            HBLANK = 0,
            VBLANK = 1,
            SEARCH_OAM = 2,
            TRANSFER = 3
        };

        void set_mode(Mode mode) { _state = (_state & ~0b11) | mode; }
        Mode mode() const { return Mode(_state % 4); }

    private:
        void SetStateBit(int b, bool val)
        {
            if (val)
                _state |= (1 << b);
            else
                _state &= ~(1 << b);
        }

        bool GetStateBit(int b) const { return _state & (1 << b); }

        byte _state;
};

class Video
{
    public:
        Video() : _window(sf::VideoMode(160*4, 144*4), "Gameboy") {}

        void Process();

        void set_lcdc(byte b) { _ctrl.Set(b); }
        byte lcdc() const { return _ctrl.Get(); }

        void set_lcd_status(byte v) { _state.Set(v); }
        byte lcd_status() const { return _state.Get(); }

        byte lcdc_y_coordinate() { return _y_coord; }

        byte vram(uint16_t idx) const;
        void set_vram(uint16_t idx, uint8_t val);

        byte scroll_x() const { return _scroll_x; }
        void set_scroll_x(byte val) { _scroll_x = val; }

        byte scroll_y() const { return _scroll_y; }
        void set_scroll_y(byte val) { _scroll_y = val; }

        bool vblank() const { return _state.vblank(); }

        void Clock();

    private:
        void SetCtrlBit(int b, bool val);
        bool GetCtrlBit(int b);

        void SetStateBit(int b, bool val);
        bool GetStateBit(int b);
        int _clock = 0;
        int _line = 0;
        LCDCtrl _ctrl;
        LCDStatus _state;
        byte _y_coord;
        byte _vram[0xA000 - 0x8000];
        byte _scroll_x;
        byte _scroll_y;

        sf::RenderWindow _window;
};
