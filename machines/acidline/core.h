#pragma once
#include <opxy/audio.hpp>
namespace acidline {
struct Voice {
    opxy::u32 phase;
    opxy::i32 pole[4], envelope, cutoff, resonance, drive, wave;
    opxy::i32 dc_input, dc_output;
    opxy::i32 fir1[32];
    opxy::u32 cursor1;
};
struct Shared {};
}
