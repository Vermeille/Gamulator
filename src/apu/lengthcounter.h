#pragma once

#include <atomic>

class LengthCounter {
   public:
    void set_timed(bool cont) { _timed = cont; }
    bool timed() const { return _timed.load(); }

    void set_len(int ms) {
        _total_samples = _samples_to_play = (ms * 44100) / 1000;
    }

    void Reset() { _samples_to_play = _total_samples.load(); }

    void Process(int16_t* data, int n) {
        if (!_timed) {
            return;
        }
        for (int i = 0; i < n; ++i) {
            if (_samples_to_play == 0) {
                data[i] = 0;
            } else {
                --_samples_to_play;
            }
        }
    }

    bool ended() const { return _samples_to_play == 0; }

   private:
    std::atomic<int> _samples_to_play;
    std::atomic<int> _total_samples;
    std::atomic<bool> _timed;
};
