#include "../vendor/macro.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok,const char* text) {
    if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}
}
static uint32_t target_at(uint32_t inc,const uint8_t* p) {
    choir_dsp::macro_voice voice={};choir_dsp::macro_init(&voice);int16_t out[32];
    choir_dsp::macro_render_chord_device32(&voice,p,inc,out);
    return voice.e.chord.prepared_target[0];
}
int main() {
    using namespace choir_dsp;
    macro_voice reference={},candidate={},direct={};macro_init(&reference);macro_init(&candidate);macro_init(&direct);
    uint8_t p[7]={64,51,64,51,64,0,0};uint32_t random=91337u,inc=25479940u;unsigned full=0;
    for(unsigned block=0;block<10000;++block) {
        if(block%251==0)for(unsigned k=1;k<5;++k){random=random*1664525u+1013904223u;p[k]=static_cast<uint8_t>(random>>25);}
        if(block%211==0){random=random*1664525u+1013904223u;inc=1000000u+random%400000000u;}
        const uint32_t next=block%23==0?inc+17:inc;int16_t before[64],after[64];
        macro_render_chord_device32(&reference,p,inc,before);macro_render_chord_device32(&reference,p,next,before+32);
        if(inc==next && macro_chord_ready64(&candidate,p,inc)) {
            macro_render_chord_device64(&candidate,p,inc,after);++full;
        } else {macro_render_chord_device32(&candidate,p,inc,after);macro_render_chord_device32(&candidate,p,next,after+32);}
        int32_t direct_output[64];
        if(!(inc==next && macro_render_chord_try64(&direct,p,inc,direct_output))) {macro_render_chord_q31(&direct,p,inc,direct_output,32);macro_render_chord_q31(&direct,p,next,direct_output+32,32);}
        for(unsigned i=0;i<64;++i)require(direct_output[i]==static_cast<int32_t>(before[i])*16384,"direct device output preserves every PCM sample");
        require(!std::memcmp(&reference,&direct,sizeof(reference)),"direct device output preserves complete DSP state");
        require(!std::memcmp(before,after,sizeof(before)),"64-frame scheduling preserves every PCM sample");
        require(!std::memcmp(&reference,&candidate,sizeof(reference)),"64-frame scheduling preserves complete DSP state");
    }
    require(full>3000,"held controls accelerate after both cold starts and moving controls");
    // Force the reference to resolve bands every half-block. This catches a
    // one-unit downward pitch step whose old increment still selects the old mip.
    const uint32_t boundaries[]={10821965u,21815706u,44335146u,91625968u,196341362u,458129844u};
    uint8_t controls[7]={64,51,64,51,64,0,0};unsigned crossings=0;
    for(uint32_t boundary:boundaries) {
        uint32_t lo=4295u,hi=1168698583u;
        while(lo<hi){const uint32_t mid=lo+(hi-lo)/2;if(target_at(mid,controls)>boundary)hi=mid;else lo=mid+1;}
        const uint32_t edge=lo;macro_voice oracle={},actual={};macro_init(&oracle);macro_init(&actual);
        for(unsigned block=0;block<20;++block) {
            const uint32_t frequency=block<6 || block>=13?edge+1:edge-1;
            int16_t reference_pcm[64];int32_t actual_pcm[64];
            if(oracle.engine==MACRO_CHORD)oracle.e.chord.held64=0;
            macro_render_chord_device32(&oracle,controls,frequency,reference_pcm);
            if(oracle.engine==MACRO_CHORD)oracle.e.chord.held64=0;
            macro_render_chord_device32(&oracle,controls,frequency,reference_pcm+32);
            if(!macro_render_chord_try64(&actual,controls,frequency,actual_pcm)) {
                macro_render_chord_q31(&actual,controls,frequency,actual_pcm,32);
                macro_render_chord_q31(&actual,controls,frequency,actual_pcm+32,32);
            }
            for(unsigned i=0;i<64;++i)require(actual_pcm[i]==static_cast<int32_t>(reference_pcm[i])*16384,"mip-edge pitch steps match uncached half-block rendering");
            require(!std::memcmp(oracle.e.chord.voice,actual.e.chord.voice,sizeof(oracle.e.chord.voice)),"mip-edge transitions preserve oscillator state");
        }
        ++crossings;
    }
    require(crossings==6,"all mip boundaries exercised in both directions");
    std::printf("PASS: 10000 moving/held blocks, %u full-block calls, exact PCM and complete DSP state\n",full);
}
