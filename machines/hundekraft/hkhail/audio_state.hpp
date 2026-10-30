#pragma once
#include "send_level.hpp"
#include "dsp.hpp"
namespace hkhail {
// Only audio memories change units. Pointers, phases, coefficients and controls
// retain their original representation when the send's working gain falls.
inline void audio_memory(port::Dsp& d, AudioScale& scale) {
    auto& s=d.bd[0];
    scale.samples(s.buf,65537);
    scale.samples(s.rev,16384);
    scale.values(s.prev,1,131071);
    scale.values(s.dc,1,33554431);
    scale.values(s.lin.pk,1,8388607);
    scale.values(s.lin.lp,1,8388607);
    scale.values(s.lpre.pk,1,67108863);
    scale.values(s.lpre.lp,1,67108863);
    scale.values(s.lout.pk,1,67108863);
    scale.values(s.lout.lp,1,67108863);
    scale.values(s.rlp1);
    scale.values(s.rlp2);
    scale.values(s.rwo);
    scale.values(s.level);
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
