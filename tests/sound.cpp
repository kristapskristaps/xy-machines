#include "../fx.cpp"
#include <cstdio>
#include <cstring>
#include <cmath>
static halo::State state;
static opxy::i32 memory[halo::buffer_words],left[64],right[64];
static opxy::Buffer buffer={memory,halo::buffer_words};
static opxy::Transport transport={1200,0,true};
static opxy::i32 p[8]={858993459,1181116006,257698038,107374182,966367642,1460288880,0,2147483647};
static void zero(){for(unsigned i=0;i<64;++i)left[i]=right[i]=0;}
int main() {
    std::printf("state bytes: %zu; scratch: %u\n",sizeof(state),halo::buffer_words*4);
    std::memset(memory,0x7f,sizeof memory);halo::initialize(state);
    for(unsigned b=0;b<100;++b){zero();halo::process(state,buffer,transport,left,right,p);for(unsigned i=0;i<64;++i)if(left[i]||right[i])return 1;}
    halo::initialize(state);double energy=0,stereo=0,late=0;
    for(unsigned b=0;b<3500;++b){zero();if(!b)left[0]=right[0]=536870912;halo::process(state,buffer,transport,left,right,p);
        for(unsigned i=0;i<64;++i){energy+=double(left[i])*left[i];stereo+=std::fabs(double(left[i])-right[i]);if(b>700)late+=double(left[i])*left[i];}
    }
    std::printf("IR energy %.3g, stereo %.3g, late %.3g\n",energy,stereo,late);
    if(energy<1e12 || stereo<1e8 || late<1e7)return 1;
    // A reset makes all previously written scratch inaudible, including Hold.
    halo::initialize(state);p[6]=2147483647;
    for(unsigned b=0;b<200;++b){zero();halo::process(state,buffer,transport,left,right,p);for(unsigned i=0;i<64;++i)if(left[i]||right[i])return 1;}
    // Adversarial full-scale stereo and moving controls across tempo endpoints.
    opxy::u32 rng=31;
    for(unsigned b=0;b<6000;++b) {
        if(!(b%64))for(auto& x:p)x=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>1);
        transport.bpm_x10=b%3==0?0:b%3==1?400:3000;
        for(unsigned i=0;i<64;++i){left[i]=static_cast<opxy::i32>(rng=rng*1664525u+1013904223u);right[i]=-(left[i]/2);}
        halo::process(state,buffer,transport,left,right,p);
    }
    opxy::Buffer empty={nullptr,0};halo::process(state,empty,transport,left,right,p);
    for(unsigned i=0;i<64;++i)if(left[i]||right[i])return 1;
    std::puts("PASS: dirty scratch, audible stereo tail, reset, hold, control motion and missing buffer");
}
