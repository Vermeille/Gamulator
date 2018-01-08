#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

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
        PixelIterator(sf::Color* color, byte* prio) : _color(color), _z(prio) {}

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
            return Pixel<sf::Color&, byte&>(_color[i], _z[i]);
        }

       private:
        sf::Color* _color;
        byte* _z;
    };

    RenderZone()
        : _window(sf::VideoMode(160 * 4, 144 * 4), "Gameboy"),
          _pixels(160 * 144),
          _z(160 * 144) {
        _texture.create(160, 144);
        _window.setFramerateLimit(60);
        std::fill(_z.begin(), _z.end(), 0);
    }

    void Render();

    PixelIterator pixs(int line = 0) {
        return PixelIterator(&_pixels[160 * line], &_z[160 * line]);
    }

   private:
    sf::RenderWindow _window;
    std::vector<sf::Color> _pixels;
    std::vector<byte> _z;
    sf::Texture _texture;
};

template <class P, class Z>
RenderZone::Pixel<P, Z> make_pixel(P p, Z z) {
    return RenderZone::Pixel<P, Z>(p, z);
}
