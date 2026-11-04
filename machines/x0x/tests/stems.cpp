#include "../engine.cpp"
#include "reference/primitives.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
static void require(bool ok,const char* message){if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
    using namespace x0x;using namespace x0x::dsp;
    double old_energy=0,error_energy=0;int maximum_error=0;
    for(unsigned n=0;n<65536;++n){
        const unsigned phase=n<<16;
        const int a=legacy_square(phase),b=square(phase),c=legacy_square_sixth(phase),d=square_sixth(phase);
        old_energy+=double(a)*a+double(c)*c;error_energy+=double(a-b)*(a-b)+double(c-d)*(c-d);
        if(std::abs(a-b)>maximum_error)maximum_error=std::abs(a-b);
        if(std::abs(c-d)>maximum_error)maximum_error=std::abs(c-d);
    }
    require(maximum_error<32 && std::sqrt(error_energy/old_energy)<.001,"three-partial table preserves the original shape within 0.1% RMS");
    for(unsigned anchor=0;anchor<3;++anchor)for(unsigned branch=0;branch<3;++branch){
        const auto* row=tables::metal_stems[anchor][branch];
        require(row[0]==row[22050],"periodic interpolation endpoint");
        long long sum=0,energy=0;int largest_step=0;
        for(unsigned i=0;i<22050;++i){sum+=row[i];energy+=static_cast<long long>(row[i])*row[i];const int step=std::abs(int(row[i+1])-int(row[i]));if(i+1<22050 && step>largest_step)largest_step=step;}
        require(energy>100000 && std::abs(double(sum)/22050)<8,"periodic stem energy/DC bounds");
        require(std::abs(int(row[0])-int(row[22049]))<=largest_step,"loop seam is bounded by natural source steps");
    }
    for(unsigned character=0;character<128;++character){
        opxy::i32 p[8]={opxy::i32(family_raw(cymbal,2))<<24,64<<24,64<<24,64<<24,0,opxy::i32(character<<24),64<<24,0};
        const auto s=settings(cymbal,p);
        require(s.sample_step==tables::tuning[64],"Character changes the oscillator texture without duplicating Tune");
        require(s.metal_reciprocals[0]<=1 && s.metal_reciprocals[1]<=32768,"bounded Character anchor interpolation");
    }
    std::printf("PASS: periodic stems and all Character settings; square point error%d, relative RMS%.8f\n",maximum_error,std::sqrt(error_energy/old_energy));
}
