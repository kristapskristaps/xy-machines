#pragma once
#include <opxy/effect.hpp>

namespace lustre {
const opxy::u32 kHistory = 32768;
const opxy::u32 kReverb = 4096;
const opxy::u32 kDiffuser = 512;
const opxy::u32 kBufferWords = 2 * kHistory + 4 * kReverb + 4 * kDiffuser;
struct Grain {
    opxy::u32 position, phase, increment, remaining;
    opxy::i32 speed, pan;
};
struct State {
    Grain grains[6];
    opxy::u32 write, written, reverb_write, reverb_written;
    opxy::u32 countdown, next, random, half;
    opxy::i32 input[2], antialias[2], previous[2], output[2];
    opxy::i32 damping[4], feedback[2], tone_memory[2], dc[2];
    opxy::i32 controls[8];
    opxy::u32 started;
};
void initialize(State&);
void process(State&, opxy::Buffer, opxy::Channel, opxy::Channel, opxy::Parameters);
}
