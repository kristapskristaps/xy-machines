// Granular sampler adapted from pcppcp/op1r and Hundekraft 3.0.3.
// GPL-3.0-or-later with the retained OP-1 firmware permission; see LICENSE and NOTICE.
// Four overlapping grains with Hann windows, position jitter and pitch jitter.
#include "granular.h"
#include "sine.hpp"

namespace hkgranular {
namespace {
using opxy::i32;
using opxy::u32;
const u32 kMaximumFrames=4194303u;
const u32 kRootIncrement=25480370u;

i32 normalized(i32 value) {return value<0?0:static_cast<i32>(static_cast<u32>(value)>>16);}
i32 clamp16(i32 value) {return value>32767?32767:value< -32768?-32768:value;}
u32 random(Voice& voice) {return voice.random=voice.random*1664525u+1013904223u;}
i32 multiply15(i32 a,i32 b) {return (a>>15)*b+(((a&32767)*b)>>15);}
u32 multiply16(u32 a,u32 b) {
    return a*(b>>16)+(((a>>10)*(b&65535u))>>6)+(((a&1023u)*(b&65535u))>>16);
}
u32 tuning(i32 cents) {
    const u32 ratio[13]={65536,69433,73562,77936,82570,87480,92682,98193,104032,110218,116772,123715,131072};
    if(cents< -4800)cents=-4800;
    if(cents>4800)cents=4800;
    i32 octave=cents/1200,remainder=cents%1200;
    if(remainder<0){remainder+=1200;--octave;}
    const u32 index=static_cast<u32>(remainder)/100u,fraction=static_cast<u32>(remainder)%100u;
    u32 value=ratio[index]+(ratio[index+1]-ratio[index])*fraction/100u;
    return octave<0?value>>static_cast<u32>(-octave):value<<static_cast<u32>(octave);
}
void spawn(Voice& voice,const i32* q,u32 rate,i32 size) {
    unsigned slot=0;
    while(slot<4 && voice.grains[slot].increment)++slot;
    if(slot==4)return;
    const u32 length=voice.end-voice.begin;
    i32 position=static_cast<i32>((length>>15)*static_cast<u32>(q[0])+((length&32767u)*static_cast<u32>(q[0])>>15));
    const i32 spray=static_cast<i32>(static_cast<u32>(q[2])*66150u>>15);
    if(spray)position+=static_cast<i32>(random(voice)%static_cast<u32>(2*spray+1))-spray;
    if(position<0)position=0;
    if(static_cast<u32>(position)>=length)position=static_cast<i32>(length-1);
    if(q[2]>4096) {
        const i32 detune=static_cast<i32>(random(voice)>>22)-512;
        rate=static_cast<u32>(static_cast<i32>(rate)+((static_cast<i32>(rate>>10)*detune*(q[2]>>8))>>12));
    }
    Grain& grain=voice.grains[slot];
    grain.position=static_cast<u32>(position)<<10;grain.rate=rate;
    grain.window=0;grain.increment=0x80000000u/static_cast<u32>(size);
}
i32 source(const Voice& voice,u32 relative,u32 channel) {
    const u32 frame=voice.reverse?voice.end-1-relative:voice.begin+relative;
    return opxy::sample_at(voice.sample,frame,channel);
}
i32 scale_level(i32 sample,u32 level) {
    const u32 magnitude=static_cast<u32>(sample<0?-sample:sample);
    const u32 scaled=(((level>>16)*magnitude)+(((level&65535u)*magnitude)>>16))>>11;
    const i32 clipped=static_cast<i32>(scaled>32767u?32767u:scaled);
    return sample<0?-clipped:clipped;
}
}

void initialize(Voice& voice,const opxy::SampleView& sample) {
    voice.sample=sample;voice.random=0x9e3779b9u^sample.frames;
    voice.begin=voice.end=0;voice.reverse=false;
    start(voice);
}
void start(Voice& voice) {
    for(unsigned i=0;i<4;++i)voice.grains[i]=Grain{};
    voice.next=0;
}
void render_sample(Voice& voice,const opxy::SampleBlock& block) {
    const u32 channels=block.output_channels==2?2u:1u;
    for(u32 i=0;i<64*channels;++i)block.output[i]=0;
    if(!voice.sample.data || voice.sample.frames<2 || voice.sample.rate<8000 || voice.sample.rate>192000 ||
       (voice.sample.channels!=1 && voice.sample.channels!=2))return;
    const u32 begin=block.region.begin<voice.sample.frames?block.region.begin:voice.sample.frames;
    u32 end=block.region.end<voice.sample.frames?block.region.end:voice.sample.frames;
    if(end<=begin+1)return;
    if(end-begin>kMaximumFrames)end=begin+kMaximumFrames;
    if(voice.begin!=begin || voice.end!=end || voice.reverse!=block.region.reverse) {
        start(voice);voice.begin=begin;voice.end=end;voice.reverse=block.region.reverse;
    }
    i32 q[5];for(unsigned i=0;i<5;++i)q[i]=normalized(block.parameters[i]);
    u32 rate=block.pitch[0]/(kRootIncrement>>10);
    rate=(rate*(voice.sample.rate/100u))/441u;
    rate=multiply16(rate,tuning(block.region.pitch_cents));
    if(rate<16)rate=16;
    const i32 size=441+static_cast<i32>(static_cast<u32>(q[1])*16000u>>15);
    const i32 overlap=4+(q[3]>>10);
    i32 interval=size*8/overlap;if(interval<32)interval=32;
    i32 gain=overlap<=8?32767:32767*8/overlap+6000;if(gain>32767)gain=32767;
    const i32 sample_gain=q[4];
    for(unsigned n=0;n<64;++n) {
        if(--voice.next<=0){spawn(voice,q,rate,size);voice.next=interval;}
        i32 sum[2]={};
        for(unsigned k=0;k<4;++k) {
            Grain& grain=voice.grains[k];if(!grain.increment)continue;
            const u32 at=grain.position>>10;
            if(at>=end-begin-1){grain.increment=0;continue;}
            const i32 sine=port::sine_table[grain.window>>19];
            const i32 window=(sine*sine)>>15;
            for(u32 channel=0;channel<channels;++channel) {
                const i32 a=source(voice,at,channel),b=source(voice,at+1,channel);
                const i32 interpolated=a+(((b-a)*static_cast<i32>(grain.position&1023u))>>10);
                sum[channel]+=(interpolated*window)>>15;
            }
            const u32 limit=(end-begin-1)<<10;
            if(grain.rate>limit-grain.position)grain.increment=0;
            else grain.position+=grain.rate;
            grain.window+=grain.increment;
            if(grain.window>=0x80000000u)grain.increment=0;
        }
        for(u32 channel=0;channel<channels;++channel) {
            i32 y=multiply15(sum[channel],gain);
            y=(y>>13)*sample_gain+(((y&8191)*sample_gain)>>13);
            y=scale_level(clamp16(y),block.region.level);
            block.output[n*channels+channel]=y*32768;
        }
    }
}
}
