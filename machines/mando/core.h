#pragma once
#include <opxy/audio.hpp>

namespace mando {
using opxy::i32;
using opxy::u32;
struct FilterCoefficients {
    i32 g, resonance, inverse, weights[4], compensation;
};
struct Voice {
    u32 phase[2];
    // Each accumulator stores half the even pole state in its high bits,
    // and the unchanged 15-bit fractional charge in its low bits.
    i32 ladder[4];
    i32 controls[8];
    u32 ready;
    i32 wave[2];
    i32 dc_input, dc_output, dc_residue;
    FilterCoefficients cached_filter;
    i32 cached_resonance;
};
struct Shared {};
}
