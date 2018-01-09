#pragma once

#include <cassert>

#include <SDL2/SDL.h>

class Keypad {
   public:
    using byte = unsigned char;

    Keypad() : _keys(0) {}

    void set_joyp(byte v) {
        _dir_keys = ((v >> 4) & 1) == 0;
        _buttons_keys = ((v >> 5) & 1) == 0;
    }

    byte joyp() {
        RefreshKeys();
        if (_dir_keys) {
            return ~(~0x1f | (_keys >> 4));
        } else {
            return ~(~0x2f | (_keys & 0xf));
        }
    }

    bool poweroff() const { return _keys & 0x100; }

   private:
    void RefreshKeys() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN) {
                _keys |= ((event.key.keysym.sym == SDLK_DOWN) << 7) |
                         ((event.key.keysym.sym == SDLK_UP) << 6) |
                         ((event.key.keysym.sym == SDLK_LEFT) << 5) |
                         ((event.key.keysym.sym == SDLK_RIGHT) << 4) |
                         ((event.key.keysym.sym == SDLK_RETURN) << 3) |
                         ((event.key.keysym.sym == SDLK_BACKSPACE) << 2) |
                         ((event.key.keysym.sym == SDLK_d) << 1) |
                         ((event.key.keysym.sym == SDLK_s) << 0);
            } else if (event.type == SDL_KEYUP) {
                _keys &= ~(((event.key.keysym.sym == SDLK_DOWN) << 7) |
                           ((event.key.keysym.sym == SDLK_UP) << 6) |
                           ((event.key.keysym.sym == SDLK_LEFT) << 5) |
                           ((event.key.keysym.sym == SDLK_RIGHT) << 4) |
                           ((event.key.keysym.sym == SDLK_RETURN) << 3) |
                           ((event.key.keysym.sym == SDLK_BACKSPACE) << 2) |
                           ((event.key.keysym.sym == SDLK_d) << 1) |
                           ((event.key.keysym.sym == SDLK_s) << 0));
            }
        }
    }

    bool _dir_keys;
    bool _buttons_keys;
    int _keys;
};
