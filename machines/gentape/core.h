#pragma once
#include <opxy/effect.hpp>

namespace gentape {
const opxy::u32 line_frames = 4096;
const opxy::u32 line_words = line_frames + 4;
const opxy::u32 buffer_words = 2 * line_words;
struct State {
    opxy::u32 write, written, ready, random, wait, event, duration, kind;
    opxy::i32 controls[7], wow[2][2], flutter[2][2];
    opxy::i32 delay[2], gate[2], drive, makeup, amount, hiss;
    opxy::i32 hp, lp, bump, gain;
    opxy::i32 dc[2], low[2][2], bass[2], record[2][2];
};
void initialize(State &state);
void process(State &state, opxy::Buffer buffer, opxy::Channel left, opxy::Channel right,
             opxy::Parameters parameters);
} // namespace gentape
