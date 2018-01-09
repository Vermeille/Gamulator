#pragma once

#include <algorithm>
#include <vector>

#include "chunk.h"
#include "sdl.h"
#include "toneosc.h"
#include "utils.h"
#include "waveoutput.h"

inline void InitAudio() { SDL_InitSubSystem(SDL_INIT_AUDIO); }

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

        SDL_AudioSpec spec;

        SDL_memset(&spec, 0, sizeof(spec));
        spec.freq = 44100;
        spec.format = AUDIO_S16;
        spec.channels = 1;
        spec.samples = 4096;
        spec.callback = Sound::_Run;
        spec.userdata = this;
        _dev = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
        std::cout << "DEV: " << _dev << "\n";
        std::cout << SDL_GetError() << "\n";
        SDL_PauseAudioDevice(_dev, 0);
    }

    WaveOutput& wave() { return _wav; }

    ToneOsc& channel_1() { return _tone1; }
    ToneOsc& channel_2() { return _tone2; }

    void set_mixer(byte x) { _mixer = GetBit(x, 6) || GetBit(x, 2); }

    byte on_off() const { return _on_off | 0x70; }
    void set_on_off(byte x) { _on_off = x; }

    ~Sound() { SDL_CloseAudioDevice(_dev); }

   private:
    void Run(uint8_t* stream, int len) {
        SDL_memset(stream, 0, len);

        Chunk c;
        _tone1.Process(c);
        SDL_MixAudioFormat(stream,
                           reinterpret_cast<uint8_t*>(c.samples),
                           AUDIO_S16,
                           c.sampleCount * 2,
                           SDL_MIX_MAXVOLUME);
        _tone2.Process(c);
        SDL_MixAudioFormat(stream,
                           reinterpret_cast<uint8_t*>(c.samples),
                           AUDIO_S16,
                           c.sampleCount * 2,
                           SDL_MIX_MAXVOLUME);
        _wav.Process(c);
        if (_mixer) {
            SDL_MixAudioFormat(stream,
                               reinterpret_cast<uint8_t*>(c.samples),
                               AUDIO_S16,
                               c.sampleCount * 2,
                               SDL_MIX_MAXVOLUME);
        }
    }

    void static _Run(void* thisptr, uint8_t* stream, int len) {
        static_cast<Sound*>(thisptr)->Run(stream, len);
    }

    bool _mute;
    byte _mixer;
    byte _on_off;
    WaveOutput _wav;
    ToneOsc _tone1;
    ToneOsc _tone2;
    Noise _noise;
    SDL_AudioDeviceID _dev;
};
