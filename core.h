#pragma once
#include <opxy/effect.hpp>
namespace vapour {
const unsigned ring_frames = 65536;
const unsigned grain_capacity = 32;
struct Grain {
    opxy::u32 phase, increment, envelope, envelope_increment, remaining, pre_delay;
    opxy::i32 slope, smoothness, left_gain, right_gain;
};
struct Workspace {
    Grain grains[grain_capacity];
    opxy::i32 diffusion[4096];
    opxy::i32 feedback[128];
    opxy::u32 freeze_tail[256];
};
struct State {
    opxy::u32 initialized, write, written, random, clock, size_hint;
    opxy::u32 diffusion_position[8], diffusion_written;
    opxy::i32 feedback_left, feedback_right, dc_left, dc_right;
    opxy::i32 feedback, freeze, normalization;
    opxy::u32 freeze_gate, tail_count, tail_read;
};
static_assert(sizeof(State) <= opxy::effect_buffered_state_bytes, "effect state");
static_assert(ring_frames * 4 + sizeof(Workspace) <= 282624, "scratch layout");
void initialize(State&);
void process(State&, opxy::Buffer, opxy::Channel, opxy::Channel, opxy::Parameters);
}
