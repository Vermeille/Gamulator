#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <vector>

class Osc {
   public:
    Osc() : _data(1024) {}

    int FreqToNbSamples(int freq) { return 44100 / freq; }

    void set_freq(int f) { _freq = f; }
    void set_duty(int d) { _duty = d & 3; }

    int16_t* GenSamples();

    int nb_samples() const { return _data.size(); }

   private:
    int GetPhaseLen(int period_len) const;

    std::vector<int16_t> _data;
    std::atomic<int> _freq;
    std::atomic<int> _phase;
    std::atomic<int> _duty;
};
