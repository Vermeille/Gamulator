#pragma once

#include <functional>
#include <vector>

#include <sdl.h>

#include "color.h"
#include "utils.h"

inline void InitVideo() {
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);
}

class RenderZone {
   public:
    template <class Px, class Z>
    class Pixel {
       public:
        Pixel(Px c, Z z) : _color(c), _z(z) {}

        template <class Px2, class Z2>
        void operator=(const Pixel<Px2, Z2>& p) {
            _color = p.color();
            _z = p.z();
        }

        template <class Px2, class Z2>
        void Render(const Pixel<Px2, Z2>& p) {
            if (p.z() >= _z) {
                _z = p.z();
                _color = p.color();
            }
        }

        Px color() const { return _color; }
        void set_color(Px p) { _color = p; }

        byte z() const { return _z; }
        void set_z(byte z) { _z = z; }

       private:
        Px _color;
        Z _z;
    };

    class PixelIterator {
       public:
        PixelIterator(Color* color, byte* prio) : _color(color), _z(prio) {}

        void operator++() {
            ++_color;
            ++_z;
        }

        bool operator==(const PixelIterator& it) const {
            return it._color == _color && it._z == _z;
        }

        bool operator!=(const PixelIterator& it) const {
            return !operator==(it);
        }

        auto operator*() const {
            return Pixel<decltype(std::ref(*_color)), decltype(std::ref(*_z))>(
                std::ref(*_color), std::ref(*_z));
        }

        auto operator[](int i) const {
            return Pixel<Color&, byte&>(_color[i], _z[i]);
        }

       private:
        Color* _color;
        byte* _z;
    };

    RenderZone() : _tx(_win), _pixels(160 * 144), _z(160 * 144) {
        std::fill(_z.begin(), _z.end(), 0);
    }

    void Render();

    PixelIterator pixs(int line = 0) {
        return PixelIterator(&_pixels[160 * line], &_z[160 * line]);
    }

   private:
    struct Window {
        Window()
            : _win(SDL_CreateWindow("Gameboy",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    160 * 4,
                                    144 * 4,
                                    0)),
              _renderer(
                  SDL_CreateRenderer(_win, -1, SDL_RENDERER_PRESENTVSYNC)) {}
        ~Window() {
            SDL_DestroyRenderer(_renderer);
            SDL_DestroyWindow(_win);
        }
        operator SDL_Window*() const { return _win; }
        operator SDL_Renderer*() const { return _renderer; }

        void Clear() { SDL_RenderClear(_renderer); }
        void Display() { SDL_RenderPresent(_renderer); }

       private:
        SDL_Window* _win;
        SDL_Renderer* _renderer;
    };

    struct Texture {
        Texture(Window& w)
            : _texture(SDL_CreateTexture(w,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         160,
                                         144)) {}

        ~Texture() { SDL_DestroyTexture(_texture); }

        operator SDL_Texture*() const { return _texture; }

        void Update(Color* c) {
            SDL_UpdateTexture(_texture, nullptr, c, 160 * sizeof(Color));
        }

        void Draw(Window& w) { SDL_RenderCopy(w, _texture, nullptr, nullptr); }

       private:
        SDL_Texture* _texture;
    };

    Window _win;
    Texture _tx;
    std::vector<Color> _pixels;
    std::vector<byte> _z;
};

template <class P, class Z>
RenderZone::Pixel<P, Z> make_pixel(P p, Z z) {
    return RenderZone::Pixel<P, Z>(p, z);
}
