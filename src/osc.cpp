#include "osc.h"

#include <limits>

int16_t* Osc::GenSamples() {
    if (_freq == 0) {
        std::fill(_data.begin(), _data.end(), 0);
        return &_data[0];
    }
    const int period_len = FreqToNbSamples(_freq);
    const int phase_len = GetPhaseLen(period_len);
    for (int i = 0; i < 1024; ++i) {
        if (_timed) {
            if (_samples_to_play == 0) {
                _data[i] = 0;
                continue;
            } else {
                --_samples_to_play;
            }
        }

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

int Osc::GetPhaseLen(int period_len) const {
    switch (_duty) {
        default:
            assert(false);
        case 0:
            return period_len / 8;
        case 1:
            return period_len / 4;
        case 2:
            return period_len / 2;
        case 3:
            return (period_len * 3) / 4;
    }
}
