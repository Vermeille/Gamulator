#pragma once

#include <atomic>

class Sweep {
   public:
    Sweep() : _nb(0) {}
    void set_time(int ms) { _count = _sweep_count = ms * 44100 / 1000; }
    void set_direction(bool increasing) { _ascending = increasing; }
    void set_nb_of_shifts(int n) { _nb = n; }

    int Process();

    void Reset(int freq) {
        _count = _sweep_count.load();
        _f = freq;
    }

   private:
    std::atomic<int> _sweep_count;
    std::atomic<int> _count;
    std::atomic<int> _f;
    std::atomic<int> _nb;
    std::atomic<bool> _ascending;
};
