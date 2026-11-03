#include "../engine.cpp"
#include <cmath>
#include <cstdio>

static double peak(const matter::Filter& mode) {
    const double cosine=2.*mode.a1/(2147483648.+mode.a2);
    return std::acos(cosine)/(6.283185307179586*mode.rate);
}
int main() {
    double worst=0;
    for(unsigned bowed=0;bowed<2;++bowed)for(unsigned trace=0;trace<3;++trace) {
        matter::Voice voice={};voice.key[0]=-1;
        unsigned changed=0;
        for(unsigned step=0;step<2048;++step) {
            const double hz=trace==0?261.625565+std::sin(step*.13)*.0001:
                            trace==1?110+step/2048.:440*std::exp2(std::sin(step*.045)*50/1200.);
            const opxy::u32 increment=static_cast<opxy::u32>(hz/44100.*4294967296.+.5);
            const opxy::i32 previous=voice.key[0];
            if(bowed)matter::configure<true>(voice,increment,6553,16384,9830);else matter::coefficients(voice,increment,6553,16384,9830);
            if(previous!=voice.key[0])++changed;
            const opxy::u32 cached=static_cast<opxy::u32>(voice.key[0]);
            const double delta=std::fabs(1200*std::log2(double(cached)/increment));
            if(delta>.423)return 1;
            matter::Voice reference={};reference.key[0]=-1;
            if(bowed)matter::configure<true>(reference,increment,6553,16384,9830);else matter::coefficients(reference,increment,6553,16384,9830);
            for(unsigned mode=0;mode<(bowed?8:matter::mode_count);++mode) {
                if(!(voice.active_modes[mode/32]&reference.active_modes[mode/32]&(1u<<(mode%32))))continue;
                const double candidate=bowed?std::acos(2.*voice.bowed[mode].a1/(2147483648.+voice.bowed[mode].a2))/voice.bow_rate:peak(voice.modes[mode]);
                const double fresh=bowed?std::acos(2.*reference.bowed[mode].a1/(2147483648.+reference.bowed[mode].a2))/reference.bow_rate:peak(reference.modes[mode]);
                const double error=std::fabs(1200*std::log2(candidate/fresh));
                if(error>worst)worst=error;
                if(error>.6)return 1;
            }
        }
        if((trace==0 && changed!=1) || (trace!=0 && (changed<2 || changed>=2048)))return 1;
        std::printf("bowed%u trace%u coefficient refreshes %u/2048\n",bowed,trace,changed);
    }
    for(unsigned rate=2;rate<=8;rate*=2)for(unsigned reverse=0;reverse<2;++reverse) {
        opxy::i32 output[8]={},last=reverse?1610612736:-1610612736;
        const opxy::i32 current=-last;
        if(rate==2)matter::upsample<2>(current,last,output,false);
        if(rate==4)matter::upsample<4>(current,last,output,false);
        if(rate==8)matter::upsample<8>(current,last,output,false);
        if(last!=current || output[rate-1]!=current)return 1;
    }
    std::printf("PASS: jitter/glide/vibrato coefficient peak error <=%.4f cents and full-bank interpolation headroom\n",worst);
}
