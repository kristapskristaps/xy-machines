#include "core.h"
#include "sine.hpp"
#include "dsp.hpp"
namespace hkfather {
void initialize(State& state) { state=State{}; }
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
        
        
        port::int16_t audio_left[64],audio_right[64];
        for(unsigned i=0;i<64;++i) {
            audio_left[i]=static_cast<port::int16_t>((channel?right:left)[i]>>16);
            audio_right[i]=static_cast<port::int16_t>(right[i]>>16);
        }
        d.stereo(0,audio_left,audio_right,q);
        for(unsigned i=0;i<64;++i) {left[i]=static_cast<opxy::i32>(audio_left[i])*65536;right[i]=static_cast<opxy::i32>(audio_right[i])*65536;}
    }
    state.ready=1;
    
}
}
