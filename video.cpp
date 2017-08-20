/*
** video.c for gameboy
**
** Made by Guillaume "Vermeille" Sanchez
** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
**
** Started on  mar. 24 avril 2012 13:39:35 CEST Guillaume "Vermeille" Sanchez
** Last update 2014-02-20 17:50 vermeille
*/

#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <boost/format.hpp>

#include "video.h"

byte Video::vram(uint16_t idx) const
{
    return _vram[idx - 0x8000];
}

void Video::set_vram(uint16_t idx, byte val)
{
    _vram[idx - 0x8000] = val;
}

void Video::Clock()
{
    ++_clock;
    char mode = _state.mode();

    if (mode == LCDStatus::SEARCH_OAM && _clock == 80)
    {
        _clock = 0;
        _state.set_mode(LCDStatus::TRANSFER);
    }
    else if (mode == LCDStatus::TRANSFER && _clock == 172)
    {
        _clock = 0;
        _state.set_mode(LCDStatus::HBLANK);
    }
    else if (mode == LCDStatus::HBLANK && _clock == 204)
    {
        _clock = 0;
        ++_line;
        if (_line == 145)
        {
            _state.set_mode(LCDStatus::VBLANK);
            _state.set_vblank(true);
        }
        else
        {
            _state.set_mode(LCDStatus::SEARCH_OAM);
        }
    }
    else if (mode == LCDStatus::VBLANK && _clock == 4560)
    {
        _state.set_vblank(false);
        _line = 0;
        _clock = 0;
        _state.set_mode(LCDStatus::SEARCH_OAM);
    }
}

void Video::Process()
{
    while (true)
    {
        _y_coord = (_y_coord + 1) % 154;
    }
}
