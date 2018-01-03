#include "osc.h"

#include <limits>

int16_t* Osc::GenSamples() {
    if (_freq == 0) {
        std::fill(_data.begin(), _data.end(), 0);
        return &_data[0];
    }
    for (int i = 0; i < 1024; ++i) {
        _freq = _sweep.Process(_freq);
        const int period_len = FreqToNbSamples(_freq);
        const int phase_len = GetPhaseLen(period_len);
        if (_phase < phase_len) {
            _data[i] = -1;
        } else {
            _data[i] = 1;
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

    int Sweep::Process(int f) {
        if (_nb == 0 || _sweep_count == 0) {
            return f;
        }

        if (_count == 0) {
            _count = _sweep_count.load();

            if (_ascending) {
                return f + (f / (2 << _nb));
            } else {
                std::cout << "f: " << f << " nb:" << _nb << "\n";
                int r = f - (f / ( 1 << (_nb + 1)));
                if (r <= 0) {
                    return f;
                }
                return r;
                //return f - (f / (2 << _nb));
            }
        }
        --_count;
        return f;
    }

