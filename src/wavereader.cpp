#include "wavereader.h"

int16_t* WaveReader::GenSamples() {
    if (!_active) {
        std::fill(_cache.begin(), _cache.end(), 0);
        return &_cache[0];
    }

    for (unsigned int i = 0; i < _cache.size(); ++i) {
        _cache[i] =
            AdjustLevel(NibbleToInt16(NthNibble(_cursor * _data.size() * 2)));
        _cursor += _freq / 44100.;
        if (_cursor >= 1) {
            _cursor -= 1;
        }
    }

    return &_cache[0];
}
