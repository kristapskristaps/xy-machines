#include "../fx.cpp"
#include <cstdio>
static opxy::i32 ring[vapour::ring_frames];
int main(){
    using opxy::i32;using opxy::u32;
    vapour::State state={};u32 random=7139;
    for(auto& packed:ring)packed=static_cast<i32>(random=random*1664525u+1013904223u);
    const u32 written[]={0,1,2,3,32768,65534,65535,65536};
    for(auto count:written){
        state.written=count;
        for(unsigned trial=0;trial<100000;++trial){
            const u32 phase=trial<16?0xffffffffu-trial*32768u:(random=random*1664525u+1013904223u);
            i32 left,right;
            vapour::read_stereo<false>(state,ring,phase,left,right);
            if(left!=vapour::granular::read(state,ring,phase,0,false) || right!=vapour::granular::read(state,ring,phase,1,false))return 1;
            vapour::read_stereo<true>(state,ring,phase,left,right);
            if(left!=vapour::granular::read(state,ring,phase,0,true) || right!=vapour::granular::read(state,ring,phase,1,true))return 1;
        }
    }
    std::puts("PASS: exact packed stereo reads against independent channel reads through startup, wrap, linear and cubic interpolation");
}
