#include "core.h"
#include "sine.hpp"
#include "dsp.hpp"
#include "audio_state.hpp"
namespace hkfather {
void initialize(State& state) { state=State{}; state.lift=20; }
void process(State& state, opxy::Buffer buffer, opxy::Channel left,
             opxy::Channel right, opxy::Parameters parameters) {
    if (!buffer.words || buffer.count<buffer_bytes/4) {
        for(unsigned i=0;i<64;++i) left[i]=right[i]=0;
        return;
    }
    if(state.cleared<buffer_bytes/4) {
        const opxy::u32 end=state.cleared+1024<buffer_bytes/4?state.cleared+1024:buffer_bytes/4;
        while(state.cleared<end) buffer.words[state.cleared++]=0;
        for(unsigned i=0;i<64;++i) left[i]=right[i]=0;
        return;
    }
    const SendLevel level=prepare_level(state,buffer,left,right,1,44162);
    const opxy::u32 lower=state.lift-level.lift;
    port::int16_t q[8]={};
    q[0] = parameters[0] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[0]) >> 16);
    q[1] = parameters[1] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[1]) >> 16);
    q[2] = parameters[2] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[2]) >> 16);
    q[3]=32767;

    
    for(unsigned channel=0;channel<1;++channel) {
        opxy::i32* memory=buffer.words+channel*44162;
        port::Dsp& d=*reinterpret_cast<port::Dsp*>(memory);
        d.scratch=reinterpret_cast<port::uint8_t*>(memory)+184;
        if(!state.ready) {d.defaults();d.init(0);}
        else if(lower) lower_audio(d,lower);
        
        
        port::int16_t audio_left[64],audio_right[64];
        for(unsigned i=0;i<64;++i) {
            audio_left[i]=level.input((channel?right:left)[i]);
            audio_right[i]=level.input(right[i]);
        }
        d.stereo(0,audio_left,audio_right,q);
        for(unsigned i=0;i<64;++i) {left[i]=level.output(audio_left[i]);right[i]=level.output(audio_right[i]);}
    }
    state.lift=level.lift;
    state.ready=1;
    
}
}
