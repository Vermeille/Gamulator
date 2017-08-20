/*
 ** cartridge.cpp for gameboy
 **
 ** Made by Guillaume "Vermeille" Sanchez
 ** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
 **
 ** Started on  lun. 30 avril 2012 03:32:13 CEST Guillaume "Vermeille" Sanchez
 ** Last update 2014-02-20 16:22 vermeille
 */

#include <stdexcept>
#include <iostream>

#include "cartridge.h"
#include "z80.h"
#include "utils.h"

    Cartridge::Cartridge(std::string filename)
: _ram(0x2000, 0), _rom_bank(1), _ram_bank(0)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    while (file.good())
    {
        _data.push_back(file.get());
    }
    std::cout << "Game is " << reinterpret_cast<char*>(&_data[0x134])
        << std::endl;
    _mbc = _data[0x147];
}


void Cartridge::Set(word index, byte val)
{
    switch (index & 0xF000)
    {
        case 0x0000:
        case 0x1000:
            std::cout << "Toggle external ram" << std::endl;
            break;
        case 0x2000:
        case 0x3000:
            std::cout << "LOW bank number" << std::endl;
            _rom_bank = (_rom_bank & 11100000_b) + (val & 00011111_b);
            break;
        case 0x4000:
        case 0x5000:
            if (_selector == Ram)
                _ram_bank = val;
            else
                _rom_bank = (_rom_bank & 00011111_b)
                    + ((11_b & val) << 5);
            break;
        case 0x6000:
        case 0x7000:
            _selector = (val) ? Ram : Rom;
            break;
        case 0x8000:
        case 0x9000:
            throw "Erreur d'acces, 0x8xxx ou 0x9xxx non cartouche";
            break;
        case 0xA000:
        case 0xB000:
            std::cout << "Writing in RAM";
            _ram[_ram_bank*0x2000+(index-0xA000)] = val;
            break;
    }
}

byte Cartridge::Get(word index) const
{
    if (index < 0x4000)
        return _data[index];
    else if (index < 0x8000)
        return _data[_rom_bank*0x4000+(index-0x4000)];
    else if (index < 0xC000)
        return _ram[_ram_bank*0x2000+(index-0xA000)];
    throw std::runtime_error("No such address in cartridge");
}

std::string Cartridge::Print(uint16_t index) const {
    if (index < 0x100)
        throw "invalid addr";
    else if (index < 0x150)
        return "cartridge_header_area";
    else if (index < 0x4000)
        return "cartridge_rom_0";
    else if (index < 0x8000)
        return "cartridge_rom_bank_" + std::to_string(_rom_bank);
    else if (index < 0xA000)
        throw "not cartridge addr";
    else if (index < 0xC000)
        return "cartridge_ram_bank_" + std::to_string(_ram_bank);
    else
        throw "not cartridge addr";
}
