#pragma once
#include <opxy/sample_engine.hpp>

namespace granular {
const unsigned grain_count = 8;
struct Grain {
    opxy::u32 frame, fraction, whole_step, fractional_step;
    opxy::u32 phase, phase_step, remaining;
    opxy::i32 left, right;
};
struct Voice {
    opxy::SampleView sample;
    Grain grains[grain_count];
    opxy::u32 begin, end, scan_frame, scan_fraction, countdown, random;
    bool reverse, configured;
};
void initialize(Voice&, const opxy::SampleView&);
void start(Voice&);
void render_sample(Voice&, const opxy::SampleBlock&);
}
