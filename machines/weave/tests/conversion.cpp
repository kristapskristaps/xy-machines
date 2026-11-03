#include "../engine.cpp"
#include <cstdio>

namespace {
struct Reference {
    weave_dsp::macro_voice oscillator;
    opxy::i16 samples[32],history[24];
    unsigned read,write,phase,selected,pending;
};
static Reference reference;
static weave::Voice voice;

void reference_render(opxy::i32* out,const opxy::i32* p,const opxy::u32* pitch) {
    const unsigned selected=static_cast<unsigned>(p[0]>>16)*15u>>15;
    opxy::u8 controls[7]={};controls[0]=static_cast<opxy::u8>(selected*8);
    for(unsigned i=1;i<5;++i)controls[i]=static_cast<opxy::u8>(p[i]>>24);
    if(reference.pending || selected!=reference.selected) {
        weave_dsp::macro_trig(&reference.oscillator);reference.read=32;
    }
    reference.selected=selected;reference.pending=0;
    for(unsigned i=0;i<64;++i) {
        reference.phase+=160;
        while(reference.phase>=147) {
            reference.phase-=147;
            if(reference.read==32) {
                const opxy::u32 increment=(pitch[i]/160u)*147u+((pitch[i]%160u)*147u)/160u;
                weave_dsp::macro_render(&reference.oscillator,controls,increment,reference.samples,32);
                reference.read=0;
            }
            reference.history[reference.write]=reference.samples[reference.read++];
            reference.write=(reference.write+1)%24;
        }
        opxy::i32 sum=0;
        for(unsigned tap=0;tap<24;++tap)
            sum+=static_cast<opxy::i32>(reference.history[(reference.write+23-tap)%24])*weave::resample_coefficients[reference.phase][tap];
        const opxy::i32 sample=sum>>15;
        out[i]=(sample<-32768?-32768:sample>32767?32767:sample)*16384;
    }
}
}
int main() {
    for(const auto& coefficients:weave::resample_coefficients){
        if(coefficients[0] || coefficients[23]) {
            std::puts("FAIL: omitted FIR endpoint has a nonzero coefficient");return 1;
        }
        opxy::i64 bound=0;
        for(auto coefficient:coefficients)bound+=static_cast<opxy::i64>(coefficient<0?-coefficient:coefficient)*32768;
        if(bound>2147483647){std::puts("FAIL: full-scale FIR accumulation can overflow");return 1;}
    }
    weave::Shared shared={};
    weave::initialize(voice,shared,1);
    weave_dsp::macro_init(&reference.oscillator);
    reference.oscillator.rng=0x9e3779b9u+1u;
    reference.read=32;reference.pending=1;
    opxy::i32 p[8]={},actual[64],expected[64];opxy::u32 pitch[64],rng=5;
    for(unsigned model=0;model<15;++model)for(unsigned block=0;block<64;++block) {
        p[0]=static_cast<opxy::i32>((2*model+1)*2147483647LL/30);
        for(unsigned i=1;i<5;++i)p[i]=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        for(auto& increment:pitch)increment=400000u+(rng%100000000u);
        if(block==31) {weave::start(voice,shared,1);reference.pending=1;}
        weave::render(voice,shared,1,actual,p,pitch);reference_render(expected,p,pitch);
        for(unsigned i=0;i<64;++i)if(actual[i]!=expected[i]) {
            std::printf("FAIL: converter model %u block %u frame %u\n",model,block,i);return 1;
        }
    }
    std::puts("PASS: bit-exact 48/44.1 conversion against circular convolution for all 15 models");
}
