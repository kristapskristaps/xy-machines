#include "fx.cpp"
#include <cmath>
#include <cstdio>
#include <initializer_list>
static opxy::i32 memory[halo_plate::buffer_words];
static bool tap_matches(const halo_plate::State& state,unsigned line,opxy::i32 delay,unsigned channel) {
    const unsigned age=static_cast<opxy::u32>(delay)>>8;
    const auto a=halo_plate::line_sample(state,memory,line,age,channel);
    const auto b=halo_plate::line_sample(state,memory,line,age+1,channel);
    const long long scale=2147483648LL;
    const long long product=static_cast<long long>(b-a)*(delay&255)*8388608LL;
    const auto expected=a+static_cast<opxy::i32>(product/scale-(product<0 && product%scale!=0));
    if(halo_plate::tap(state,memory,line,delay,channel)==expected)return true;
    std::printf("FAIL fractional tap line %u, position %u, written %u, delay %d, channel %u\n",
                line,state.positions[line],state.written,delay,channel);
    return false;
}
int main() {
    halo_plate::State state={};
    // Interleaved rings retain the independent left/right sample at each age.
    for(unsigned frame=0;frame<5000;++frame) {
        halo_plate::write(state,memory,16,static_cast<opxy::i32>(frame*21));
        halo_plate::write(state,memory,16,-static_cast<opxy::i32>(frame*17));
        ++state.written;
    }
    for(unsigned age=1;age<2000;++age) {
        if(halo_plate::line_sample(state,memory,16,age,0)!=static_cast<opxy::i32>((5000-age)*21))return 1;
        if(halo_plate::line_sample(state,memory,16,age,1)!=-static_cast<opxy::i32>((5000-age)*17))return 1;
    }
    // Every ring's wrap boundaries, including packed echo and stereo channels,
    // must agree with ordinary modulo indexing before optimizing wrap arithmetic.
    state.written=halo_plate::echo_frames;
    for(unsigned line=0;line<19;++line){
        const auto layout=halo_plate::lines[line];
        const unsigned stride=line==16 || line==17?2:1;
        for(unsigned i=0;i<layout.length;++i)memory[layout.offset+i]=static_cast<opxy::i32>((i*17239u+17u)*(line+1));
        const unsigned positions[]={0,1,layout.length/2,layout.length-1};
        for(auto position:positions){
            state.positions[line]=position;
            for(unsigned age=1;age<layout.length/stride;++age)for(unsigned channel=0;channel<(stride==2 || line==18?2u:1u);++channel){
                const unsigned index=(position+layout.length-age*stride+(stride==2?channel:0))%layout.length;
                const auto packed=memory[layout.offset+index];
                const auto expected=line==18?static_cast<opxy::i16>(channel?static_cast<opxy::u32>(packed)>>16:static_cast<opxy::u32>(packed)&65535u)*512:packed;
                if(halo_plate::line_sample(state,memory,line,age,channel)!=expected)return 1;
            }
            if(halo_plate::line_sample(state,memory,line,0) || halo_plate::line_sample(state,memory,line,layout.length/stride))return 1;
        }
    }
    // Compare interpolation against the independently checked sample reader and
    // a wide integer product, including unwritten history and both wrap edges.
    unsigned tap_rng=371;
    for(unsigned line=0;line<19;++line){
        const auto layout=halo_plate::lines[line];
        const unsigned stride=line==16 || line==17?2:1;
        const unsigned frames=layout.length/stride;
        for(unsigned i=0;i<layout.length;++i)
            memory[layout.offset+i]=static_cast<opxy::i32>((tap_rng=tap_rng*1664525u+1013904223u)&33554431u)-halo_plate::unit;
        const unsigned positions[]={0,1,layout.length/2,layout.length-1};
        const unsigned histories[]={0,1,frames/2,frames-2,frames-1,frames,halo_plate::echo_frames,0xffffffffu};
        for(auto position:positions)for(auto written:histories){
            state.positions[line]=position;state.written=written;
            const unsigned ages[]={0,1,frames-2,frames-1,frames,frames+1,written?written-1:0,written,written+1};
            for(auto age:ages)for(unsigned fraction:{0u,1u,127u,128u,255u})
                for(unsigned channel=0;channel<(stride==2 || line==18?2u:1u);++channel)
                    if(!tap_matches(state,line,static_cast<opxy::i32>((age<<8)|fraction),channel))return 1;
        }
        state.written=halo_plate::echo_frames;
        for(auto position:positions){
            state.positions[line]=position;
            for(unsigned age=0;age<=frames;++age)for(unsigned fraction:{0u,1u,127u,128u,255u})
                for(unsigned channel=0;channel<(stride==2 || line==18?2u:1u);++channel)
                    if(!tap_matches(state,line,static_cast<opxy::i32>((age<<8)|fraction),channel))return 1;
        }
        for(opxy::i32 delay:{-2147483647-1,-256,-1,2147483647})
            if(!tap_matches(state,line,delay,0))return 1;
    }
    const double pi=3.141592653589793;
    for(unsigned division=0;division<8;++division)for(unsigned bpm=400;bpm<=2400;bpm+=100) {
        const opxy::i32 control=static_cast<opxy::i32>(division*268435456u);
        const double actual=halo_plate::echo_delay(control,bpm)/256.;
        const double expected=halo_plate::network_rate*600.0/bpm*opxy::division_sixteenths[division]/16.;
        if(std::fabs(actual-expected)>.004)return 1;
    }
    // Check both conversion filters' DC gain and downsampling rejection.
    long long sum=0;for(auto coefficient:halo_plate::tables::halfband)sum+=coefficient;
    if(sum!=2147483648LL)return 1;
    if(halo_plate::tables::halfband[0]!=0)return 1;
    if(halo_plate::tables::halfband[30]!=0)return 1;
    if(halo_plate::tables::halfband[15]!=1073741824)return 1;
    for(unsigned tap=0;tap<31;++tap){
        if(halo_plate::tables::halfband[tap]!=halo_plate::tables::halfband[30-tap])return 1;
        if(tap%2 && tap!=15 && halo_plate::tables::halfband[tap])return 1;
    }
    // Compare the paired/mirrored filters to an independent circular convolution.
    unsigned rng=7;
    opxy::i32 down[62]={},up[32]={};
    for(unsigned trial=0;trial<1000;++trial) {
        for(unsigned i=0;i<31;++i)down[i]=down[i+31]=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>7)-halo_plate::unit;
        for(unsigned i=0;i<16;++i)up[i]=up[i+16]=static_cast<opxy::i32>((rng=rng*1664525u+1013904223u)>>7)-halo_plate::unit;
        for(unsigned pos=0;pos<31;++pos) {
            opxy::i32 expected=down[(pos+31-15)%31]/2;
            for(unsigned tap=0;tap<31;tap+=2)expected+=static_cast<opxy::i32>(std::floor(double(down[(pos+31-tap)%31])*halo_plate::tables::halfband[tap]/2147483648.));
            if(std::abs(halo_plate::downsample(down,pos)-expected)>8)return 1;
        }
        for(unsigned pos=0;pos<16;++pos) {
            opxy::i32 expected=0;
            for(unsigned tap=0;tap<16;++tap)expected+=static_cast<opxy::i32>(std::floor(double(up[(pos+16-tap)%16])*halo_plate::tables::halfband[tap*2]*2/2147483648.));
            if(std::abs(halo_plate::upsample(up,pos,0)-expected)>8 || halo_plate::upsample(up,pos,1)!=up[(pos+16-7)%16])return 1;
        }
    }
    double powers[2]={};
    for(unsigned tone=0;tone<2;++tone) {
        opxy::i32 history[62]={};unsigned position=0;
        const double frequency=tone?16000:1000;
        for(unsigned i=0;i<44100;++i) {
            history[position]=history[position+31]=static_cast<opxy::i32>(std::sin(2*pi*frequency*i/44100.)*halo_plate::unit/2);
            if(i>100 && !(i&1)){const double y=halo_plate::downsample(history,position);powers[tone]+=y*y;}
            if(++position==31)position=0;
        }
    }
    const double rejection=10*std::log10(powers[1]/powers[0]);
    std::printf("16 kHz alias rejection: %.1f dB\n",rejection);
    if(rejection> -55)return 1;
    // The full 4:1 cascade must reject audio above the new wet Nyquist limit.
    double cascade[2]={};
    for(unsigned tone=0;tone<2;++tone) {
        opxy::i32 history[2][62]={};unsigned position[2]={};
        const double frequency=tone?8000:1000;
        for(unsigned i=0;i<44100;++i) {
            history[0][position[0]]=history[0][position[0]+31]=static_cast<opxy::i32>(std::sin(2*pi*frequency*i/44100.)*halo_plate::unit/2);
            if(!(i&1)) {
                history[1][position[1]]=history[1][position[1]+31]=halo_plate::downsample(history[0],position[0]);
                if(i>200 && !(i&3)){const double y=halo_plate::downsample(history[1],position[1]);cascade[tone]+=y*y;}
                if(++position[1]==31)position[1]=0;
            }
            if(++position[0]==31)position[0]=0;
        }
    }
    const double cascade_rejection=10*std::log10(cascade[1]/cascade[0]);
    std::printf("8 kHz rejection through the 4:1 cascade: %.1f dB\n",cascade_rejection);
    if(cascade_rejection> -55)return 1;
    // A 100 ms grain is phase-coherent for A3; check its octave at A4.
    halo_plate::initialize(state);const unsigned width=halo_plate::network_rate/10,step=16777216u/width;
    double energy[2]={};double re[2]={},im[2]={};
    for(unsigned i=0;i<66150;++i) {
        const double input=std::sin(2*pi*220*i/halo_plate::network_rate)*halo_plate::unit/4;
        const opxy::i32 shifted=halo_plate::pitch_up(state,memory,0,width);
        if(i>11025)for(unsigned bin=0;bin<2;++bin){const double angle=2*pi*(bin?440:220)*i/halo_plate::network_rate;re[bin]+=shifted*std::cos(angle);im[bin]+=shifted*std::sin(angle);}
        halo_plate::write(state,memory,17,static_cast<opxy::i32>(input));halo_plate::write(state,memory,17,static_cast<opxy::i32>(input));
        state.grain_phase=(state.grain_phase+step)&16777215u;if(state.written<halo_plate::echo_frames)++state.written;
    }
    for(unsigned bin=0;bin<2;++bin)energy[bin]=re[bin]*re[bin]+im[bin]*im[bin];
    const double octave=10*std::log10(energy[1]/energy[0]);
    std::printf("440 Hz over original 220 Hz: %.1f dB\n",octave);
    if(octave<30)return 1;
    std::puts("PASS: stereo indexing, exact tempo divisions, anti-alias conversion and octave heads");
}
