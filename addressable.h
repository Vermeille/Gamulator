#pragma once

#include <cstdint>

class Addressable
{
    public:
        typedef unsigned char byte;
        typedef uint16_t word;

        virtual void Set(word index, byte val) = 0;
        virtual byte Get(word index) const = 0;
};
