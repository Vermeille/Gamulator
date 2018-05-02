#include "renderzone.h"

#include <thread>

void RenderZone::Render() {
    if (!_max_speed) {
        //*
        std::this_thread::sleep_until(
            _frame_start + std::chrono::nanoseconds(1000000000 / 60));
        _frame_start = std::chrono::high_resolution_clock::now();
        //*/
        // SDL_Delay(16);
    }
    _tx.Update(&_pixels[0]);
    _win.Clear();
    _tx.Draw(_win);
    _win.Display();

    std::fill(_z.begin(), _z.end(), 0);
}
