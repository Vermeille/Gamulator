#include "renderzone.h"

void RenderZone::Render() {
#if 0
    sf::Event event;
    while (_window.pollEvent(event)) {
        // Request for closing the window
        if (event.type == sf::Event::Closed) {
            _window.close();
            exit(0);
        }
    }
#endif
    _tx.Update(&_pixels[0]);
    _win.Clear();
    _tx.Draw(_win);
    _win.Display();

    std::fill(_z.begin(), _z.end(), 0);
}
