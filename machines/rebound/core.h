#pragma once
#include <opxy/effect.hpp>

namespace rebound {
const opxy::u32 line_frames = 65536;
const opxy::u32 buffer_words = 2 * line_frames;
struct State {
    opxy::u32 write, written, ready;
    opxy::i32 delay;  // Q8 samples
    opxy::i32 feedback_half, pingpong, level, highpass, lowpass;
    opxy::i32 dc[2], low[2];
};
void initialize(State& state);
void process(State& state, opxy::Buffer buffer, const opxy::Transport& transport,
             opxy::Channel left, opxy::Channel right, opxy::Parameters parameters);
}
