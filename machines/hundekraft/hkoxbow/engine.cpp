#include "core.h"
namespace hkoxbow {
inline port::Dsp& dsp(Voice& voice) { return *reinterpret_cast<port::Dsp*>(voice.storage); }
inline void initialize(Voice& voice, Shared&, opxy::u32) {
    for (unsigned i=0; i<sizeof(voice.storage)/sizeof(voice.storage[0]); ++i) voice.storage[i]=0;
    dsp(voice).defaults(); voice.pending=1;voice.half=0;
    for(unsigned i=0;i<128;++i) voice.audio[i]=0;
}
inline void start(Voice& voice, Shared&, opxy::u32) { voice.pending=1;voice.half=0;for(unsigned i=0;i<128;++i)voice.audio[i]=0; }
inline void destroy(Voice&, Shared&, opxy::u32) {}
inline void render(Voice& voice, Shared&, opxy::u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {
    port::Dsp& d=dsp(voice);
    port::int16_t q[8]={};
    q[0] = parameters[0] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[0]) >> 16);
    q[1] = parameters[1] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[1]) >> 16);
    q[2] = parameters[2] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[2]) >> 16);
    q[3] = parameters[3] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[3]) >> 16);
    q[5] = parameters[4] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[4]) >> 16);
    q[6] = parameters[5] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[5]) >> 16);
    q[7] = parameters[6] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[6]) >> 16);
    q[5]=static_cast<port::int16_t>((static_cast<unsigned>(q[5])*6u>>15)*512u+256u);
    q[6]=static_cast<port::int16_t>((static_cast<unsigned>(q[6])*6u>>15)*512u+256u);
    const unsigned offset=voice.half*64;
    for(unsigned i=0;i<64;++i)voice.pitch[offset+i]=pitch[i];
    if(!voice.half) {
        for(unsigned i=0;i<64;++i)output[i]=static_cast<opxy::i32>(voice.audio[64+i])*32768;
        voice.half=1;return;
    }
    d.pitch_input=reinterpret_cast<const port::int32_t*>(voice.pitch);
    if (voice.pending) { d.obv_note_on(&voice,q); voice.pending=0; }
    d.obv_render(&voice,voice.audio,q);
    d.pitch_input=0;
    for(unsigned i=0;i<64;++i) output[i]=static_cast<opxy::i32>(voice.audio[i])*32768;
    voice.half=0;
}
}
