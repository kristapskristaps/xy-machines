#pragma once
#include <opxy/audio.hpp>

namespace bitflip {
struct Voice {
    opxy::u32 phase, phase2, mod_phase, noise_phase, arp_phase, arp_step, arp_type;
    opxy::u32 long_noise, short_noise, clock_phase, initialized, model;
    opxy::i32 color, edge, sweep_envelope, sweep_ratio, fade, model_fade;
    opxy::i32 model_from, last_raw, held, dc_input, dc_output;
};
struct Shared {};
}
