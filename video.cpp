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

//////////////////////////////////////////
////// CONTROL REGISTER //////////////////
//////////////////////////////////////////
void Video::set_lcd_display_enable(bool d)
{
    SetCtrlBit(7, d);
}

bool Video::lcd_display_enable()
{
    return GetCtrlBit(7);
}

void Video::set_tile_map(int mode)
{
    SetCtrlBit(6, mode);
}

int Video::tile_map()
{
    return GetCtrlBit(6);
}

void Video::set_win_display_enable(bool b)
{
    SetCtrlBit(5, b);
}

bool Video::win_display_enable()
{
    return GetCtrlBit(5);
}
void Video::set_tile_data_mode(bool b)
{
    SetCtrlBit(4, b);
}

bool Video::tile_data_mode()
{
    return GetCtrlBit(4);
}

void Video::set_tile_map_mode(bool b)
{
    SetCtrlBit(3, b);
}

bool Video::tile_map_mode()
{
    return GetCtrlBit(3);
}

void Video::set_sprite_size(int mode)
{
    SetCtrlBit(2, mode);
}

bool Video::sprite_size()
{
    return GetCtrlBit(2);
}

void Video::set_sprite_display_enable(bool b)
{
    SetCtrlBit(1,b);
}

bool Video::sprite_display_enable()
{
    return GetCtrlBit(0);
}

void Video::set_bg_display(bool b)
{
    SetCtrlBit(0, b);
}

bool Video::bg_display()
{
    return GetCtrlBit(0);
}

void Video::SetCtrlBit(int b, bool val)
{
    if (val)
        _ctrl |= (1 << b);
    else
        _ctrl &= ~(1 << b);
}

bool Video::GetCtrlBit(int b)
{
    return _ctrl & (1 << b);
}

///////////////////////////////////////////////////////////
///////////// STATE REGISTER //////////////////////////////
///////////////////////////////////////////////////////////

void Video::set_lyc_interrupt(bool b)
{
    SetStateBit(6, b);
}

bool Video::lyc_interrupt()
{
    return GetStateBit(6);
}

void Video::set_oam_interrupt(bool b)
{
    SetStateBit(5, b);
}

bool Video::oam_interrupt()
{
    return GetStateBit(5);
}

void Video::set_vblank(bool b)
{
    std::cout << "vblank interrupt " << b << std::endl;
    SetStateBit(4, b);
}

bool Video::vblank()
{
    return GetStateBit(4);
}

void Video::set_hblank(bool b)
{
    SetStateBit(3, b);
}

bool Video::hblank()
{
    return GetStateBit(3);
}

bool Video::coincidence()
{
    return GetStateBit(2);
}

int Video::mode()
{
    return _state % 4;
}

void Video::SetStateBit(int b, bool val)
{
    if (val)
        _state |= (1 << b);
    else
        _state &= ~(1 << b);
}

bool Video::GetStateBit(int b)
{
    return _state & (1 << b);
}

unsigned char Video::lcdc_y_coordinate()
{
    return _y_coord;
}

void Video::Set(uint16_t idx, byte val)
{
    if (0x8000 <= idx && idx < 0xA000)
    {
        _vram[idx - 0x8000] = val;
        return;
    }
    throw std::runtime_error(boost::str(
                boost::format("invalid access in VRAM at %x") % idx));
}

Video::byte Video::Get(uint16_t idx) const
{
    if (0x8000 <= idx && idx < 0xA000)
        return _vram[idx - 0x8000];
    switch (idx)
    {
        case 0xFF44:
            std::cout << "_line = " << _line << std::endl;
            return _line;
    }
    throw std::runtime_error("invalid memory access in VRAM");
}

void Video::Clock()
{
    ++_clock;
    char mode = _state & 0x3;

    if (mode == 2 && _clock == 80)
    {
        _clock = 0;
        _state &= ~3;
        _state |= 3;
    }
    else if (mode == 3 && _clock == 172)
    {
        _clock = 0;
        _state &= ~3;
    }
    else if (mode == 0 && _clock == 204)
    {
        _clock = 0;
        _state &= ~3;
        ++_line;
        if (_line == 145)
        {
            _state |= 1;
            set_vblank(true);
        }
        else
        {
            _state |= 2;
        }
    }
    else if (mode == 1 && _clock == 4560)
    {
        set_vblank(false);
        _line = 0;
        _clock = 0;
        _state &= ~3;
        _state |= 2;
    }
}

void Video::Process()
{
    while (true)
    {
        _y_coord = (_y_coord + 1) % 154;
    }
}
