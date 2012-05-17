#pragma once

#include "addressable.h"
#include "cartridge.h"

class AddressBus : public Addressable
{
    public:
        typedef AddressBus::byte byte;

        AddressBus(Cartridge& card);

        void Set(uint16_t index, byte val) override;

        byte Get(uint16_t index) const override;

    private:
        Cartridge& _card;
};
