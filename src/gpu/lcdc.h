#pragma once

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
