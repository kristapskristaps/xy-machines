#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
static void require(bool ok,const char* text){if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(){
    using namespace mando;using namespace mando::dsp;
    for(auto g:tables::pole)require(g>=70&&g<=22971,"immutable pole table fits the signed16 proof");
    // For y=H+floor((H*32768+r+(x-2H)*g)/32768), 0<=r<32768,
    // |y|<=ceil((|32768-2g|*8192+g*X)/32768). Max over legal g.
    int bound=8192;
    for(unsigned pole=0;pole<3;++pole){
        int next=0;
        for(int g=70;g<=22971;++g){
            const int candidate=(std::abs(32768-2*g)*8192+g*bound+32767)/32768;
            if(candidate>next)next=candidate;
            Ladder filter(g,2147483647);int sum=0;
            for(auto weight:filter.weights){require(weight>=0&&weight<32768,"each feedback weight is signed16");sum+=weight;}
            require((-8192*sum>>14)*2>=-32768 && (8192*sum>>14)*2<=32767,"converted Q14 feedback memory fits signed16");
            require(sum<=32767,"feedback sum bounds the converted Q14 memory to signed16");
        }
        require(bound+16384<=32767,"stage difference fits signed16");
        std::printf("Q13 pole%u input bound%d, output bound%d\n",pole,bound,next);bound=next;
    }
    require(bound<16384,"pole2 compensation input fits signed16");
    opxy::u32 seed=7139;unsigned fast_calls=0;
    for(unsigned trial=0;trial<1024;++trial){
        Voice narrow={},wide={};
        for(unsigned i=0;i<4;++i){seed=seed*1664525u+1013904223u;narrow.ladder[i]=(int(seed%16385)-8192)*32768+int(seed>>17);}
        wide=narrow;seed=seed*1664525u+1013904223u;Ladder filter(70+int(seed%22902),int(seed>>1));
        require(filter.short_safe(narrow),"all generated legal states enter the signed16 path");
        for(unsigned frame=0;frame<64;++frame){
            seed=seed*1664525u+1013904223u;const int input=int(seed%262145)-131072;
            const int a=filter.step<true,true>(narrow,input),b=filter.step<true,false>(wide,input);
            require(a==b,"signed16 products equal independent generic32 Q13 arithmetic");
            for(unsigned i=0;i<4;++i)require(narrow.ladder[i]==wide.ladder[i],"all fractional charges match the generic32 Q13 path");
            require(filter.short_safe(narrow),"guarded signed16 updates retain their state invariant");++fast_calls;
        }
    }
    Voice unsafe={};unsafe.ladder[2]=9000*32768;const Ladder normal(22971,2147483647);
    require(!normal.short_safe(unsafe),"out-of-domain state selects generic fallback");
    Voice repaired=unsafe;Ladder::repair_state(repaired);
    const int fallback=normal.step(unsafe,100000),generic=normal.step<true,false>(repaired,100000);
    require(fallback==generic,"guarded wrapper executes the bounded generic fallback");
    for(unsigned i=0;i<4;++i)require(unsafe.ladder[i]==repaired.ladder[i],"fallback preserves the repaired generic fractional charges");
    require(normal.short_safe(unsafe),"fallback repairs the state before bounded generic arithmetic");
    require(!Ladder(32767,0).short_safe(unsafe)&&!Ladder(1,0).short_safe(unsafe),"out-of-table coefficients cannot enter the narrow path");
    std::printf("PASS: %u direct signed16/generic32 frame comparisons, all legal table coefficients, overflow bounds and fallback guard\n",fast_calls);
}
