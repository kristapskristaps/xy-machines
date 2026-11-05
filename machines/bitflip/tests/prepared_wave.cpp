#include "../engine.cpp"
#include "reference/pre_model_specialization.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

static void require(bool value,const char* message) {
    if (!value) { std::puts(message); std::exit(1); }
}

int main() {
    using opxy::i32; using opxy::u32;
    u32 random=7119;
    i32 maximum_error=0;
    for (u32 family=0;family<4;++family)
    for (i32 color : {0,1,4095,4096,16383,16384,28672,32766,32767})
    for (u32 band=0;band<5;++band)
    for (i32 rich : {0,8192,16384,24576,32768}) {
        bitflip::Voice v={}; bitflip::Shared shared={};
        bitflip::start(v,shared,1);
        const bitflip::Band limit={band,rich};
        for (u32 block=0;block<9;++block)
            require(!bitflip::prepare_wave(v,family,color,limit,true),"FAIL cache used before completion");
        require(bitflip::prepare_wave(v,family,color,limit,true),"FAIL cache did not finish in ten callbacks");
        for (u32 sample=0;sample<1024;++sample) {
            random=random*1664525u+1013904223u;
            const i32 cached=bitflip::read(v.prepared_wave,random);
            const i32 original=bitflip::wave(family,color,limit,random);
            const i32 error=std::abs(cached-original);
            if (error>maximum_error)maximum_error=error;
            require(error<=3,"FAIL cached waveform exceeds three signed16 LSBs");
        }
        require(!bitflip::prepare_wave(v,family,color,limit,false),"FAIL moving Color used stale waveform");
        require(!bitflip::prepare_wave(v,family,color,limit,true),"FAIL invalidated cache reused");
    }
    double worst_relative_error=0;
    for (u32 model=0;model<8;++model)
    for (double hz : {55.,261.625565,880.,3520.}) {
        bitflip::Voice actual={},expected={}; bitflip::Shared shared={};
        bitflip::initialize(actual,shared,3);bitflip_reference::initialize(expected,shared,3);
        i32 parameters[8]={static_cast<i32>((2*model+1)*0x08000000u),0x40000000,
            0x40000000,0x40000000,0,0,0x7fffffff,0x7fffffff};
        u32 pitch[64];i32 a[64],b[64];double energy=0,error=0;
        for (u32 block=0;block<256;++block) {
            if (block==96)parameters[1]=0x60000000;
            if (block==160)hz*=1.5;
            for (auto& word:pitch)word=static_cast<u32>(hz/44100.*4294967296.);
            bitflip::render(actual,shared,3,a,parameters,pitch);
            bitflip_reference::render(expected,shared,3,b,parameters,pitch);
            require(std::abs(actual.last_raw-expected.last_raw)<=4,"FAIL cache changed raw model beyond rounding bound");
            require(actual.phase==expected.phase && actual.phase2==expected.phase2
                && actual.mod_phase==expected.mod_phase,"FAIL cache changed oscillator tuning");
            for (u32 i=0;i<64;++i) {
                const double delta=static_cast<double>(a[i])-b[i];
                error+=delta*delta;energy+=static_cast<double>(b[i])*b[i];
            }
        }
        const double relative=std::sqrt(error/(energy+1));
        if (relative>worst_relative_error)worst_relative_error=relative;
        require(relative<.002,"FAIL rendered audio differs by more than 0.2% RMS");
    }
    std::printf("PASS: cached waveform error <=%d signed16 LSBs; worst rendered RMS error %.6f; tuning and invalidation preserved\n",maximum_error,worst_relative_error);
}
