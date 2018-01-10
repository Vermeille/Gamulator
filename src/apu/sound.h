#pragma once

#include <algorithm>
#include <vector>

#include "chunk.h"
#include "sdl.h"
#include "toneosc.h"
#include "utils.h"
#include "waveoutput.h"

inline void InitAudio() { SDL_InitSubSystem(SDL_INIT_AUDIO); }

class NoiseOsc {
   public:
    NoiseOsc() : _data(2048), _rnd(53934) {}

    int16_t* GenSamples() {
        if (_sample_max == 0) {
            std::fill(_data.begin(), _data.end(), 0);
            return &_data[0];
        }

        for (unsigned int i = 0; i < _data.size(); ++i) {
            ++_sample_cnt;
            if (_sample_cnt >= _sample_max) {
                _sample_cnt = 0;
                StepRng();
            }

            if (_bit) {
                _data[i] = -1;
            } else {
                _data[i] = 1;
            }
        }
        return &_data[0];
    }

    void set_clock_freq(int f) {
        _f = f;
        UpdateSampleCount();
    }
    void set_wide(bool w) { _wide = w; }
    void set_divider(int mode) {
        _mode = mode;
        UpdateSampleCount();
    }

    int nb_samples() const { return _data.size(); }

    void Reset() { _rnd = 0x5f3b; }

   private:
    void UpdateSampleCount() {
        if (_mode == 0) {
            _sample_max = 44100 / (524288 * 2 / (1 << (_f + 1)));
        } else {
            _sample_max = 44100 / (524288 / _mode / (1 << (_f + 1)));
        }
    }

    void StepRng() {
        bool new_bit = ((_rnd >> 1) ^ _rnd) & 1;
        _rnd >>= 1;
        _rnd = SetBit(_rnd, 14, new_bit);
        if (!_wide) {
            _rnd = SetBit(_rnd, 6, new_bit);
        }
        _bit = ~_rnd & 1;
    }

    int _f;
    int _mode;
    // std::atomic<int> _f;
    // std::atomic<int> _mode;
    bool _wide;
    bool _bit;
    int _rnd;
    int _sample_cnt;
    int _sample_max;
    std::vector<int16_t> _data;
};

class Noise {
   public:
    void set_len(byte x) {
        x &= 0x3F;
        _length.set_len((64 - x) * 1000 / 256);
    }
    byte len() const { return 0xFF; }

    void set_env(byte x) {
        _env_cache = x;
        _env.set_volume(x >> 4);
        _env.set_direction(GetBit(x, 3));
        _env.set_len((x & 7) * 1000 / 64);
    }
    byte env() const { return _env_cache; }

    void set_poly_counter(byte x) {
        _poly_cache = x;
        _noise.set_clock_freq(x >> 4);
        _noise.set_wide(!GetBit(x, 3));
        _noise.set_divider(x & 7);
    }
    byte poly_counter() const { return _poly_cache; }

    void set_consecutive(byte x) {
        _consecutive_cache = x;
        _length.set_timed(GetBit(x, 6));
        if (GetBit(x, 7)) {
            _length.Reset();
            _env.Reset();
            //_noise.Reset();
        }
    }
    byte consecutive() const { return _consecutive_cache; }

    void Process(Chunk& c) {
        c.samples = _noise.GenSamples();
        c.sampleCount = _noise.nb_samples();

        _length.Process(c.samples, c.sampleCount);
        _env.Process(c.samples, c.sampleCount);
    }

   private:
    byte _env_cache;
    byte _poly_cache;
    byte _consecutive_cache;
    LengthCounter _length;
    Envelope _env;
    NoiseOsc _noise;
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
    Noise& noise() { return _noise; }
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
        _noise.Process(c);
        SDL_MixAudioFormat(stream,
                           reinterpret_cast<uint8_t*>(c.samples),
                           AUDIO_S16,
                           c.sampleCount * 2,
                           SDL_MIX_MAXVOLUME);
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
