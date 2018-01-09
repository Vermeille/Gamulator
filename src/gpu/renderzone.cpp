#include "renderzone.h"

void RenderZone::Render() {
    _tx.Update(&_pixels[0]);
    _win.Clear();
    _tx.Draw(_win);
    _win.Display();

    std::fill(_z.begin(), _z.end(), 0);
}
