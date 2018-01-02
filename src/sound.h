#pragma once

#include <algorithm>
#include <vector>

#include <SFML/Audio.hpp>

#include "toneosc.h"
#include "utils.h"

class WaveReader {
   public:
    WaveReader() : _active(true), _freq(440), _cache(1024) {}

    int16_t* GenSamples() {
        if (!_active) {
            std::fill(_cache.begin(), _cache.end(), 0);
            return &_cache[0];
        }

        for (int i = 0; i < _cache.size(); ++i) {
            if (_cursor >= _data.size() * 2) {
                _cursor = 0;
            }
            _cache[i] = AdjustLevel(
                NibbleToInt16(NthNibble(_cursor * _data.size() * 2)));
            _cursor += _freq / 44100.;
            if (_cursor >= 1) {
                _cursor -= 1;
            }
        }

        return &_cache[0];
    }

    int nb_samples() const { return _cache.size(); }

    void Write(uint16_t addr, byte x) { _data[addr - 0xFF30] = x; }
    byte Read(uint16_t addr) const { return _data[addr - 0xFF30]; }

    void set_freq(int f) { _freq = f; }

    void set_level(byte lvl) { _level = lvl & 3; }
    void set_active(bool b) { _active = b; }
    bool active() const { return _active; }

   private:
    byte NthNibble(int16_t n) const {
        byte b = _data[n / 2];
        if (n % 2) {
            return b >> 4;
        } else {
            return b & 0xf;
        }
    }

    int16_t NibbleToInt16(byte x) const {
        return std::numeric_limits<int16_t>::min() + x * (65535 / 16);
    }

    int16_t AdjustLevel(int16_t x) const {
        switch (x) {
            default:
                assert(false);
            case 0:
                return 0;
            case 1:
                return x;
            case 2:
                return x / 2;
            case 3:
                return x / 4;
        }
    }

    float _cursor;
    bool _active;
    int _freq;
    byte _level;
    std::vector<int16_t> _cache;
    std::array<byte, 0xFF40 - 0xFF30> _data;
};

class WaveOutput : public sf::SoundStream {
   public:
    WaveOutput() { initialize(1, 44100); }
    void set_active(byte x) { _wav.set_active(x & (1 << 7)); }
    bool active() const { return _wav.active(); }

    void set_length(byte x) { _length.set_len((256 - x) * 1000 / 256); }

    void set_level(byte x) {
        _level = x;
        _wav.set_level((x >> 6) & 3);
    }
    byte level() { return _level; }

    void set_freq_lo(byte x) {
        _freq = (_freq & 0xff00) | x;
        wav_set_freq();
    }

    byte freq_hi() const { return _hi_cache; }
    void set_freq_hi(byte x) {
        _hi_cache = x;
        _freq = (_freq & 0xff) | ((x & 7) << 8);
        wav_set_freq();
        _length.set_timed(x & (1 << 6));
        if (x & (1 << 7)) {
            _length.Reset();
        }
    }

    void Write(uint16_t addr, byte x) { _wav.Write(addr, x); }
    byte Read(uint16_t addr) const { return _wav.Read(addr); }

   private:
    void wav_set_freq() { _wav.set_freq(65536 / (2048 - _freq)); }

    virtual bool onGetData(Chunk& data) override {
        int16_t* buffer = _wav.GenSamples();
        int nb = _wav.nb_samples();

        _length.Process(buffer, nb);

        data.samples = buffer;
        data.sampleCount = nb;
        return true;
    }

    virtual void onSeek(sf::Time) override {}

    WaveReader _wav;
    LengthCounter _length;
    int _freq;
    int _level;
    byte _hi_cache;
};

class Sound {
   public:
    Sound() {
        _tone1.play();
        _tone2.play();
        _wav.play();
    }

    WaveOutput& wave() { return _wav; }

    ToneOsc& channel_1() { return _tone1; }
    ToneOsc& channel_2() { return _tone2; }

   private:
    WaveOutput _wav;
    ToneOsc _tone1;
    ToneOsc _tone2;
};
