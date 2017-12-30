#pragma once

#include <cassert>

#include <SFML/Graphics.hpp>

class Keypad {
   public:
    using byte = unsigned char;

    void set_joyp(byte v) {
        _dir_keys = ((v >> 4) & 1) == 0;
        _buttons_keys = ((v >> 5) & 1) == 0;
    }

    byte joyp() const {
        if (_dir_keys) {
            return ~(~0x1f |
                     (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) << 3) |
                     (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) << 2) |
                     (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) << 1) |
                     (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) << 0));
        } else {
            return ~(
                ~0x2f |
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Return) << 3) |
                (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace) << 2) |
                (sf::Keyboard::isKeyPressed(sf::Keyboard::D) << 1) |
                (sf::Keyboard::isKeyPressed(sf::Keyboard::S) << 0));
        }
    }

   private:
    bool _dir_keys;
    bool _buttons_keys;
};
