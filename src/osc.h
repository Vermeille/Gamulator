#pragma once

#include <iostream>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <vector>

class Sweep {
   public:
    Sweep() : _nb(0) {}
    void set_time(int ms) { std::cout <<std::dec << "SWEEP TIME: " << ms << "ms\n";_count = _sweep_count = ms * 44100 / 1000;  std::cout << _sweep_count << " SAMPLES\n";}
    void set_direction(bool increasing) { _ascending = increasing; }
    void set_nb_of_sweeps(int n) { _remaining = _nb = n; }

    int Process(int f);

    void Reset() {
        _count = _sweep_count.load();
        _remaining = _nb.load();
    }

   private:
    std::atomic<int> _sweep_count;
    std::atomic<int> _count;
    std::atomic<int> _remaining;
    std::atomic<int> _nb;
    std::atomic<bool> _ascending;
};

class Osc {
   public:
    Osc() : _data(1024) {}

    int FreqToNbSamples(int freq) { return 44100 / freq; }

    void set_freq(int f) { _freq = f; }
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
