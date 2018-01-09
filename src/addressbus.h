#pragma once

#include <array>
#include <functional>
#include <vector>

#include "utils.h"

class Keypad;
class Cartridge;
class Video;
class LinkCable;
class Timer;
class Sound;

class AddressBus {
   public:
    AddressBus(Cartridge& card,
               Video& v,
               LinkCable& lk,
               Keypad& kp,
               Timer& timer,
               Sound& snd);

    void Set(uint16_t index, Data8 val);

    Data8 Get(uint16_t index) const;
    std::string Print(uint16_t index) const;

   private:
    struct Addr {
        std::string _name;
        uint16_t _begin;
        uint16_t _end;
        std::function<byte(uint16_t)> _get;
        std::function<void(uint16_t, byte)> _set;
    };

    const Addr& FindAddr(uint16_t) const;

    byte GetIntByte() const;
    Cartridge& _card;
    Video& _vid;
    LinkCable& _lk;
    Keypad& _kp;
    Timer& _timer;
    Sound& _snd;
    std::array<Data8, 0xFFFF - 0xFF80> _hram;
    std::array<Data8, 0xE000 - 0xC000> _wram0;

    byte _int_mask;
    std::vector<Addr> _mem_map;
    mutable byte _interrupts;
};
