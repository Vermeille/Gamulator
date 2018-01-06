#pragma once

#include <algorithm>
#include <vector>

#include <SFML/Audio.hpp>

#include "toneosc.h"
#include "utils.h"
#include "wavereader.h"

class WaveOutput : public sf::SoundStream {
   public:
    WaveOutput() { initialize(1, 44100); }
    void set_active(byte x) { _wav.set_active((x & (1 << 7)) != 0); }
    bool active() const { return _wav.active() << 7; }

    void set_length(byte x) { _length.set_len((256 - x) * 1000 / 256); }

    void set_level(byte x) {
        _level = x;
        _wav.set_level((x >> 5) & 3);
    }
    byte level() { return _level; }

    void set_freq_lo(byte x) {
        _freq = (_freq & 0xff00) | x;
        wav_set_freq();
    }

    byte freq_hi() const { return _hi_cache; }
    void set_freq_hi(byte x) {
        _hi_cache = x;
        _freq = (_freq & 0xff) | ((int(x) & 7) << 8);
        _length.set_timed(x & (1 << 6));
        wav_set_freq();
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

class Noise {
    LengthCounter _length;
    Envelope _env;
};

class Sound {
   public:
    Sound(bool mute = false) : _mute(mute) {
        if (mute) {
            return;
        }

        _tone1.play();
        _tone2.play();
        _wav.play();
    }

    WaveOutput& wave() { return _wav; }

    ToneOsc& channel_1() { return _tone1; }
    ToneOsc& channel_2() { return _tone2; }

    void set_mixer(byte x) {
        _wav.setVolume((GetBit(x, 6) || GetBit(x, 2)) ? 100 : 0);
    }

    byte on_off() const { return _on_off | 0x70; }
    void set_on_off(byte x) { _on_off = x; }

    ~Sound() {
        _tone1.stop();
        _tone2.stop();
        _wav.stop();
    }

   private:
    bool _mute;
    byte _on_off;
    WaveOutput _wav;
    ToneOsc _tone1;
    ToneOsc _tone2;
};
