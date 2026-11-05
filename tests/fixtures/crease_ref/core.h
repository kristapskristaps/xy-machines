#pragma once
#include <opxy/audio.hpp>
#include "vendor/macro.h"
namespace crease_ref {
struct Voice {
    crease_ref_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];

};
struct Shared {};
void initialize(Voice&,Shared&,opxy::u32);
void start(Voice&,Shared&,opxy::u32);
void destroy(Voice&,Shared&,opxy::u32);
void render(Voice&,Shared&,opxy::u32,opxy::Output,opxy::Parameters,opxy::Pitch);
}
