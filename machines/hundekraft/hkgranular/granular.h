#pragma once
#include <opxy/sample_engine.hpp>

namespace hkgranular {
struct Grain {
    opxy::u32 position, rate, window, increment;
};
struct Voice {
    opxy::SampleView sample;
    Grain grains[4];
    opxy::u32 random;
    opxy::i32 next;
    opxy::u32 begin, end;
    bool reverse;
};
void initialize(Voice&,const opxy::SampleView&);
void start(Voice&);
void render_sample(Voice&,const opxy::SampleBlock&);
}
