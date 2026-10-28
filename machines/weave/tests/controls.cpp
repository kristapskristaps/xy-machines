#include "../engine.cpp"
#include <cstdio>
#include <cstring>
#include <initializer_list>

static opxy::i32 parameter(unsigned value, unsigned count) {
    return static_cast<opxy::i32>((2u*value+1u)*2147483647ull/(2u*count));
}
static unsigned hash_audio(unsigned model,unsigned strike,unsigned mode,unsigned seed,unsigned owner) {
    static weave::Voice v;weave::Shared shared;
    opxy::i32 p[8]={parameter(model,15),0x40000000,0x40000000,0x40000000,0x40000000,
                    parameter(strike,4),parameter(mode,2),parameter(seed,128)};
    opxy::u32 pitch[64];for(auto& x:pitch)x=42852281u;
    opxy::i32 out[64];unsigned hash=2166136261u;
    weave::initialize(v,shared,owner);
    for(unsigned block=0;block<500;++block) {
        if(block%137==0)weave::start(v,shared,owner);
        weave::render(v,shared,owner,out,p,pitch);
        for(auto x:out) {
            if(x<-536870912 || x>536870912)return 0;
            hash=(hash^static_cast<unsigned>(x))*16777619u;
        }
    }
    return hash;
}
int main() {
    // Excitation must make an audible change for every advertised model.
    for(unsigned model:{4u,5u,6u,12u,13u}) {
        const unsigned stock=hash_audio(model,0,0,17,1);
        for(unsigned strike=1;strike<4;++strike) {
            unsigned changed=hash_audio(model,strike,0,17,1);
            if(!changed || changed==stock){std::printf("FAIL strike %u/%u\n",model,strike);return 1;}
        }
    }
    // Fixed random streams remove owner dependence; free streams keep it.
    for(unsigned model:{2u,3u,4u,5u,6u,9u,13u}) {
        unsigned fixed=hash_audio(model,0,0,17,1);
        if(!fixed || fixed!=hash_audio(model,0,0,17,55) || fixed==hash_audio(model,0,0,73,1) ||
           hash_audio(model,0,0,0,1)==hash_audio(model,0,0,0,55)) {
            std::printf("FAIL seed %u\n",model);return 1;
        }
    }
    for(unsigned model:{9u,14u})if(hash_audio(model,0,0,17,1)==hash_audio(model,0,1,17,1)) {
        std::printf("FAIL hold %u\n",model);return 1;
    }
    // The hold selector can revive a swarm after its burst rate has decayed.
    weave_dsp::macro_genv envelope={};unsigned rng=1;
    envelope.fm=1;
    weave_dsp::genv_step(&envelope,&rng,1234,0,1);
    if(envelope.fm!=(1<<30))return 1;
    // Every model/output remains bounded with all added controls moving.
    static weave::Voice voice;weave::Shared shared;
    opxy::i32 p[8]={},out[64];opxy::u32 pitch[64];
    for(auto& x:pitch)x=42852281u;
    weave::initialize(voice,shared,8);rng=42;
    for(unsigned block=0;block<4000;++block) {
        for(auto& x:p)x=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        if(block%7==0)weave::start(voice,shared,8);
        weave::render(voice,shared,8,out,p,pitch);
        for(auto x:out)if(x<-536870912 || x>536870912)return 1;
    }
    std::puts("PASS: strike on five models, free/fixed seeds, held swarm/FM, live mode switching, moving eight-slot controls and bounds");
}
