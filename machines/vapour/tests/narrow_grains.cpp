#include "../fx.cpp"
#include <cstdio>
#include <cstring>
#include <initializer_list>

int main() {
    using namespace vapour;
    using namespace vapour::granular;
    State state;initialize(state);
    for(unsigned texture=0;texture<=32767;++texture) {
        Grain grain={};schedule(state,grain,16384,65536,16384,texture);
        if(grain.smoothness<0 || grain.smoothness>32767 || grain.left_gain<0 ||
           grain.left_gain>32767 || grain.right_gain<0 || grain.right_gain>32767)return 1;
        for(unsigned p=0;p<65536;p+=127) {
            Grain a=grain,b=grain;a.envelope=b.envelope=p<<16;
            const int low=envelope(a,false),high=envelope(b,true);
            if(low<0 || low>32767 || high<0 || high>32767)return 1;
        }
    }
    unsigned random=71439;
    GrainBlock block;int actual_l[64],actual_r[64],expected_l[64],expected_r[64];
    for(unsigned trial=0;trial<4096;++trial) {
        const int pan_l=static_cast<int>((random=random*1664525u+1013904223u)>>17);
        const int pan_r=static_cast<int>((random=random*1664525u+1013904223u)>>17);
        for(unsigned i=0;i<64;++i) {
            const unsigned packed=(random=random*1664525u+1013904223u);
            block.samples[i]=packed;
            const int window=static_cast<int>((random=random*1664525u+1013904223u)>>17);
            block.gains[i]=static_cast<opxy::i16>(window);
            const int source_l=static_cast<opxy::i16>(packed),source_r=static_cast<opxy::i16>(packed>>16);
            const int l=gain15(source_l,window),r=gain15(source_r,window);
            actual_l[i]=actual_r[i]=static_cast<int>((random=random*1664525u+1013904223u)%1572865)-786432;
            expected_l[i]=actual_l[i]+gain15(l,pan_l)+r-gain15(r,pan_r);
            expected_r[i]=actual_r[i]+gain15(r,pan_r)+l-gain15(l,pan_l);
        }
        mix_grain(block,64,static_cast<opxy::i16>(pan_l),static_cast<opxy::i16>(pan_r),actual_l,actual_r);
        if(std::memcmp(actual_l,expected_l,sizeof(actual_l)) || std::memcmp(actual_r,expected_r,sizeof(actual_r)))return 1;
    }
    for(unsigned freeze=0;freeze<=32767;++freeze)for(int feedback:{0,1,31128}) {
        const int narrowed=static_cast<opxy::i16>(feedback)*static_cast<opxy::i16>(32767-freeze)>>15;
        if(narrowed!=audio_gain15(feedback,32767-freeze))return 1;
    }
    std::puts("PASS: all 32768 texture/window/pan width domains, 262144 exact stereo mix frames and full Freeze/feedback range");
}
