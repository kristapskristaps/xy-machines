#pragma once
#include <opxy/effect.hpp>
#include <opxy/transport.hpp>
namespace halo {
const unsigned rate_numerator = 11025, rate_denominator = 2;
const unsigned buffer_words = 24384;
const unsigned echo_frames = 16896;
struct Cic {
    opxy::u32 integrator[3], comb[3];
};
struct State {
    opxy::u32 ready, written, clock, echo_position, phase, grain_phase;
    Cic down[2], up[2];
    opxy::i32 controls[7], delay, input_high[2], input_low[2], echo_low[2];
    opxy::i32 damping[4], feedback_dc[4], shimmer_high, shimmer_low[2];
    opxy::i32 peak, body, duck;
};
static_assert(sizeof(State) <= opxy::effect_buffered_state_bytes,
              "Halo state exceeds send storage");
void initialize(State &);
void process(State &, opxy::Buffer, const opxy::Transport &, opxy::Channel,
             opxy::Channel, opxy::Parameters);
} // namespace halo
