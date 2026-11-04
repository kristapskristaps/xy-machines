#pragma once
#include <opxy/audio.hpp>
#include "vendor/macro.h"
namespace weave_waves {
struct Voice {
    weave_waves_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];
    opxy::i16 history[48];
    opxy::u32 cache_read,history_write,resample_phase;
};
struct Shared {};
void initialize(Voice&,Shared&,opxy::u32);
void start(Voice&,Shared&,opxy::u32);
void destroy(Voice&,Shared&,opxy::u32);
void render(Voice&,Shared&,opxy::u32,opxy::Output,opxy::Parameters,opxy::Pitch);
}
