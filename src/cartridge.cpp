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
    Raw(std::vector<byte>&& data)
        : Cartridge::Controller(std::move(data), 0xC000 - 0xA000) {
        _mem_map = {
            {"rom_bank_0",
             0x0000,
             0x3FFF,
             [&](uint16_t idx) { return Rom(idx); },
             [&](uint16_t, byte) {
                 cerror << "Can't switch ROM bank without MBC\n";
             }},
            {"rom_bank_switchable",
             0x4000,
             0x7FFF,
             [&](uint16_t idx) { return Rom(idx); },
             [&](uint16_t, byte) {
                 cerror << "Can't switch ROM bank without MBC\n";
             }},
            {"ram_bank",
             0xA000,
             0xBFFF,
             [&](uint16_t idx) { return Ram(idx - 0xA000); },
             [&](uint16_t idx, byte b) { Ram(idx - 0xA000) = b; }},
        };
    }
};

class MBC1 : public Cartridge::Controller {
   public:
    MBC1(std::vector<byte>&& data)
        : Cartridge::Controller(std::move(data), 4 * 0x2000),
          _lo(1),
          _hi(0),
          _selector(RamRomSelector::Rom),
          _ram_enable(false) {
        _mem_map = {{"rom_bank_0",
                     0x0000,
                     0x1FFF,
                     [&](uint16_t idx) {
                         if (_selector == RamRomSelector::Rom) {
                             return Rom(idx);
                         } else {
                             return Rom(idx + (rom_bank() & 0xE0) * 0x4000);
                         }
                     },
                     [&](uint16_t, byte x) { _ram_enable = (x & 0xF) == 0xA; }},
                    {"rom_bank_0/low_bits",
                     0x2000,
                     0x3FFF,
                     [&](uint16_t idx) {
                         if (_selector == RamRomSelector::Rom) {
                             return Rom(idx);
                         } else {
                             return Rom(idx + (rom_bank() & 0xE0) * 0x4000);
                         }
                     },
                     [&](uint16_t, byte b) {
                         b = b & 0b1'1111;
                         b = (b == 0) ? 1 : b;
                         _lo = b;
                     }},
                    {"rom_bank_switchable/high_bits",
                     0x4000,
                     0x5FFF,
                     [&](uint16_t idx) {
                         return Rom(idx - 0x4000 + rom_bank() * 0x4000);
                     },
                     [&](uint16_t, byte b) { _hi = b & 3; }},
                    {"rom_bank_switchable/ram_rom_select",
                     0x6000,
                     0x7FFF,
                     [&](uint16_t idx) {
                         return Rom(idx - 0x4000 + rom_bank() * 0x4000);
                     },
                     [&](uint16_t, byte b) {
                         _selector = RamRomSelector(b & 1);
                         _hi = 0;
                     }},
                    {"cartridge_ram",
                     0xA000,
                     0xBFFF,
                     [&](uint16_t idx) -> byte {
                         if (!_ram_enable) {
                             return 0xFF;
                         }

                         return Ram((idx - 0xA000) + ram_bank() * 0x2000);
                     },
                     [&](uint16_t idx, byte b) {
                         if (!_ram_enable) {
                             return;
                         }

                         Ram((idx - 0xA000) + ram_bank() * 0x2000) = b;
                     }}};
    }

   private:
    int ram_bank() const {
        if (_selector == RamRomSelector::Rom) {
            return 0;
        } else {
            return _hi;
        }
    }

    int rom_bank() const { return ((_hi << 5) | _lo) & (rom_banks() - 1); }

    byte _lo;
    byte _hi;
    enum class RamRomSelector { Rom = 0, Ram = 1 } _selector;
    bool _ram_enable;
};

class MBC3 : public Cartridge::Controller {
   public:
    MBC3(std::vector<byte>&& data)
        : Cartridge::Controller(std::move(data), 4 * 0x2000 + 48),
          _rom_nbr(1),
          _rtc_select(RTCSelect::None) {
        _mem_map = {{"rom_bank_0",
                     0x0000,
                     0x1FFF,
                     [&](uint16_t idx) { return Rom(idx); },
                     [&](uint16_t, byte) {
                         std::cout << "RAM/Timer enable not implmented\n";
                     }},
                    {"rom_bank_0",
                     0x2000,
                     0x3FFF,
                     [&](uint16_t idx) { return Rom(idx); },
                     [&](uint16_t, byte b) { _rom_nbr = (b == 0 ? 1 : b); }},
                    {"rom_bank_switchable",
                     0x4000,
                     0x5FFF,
                     [&](uint16_t idx) {
                         return Rom(idx - 0x4000 + _rom_nbr * 0x4000);
                     },
                     [&](uint16_t, byte b) {
                         if (b < 8) {
                             _ram_nbr = b & 0b11;
                             _rtc_select = RTCSelect::None;
                         } else {
                             std::cout << "RTC SELECT: " << int(b) << "\n";
                             _rtc_select = RTCSelect(b);
                         }
                     }},
                    {"rom_bank_switchable",
                     0x6000,
                     0x7FFF,
                     [&](uint16_t idx) {
                         return Rom(idx - 0x4000 + _rom_nbr * 0x4000);
                     },
                     [&](uint16_t, byte x) {
                         std::cout << "RTC LATCH: " << int(x) << "\n";
                     }},
                    {
                        "cartridge_ram",
                        0xA000,
                        0xBFFF,
                        [&](uint16_t idx) -> byte {
                            if (_rtc_select == RTCSelect::None) {
                                return Ram((idx - 0xA000) + _ram_nbr * 0x2000);
                            } else {
                                std::cout << "READ RTC\n";
                                std::time_t now = std::time(nullptr);
                                std::tm* time = std::localtime(&now);
                                switch (_rtc_select) {
                                    case RTCSelect::Sec:
                                        return time->tm_sec;
                                    case RTCSelect::Min:
                                        return time->tm_min;
                                    case RTCSelect::Hour:
                                        return time->tm_hour;
                                    case RTCSelect::DayLow:
                                        return time->tm_mday & 0xFF;
                                    case RTCSelect::DayHigh:
                                        return GetBit(time->tm_mday, 9);
                                    default:
                                        return 0xFF;
                                }
                            }
                        },
                        [&](uint16_t idx, byte b) {
                            if (_rtc_select == RTCSelect::None) {
                                Ram((idx - 0xA000) + _ram_nbr * 0x2000) = b;
                            } else {
                                std::cout << "WRITE RTC\n";
                                std::time_t now = std::time(nullptr);
                                std::tm* time = std::localtime(&now);
                                switch (_rtc_select) {
                                    case RTCSelect::Sec:
                                    case RTCSelect::Min:
                                    case RTCSelect::Hour:
                                    case RTCSelect::DayLow:
                                    case RTCSelect::DayHigh:
                                    default:
                                        return;
                                }
                            }
                        },

                    }};
    }

