#include "../engine.cpp"
#include <cstdio>

static bool row(const opxy::i16* table,int& maximum) {
    for(unsigned i=0;i<1024;++i) {
        const int delta=table[i+1]-table[i];
        if(delta<-32768 || delta>32767)return false;
        const int magnitude=delta<0?-delta:delta;if(magnitude>maximum)maximum=magnitude;
        for(unsigned fraction=0;fraction<1024;fraction+=31) {
            const unsigned at=(i<<10)|fraction;
            if(mando::dsp::linear(table,at,10)!=table[i]+(delta*static_cast<int>(fraction)>>10))return false;
        }
    }
    return true;
}
int main() {
    int slope=0;
    for(unsigned band=0;band<9;++band)
        if(!row(mando::tables::saw[band],slope) || !row(mando::tables::square[band],slope))return 1;
    for(unsigned i=0;i<512;++i) {
        const int d=mando::tables::tanh_q14[i+1]-mando::tables::tanh_q14[i];
        if(d<-32768 || d>32767)return 1;
        for(unsigned fraction=0;fraction<256;++fraction)
            if(mando::dsp::linear(mando::tables::tanh_q14,i*256+fraction,8)!=mando::tables::tanh_q14[i]+(d*static_cast<int>(fraction)>>8))return 1;
    }
    // Every reachable pole coefficient lies between these table endpoints.
    // Both limited and proven no-limit kernels keep half-states within±16384.
    // Thus the dot product's memory fits signed16 when the weight sum≤32767.
    for(int g=1;g<=32767;++g) {
        const mando::dsp::Ladder filter(g,2147483647);
        int sum=0;
        for(int weight:filter.weights){if(weight<0 || weight>32767)return 1;sum+=weight;}
        if(sum>32767 || filter.resonance<0 || filter.resonance>32767)return 1;
        const long long maximum=16384ll*sum;
        if((maximum>>14)>32767 || ((-maximum)>>14)<-32768)return 1;
    }
    std::printf("PASS: all18 Q14 waveform slopes≤%d, every tanh interpolant and32767 ladder coefficient/feedback width proofs\n",slope);
}
