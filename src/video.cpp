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
        if (_line == 145) {
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
        if (event.type == sf::Event::Closed) _window.close();
    }

    _disp.display();
    auto persistence = sf::Sprite(_disp.getTexture());
    _window.draw(persistence);
    _window.display();
    _window.clear(sf::Color::Blue);
    _disp.clear(sf::Color::Red);
    _disp.draw(persistence);
    std::cerr << "NEW\n";
}

void Video::Render(int line) {
    std::vector<sf::Color> colors = {sf::Color(255, 255, 255),
                                     sf::Color(192, 192, 192),
                                     sf::Color(96, 96, 96),
                                     sf::Color(0, 0, 0)};

    const int y = line;
    std::cerr << y << " " << _ctrl.bg_display() << "\n";
    assert(y < 145);
    if (_ctrl.bg_display()) {
        for (int x = 0; x < 160; ++x) {
            Data8 tile = bg_tilemap()[(x / 8) + (y / 8) * 32];
            int color = GetTilePix(tile, y % 8, x % 8);

            sf::RectangleShape r;
            r.setSize(sf::Vector2f(4, 4));
            r.setPosition(x * 4, y * 4);
            r.setFillColor(colors[color]);
            _disp.draw(r);
        }
    }

#if 0
    if (_ctrl.win_display_enable()) {
        for (int x = 0; x < 144; ++x) {
            int y_win = y - _wy;
            int x_win = x - _wx;
            if (y_win < 0 || x_win < 0) {
                continue;
            }
            Data8 tile = win_tilemap()[(x_win / 8) + (y_win / 8) * 32];
            int color = GetTilePix(tile, y_win % 8, x_win % 8);

            sf::RectangleShape r;
            r.setSize(sf::Vector2f(4, 4));
            r.setPosition(x * 4, y * 4);
            r.setFillColor(colors[color]);
            _window.draw(r);
        }
    }

    for (uint32_t i = 0; i < 40; ++i) {
        auto addr = i * 4;
        byte x_pos = _oam[addr].u;
        byte y_pos = _oam[addr + 1].u;
        Data8 tile = _oam[addr + 2];
        byte flags = _oam[addr + 3].u;

        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                int color = GetTilePix(tile, y, x);

                if (color == 0) {
                    continue;
                }

                sf::RectangleShape r;
                r.setSize(sf::Vector2f(4, 4));
                r.setPosition(x * 4 + x_pos * 4, y * 4 + y_pos * 4);
                r.setFillColor(colors[color]);
                _window.draw(r);
            }
        }
    }
#endif
}
