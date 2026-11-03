#pragma once
#include <opxy/types.hpp>
namespace x0x { namespace dsp {
// The retained six-word metal_reciprocals slot stores stem anchor/blend in
// words 0/1; keeping its layout also preserves the frozen legacy test contract.
struct Settings {
    opxy::u32 family, raw[8], duration, increments[6], sample_step, metal_reciprocals[6], attack_frames, attack_step;
    opxy::i32 amplitude_rate, transient_rate, pitch_rate, filter, noise_filter;
    opxy::i32 sample, alternate;
    opxy::u32 clap_period, clap_inverse, pcm_delay;
    bool metal808;
};
}}
