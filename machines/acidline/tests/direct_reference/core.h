// Frozen f83afb15 direct1x renderer for cache scheduling tests.
#pragma once
#include <opxy/audio.hpp>
namespace acidline_direct {
struct Voice {
    opxy::u32 phase;
    opxy::i32 pole[4], envelope, cutoff, resonance, drive, wave;
    opxy::i32 dc_input, dc_output;
    opxy::i32 filter_coefficient, filter_feedback, feedback_low, input_low;
    opxy::u32 sub_phase, sub_remainder;
    opxy::i32 sub, tone, tone_memory;
};
struct Shared {};
}
