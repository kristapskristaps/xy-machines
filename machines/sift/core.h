#pragma once
#include "vendor/macro.h"
#include <opxy/audio.hpp>
namespace sift {
struct Voice {
    sift_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];
    opxy::i16 history[48];
    opxy::u32 cache_read, history_write, resample_phase;
    opxy::i16 cycle[2][257];
    opxy::i32 cycle_mean[2], build_sum;
    opxy::u32 phase, active, valid, building, build_at, fade;
    opxy::u32 prepared[5];
    opxy::u32 ratio[3];
    opxy::i32 shape, bleed, inverse, mix, harmonic, morph;
};
struct Shared {};
void initialize(Voice &, Shared &, opxy::u32);
void start(Voice &, Shared &, opxy::u32);
void destroy(Voice &, Shared &, opxy::u32);
void render(Voice &, Shared &, opxy::u32, opxy::Output, opxy::Parameters, opxy::Pitch);
} // namespace sift
