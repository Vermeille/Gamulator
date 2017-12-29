#pragma once

class Keypad {
   public:
    using byte = unsigned char;

    void set_joyp(byte v) { _joyp = (0b1100'1111 & _joyp) | (0b0011'0000 & v); }
    byte joyp() const { return 0xF; }

   private:
    byte _joyp;
};
