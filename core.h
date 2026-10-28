#pragma once
#include <opxy/audio.hpp>
#include "vendor/macro.h"

namespace weave {
struct Voice {
    weave_dsp::macro_voice oscillator;
    opxy::u32 pending, model;
    opxy::i16 cache[32], history[24];
    opxy::u32 cache_read, history_write, resample_phase;
};
struct Shared {};
void initialize(Voice&, Shared&, opxy::u32);
void start(Voice&, Shared&, opxy::u32);
void destroy(Voice&, Shared&, opxy::u32);
void render(Voice&, Shared&, opxy::u32, opxy::Output, opxy::Parameters, opxy::Pitch);
}
