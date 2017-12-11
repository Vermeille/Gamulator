/*
 ** cartridge.cpp for gameboy
 **
 ** Made by Guillaume "Vermeille" Sanchez
 ** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
 **
 ** Started on  lun. 30 avril 2012 03:32:13 CEST Guillaume "Vermeille" Sanchez
 ** Last update 2014-02-20 16:22 vermeille
 */

#include <iostream>
#include <stdexcept>

#include "cartridge.h"
#include "utils.h"
#include "z80.h"

void Error(uint16_t idx) {
    throw std::runtime_error("Invalid access at " + hex(idx));
}

class Raw : public Cartridge::Controller {
   public:
    Raw(std::vector<byte>&& data) : Cartridge::Controller(std::move(data)) {
        _mem_map = {
            {"rom_bank_0",
             0x0000,
             0x3FFF,
             [&](uint16_t idx) { return ReadRom(idx); },
             [&](uint16_t, byte) {
                 cerror << "Can't switch ROM bank without MBC\n";
             }},
            {"rom_bank_switchable",
             0x4000,
             0x7FFF,
             [&](uint16_t idx) { return ReadRom(idx); },
             [&](uint16_t, byte) {
                 cerror << "Can't switch ROM bank without MBC\n";
             }},
            {"ram_bank",
             0xA000,
             0xBFFF,
             [&](uint16_t idx) { return _ram[idx - 0xA000]; },
             [&](uint16_t idx, byte b) { _ram[idx - 0xA000] = b; }},
        };
    };

   private:
    std::array<byte, 0xC000 - 0xA000> _ram;
};

class MBC1 : public Cartridge::Controller {
   public:
    MBC1(std::vector<byte>&& data)
        : Cartridge::Controller(std::move(data)), _rom_nbr(1) {
        _mem_map = {{"rom_bank_0",
                     0x0000,
                     0x1FFF,
                     [&](uint16_t idx) { return ReadRom(idx); },
                     [&](uint16_t, byte) {
                         // enable RAM aka do nothing
                     }},
                    {"rom_bank_0",
                     0x2000,
                     0x3FFF,
                     [&](uint16_t idx) { return ReadRom(idx); },
                     [&](uint16_t, byte b) {
                         b = b & 0b1'1111;
                         b = b == 0 ? 1 : b;
                         _rom_nbr = (_rom_nbr & ~0b1'1111) | b;
                     }},
                    {"rom_bank_switchable",
                     0x4000,
                     0x5FFF,
                     [&](uint16_t idx) {
                         return ReadRom(idx - 0x4000 + _rom_nbr * 0x4000);
                     },
                     [&](uint16_t, byte b) {
                         if (_selector == Rom) {
                             _rom_nbr =
                                 (_rom_nbr & ~(0b11 << 5)) | ((b & 0b11) << 5);
                         } else {
                             _ram_nbr = b & 0b11;
                         }
                     }},
                    {"rom_bank_switchable",
                     0x6000,
                     0x7FFF,
                     [&](uint16_t idx) {
                         return ReadRom(idx - 0x4000 + _rom_nbr * 0x4000);
                     },
                     [&](uint16_t, byte b) { _selector = RamRomSelector(b); }},
                    {
                        "cartridge_ram",
                        0xA000,
                        0xBFFF,
                        [&](uint16_t idx) {
                            return _ram[(idx - 0xA000) + _ram_nbr * 0x2000];
                        },
                        [&](uint16_t idx, byte b) {
                            _ram[(idx - 0xA000) + _ram_nbr * 0x2000] = b;
                        },

                    }};
    }

   private:
    byte _rom_nbr;
    std::array<byte, 4 * 0x2000> _ram;
    byte _ram_nbr;
    enum RamRomSelector { Rom = 0, Ram = 1 } _selector;
};

class MBC3 : public Cartridge::Controller {
   public:
    MBC3(std::vector<byte>&& data)
        : Cartridge::Controller(std::move(data)), _rom_nbr(1) {
        _mem_map = {
            {"rom_bank_0",
             0x0000,
             0x1FFF,
             [&](uint16_t idx) { return ReadRom(idx); },
             [&](uint16_t, byte) {
                 std::cout << "RAM/Timer enable not implmented\n";
             }},
            {"rom_bank_0",
             0x2000,
             0x3FFF,
             [&](uint16_t idx) { return ReadRom(idx); },
             [&](uint16_t, byte b) { _rom_nbr = (b == 0 ? 1 : b); }},
            {"rom_bank_switchable",
             0x4000,
             0x5FFF,
             [&](uint16_t idx) {
                 return ReadRom(idx - 0x4000 + _rom_nbr * 0x4000);
             },
             [&](uint16_t, byte b) { _ram_nbr = b & 0b11; }},
            {"rom_bank_switchable",
             0x6000,
             0x7FFF,
             [&](uint16_t idx) {
                 return ReadRom(idx - 0x4000 + _rom_nbr * 0x4000);
             },
             [&](uint16_t, byte) { std::cout << "RTC not implemented\n"; }},
            {
                "cartridge_ram",
                0xA000,
                0xBFFF,
                [&](uint16_t idx) {
                    return _ram[(idx - 0xA000) + _ram_nbr * 0x2000];
                },
                [&](uint16_t idx, byte b) {
                    _ram[(idx - 0xA000) + _ram_nbr * 0x2000] = b;
                },

            }};
    }

   private:
    byte _rom_nbr;
    std::array<byte, 4 * 0x2000> _ram;
    byte _ram_nbr;
};

Cartridge::Cartridge(std::string filename) {
    std::vector<byte> data;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    while (file.good()) {
        data.push_back(file.get());
    }
    std::cout << "Game is " << reinterpret_cast<char*>(&data[0x134])
              << " size=" << std::hex << data.size() << std::endl;
    int mbc = data[0x147];
    std::cout << "CARTRIDGE TYPE: " << std::hex << int(mbc) << "\n";

    switch (mbc) {
        case 0x00:
            _ctrl = std::make_unique<Raw>(std::move(data));
            break;
        case 0x01:
            _ctrl = std::make_unique<MBC1>(std::move(data));
            break;
        case 0x13:
            _ctrl = std::make_unique<MBC3>(std::move(data));
            break;
    }
}

const Cartridge::Addr& Cartridge::Controller::FindAddr(uint16_t addr) const {
    int b = 0;
    int e = _mem_map.size();

    while (true) {
        if (b == e) {
            Error(addr);
        }

        int m = (b + e) / 2;

        if (_mem_map[m]._begin <= addr && addr <= _mem_map[m]._end) {
            return _mem_map[m];
        }

        if (addr < _mem_map[m]._begin) {
            e = m;
        } else {
            b = m + 1;
        }
    }
}
