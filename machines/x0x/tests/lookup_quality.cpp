#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
static void require(bool okay,const char* why){if(!okay){std::puts(why);std::exit(1);}}
int main(){
    using namespace x0x;using namespace x0x::dsp;
    double energy=0,error=0;int maximum=0;
    for(opxy::u32 phase=0,count=0;count<1048576;++count,phase+=4093){
        const auto index=phase>>23;
        const int a=tables::sine[index],b=tables::sine[index+1];
        const int reference=a+multiply16(b-a,int((phase>>8)&32767));
        const int candidate=sine(phase),difference=candidate-reference;
        if(std::abs(difference)>maximum)maximum=std::abs(difference);
        energy+=double(reference)*reference;error+=double(difference)*difference;
    }
    require(maximum<=53 && std::sqrt(error/energy)<.001,"FAIL tonal sine error bound");
    unsigned random=7139;
    for(unsigned anchor=0;anchor<3;++anchor)for(unsigned branch=0;branch<3;++branch)
    for(unsigned i=0;i<100000;++i){
        random=random*1664525u+1013904223u;const unsigned cursor=random%22050;
        const unsigned fraction=i%3==0?0:i%3==1?32768:((random>>8)&65535);
        const int a=tables::metal_stems[anchor][branch][cursor],b=tables::metal_stems[anchor][branch][cursor+1];
        require(stem_read(anchor,branch,cursor,fraction)==a+multiply16(b-a,int(fraction>>1)),"FAIL stem interpolation changed");
    }
    std::printf("PASS: 1,048,576 sine phases, peak error %d/32768, relative RMS %.6f; 900,000 exact stem reads\n",maximum,std::sqrt(error/energy));
}
