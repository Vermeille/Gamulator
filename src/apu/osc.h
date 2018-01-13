#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include "sweep.h"

class Osc {
   public:
    Osc(int samples) : _data(samples) {}

    int FreqToNbSamples(int freq) { return 44100 / freq; }

    void set_freq(int f) { _freq = f; }
    int freq() const { return _freq; }
    void set_duty(int d) { _duty = d & 3; }

    int16_t* GenSamples();

    int nb_samples() const { return _data.size(); }

    Sweep& sweep() { return _sweep; }

   private:
    int GetPhaseLen(int period_len) const;

    Sweep _sweep;
    std::vector<int16_t> _data;
    std::atomic<int> _freq;
    std::atomic<int> _phase;
    std::atomic<int> _duty;
};
