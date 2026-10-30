#include "core.h"
#include "sine.hpp"
#include "dsp.hpp"
#include "audio_state.hpp"
namespace hksnowglobe {
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
    const SendLevel level=prepare_level(state,buffer,left,right,2,12392);
    const opxy::u32 lower=state.lift-level.lift;
    port::int16_t q[8]={};
    q[0] = parameters[0] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[0]) >> 16);
    q[1] = parameters[1] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[1]) >> 16);
    q[2] = parameters[2] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[2]) >> 16);
    q[5] = parameters[4] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[4]) >> 16);
    q[3]=32767;

    
    for(unsigned channel=0;channel<2;++channel) {
        opxy::i32* memory=buffer.words+channel*12392;
        port::Dsp& d=*reinterpret_cast<port::Dsp*>(memory);
        d.scratch=0;
        if(!state.ready) {d.defaults();d.fx_reset(0);}
        else if(lower) lower_audio(d,lower);
        d.fx_page[0][3]=parameters[4]>=0x40000000;
        
        port::int16_t audio_left[64];
        for(unsigned i=0;i<64;++i) {
            audio_left[i]=level.input((channel?right:left)[i]);
            
        }
        d.fx_process(0,audio_left,64,q);
        for(unsigned i=0;i<64;++i) (channel?right:left)[i]=level.output(audio_left[i]);
    }
    state.lift=level.lift;
    state.ready=1;
    
}
}
