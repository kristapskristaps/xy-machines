#include "core.h"
#include "sine.hpp"
#include "dsp.hpp"
#include "audio_state.hpp"
namespace hkinaroom {
void initialize(State& state) { state=State{}; state.lift=20; }
void process(State& state, opxy::Buffer buffer, const opxy::Transport& transport, opxy::Channel left,
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
    const SendLevel level=prepare_level(state,buffer,left,right,2,44766);
    const opxy::u32 lower=state.lift-level.lift;
    port::int16_t q[8]={};
    q[0] = parameters[0] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[0]) >> 16);
    q[1] = parameters[1] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[1]) >> 16);
    q[2] = parameters[2] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[2]) >> 16);
    q[5] = parameters[4] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[4]) >> 16);
    q[6] = parameters[5] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[5]) >> 16);
    q[7] = parameters[6] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[6]) >> 16);
    q[3]=32767;
    q[1]=static_cast<port::int16_t>((static_cast<unsigned>(q[1])*4u>>15)*512u+256u);
    const opxy::u32 bpm=transport.bpm_x10>=200&&transport.bpm_x10<=3000?transport.bpm_x10:1200;
    const opxy::u32 beat_length=((26460000u/bpm)<<14)+((26460000u%bpm)<<14)/bpm;
    if(state.clock_tempo!=bpm || transport.ticks96-state.clock_tick>96u) {
        state.clock_phase=(transport.ticks96%96u)*(beat_length/96u);state.clock_tempo=bpm;
    }
    state.clock_tick=transport.ticks96;
    for(unsigned channel=0;channel<2;++channel) {
        opxy::i32* memory=buffer.words+channel*44766;
        port::Dsp& d=*reinterpret_cast<port::Dsp*>(memory);
        d.scratch=reinterpret_cast<port::uint8_t*>(memory)+2600;
        if(!state.ready) {d.defaults();d.sit_init(0);}
        else if(lower) lower_audio(d,lower);
        d.fx_page[0][3]=parameters[7]>=0x40000000;
        
        d.beat.len=static_cast<port::int32_t>(beat_length);
        d.beat.next=state.clock_phase?static_cast<port::int32_t>(beat_length-state.clock_phase):0;
        d.beat.step=1048576;d.beat.tape_pos=0;d.beat.follows_tape=0;

        port::int16_t audio_left[64];
        for(unsigned i=0;i<64;++i) {
            audio_left[i]=level.input((channel?right:left)[i]);
            
        }
        d.sit_render(0,audio_left,q);
        for(unsigned i=0;i<64;++i) (channel?right:left)[i]=level.output(audio_left[i]);
    }
    state.lift=level.lift;
    state.ready=1;
    state.clock_phase+=1048576u;if(state.clock_phase>=beat_length)state.clock_phase-=beat_length;
}
}
