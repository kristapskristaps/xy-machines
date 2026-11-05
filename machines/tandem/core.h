#pragma once
#include "vendor/macro.h"
#include <opxy/audio.hpp>
namespace tandem {
struct Shape {
    opxy::u32 ratio, pulse, up, down;
    opxy::i32 triangle, square;
};
struct Voice {
    tandem_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];
    opxy::i16 history[48];
    opxy::u32 cache_read, history_write, resample_phase;
    opxy::i16 cycle[2][2][257];
    opxy::i32 mean[2][2], sum[2];
    opxy::u32 phase[2], active, valid, building, build_at, fade;
    opxy::u32 key[5], detune;
    Shape pulse, primary, auxiliary, saw;
    opxy::i32 pulse_gain, saw_gain, mix;
};
struct Shared {};
void initialize(Voice &, Shared &, opxy::u32);
void start(Voice &, Shared &, opxy::u32);
void destroy(Voice &, Shared &, opxy::u32);
void render(Voice &, Shared &, opxy::u32, opxy::Output, opxy::Parameters, opxy::Pitch);
} // namespace tandem
