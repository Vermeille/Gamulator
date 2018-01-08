#pragma once

#include <algorithm>
#include <vector>

#include <SFML/Audio.hpp>

#include "toneosc.h"
#include "utils.h"
#include "waveoutput.h"

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
