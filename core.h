#pragma once
#include <opxy/effect.hpp>
#include <opxy/transport.hpp>

namespace halo {
const unsigned network_rate=22050;
const unsigned buffer_words=114688;
const unsigned echo_frames=67584;
struct State {
    opxy::u32 ready, written, positions[19];
    opxy::u32 down_write, up_write, half, phase[2], grain_phase;
    opxy::i32 controls[7], delay;
    opxy::i32 input_low[2], input_high[2], echo_low[2], echo_high[2], echo_error[2];
    opxy::i32 plate[2], damping[2], shimmer_high[2], shimmer_low[2][2];
    opxy::i32 body[2], duck[2];
};
static_assert(sizeof(State)<=opxy::effect_buffered_state_bytes,"Halo state exceeds send storage");
void initialize(State&);
void process(State&,opxy::Buffer,const opxy::Transport&,opxy::Channel,opxy::Channel,opxy::Parameters);
}
