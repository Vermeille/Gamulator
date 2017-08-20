#pragma once

#include <stdint.h>

class Video
{
    public:
        void Process();
        typedef unsigned char byte;

        void set_lcd_display_enable(bool d);
        bool lcd_display_enable();

        void set_tile_map(int mode);
        int tile_map();

        void set_win_display_enable(bool b);
        bool win_display_enable();

        void set_tile_data_mode(bool b);
        bool tile_data_mode();

        void set_tile_map_mode(bool h);
        bool tile_map_mode();

        void set_sprite_size(int mode);
        bool sprite_size();

        void set_sprite_display_enable(bool b);
        bool sprite_display_enable();

        void set_bg_display(bool b);
        bool bg_display();

        void set_lyc_interrupt(bool b);
        bool lyc_interrupt();

        void set_oam_interrupt(bool b);
        bool oam_interrupt();

        void set_vblank(bool b);
        bool vblank();

        void set_hblank(bool b);
        bool hblank();

        bool coincidence();

        byte lcdc_y_coordinate();

        int mode();

        byte vram(uint16_t idx) const;
        void set_vram(uint16_t idx, uint8_t val);

        byte scroll_x() const { return _scroll_x; }
        void set_scroll_x(byte val) { _scroll_x = val; }

        byte scroll_y() const { return _scroll_y; }
        void set_scroll_y(byte val) { _scroll_y = val; }


        void Clock();

    private:
        void SetCtrlBit(int b, bool val);
        bool GetCtrlBit(int b);

        void SetStateBit(int b, bool val);
        bool GetStateBit(int b);
        int _clock = 0;
        int _line = 0;
        byte _ctrl;
        byte _state;
        byte _y_coord;
        byte _vram[0xA000 - 0x8000];
        byte _scroll_x;
        byte _scroll_y;
};
