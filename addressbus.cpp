/*
 ** addressbus.cpp for gameboy
 **
 ** Made by Guillaume "Vermeille" Sanchez
 ** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
 **
 ** Started on  lun. 30 avril 2012 03:27:30 CEST Guillaume "Vermeille" Sanchez
 ** Last update mer. 02 mai 2012 13:14:28 CEST Guillaume "Vermeille" Sanchez
 */

#include <iostream>

#include "addressbus.h"
#include "z80.h"

    AddressBus::AddressBus(Cartridge& card)
: _card(card)
{
}

void AddressBus::Set(uint16_t index, byte val)
{
    switch (index & 0xF000)
    {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            _card.Set(index, val);
            break;
        case 0x8000:
        case 0x9000:
            // VRAM
            break;
        case 0xA000:
        case 0xB000:
            _card.Set(index, val);
            break;
        case 0xC000:
            // WRAM Bank 0
        case 0xD000:
            // WRAM Bank 1 - N
            break;
        case 0xE000:
            // Mirror
            break;
        case 0xF000:
            break;
    }
}

byte AddressBus::Get(uint16_t index) const
{
    if (index <= 0x7FFF)
        return _card.Get(index);
}

