#pragma once

#include <cassert>
#include "utils.h"

class LinkCable {
   public:
    using byte = unsigned char;

    void Clock() { _transferred = false; }

    byte Read() {
        // TODO: implement
        return 0xFF;
    }

    void Send(byte b) { _data = b; }

    bool transferred() const { return _transferred; }

    byte serial_control() const { return _serial_control; }
    void set_serial_control(byte v) {
        if (v == 0x81) {
            _serial_control = 1;
            _transferred = true;
            serial << _data;
            std::cout.flush();
        }
    }

   private:
    byte _serial_control;
    byte _transferred;
    byte _data;
};
