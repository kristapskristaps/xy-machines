#include "../engine.cpp"
#include <cstdio>
static matter::Voice voices[matter::bow_capacity+1];
static matter::Shared shared;
int main() {
    opxy::i32 p[8]={429496730,1073741824,644245094,644245094,1200000000,1073741824,1288490188,2147483647};
    opxy::u32 pitch[64];for(auto& x:pitch)x=42852281u;
    opxy::i32 out[64];
    for(unsigned i=0;i<matter::bow_capacity;++i) {
        matter::initialize(voices[i],shared,i+1);matter::render(voices[i],shared,i+1,out,p,pitch);
        if(!shared.bows.get(i+1,voices[i].lease))return 1;
        for(unsigned j=0;j<i;++j)if(voices[j].lease==voices[i].lease)return 1;
    }
    // An extra owner steals the oldest lease; destroying its previous owner
    // must not release the new owner's resource.
    const unsigned extra=matter::bow_capacity;
    matter::initialize(voices[extra],shared,extra+1);matter::render(voices[extra],shared,extra+1,out,p,pitch);
    if(shared.bows.get(1,voices[0].lease))return 1;
    matter::destroy(voices[0],shared,1);
    if(!shared.bows.get(extra+1,voices[extra].lease))return 1;
    const unsigned increments[]={0,1,100000u,1073741824u,1200000000u};
    for(unsigned increment:increments)for(unsigned mask=0;mask<256;++mask) {
        for(unsigned i=0;i<8;++i)p[i]=mask&(1u<<i)?2147483647:0;
        for(auto& x:pitch)x=increment;
        matter::start(voices[extra],shared,extra+1);matter::render(voices[extra],shared,extra+1,out,p,pitch);
        for(auto x:out)if(x<-536870912 || x>536870912)return 1;
    }
    for(unsigned i=1;i<=extra;++i)matter::destroy(voices[i],shared,i+1);
    for(auto owner:shared.bows.owners)if(owner)return 1;
    // Ordinary exciters never acquire or clear the large bowed delay resource.
    for(unsigned model=0;model<6;++model) {
        if(model==3)continue;
        p[4]=static_cast<opxy::i32>((model*32768u+1200u)/6u*65536u);
        matter::initialize(voices[0],shared,1);
        matter::render(voices[0],shared,1,out,p,pitch);
        if(voices[0].lease)return 1;
        for(auto owner:shared.bows.owners)if(owner)return 1;
    }
    // Switching away from bow releases its resource immediately.
    p[4]=1200000000;matter::render(voices[0],shared,1,out,p,pitch);
    if(!shared.bows.get(1,voices[0].lease))return 1;
    p[4]=0;matter::render(voices[0],shared,1,out,p,pitch);
    if(voices[0].lease)return 1;
    for(auto owner:shared.bows.owners)if(owner)return 1;
    std::puts("PASS: bounded bow resources, safe stealing/release, pitch/control boundaries");
}