   private:
    struct RTCRegs {
        int secs;
        int mins;
        int hours;
        int days;
        int days_high;
        int secs_latch;
        int mins_latch;
        int hours_latch;
        int days_latch;
        int days_high_latch;
        int timestamp;
        int dummy;
    };

    MBC3::RTCRegs& GetRTC() {
        return reinterpret_cast<RTCRegs&>(Ram(4 * 0x2000));
    }

    enum class RTCSelect {
        None = 0,
        Sec = 0x8,
        Min = 0x9,
        Hour = 0xA,
        DayLow = 0xB,
        DayHigh = 0xC
    } _rtc_select;
    byte _rom_nbr;
    byte _ram_nbr;
};

class MBC5 : public Cartridge::Controller {
   public:
    MBC5(std::vector<byte>&& data)
        : Cartridge::Controller(std::move(data), 0x80 * 0x2000), _rom_nbr(1) {
        _mem_map = {
            {"rom_bank_0",
             0x0000,
             0x1FFF,
             [&](uint16_t idx) { return Rom(idx); },
             [&](uint16_t, byte) {
                 std::cout << "RAM/Timer enable not implmented\n";
             }},
            {"rom_bank_0",
             0x2000,
             0x2FFF,
             [&](uint16_t idx) { return Rom(idx); },
             [&](uint16_t, byte b) { _rom_nbr = (_rom_nbr & ~(1 << 9)) | b; }},
            {"rom_bank_0",
             0x3000,
             0x3FFF,
             [&](uint16_t idx) { return Rom(idx); },
             [&](uint16_t, byte idx) {
                 _rom_nbr = (_rom_nbr & 0xff) | ((idx & 1) << 9);
             }},
            {"rom_bank_switchable",
             0x4000,
             0x5FFF,
             [&](uint16_t idx) {
                 return Rom(idx - 0x4000 + _rom_nbr * 0x4000);
             },
             [&](uint16_t, byte b) { _ram_nbr = b & 0xf; }},
            {"rom_bank_switchable",
             0x6000,
             0x7FFF,
             [&](uint16_t idx) {
                 return Rom(idx - 0x4000 + _rom_nbr * 0x4000);
             },
             [&](uint16_t, byte) { std::cout << "RTC not implemented\n"; }},
            {
                "cartridge_ram",
                0xA000,
                0xBFFF,
                [&](uint16_t idx) {
                    return Ram((idx - 0xA000) + _ram_nbr * 0x2000);
                },
                [&](uint16_t idx, byte b) {
                    Ram((idx - 0xA000) + _ram_nbr * 0x2000) = b;
                },

            }};
    }

   private:
    int _rom_nbr;
    byte _ram_nbr;
};

std::vector<byte> Cartridge::LoadGame(const std::string& filename) {
    std::vector<byte> data;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    while (true) {
        auto x = file.get();
        if (!file.good()) {
            break;
        }
        data.push_back(x);
    }
    return data;
}

Cartridge::Cartridge(std::string filename) : _has_battery(false) {
    auto data = LoadGame(filename);
    _game_name = std::string(reinterpret_cast<char*>(&data[0x134]));
    std::replace(_game_name.begin(), _game_name.end(), ' ', '_');

    std::cout << "Game is " << _game_name << " size=" << std::hex << data.size()
              << std::endl;
    int mbc = data[0x147];
    std::cout << "CARTRIDGE TYPE: " << std::hex << int(mbc) << "\n";

    switch (mbc) {
        case 0x00:
            _ctrl = std::make_unique<Raw>(std::move(data));
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            _ctrl = std::make_unique<MBC1>(std::move(data));
            _has_battery = mbc == 0x3;
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            _ctrl = std::make_unique<MBC3>(std::move(data));
            _has_battery = ((mbc & 1) ^ ((mbc >> 1) & 1)) == 0;
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            _ctrl = std::make_unique<MBC5>(std::move(data));
            _has_battery = mbc == 0x1B || mbc == 0x1E;
            break;
    }

    if (_has_battery) {
        _ctrl->LoadRam(_game_name + ".save");
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

void Cartridge::Controller::LoadRam(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        return;
    }

    file.read(reinterpret_cast<char*>(&_ram[0]), _ram.size());
}

void Cartridge::Controller::SaveRam(const std::string& filename) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        return;
    }

    file.write(reinterpret_cast<char*>(&_ram[0]), _ram.size());
}
