#include "../engine.cpp"
#include "reference/wt_before_settled.h"
#include <cstdio>
#include <cstdlib>
static void require(bool value,const char* message){if(!value){std::puts(message);std::exit(1);}}
static int maximum_error=0;
static void checked_table(const opxy::i32* t){
    for(int i=0;i<128;++i)for(int j=0;j<=32;++j){
        const opxy::i32 f=static_cast<opxy::i32>(0x7fffffffull*j/32),fraction=f>>17;
        const opxy::i32 c=(t[i+2]-t[i])>>1,v=t[i+1]-t[i+2],w=c+v;
        const opxy::i32 a=w+v+((t[i+3]-t[i+1])>>1),bn=w+a;
        long long product=static_cast<long long>(a>>3)*fraction;
        require(product>=-2147483648ll && product<=2147483647ll,"FAIL first interpolation product overflow");
        long long value=(product>>14)-(bn>>3);product=value*fraction;
        require(product>=-2147483648ll && product<=2147483647ll,"FAIL second interpolation product overflow");
        value=(product>>14)+(c>>3);product=value*fraction;
        require(product>=-2147483648ll && product<=2147483647ll,"FAIL third interpolation product overflow");
        value=(product>>14)*8+t[i+1];
        const opxy::i32 actual=atlas_dsp::wt_hermite(t,i,f);
        require(actual==value,"FAIL narrow interpolation differs from independent wide arithmetic");
        const int error=std::abs(actual-atlas_dsp::wt_hermite_q31_reference(t,i,f));
        if(error>maximum_error)maximum_error=error;
        require(error<=128,"FAIL interpolation error exceeds half a source-table unit");
    }
}
int main(){
    opxy::i32 table[132],coarse[132];
    for(unsigned wave=0;wave<192;++wave){
        for(unsigned i=0;i<132;++i)table[i]=static_cast<opxy::i32>(atlas_dsp::wt_waves[wave*132+i])*256;
        checked_table(table);
        for(unsigned i=1;i<129;i+=4)coarse[i]=table[i];
        for(unsigned i=1;i<129;i+=4){
            const opxy::i32 from=coarse[i],to=coarse[i==125?1:i+4],step=(to-from)/4;
            coarse[i+1]=from+step;coarse[i+2]=from+2*step;coarse[i+3]=from+3*step;
        }
        coarse[0]=coarse[128];coarse[129]=coarse[1];coarse[130]=coarse[2];coarse[131]=coarse[3];
        checked_table(coarse);
    }
    unsigned random=7719;
    for(unsigned trial=0;trial<512;++trial){
        opxy::i32 key[3];for(auto& value:key)value=static_cast<opxy::i32>((random=random*1664525u+1013904223u)%117440512u);
        atlas_dsp::wt_blend_tile<1>(key,table,0,132);checked_table(table);
    }
    std::printf("PASS: every source/coarse wave and 512 blended waves have bounded signed32 products; maximum interpolation error %d Q8 units\n",maximum_error);
}
