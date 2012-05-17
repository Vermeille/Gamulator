#pragma once

#include "addressable.h"
#include <iostream>
#include <vector>
#include <fstream>

class Cartridge : public Addressable
{
    public:
        Cartridge(std::string filename);
        virtual void Set(word index, byte val) override;
        virtual byte Get(word index) const override;

    private:
        std::vector<byte> _data;
        std::vector<byte> _ram;
        unsigned char _mbc;
        unsigned char _rom_bank;
        unsigned char _ram_bank;
        enum { Ram, Rom } _selector;
};

