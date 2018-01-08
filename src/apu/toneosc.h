#pragma once

#include <SFML/Audio.hpp>

#include "envelope.h"
#include "lengthcounter.h"
#include "osc.h"

class ToneOsc : public sf::SoundStream {
   public:
    ToneOsc() : _freq(440) { initialize(1, 44100); }

    byte sweep() const { return 0x80 | _sweep_cache; }
    void set_sweep(byte x) {
        _sweep_cache = x;
        _osc.sweep().set_time((((int(x) >> 4) & 7) * 1000) / 128);
        _osc.sweep().set_direction(!GetBit(x, 3));
        _osc.sweep().set_nb_of_shifts(x & 7);
    }
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
            _osc.sweep().Reset(_osc.freq());
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
    Sweep _sweep;
    byte _sweep_cache;
    byte _len_pattern_cache;
    byte _env_cache;
    byte _freq_hi_cache;
};
