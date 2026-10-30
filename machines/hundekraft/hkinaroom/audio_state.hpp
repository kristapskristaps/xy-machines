#pragma once
#include "send_level.hpp"
#include "dsp.hpp"
namespace hkinaroom {
// Only audio memories change units. Pointers, phases, coefficients and controls
// retain their original representation when the send's working gain falls.
inline void audio_memory(port::Dsp& d, AudioScale& scale) {
    auto& s=d.st[0];
    scale.samples(reinterpret_cast<port::int16_t*>(d.scratch),84096+2048+2048);
    for(unsigned i=0;i<6;++i) {scale.values(s.res[i].lp);scale.values(s.res[i].bp);}
    scale.values(s.dcx,1,4194303);
    scale.values(s.dcy,1,4194303);
    scale.values(s.z,1,2097151);
    scale.values(s.peak);
    scale.values(s.ipeak);
    scale.values(s.cpeak);
    scale.values(s.pw);
    scale.values(s.x);
    scale.values(s.lw);
    scale.values(s.e1);
    scale.values(s.e2);
    scale.values(s.e3);
    scale.values(s.f1);
    scale.values(s.f2);
    scale.values(s.f3);
    scale.values(s.r1);
    scale.values(s.r2);
    scale.values(s.r3);
    scale.values(s.inb);
    scale.values(s.bhi);
    scale.values(s.blo);
    scale.values(s.env);
    scale.values(s.dkv);
}
inline void lower_audio(port::Dsp& d,unsigned by) {
    AudioScale scale(-1,by);audio_memory(d,scale);
}
// Both sends/channels keep their histories; regain precision only when every
// stored sample and energy detector has enough headroom for the same exponent.
inline SendLevel prepare_level(State& state,opxy::Buffer buffer,
                              opxy::Channel left,opxy::Channel right,
                              unsigned channels,unsigned stride) {
    SendLevel level(left,right,20);
    if(level.lift>state.lift && state.ready) {
        if(state.gain_check<689) level.set_lift(state.lift);
        else {
            unsigned room=level.lift-state.lift;
            for(unsigned channel=0;channel<channels;++channel) {
                port::Dsp& d=*reinterpret_cast<port::Dsp*>(buffer.words+channel*stride);
                AudioScale scan(0,room);audio_memory(d,scan);room=scan.room;
            }
            level.set_lift(state.lift+room);
            if(room) for(unsigned channel=0;channel<channels;++channel) {
                port::Dsp& d=*reinterpret_cast<port::Dsp*>(buffer.words+channel*stride);
                AudioScale raise(1,room);audio_memory(d,raise);
            }
            state.lift=level.lift;
            state.gain_check=0;
        }
    }
    if(level.lift<state.lift) state.gain_check=0;
    else if(state.gain_check<689) ++state.gain_check;
    return level;
}
}
