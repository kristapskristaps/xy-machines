#pragma once
#include <opxy/audio.hpp>

namespace mando {
using opxy::i32;
using opxy::u32;
struct FilterCoefficients {
    i32 g, b0[2], a1[2], a2[2];
};
struct Voice {
    u32 phase[2];
    // Q24 input history, then shared first-section and final-output histories.
    i32 history[6];
    i32 controls[8];
    u32 ready;
    i32 wave[2];
    i32 dc_input, dc_output, dc_residue;
    FilterCoefficients cached_filter;
    i32 cached_resonance;
    i32 reconstruction[3], last_output;
    u32 rate_mode;
};
struct Shared {};
} // namespace mando
