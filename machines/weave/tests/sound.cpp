#include "../engine.cpp"
#include <cstdio>
#include <cmath>
#include <cstring>

int main() {
    static weave::Voice voice, other;
    static weave::Shared shared;
    opxy::i32 parameters[8] = {};
    opxy::u32 pitch[64];
    opxy::i32 output[64], comparison[64];
    for (auto& p : pitch) p = 42852281u;
    for (unsigned model = 0; model < 15; ++model) {
        parameters[0] = static_cast<opxy::i32>((model * 32768u + 1200u) / 15u * 65536u);
        for (unsigned i=1;i<4;++i) parameters[i] = 0x40000000;
        weave::initialize(voice,shared,1); weave::start(voice,shared,1);
        double energy = 0;
        for (unsigned block=0;block<690;++block) {
            weave::render(voice,shared,1,output,parameters,pitch);
            for (auto s : output) {
                if (s < -536870912 || s > 536870912) return 1;
                energy += double(s)*s;
            }
        }
        if (energy < 1e12) { std::printf("FAIL: model %u silent\n",model); return 1; }
        std::printf("model %2u RMS %.6f\n",model,std::sqrt(energy/44160)/2147483648.0);
    }
    // Identically seeded voices must remain deterministic despite interleaving.
    parameters[0]=0; parameters[4]=0;
    weave::initialize(voice,shared,7); weave::initialize(other,shared,7);
    for (unsigned block=0;block<100;++block) {
        weave::render(voice,shared,7,output,parameters,pitch);
        weave::render(other,shared,7,comparison,parameters,pitch);
        if (std::memcmp(output,comparison,sizeof output)) return 1;
    }
    // Model changes and both outputs must stay bounded at moving/end controls.
    opxy::u32 rng=42;
    for (unsigned block=0;block<3000;++block) {
        if (!(block%23)) weave::start(voice,shared,7);
        for (unsigned p=0;p<5;++p) parameters[p]=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        if (block%5==0) for (unsigned p=1;p<5;++p) parameters[p]=(block&1)?0:2147483647;
        for (unsigned i=0;i<64;++i) pitch[i]=100000u+(rng%500000000u);
        weave::render(voice,shared,7,output,parameters,pitch);
        for (auto s : output) if (s < -536870912 || s > 536870912) return 1;
    }
    weave::destroy(voice,shared,7);
    std::puts("PASS: fifteen models, outputs, retriggers, independent voices and moving controls");
}
