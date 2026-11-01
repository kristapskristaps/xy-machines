#pragma once
#include <opxy/types.hpp>
namespace x0x { namespace dsp {
struct Settings {
    opxy::u32 family, raw[8], duration, increments[6], sample_step, metal_reciprocals[6], attack_frames, attack_step;
    opxy::i32 amplitude_rate, transient_rate, pitch_rate, filter, noise_filter;
    opxy::i32 sample, alternate;
    bool metal808;
};
}}
