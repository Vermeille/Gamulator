#include "spritestable.h"
#include "video.h"

int8_t SpritesTable::GetSpritePix(const SpriteAttributes& sprite,
                                  int32_t y,
                                  int32_t x) const {
    int tile = sprite.tileno();
    if (sprite.x_flip()) {
        x = 7 - x;
    }

    if (sprite.y_flip()) {
        y = (_video.lcdc().sprite_size() ? 15 : 7) - y;
    }

    if (_video.lcdc().sprite_size()) {
        tile = tile & ~1;
    }
    uint32_t addr = 0x8000 + tile * 16 + y * 2;

    int8_t l = (_video.vram(addr) >> (7 - x)) & 1;
    int8_t h = (_video.vram(addr + 1) >> (7 - x)) & 1;
    return (h << 1) | l;
}

const SpriteAttributes& SpritesTable::GetSpriteAttr(int sprite_id) const {
    return reinterpret_cast<const SpriteAttributes*>(
        _video.oam_ptr())[sprite_id];
}

void SpritesTable::Render(int line) {
    auto pixs = _video.render_zone().pixs(line);
    const int height = _video.lcdc().sprite_size() ? 16 : 8;
    for (uint32_t i = 0; i < 40; ++i) {
        auto& sprite = GetSpriteAttr(i);
        if (sprite.y_pos() > line || sprite.y_pos() + height < line) {
            continue;
        }

        int y = line - sprite.y_pos();

        for (int x = 0; x < 8; ++x) {
            int color = GetSpritePix(sprite, y, x);

            if (color == 0) {
                continue;
            }

            if (x + sprite.x_pos() < 0 || x + sprite.x_pos() >= 160) {
                continue;
            }
            auto color_val = sprite.obj1_palette()
                                 ? _obj1_palette.GetColor(color)
                                 : _obj0_palette.GetColor(color);
            pixs[x + sprite.x_pos()].Render(
                make_pixel(color_val, sprite.under_bg() ? 1 : 5));
        }
    }
}
