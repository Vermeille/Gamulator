#pragma once

#include "video.h"
#include "addressable.h"
#include "cartridge.h"

class AddressBus : public Addressable
{
    public:
        typedef AddressBus::byte byte;

        AddressBus(Cartridge& card, Video& v);

        void Set(uint16_t index, byte val) override;

        byte Get(uint16_t index) const override;

    private:
        Cartridge& _card;
        Video& _vid;
};
