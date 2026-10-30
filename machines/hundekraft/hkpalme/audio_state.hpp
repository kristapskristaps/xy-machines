#pragma once
#include "send_level.hpp"
#include "dsp.hpp"
namespace hkpalme {
// Only audio memories change units. Pointers, phases, coefficients and controls
// retain their original representation when the send's working gain falls.
inline void audio_memory(port::Dsp& d, AudioScale& scale) {
    auto& s=d.st[0];
    scale.samples(s.L-48,12*(48+1024));
    scale.values(s.fe,2,1073676289);
    scale.values(s.tgt,2,3062500);
    // S caches the sum of a moving window. Rebuild it from the quantized
    // line instead of shifting the old sum, which would accumulate DC error.
    if(scale.direction) for(unsigned voice=0;voice<12;++voice) {
        const port::int16_t* line=s.L+voice*(48+1024);
        s.S[voice]=0;
        for(unsigned j=0;j<static_cast<unsigned>(s.pw[voice]);++j)
            s.S[voice]+=line[(s.wp-1-j)&1023];
    }
    scale.values(s.env);
    scale.values(s.ein);
    scale.values(s.lp);
    scale.values(s.hq,1,8388607);
    scale.values(s.ow,1,33554431);
    scale.values(s.fb);
    scale.values(s.fbsum,1,2097151);
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
