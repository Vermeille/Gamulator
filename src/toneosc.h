#pragma once

#include <SFML/Audio.hpp>

#include "lengthcounter.h"
#include "osc.h"

class Envelope {
   public:
    Envelope()
        : _volume(15), _ascending(true), _samples_total(1), _samples(0) {}

    void set_volume(byte x) { _start_volume = _volume = x & 0xf; }
    void set_direction(bool ascending) { _ascending = ascending; }
    void set_len(int ms) { _samples_total = ms * 44100 / 1000; }

    void Process(int16_t* buffer, int n) {
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

    void Reset() {
        _samples = 0;
        _volume = _start_volume.load();
    }

   private:
    int Ascend() {
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

    int Descend() {
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

    int16_t AdjustVolume(int x, int vol) const {
        return x * ((vol * 32767) / 15);
    }

    std::atomic<int> _volume;
    std::atomic<int> _start_volume;
    std::atomic<bool> _ascending;
    std::atomic<int> _samples_total;
    std::atomic<int> _samples;
};

class ToneOsc : public sf::SoundStream {
   public:
    ToneOsc() : _freq(440) { initialize(1, 44100); }

    void set_sweep(byte) {}
    void set_len_pattern(byte x) {
        _len_pattern_cache = x;
        _osc.set_duty(x >> 6);
        set_len(x & 0x3f);
    }

    byte len_pattern() const { return _len_pattern_cache | 0x3F; }

    byte envelope() const { return _env_cache; }
    void set_envelope(byte x) {
        _env_cache = x;
        _env.set_volume(x >> 4);
        _env.set_direction(x & (1 << 3));
        _env.set_len((x & 7) * 1000 / 64);
    }

    byte freq_lo() const { return 0xff; }

    void set_freq_lo(byte x) {
        _freq = (0xff00 & _freq) | x;
        osc_set_freq();
        _env.Reset();
    }

    byte freq_hi() const {
        return ((!_length.ended() | !_length.timed()) << 6) | 0xBF;
    }
    void set_freq_hi(byte x) {
        _freq_hi_cache = x;
        _freq = (_freq & 0xff) | ((x & 0b111) << 8);
        osc_set_freq();
        _length.set_timed(x & (1 << 6));
        if (x & (1 << 7)) {
            _env.Reset();
            _length.set_timed(false);
        }
    }

   private:
    void osc_set_freq() { _osc.set_freq(131072 / (2048 - _freq)); }
    void set_len(int val) { _length.set_len((64 - val) * 1000 / 256); }

    virtual bool onGetData(Chunk& data) override {
        int16_t* buffer = _osc.GenSamples();
        int samples = _osc.nb_samples();

        _length.Process(buffer, samples);
        _env.Process(buffer, samples);

        data.samples = buffer;
        data.sampleCount = samples;
        return true;
    }

    virtual void onSeek(sf::Time) override {}

    int _freq;
    Osc _osc;
    LengthCounter _length;
    Envelope _env;
    byte _len_pattern_cache;
    byte _env_cache;
    byte _freq_hi_cache;
};
