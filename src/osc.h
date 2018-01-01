#pragma once

#include <cstdint>
#include <vector>

class Osc {
   public:
    Osc() : _data(1024), _freq(0), _phase(0) {}

    int FreqToNbSamples(int freq) { return 44100 / freq; }

    void set_freq(int f) {
        _freq = f;
        //_phase = 0;
    }

    int16_t* GenSamples();

    int nb_samples() const { return 1024; }

   private:
    std::vector<int16_t> _data;
    int _freq;
    int _phase;
};
