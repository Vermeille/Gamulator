#pragma once

#include <functional>

#include "addressable.h"
#include "cartridge.h"
#include "video.h"

class AddressBus : public Addressable {
   public:
    typedef AddressBus::byte byte;

    AddressBus(Cartridge& card, Video& v);

    void Set(uint16_t index, byte val) override;

    byte Get(uint16_t index) const override;
    std::string Print(uint16_t index) const override;

   private:
    struct Addr {
        std::string _name;
        int _begin;
        int _end;
        std::function<byte(uint16_t)> _get;
        std::function<void(uint16_t, byte)> _set;
    };

    const Addr& FindAddr(uint16_t) const;

    byte GetIntByte() const;
    Cartridge& _card;
    Video& _vid;
    byte _hram[0xFFFF - 0xFF80 + 1];
    byte _wram0[0xE000 - 0xC000];
    byte _int_mask;
    std::vector<Addr> _mem_map;
    mutable byte _interrupts;
};
