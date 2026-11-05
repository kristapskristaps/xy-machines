#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>

void require(bool value,const char* message){if(!value){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
    static_assert(sizeof(bitflip::Voice)==628,"declared voice storage");
    unsigned random=7139;
    for(unsigned i=0;i<1000000;++i){
        const unsigned pitch=(random=random*1664525u+1013904223u);
        const unsigned ratio=1024+(random=random*1664525u+1013904223u)%64513;
        const unsigned long long full=(static_cast<unsigned long long>(pitch)*ratio)>>12;
        require(bitflip::transpose(pitch,ratio)==(full>0x60000000u?0x60000000u:static_cast<unsigned>(full)),"split pitch multiply matches independent 64-bit arithmetic");
    }
    for(unsigned shift=0;shift<=14;++shift)for(int sample=-32768;sample<=32767;++sample){
        const int value=bitflip::quantize(sample,shift);
        require(std::abs(value-sample)<(1<<shift),"DAC quantization has its stated bound");
        require(value==-bitflip::quantize(-sample,shift),"quantizer is symmetric around zero");
    }
    bitflip::Voice voice={};voice.long_noise=voice.short_noise=1;
    for(unsigned i=1;i<=32767;++i){
        bitflip::noise_step(voice);require(voice.long_noise&&voice.short_noise,"noise registers cannot lock at zero");
        require((voice.long_noise==1)==(i==32767),"long LFSR has the full 32,767-step period");
        require((voice.short_noise==1)==(i%127==0),"short LFSR has the full 127-step period");
    }
    for(unsigned family=0;family<4;++family)for(unsigned color=0;color<9;++color)for(unsigned band=0;band<5;++band){
        const auto* table=bitflip::tables::waves[family][color][band];long sum=0;
        for(unsigned i=0;i<256;++i){sum+=table[i];require(std::abs(int(table[i]))<=27200,"wave banks retain fixed headroom");}
        require(table[256]==table[0]&&std::abs(sum)<=256,"wave guards are periodic and DC free");
    }
    std::puts("PASS: one million pitch products, every 2..16-bit quantizer input, complete noise periods and all waveform bank bounds");
}
