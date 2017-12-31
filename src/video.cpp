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
#include <boost/format.hpp>
#include <iostream>
#include <stdexcept>

#include "video.h"

static const sf::Color kColors[] = {sf::Color(255, 255, 255),
                                    sf::Color(192, 192, 192),
                                    sf::Color(96, 96, 96),
                                    sf::Color(0, 0, 0)};
void Video::Clock() {
    ++_clock;
    char mode = _state.mode();
    _vblank_int = 0;
    _hblank_int = 0;

    if (mode == LCDStatus::SEARCH_OAM && _clock == 80) {
        _clock = 0;
        _state.set_mode(LCDStatus::TRANSFER);
    } else if (mode == LCDStatus::TRANSFER && _clock == 172) {
        _clock = 0;
        _state.set_mode(LCDStatus::HBLANK);
        _hblank_int = 1;
    } else if (mode == LCDStatus::HBLANK && _clock == 204) {
        _clock = 0;
        Render(_line);
        ++_line;
        if (_line == 144) {
            _state.set_mode(LCDStatus::VBLANK);
            _vblank_int = 1;
            cevent << "vBLANK INT\n";
        } else {
            _state.set_mode(LCDStatus::SEARCH_OAM);
        }
    } else if (mode == LCDStatus::VBLANK && _clock == 456) {
        ++_line;
        _clock = 0;
    }
    if (mode == LCDStatus::VBLANK && _line == 154) {
        NewFrame();
        _line = 0;
        _clock = 0;
        _state.set_mode(LCDStatus::SEARCH_OAM);
    }
}

void Video::NewFrame() {
    sf::Event event;
    while (_window.pollEvent(event)) {
        // Request for closing the window
        if (event.type == sf::Event::Closed) {
            _window.close();
            exit(0);
        }
    }

    _texture.update(_pixels.get());
    sf::Sprite sp;
    sp.setScale(4, 4);
    sp.setTexture(_texture);
    _window.draw(sp);
    _window.display();
    _window.clear(sf::Color::Blue);
}

void Video::RenderBg(int line) {
    const int y = (line + _scroll_y) % 256;
    auto pixs = reinterpret_cast<sf::Color*>(&_pixels[line * 160 * 4]);

    for (int px_num = 0; px_num < 160; ++px_num) {
        const int x = (px_num + _scroll_x) % 256;
        Data8 tile = bg_tilemap()[(x / 8) + (y / 8) * 32];
        int color = GetTilePix(tile, y % 8, x % 8);

        pixs[px_num] = kColors[color];
    }
}

void Video::Render(int line) {
    const int y = line;
    cdebug << "Y COORD: " << y << " " << _ctrl.bg_display() << "\n";
    assert(y < 144);
    if (_ctrl.bg_display()) {
        RenderBg(y);
    }
    if (false && _ctrl.bg_display()) {
        auto pixs = reinterpret_cast<sf::Color*>(&_pixels[line * 160 * 4]);
        for (int x = 0; x < 160; ++x) {
            Data8 tile = bg_tilemap()[(x / 8) + (y / 8) * 32];
            int color = GetTilePix(tile, y % 8, x % 8);

            pixs[x] = kColors[color];
        }
    }

    if (_ctrl.win_display_enable()) {
        auto pixs = reinterpret_cast<sf::Color*>(&_pixels[line * 160 * 4]);
        for (int x = 0; x < 160; ++x) {
            int y_win = y - _wy;
            int x_win = x - _wx;
            if (y_win < 0 || x_win < 0) {
                continue;
            }
            Data8 tile = win_tilemap()[(x_win / 8) + (y_win / 8) * 32];
            int color = GetTilePix(tile, y_win % 8, x_win % 8);

            pixs[x] = kColors[color];
        }
    }

    auto pixs = reinterpret_cast<sf::Color*>(&_pixels[0]);
    for (uint32_t i = 0; i < 40; ++i) {
        auto addr = i * 4;
        byte y_pos = _oam[addr].u - 16;
        byte x_pos = _oam[addr + 1].u - 8;
        Data8 tile = _oam[addr + 2];
        byte flags = _oam[addr + 3].u;

        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                int color = GetTilePix(tile, y, x);

                if (color == 0) {
                    continue;
                }

                if (y + y_pos < 0 || y + y_pos >= 144) {
                    continue;
                }

                if (x + x_pos < 0 || x + x_pos >= 160) {
                    continue;
                }
                pixs[(y + y_pos) * 160 + x + x_pos] = kColors[color];
            }
        }
    }
}
