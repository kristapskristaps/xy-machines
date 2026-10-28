#pragma once
#include <opxy/audio.hpp>
#include <opxy/lease_pool.hpp>
#include "vendor/numeric.h"
namespace matter {
const unsigned mode_count=52;
struct Filter { opxy::i32 s1,s2,a1,a2,a3,gain; };
struct BowFilter { opxy::i32 s1,s2,a1,a2,a3,normalization; };
struct BowDelay { opxy::i16 samples[8][1024]; };
struct Voice {
    Filter modes[mode_count];
    BowFilter bowed[8];
    opxy::u32 bow_write[8],bow_length[8];
    opxy::u32 lease,random,trigger,model;
    opxy::u32 active_modes[2];
    opxy::u32 sample_phase,plectrum_delay,particle_delay;
    opxy::i32 particle,particle_range,flow,exc_s1,exc_s2;
    opxy::i32 bow_signal,bow_velocity,limiter;
    opxy::i32 key[5],position;
};
struct Shared { opxy::LeasePool<BowDelay,8> bows; };
void initialize(Voice&,Shared&,opxy::u32);
void start(Voice&,Shared&,opxy::u32);
void destroy(Voice&,Shared&,opxy::u32);
void render(Voice&,Shared&,opxy::u32,opxy::Output,opxy::Parameters,opxy::Pitch);
}
