#pragma once

#include <algorithm>
#include <vector>

#include <SFML/Audio.hpp>

#include "osc.h"
#include "utils.h"

class ToneSweep : public sf::SoundStream {
   public:
    ToneSweep() : _freq(440) { initialize(1, 44100); }

    void set_sweep(byte x) {}
    void set_freq_lo(byte x) {
        _freq = (0xff00 & _freq) | x;
        osc_set_freq();
    }
    void set_freq_hi(byte x) {
        _freq = (_freq & 0xff) | ((x & 0b111) << 8);
        osc_set_freq();
    }

   private:
    void osc_set_freq() { _osc.set_freq(131072 / (2048 - _freq)); }

    virtual bool onGetData(Chunk& data) override {
        data.samples = _osc.GenSamples();
        data.sampleCount = _osc.nb_samples();
        return true;
    }

    virtual void onSeek(sf::Time) override {}

    int _freq;
    Osc _osc;
};

class WaveOutput {
   public:
    void set_nr30_active(byte x) { _active = x & (1 << 7); }
    bool nr30_active() const { return _active << 7; }

    void set_nr31_length(byte x) { _length = x; }
    byte nr31_length() const { return _length; }

    void set_nr32_level(byte x) { _level = (x >> 5); }
    byte nr32_level() const { return _level; }

    void set_nr33_freq_lo(byte x) { _freq = (_freq & 0xff00) | x; }

    byte nr34_freq_hi() const { return _hi_cache; }
    void set_nr34_freq_hi(byte x) {
        _hi_cache = x;
        _freq = (_freq & 0x00ff) | (x << 8);
        _continuous = (x >> 6) & 1;
    }

    int len_as_ms() const { return (256 - _length) * (1. / 256) * 1000; }

    void Play() const {
        if (!_active) {
            return;
        }

        sf::Int16 data[(0xFF3F - 0xFF30) * 2];

        for (int i = 0, j = _data.size() * 2 - 2; i < _data.size();
             ++i, j -= 2) {
            data[j + 1] = _data[i] >> 4;
            data[j] = _data[i] & 0xf;
        }

        for (sf::Int16& x : data) {
            x = std::numeric_limits<int16_t>::min() +
                x * ((std::numeric_limits<int16_t>::max() -
                      std::numeric_limits<int16_t>::min()) /
                     16);
        }

        sf::SoundBuffer sb;
        sb.loadFromSamples(data, _data.size() * 2, 1, 65536 / (2048 - _freq));
        sf::Sound s;
        s.setBuffer(sb);

        s.play();
    }

   private:
    bool _active;
    byte _length;
    byte _level;
    int _freq;
    bool _continuous;
    std::array<byte, 0xFF3F - 0xFF30> _data;

    byte _hi_cache;
};

class Sound {
   public:
    Sound() {
        _tone1.play();
        _tone2.play();
    }

    WaveOutput& wave_output() { return _wav; }

    ToneSweep& channel_1() { return _tone1; }
    ToneSweep& channel_2() { return _tone2; }

   private:
    WaveOutput _wav;
    ToneSweep _tone1;
    ToneSweep _tone2;
};
