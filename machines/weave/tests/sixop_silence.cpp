#include "../vendor/macro.cpp"
#include <cstring>
#include <cstdio>
int main() {
    using namespace weave_dsp;
    opxy::u32 random=901;
    for(unsigned test=0;test<20000;++test) {
        const unsigned size=1+test%64;
        const int src=test%3-2,add=(test/3)&1,amount=test%8;
        random=random*1664525u+1013904223u;
        macro_dxop actual={random,static_cast<opxy::i32>(random&16383)},expected=actual;
        const opxy::u32 frequency=random*2654435761u;
        const opxy::i32 target=(random>>17)&16383;
        opxy::i32 afb[2]={static_cast<opxy::i32>(random&0x0fffffff),-static_cast<opxy::i32>(random>>4)},efb[2]={afb[0],afb[1]};
        opxy::i32 input[64],actual_out[64],expected_out[64];
        for(unsigned i=0;i<64;++i){random=random*1664525u+1013904223u;input[i]=actual_out[i]=expected_out[i]=static_cast<opxy::i32>(random);}
        const int step=(target-expected.amp)/static_cast<int>(size);
        for(unsigned i=0;i<size;++i) {
            const int modulation=src==-2?input[i]:src==0?(amount?(efb[0]+efb[1])>>(9-amount):0):0;
            expected.phase+=frequency;
            const int sample=dx_sine_product(expected.phase+(static_cast<opxy::u32>(modulation)<<6),expected.amp);
            if(sample!=0)return 1;
            expected.amp+=step;
            if(src==0){efb[1]=efb[0];efb[0]=sample;}
            expected_out[i]=add?expected_out[i]+sample:sample;
        }
        dx_op1(&actual,frequency,target,src,add,afb,amount,input,actual_out,size);
        if(std::memcmp(&actual,&expected,sizeof(actual)) || std::memcmp(afb,efb,sizeof(afb)) || std::memcmp(actual_out,expected_out,sizeof(actual_out)))return 1;
    }
    for(unsigned test=0;test<4096;++test) {
        const int size=1+test%64,count=2+test%2,src=static_cast<int>(test%5)-2,add=test&1;
        if(src>=count)continue;
        macro_dxop actual[3],expected[3];opxy::u32 frequencies[3];opxy::i32 targets[3],steps[3];
        int afb[2]={1<<25,-(1<<24)},efb[2]={afb[0],afb[1]};
        int input[64],actual_out[64],expected_out[64];
        for(int k=0;k<count;++k) {
            random=random*1664525u+1013904223u;
            actual[k].phase=expected[k].phase=random;
            actual[k].amp=expected[k].amp=random&16383;
            targets[k]=(random>>17)&16383;frequencies[k]=random*2654435761u;
            steps[k]=(targets[k]-expected[k].amp)/size;
        }
        for(int i=0;i<64;++i)input[i]=actual_out[i]=expected_out[i]=i*17365;
        for(int i=0;i<size;++i) {
            int pm=src>=0?(efb[0]+efb[1])>>2:src==-2?input[i]:0;
            for(int k=0;k<count;++k) {
                expected[k].phase+=frequencies[k];
                pm=dx_sine_product(expected[k].phase+(static_cast<opxy::u32>(pm)<<6),expected[k].amp);
                if(pm!=0)return 1;
                expected[k].amp+=steps[k];
                if(k==src){efb[1]=efb[0];efb[0]=pm;}
            }
            expected_out[i]=add?expected_out[i]+pm:pm;
        }
        dx_ops(actual,frequencies,targets,count,src,add,afb,7,input,actual_out,size);
        if(std::memcmp(actual,expected,count*sizeof(actual[0])) || std::memcmp(afb,efb,sizeof(afb)) || std::memcmp(actual_out,expected_out,sizeof(actual_out)))return 1;
    }
    std::puts("PASS: 20,000 silent operator ramps and 4,096 serial-chain cases preserve PCM, phase, amplitude and feedback for all block lengths/routes");
}
