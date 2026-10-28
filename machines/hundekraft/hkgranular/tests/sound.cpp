#include "../sample.cpp"
#include <cstdio>
using namespace hkgranular;
static opxy::i16 pcm[8192*2];
static Voice voice;
int main() {
    for(unsigned i=0;i<8192;++i){pcm[2*i]=(i%128<64?12000:-12000);pcm[2*i+1]=-pcm[2*i];}
    const opxy::SampleView sample={pcm,8192,44100,2};
    initialize(voice,sample);
    opxy::i32 output[128],parameters[8]={0,0x20000000,0,0x60000000,0x20000000};
    opxy::u32 pitch[64];for(unsigned i=0;i<64;++i)pitch[i]=25480370;
    opxy::SampleBlock block={output,parameters,pitch,{1200,0,true},{0,8192,opxy::sample_unity_level,false,0},0,0,2,0,0};
    bool sounding=false,stereo=false;
    for(unsigned b=0;b<500;++b) {
        render_sample(voice,block);
        for(unsigned i=0;i<64;++i){
            if(output[2*i]<-1073741824||output[2*i]>1073741824)return 1;
            sounding=sounding||output[2*i]!=0;
            stereo=stereo||output[2*i]!=output[2*i+1];
        }
    }
    if(!sounding||!stereo)return 2;
    block.region.begin=400;block.region.end=400;
    render_sample(voice,block);for(unsigned i=0;i<128;++i)if(output[i])return 3;
    initialize(voice,opxy::SampleView{0,0,44100,1});
    block.region.begin=0;block.region.end=8192;
    render_sample(voice,block);for(unsigned i=0;i<128;++i)if(output[i])return 4;
    // Trimming to a silent part cannot read the sounding section outside it.
    for(unsigned i=0;i<8192*2;++i)pcm[i]=i<8192?12000:0;
    initialize(voice,sample);block.region.begin=4096;block.region.end=8192;
    for(unsigned b=0;b<100;++b){render_sample(voice,block);for(unsigned i=0;i<128;++i)if(output[i])return 5;}
    return 0;
}
