#pragma once

#include "palette.h"

class Video;

class SpriteAttributes {
   public:
    SpriteAttributes(byte y_pos, byte x_pos, byte tileno, byte flags)
        : _y_pos(y_pos), _x_pos(x_pos), _tileno(tileno), _flags(flags) {}
    SpriteAttributes() = default;

    int y_pos() const { return _y_pos - 16; }
    int x_pos() const { return _x_pos - 8; }
    byte tileno() const { return _tileno; }

    bool y_flip() const { return _flags & (1 << 6); }
    bool x_flip() const { return _flags & (1 << 5); }
    bool obj1_palette() const { return _flags & (1 << 4); }
    bool under_bg() const { return _flags & (1 << 7); }

   private:
    byte _y_pos;
    byte _x_pos;
    byte _tileno;
    byte _flags;
};

class SpritesTable {
   public:
    SpritesTable(Video& video) : _video(video) {}

    void Render(int line);

    byte obj0_palette() const { return _obj0_palette.Get(); }
    void set_obj0_palette(byte x) { _obj0_palette.Set(x); }

    byte obj1_palette() const { return _obj1_palette.Get(); }
    void set_obj1_palette(byte x) { _obj1_palette.Set(x); }

   private:
    int8_t GetSpritePix(const SpriteAttributes& sprite,
                        int32_t y,
                        int32_t x) const;
    const SpriteAttributes& GetSpriteAttr(int sprite_id) const;

    Palette _obj0_palette;
    Palette _obj1_palette;
    Video& _video;
};
