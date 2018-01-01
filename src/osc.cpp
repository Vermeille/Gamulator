#include "osc.h"

#include <limits>

int16_t* Osc::GenSamples() {
    if (_freq == 0) {
        std::fill(_data.begin(), _data.end(), 0);
        return &_data[0];
    }
    const int period_len = FreqToNbSamples(_freq);
    const int phase_len = period_len / 2;
    for (int i = 0; i < 1024; ++i) {
        if (_phase < phase_len) {
            _data[i] = std::numeric_limits<int16_t>::min();
        } else {
            _data[i] = std::numeric_limits<int16_t>::max();
        }

        ++_phase;
        if (_phase >= period_len) {
            _phase = 0;
        }
    }
    return &_data[0];
}
