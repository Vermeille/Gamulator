/*
 ** addressbus.cpp for gameboy
 **
 ** Made by Guillaume "Vermeille" Sanchez
 ** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
 **
 ** Started on  lun. 30 avril 2012 03:27:30 CEST Guillaume "Vermeille" Sanchez
 ** Last update mer. 02 mai 2012 13:14:28 CEST Guillaume "Vermeille" Sanchez
 */

#include <stdexcept>
#include <iostream>

#include "addressbus.h"
#include "z80.h"

    AddressBus::AddressBus(Cartridge& card, Video& v)
: _card(card), _vid(v)
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
            _vid.Set(index, val);
            break;
        case 0xA000:
        case 0xB000:
            _card.Set(index, val);
            break;
        case 0xC000:
            // WRAM Bank 0
            _wram0[index - 0xC000] = val;
            break;
        case 0xD000:
            _wram0[index - 0xC000] = val;
            // WRAM Bank 1 - N
            break;
        case 0xE000:
            // Mirror
            break;
        case 0xF000:
            {
                if (index == 0xFF0F)
                    _interrupts = val;
                if (index >= 0xFF80 && index != 0xFFFF)
                    _hram[index - 0xFF80] = val;
                if (index == 0xFFFF)
                    _int_mask = val;
            }
            break;
    }
}

byte AddressBus::Get(uint16_t index) const
{
    if (index < 0x8000)
        return _card.Get(index);
    if (0x8000 <= index && index < 0xA000)
        return _vid.Get(index);
    if (0xA000 <= index && index < 0xC000)
        return _card.Get(index);
    if (0xC000 <= index && index < 0xE000)
        return _wram0[index - 0xC000];
    if (index == 0xFF0F)
    {
        _interrupts |= GetIntByte();
        return _interrupts;
    }
    if (index == 0xff44)
        return _vid.Get(index);
    if (index >= 0xFF80 && index != 0xFFFF)
        return _hram[index - 0xFF80];
    if (index == 0xFFFF)
        return _int_mask;
    std::cout << "invalid read at " << std::hex << index << std::endl;
    return 0;
}

byte AddressBus::GetIntByte() const
{
    return (_vid.vblank() ? 1 : 0);
}
