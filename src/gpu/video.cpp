/*
** video.c for gameboy
**
** Made by Guillaume "Vermeille" Sanchez
** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
**
** Started on  mar. 24 avril 2012 13:39:35 CEST Guillaume "Vermeille" Sanchez
** Last update 2014-02-20 17:50 vermeille
*/

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "video.h"

void Video::Clock() {
    ++_clock;
    char mode = _state.mode();
    _vblank_int = 0;
    _phase_changed = false;
    _state.set_coincidence(false);

    if (mode == LCDStatus::SEARCH_OAM && _clock == 80) {
        _clock = 0;
        _state.set_mode(LCDStatus::TRANSFER);
        _phase_changed = true;
    } else if (mode == LCDStatus::TRANSFER && _clock == 172) {
        _clock = 0;
        _state.set_mode(LCDStatus::HBLANK);
        _phase_changed = true;
    } else if (mode == LCDStatus::HBLANK && _clock == 204) {
        _clock = 0;
        Render(_line);
        ++_line;
        _state.set_coincidence(_line == _ly_comp);
        if (_line == 144) {
            _state.set_mode(LCDStatus::VBLANK);
            _phase_changed = true;
            _vblank_int = 1;
            cevent << "vBLANK INT\n";
        } else {
            _state.set_mode(LCDStatus::SEARCH_OAM);
            _phase_changed = true;
        }
    } else if (mode == LCDStatus::VBLANK && _clock == 456) {
        ++_line;
        _state.set_coincidence(_line == _ly_comp);
        _clock = 0;
    }
    if (mode == LCDStatus::VBLANK && _line == 154) {
        _render.Render();
        _line = 0;
        _clock = 0;
        _state.set_mode(LCDStatus::SEARCH_OAM);
        _phase_changed = true;
    }
}

int8_t Video::GetTilePix(Data8 tile, int32_t y, int32_t x) {
    int32_t addr = 0;
    if (_ctrl.tile_data_mode() == 0) {
        addr = 0x9000 + tile.s * 16 + y * 2;
    } else {
        addr = 0x8000 + tile.u * 16 + y * 2;
    }

    int8_t l = (_vram[addr - 0x8000].u >> (7 - x)) & 1;
    int8_t h = (_vram[addr + 1 - 0x8000].u >> (7 - x)) & 1;
    return (h << 1) | l;
}

void Video::RenderBg(int line) {
    const int y = (line + _scroll_y) % 256;
    auto pixs = _render.pixs(line);

    for (int px_num = 0; px_num < 160; ++px_num) {
        const int x = (px_num + _scroll_x) % 256;
        Data8 tile = bg_tilemap((x / 8) + (y / 8) * 32);
        int color = GetTilePix(tile, y % 8, x % 8);

        pixs[px_num].Render(
            make_pixel(_bg_palette.GetColor(color), color == 0 ? 0 : 2));
    }
}

void Video::RenderWindow(int line) {
    const int y = line;
    auto pixs = _render.pixs(line);
    for (int x = 0; x < 160; ++x) {
        int y_win = y - _wy;
        int x_win = x - _wx;
        if (y_win < 0 || x_win < 0) {
            continue;
        }
        Data8 tile = win_tilemap((x_win / 8) + (y_win / 8) * 32);
        int color = GetTilePix(tile, y_win % 8, x_win % 8);

        pixs[x].Render(make_pixel(_bg_palette.GetColor(color), color ? 4 : 4));
    }
}

void Video::Render(int line) {
    const int y = line;
    cdebug << "Y COORD: " << y << " " << _ctrl.bg_display() << "\n";
    assert(y < 144);
    if (_ctrl.bg_display()) {
        RenderBg(y);
    }

    if (_ctrl.win_display_enable()) {
        RenderWindow(line);
    }

    if (_ctrl.sprite_display_enable()) {
        _sprites.Render(line);
    }
}
