#pragma once

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

    int set_len(int ms) { _samples_to_play = (ms * 44100) / 1000; }
    void set_timed(bool cont) { _timed = cont; }

   private:
    int GetPhaseLen(int period_len) const;

    std::vector<int16_t> _data;
    int _freq;
    int _phase;
    int _duty;
    int _samples_to_play;
    bool _timed;
};
