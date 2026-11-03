#include "../dsp.h"
#include "../display_tables.h"
#include <cstring>
#include <cstdio>

static bool exact(opxy::u32 increment) {
    opxy::u32 threshold=0x60000000u>>7,index=0;
    while(index<7 && increment>threshold){threshold*=2;++index;}
    opxy::i32 fade=0;
    if(index<7 && increment>threshold/2)
        fade=static_cast<opxy::i32>((increment-threshold/2)/(threshold>>16));
    if(fade>32768)fade=32768;
    const westfold::dsp::Band band=westfold::dsp::band(increment);
    return band.index==index && band.fade==fade;
}
int main() {
    if(std::memcmp(westfold::tables::waves[1],westfold::draw_tables::waves,sizeof(westfold::draw_tables::waves)))return 1;
    if(sizeof(westfold::tables::sine)!=sizeof(westfold::draw_tables::sine) || std::memcmp(westfold::tables::sine,westfold::draw_tables::sine,sizeof(westfold::draw_tables::sine)))return 1;
    if(sizeof(westfold::tables::durations_ms)!=sizeof(westfold::draw_tables::durations_ms) || std::memcmp(westfold::tables::durations_ms,westfold::draw_tables::durations_ms,sizeof(westfold::draw_tables::durations_ms)))return 1;
    if(sizeof(westfold::tables::visual_decay)!=sizeof(westfold::draw_tables::visual_decay) || std::memcmp(westfold::tables::visual_decay,westfold::draw_tables::visual_decay,sizeof(westfold::draw_tables::visual_decay)))return 1;
    if(sizeof(westfold::tables::visual_spectrum)!=sizeof(westfold::draw_tables::visual_spectrum) || std::memcmp(westfold::tables::visual_spectrum,westfold::draw_tables::visual_spectrum,sizeof(westfold::draw_tables::visual_spectrum)))return 1;
    if(sizeof(westfold::tables::rates)!=sizeof(westfold::draw_tables::rates) || std::memcmp(westfold::tables::rates,westfold::draw_tables::rates,sizeof(westfold::draw_tables::rates)))return 1;
    if(sizeof(westfold::tables::ratios)!=sizeof(westfold::draw_tables::ratios) || std::memcmp(westfold::tables::ratios,westfold::draw_tables::ratios,sizeof(westfold::draw_tables::ratios)))return 1;
    if(sizeof(westfold::tables::spectrum)!=sizeof(westfold::draw_tables::spectrum) || std::memcmp(westfold::tables::spectrum,westfold::draw_tables::spectrum,sizeof(westfold::draw_tables::spectrum)))return 1;

    for(opxy::u32 value=0;value<=98304;++value)
        if(westfold::dsp::third(value)!=static_cast<opxy::i32>(value/3))return 1;
    for(unsigned index=0;index<7;++index) {
        const opxy::u32 threshold=(0x60000000u>>7)<<index;
        for(opxy::i32 offset=-4096;offset<=4096;++offset)
            if(!exact(threshold+offset)||!exact(threshold/2+offset))return 1;
    }
    const unsigned fractions[]={0,1,127,16383,16384,32766,32767};
    for(unsigned bank=0;bank<8;++bank)for(unsigned fold=0;fold<17;++fold)
        for(unsigned index=0;index<512;++index)for(unsigned fraction:fractions) {
            const opxy::u32 phase=(index<<23)|(fraction<<8)|255u;
            if(westfold::dsp::read(westfold::tables::waves[bank][fold],phase)!=
               westfold::dsp::read_half(westfold::half_tables::waves[bank][fold],phase))return 1;
        }
    for(unsigned index=0;index<512;++index)for(unsigned fraction:fractions) {
        const opxy::u32 phase=(index<<23)|(fraction<<8)|255u;
        if(westfold::dsp::read(westfold::tables::sine,phase)!=westfold::dsp::sine(phase))return 1;
    }
    opxy::u32 random=7139;
    for(unsigned i=0;i<1000000;++i)if(!exact(random=random*1664525u+1013904223u))return 1;
    if(!exact(0)||!exact(0xffffffffu))return 1;
    std::puts("PASS: exact compact drawing tables, every bounded third, all band transitions, half-cycle interpolation and one million phase increments");
}
