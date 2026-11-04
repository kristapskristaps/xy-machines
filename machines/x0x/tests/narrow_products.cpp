#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
static void require(bool ok,const char* text){if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(){
    using namespace x0x;using namespace x0x::dsp;
    const int coefficients[]={-32768,-32767,-16384,-127,-1,0,1,127,256,4096,16383,16384,24576,28000,32766,32767};
    for(int sample=-32768;sample<=32767;++sample)for(int coefficient:coefficients)
        require(multiply16(sample,coefficient)==int(std::floor((double(sample)*coefficient+16384)/32768)),"narrow signed product rounds within half a Q15 count");
    int largest_slope=0;
    const auto check=[&](const opxy::i16* row,unsigned count){for(unsigned i=0;i<count;++i){const int difference=int(row[i+1])-int(row[i]);require(difference>=-32768&&difference<=32767,"every narrowed interpolation slope fits signed16");if(std::abs(difference)>largest_slope)largest_slope=std::abs(difference);}};
    check(tables::sine,512);for(unsigned shape=0;shape<2;++shape)check(tables::metallic_square[shape],1024);
    for(unsigned anchor=0;anchor<3;++anchor)for(unsigned branch=0;branch<3;++branch)check(tables::metal_stems[anchor][branch],22050);
    // The noise LP is a convex update of [-16384,16383]. Its source difference
    // fits signed16; the full-scale output LP/DC differences deliberately do not.
    unsigned seed=7139;int low=0;
    for(unsigned i=0;i<1000000;++i){const int random=noise(seed),coefficient=i%2?28000:16384;
        require(random-low>=-32768&&random-low<=32767,"noise source difference fits signed16");
        low+=multiply16(random-low,coefficient);require(low>=-16384&&low<=16383,"noise lowpass invariant");}
    for(unsigned model=0;model<model_count;++model)for(unsigned family=0;family<4;++family){if(!(family_masks[model]&(1u<<family)))continue;
        Voice v;Shared s;initialize(v,s,1);start(v,s,1);drum_select(v,s,1,model);int p[8],out[64];unsigned pitch[64]={};
        for(unsigned block=0;block<128;++block){for(unsigned i=0;i<8;++i)p[i]=int((seed=seed*1664525u+1013904223u)>>1);p[0]=int(family_raw(model,family))<<24;render(v,s,1,out,p,pitch);
            require(v.dc_output>=-32767&&v.dc_output<=32767,"stored DC output is explicitly limited before its narrow feedback product");
            require(v.output_low>=-32767&&v.output_low<=32767,"output lowpass bounds the narrowed Drive input");
            require(v.noise_low>=-16384&&v.noise_low<=16383,"all model switches preserve the noise invariant");}
    }
    std::printf("PASS: exact signed16 products, max interpolation slope%d, noise/Drive domains and all47 model/family paths\n",largest_slope);
}
