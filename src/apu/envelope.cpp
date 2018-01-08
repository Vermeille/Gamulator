#include "envelope.h"

void Envelope::Process(int16_t* buffer, int n) {
    if (_samples_total == 0) {
        for (int i = 0; i < n; ++i) {
            buffer[i] = AdjustVolume(buffer[i], _volume);
        }
        return;
    }

    if (_ascending) {
        for (int i = 0; i < n; ++i) {
            buffer[i] = AdjustVolume(buffer[i], Ascend());
        }
    } else {
        for (int i = 0; i < n; ++i) {
            buffer[i] = AdjustVolume(buffer[i], Descend());
        }
    }
}

int Envelope::Ascend() {
    if (_volume >= 15) {
        return 15;
    }

    if (_samples >= _samples_total) {
        ++_volume;
        _samples = 0;
    } else {
        ++_samples;
    }
    return _volume;
}

int Envelope::Descend() {
    if (_volume == 0) {
        return 0;
    }

    if (_samples >= _samples_total) {
        --_volume;
        _samples = 0;
    } else {
        ++_samples;
    }
    return _volume;
}
