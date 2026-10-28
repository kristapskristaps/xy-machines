#include "../engine.cpp"
#include <cstdio>
static matter::Voice voices[9];
static matter::Shared shared;
int main() {
    opxy::i32 p[8]={429496730,1073741824,644245094,644245094,0,1073741824,1288490188,2147483647};
    opxy::u32 pitch[64];for(auto& x:pitch)x=42852281u;
    opxy::i32 out[64];
    for(unsigned i=0;i<8;++i) {
        matter::initialize(voices[i],shared,i+1);matter::render(voices[i],shared,i+1,out,p,pitch);
        if(!shared.bows.get(i+1,voices[i].lease))return 1;
        for(unsigned j=0;j<i;++j)if(voices[j].lease==voices[i].lease)return 1;
    }
    // A ninth owner steals the oldest lease; destroying its previous owner
    // must not release the new owner's resource.
    matter::initialize(voices[8],shared,9);matter::render(voices[8],shared,9,out,p,pitch);
    if(shared.bows.get(1,voices[0].lease))return 1;
    matter::destroy(voices[0],shared,1);
    if(!shared.bows.get(9,voices[8].lease))return 1;
    const unsigned increments[]={0,1,100000u,1073741824u,1200000000u};
    for(unsigned increment:increments)for(unsigned mask=0;mask<256;++mask) {
        for(unsigned i=0;i<8;++i)p[i]=mask&(1u<<i)?2147483647:0;
        for(auto& x:pitch)x=increment;
        matter::start(voices[8],shared,9);matter::render(voices[8],shared,9,out,p,pitch);
        for(auto x:out)if(x<-536870912 || x>536870912)return 1;
    }
    for(unsigned i=1;i<9;++i)matter::destroy(voices[i],shared,i+1);
    for(auto owner:shared.bows.owners)if(owner)return 1;
    std::puts("PASS: eight independent resources, safe stealing/release, pitch/control boundaries");
}
