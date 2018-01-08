#pragma once

#include <atomic>

#include "utils.h"

class Envelope {
   public:
    Envelope()
        : _volume(15), _ascending(true), _samples_total(1), _samples(0) {}

    void set_volume(byte x) { _start_volume = _volume = x & 0xf; }
    void set_direction(bool ascending) { _ascending = ascending; }
    void set_len(int ms) { _samples_total = ms * 44100 / 1000; }

    void Process(int16_t* buffer, int n);

    void Reset() {
        _samples = 0;
        _volume = _start_volume.load();
    }

   private:
    int Ascend();

    int Descend();

    int16_t AdjustVolume(int x, int vol) const {
        return x * ((vol * 32767) / 15);
    }

    std::atomic<int> _volume;
    std::atomic<int> _start_volume;
    std::atomic<bool> _ascending;
    std::atomic<int> _samples_total;
    std::atomic<int> _samples;
};
