#pragma once
#include <opxy/audio.hpp>
namespace acidline {
struct Voice {
    opxy::u32 phase;
    opxy::i32 pole[4], envelope, cutoff, resonance, drive, wave;
    opxy::i32 dc_input, dc_output;
    opxy::i32 fir1[64];
    opxy::u32 cursor1;
    opxy::u32 sub_phase, sub_remainder;
    opxy::i32 sub, tone, tone_memory;
};
struct Shared {};
}
