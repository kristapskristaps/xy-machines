#include "core.h"
#include "vendor/resources.h"
namespace matter {
namespace {
using opxy::i32;
using opxy::u32;
using namespace matter_dsp;
const uf one={0x40000000,0};
i32 clamp(i32 x,i32 low,i32 high) { return x<low?low:x>high?high:x; }
u32 control(i32 x) { return x<0?0u:static_cast<u32>(x)>>16; }
i32 q15(i32 a,i32 b) { return (a>>15)*b+((a&32767)*b>>15); }
u32 random(Voice& voice) { return voice.random=voice.random*1664525u+1013904223u; }

template<bool Bowed>
void configure(Voice& voice,u32 increment,i32 geometry,i32 bright,i32 damping) {
    if (voice.key[0]==static_cast<i32>(increment) && voice.key[1]==geometry && voice.key[2]==bright && voice.key[3]==damping && voice.key[4]==Bowed) return;
    voice.key[0]=static_cast<i32>(increment);voice.key[1]=geometry;voice.key[2]=bright;voice.key[3]=damping;voice.key[4]=Bowed;
    voice.position=-1;
    voice.active_modes[0]=voice.active_modes[1]=0;
    const unsigned at=static_cast<unsigned>(geometry)>>7;
    i32 stiff=stiffness[at]+(stiffness[at+1]-stiffness[at])*(geometry&127)/128;
    const i32 maximum=2147483647;
    const i32 geometry31=geometry*65536,brightness31=bright*65536;
    i32 attenuation=maximum-geometry31;
    attenuation=fmac1t(attenuation,attenuation);
    attenuation=fmac1t(attenuation,attenuation);
    attenuation=fmac1t(attenuation,attenuation);
    const i32 adjusted=fmac1t(brightness31,maximum-fmac1t(attenuation,429496730));
    const i32 inverse=maximum-adjusted;
    const i32 brightness_curve=maximum-fmac1t(inverse,inverse);
    i32 qloss=322122547+fmac1t(brightness_curve,1825361101);
    const i32 geometry_inverse=maximum-geometry31;
    const i32 qrate=fmac1t(maximum-fmac1t(geometry_inverse,geometry_inverse),214748365);
    const uf f0=uf_q(increment,32);
    uf harmonic=f0;
    uf q=uf_mul(uf_semis(damping*255+(damping*4080>>15)),uf_q(500,0));
    i32 stretch=1<<24;
    for (unsigned i=0;i<(Bowed?8:mode_count);++i) {
        const uf frequency=uf_mul(harmonic,uf_q(static_cast<u32>(stretch),24));
        const u32 raw=uf_to(frequency,32);
        const u32 f=raw>2104533975u?2104533975u:raw;
        svf_c c;
        svf_hq(&c,tan_fast(f),uf_add(one,uf_mul(uf_q(f,32),q)));
        if(raw<2104533975u) voice.active_modes[i/32] |= 1u<<(i%32);
        Filter& mode=voice.modes[i];
        mode.a1=c.a1;mode.a2=c.a2;mode.a3=c.a3;
        if(Bowed && i<8) {
            svf_hq(&c,tan_fast(f),uf_add(one,uf_mul(uf_q(f,32),uf_q(1500,0))));
            BowFilter& bow=voice.bowed[i];
            bow.a1=c.a1;bow.a2=c.a2;bow.a3=c.a3;
            const uf bow_q=uf_add(one,uf_mul(uf_q(f,32),uf_q(1500,0)));
            const u32 norm=uf_to(uf_recip(bow_q),31);
            bow.normalization=static_cast<i32>(norm>2147483647u?2147483647u:norm);
            u32 period=f?0xffffffffu/f:1023;
            while(period>=1024) period>>=1;
            voice.bow_length[i]=period?period:1;
        }
        // Modes above Nyquist carry no gain, including at extreme geometry.
        if(raw>=2104533975u) mode.gain=0;
        stretch+=stiff;
        stiff=fmac1(stiff,stiff<0?1997159793:2104533975);
        harmonic=uf_add(harmonic,f0);
        qloss+=fmac1t(qrate,2147483647-qloss);
        q=uf_mul(q,uf_q(static_cast<u32>(qloss),31));
    }
}

void coefficients(Voice& voice,u32 increment,i32 geometry,i32 bright,i32 damping) {
    configure<false>(voice,increment,geometry,bright,damping);
}

// The original approximate cosine recurrence weights the pickup position.
i32 position_coefficient(i32 position) {
    i32 frequency=position-8192,sign=1;
    if(frequency<0) frequency=-frequency;
    else if(frequency>16384) frequency-=16384;
    else sign=-1;
    return sign*8*frequency*(32768-2*frequency);
}

inline i32 filter(i32& s1,i32& s2,i32 a1,i32 a2,i32 a3,i32 input,bool lowpass=false) {
    const i32 v3=input-s2;
    const i32 bp=clamp(opxy::q31::multiply(a1,s1)+opxy::q31::multiply(a2,v3),-536870912,536870912);
    const i32 lp=clamp(s2+opxy::q31::multiply(a2,s1)+opxy::q31::multiply(a3,v3),-536870912,536870912);
    s1=clamp(bp+bp-s1,-536870912,536870912);
    s2=clamp(lp+lp-s2,-536870912,536870912);
    return lowpass?lp:bp;
}

i32 interpolate(const opxy::i16* data,u32 phase,unsigned bits) {
    const u32 index=phase>>bits;
    const i32 fraction=static_cast<i32>((phase&((1u<<bits)-1))>>(bits-15));
    return data[index]+q15(data[index+1]-data[index],fraction);
}

void excite(Voice& voice,u32 model,i32 contour,i32 timbre,i32* out) {
    const bool trigger=voice.trigger!=0;
    const unsigned cutoff_index=static_cast<unsigned>(timbre)>>7;
    const i32 amplitude=pulse_amplitude[cutoff_index];
    const u32 cutoff_increment=static_cast<u32>(cutoff[cutoff_index]);
    svf_c c={};
    if(model<4) svf_hq(&c,tan_fast(cutoff_increment),uf_q(1,1));
    if(trigger) {
        voice.sample_phase=0;
        voice.plectrum_delay=0;
        voice.particle_delay=0;
        const i32 r=static_cast<i32>(random(voice)>>17);
        voice.particle=32768-q15(q15(r,r),19661);
        voice.particle_range=32768;
        voice.flow=16384;
    }
    const i32 scale=q15(q15(22938,22938),q15(22938,22938));
    const u32 flow_probability=429497u+static_cast<u32>(scale)*16384u;
    const u32 noise_step=model==4 ? uf_to(uf_mul(uf_semis(timbre*144-3932160),uf_q(47554,16)),17) : 0;
    const u32 strike_step=model==5 ? uf_to(uf_mul(uf_semis(timbre*144-1900544),uf_q(47554,16)),16) : 0;
    const u32 selection=(32767u-static_cast<u32>(contour))*8u;
    const u32 sample_index=selection>>15;
    const u32 sample_fraction=selection&32767u;
    const u32 offset1=smp_boundaries[sample_index],offset2=smp_boundaries[sample_index+1];
    const u32 length1=offset2-offset1-1;
    const u32 length2=smp_boundaries[sample_index+2]-offset2-1;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        i32 excitation=0;
        if(model==0) excitation=trigger&&i==0?amplitude:0;
        else if(model==1) {
            if(trigger&&i==0) {
                excitation=-amplitude/10;
                voice.plectrum_delay=static_cast<u32>(q15(contour,contour))*5645u/32768u+88u;
            }
            if(voice.plectrum_delay && --voice.plectrum_delay==0) excitation=amplitude;
        } else if(model==2) {
            if(!voice.particle_delay) {
                const i32 r=static_cast<i32>(random(voice)>>17);
                const i32 amount=34406+q15(r,r)/2;
                if(random(voice)>3006477107u) voice.particle=q15(voice.particle,amount);
                else if(random(voice)<1288490188u) voice.particle=static_cast<i32>(static_cast<u32>(voice.particle)*32768u/static_cast<u32>(amount));
                voice.particle=clamp(voice.particle,655,voice.particle_range+8192);
                voice.particle_delay=static_cast<u32>(voice.particle)*6615u/32768u;
                const i32 inverse=32768-voice.particle_range;
                const i32 gain=32768-q15(inverse,inverse);
                excitation=q15(q15(amplitude,voice.particle),gain);
                const i32 decay=32768-contour;
                voice.particle_range=q15(voice.particle_range,32768-q15(decay,decay)/2);
            } else --voice.particle_delay;
        } else if(model==3) {
            const u32 r=random(voice);
            if(r<flow_probability) voice.flow=-voice.flow;
            excitation=(voice.flow+q15(static_cast<i32>(r>>17)-16384-voice.flow,scale))*32;
        } else if(model==4) {
            excitation=interpolate(smp_noise_sample,voice.sample_phase,17)*13;
            voice.sample_phase+=noise_step;
            if(random(voice)<31165296u) voice.sample_phase=static_cast<u32>(contour)*131072u;
        } else {
            i32 a=0,b=0;
            const u32 index=voice.sample_phase>>16;
            if(index<length1) a=interpolate(smp_sample_data+offset1,voice.sample_phase,16);
            if(index<length2) b=interpolate(smp_sample_data+offset2,voice.sample_phase,16);
            if(index<length1 || index<length2) voice.sample_phase+=strike_step;
            excitation=(a+q15(b-a,static_cast<i32>(sample_fraction)))*16;
        }
        if(model<4) excitation=filter(voice.exc_s1,voice.exc_s2,c.a1,c.a2,c.a3,excitation,true);
        if(model<=2 || model==5) excitation=clamp(excitation,-33554431,33554431)*4+clamp(excitation,-33554431,33554431)/5;
        out[i]=excitation;
    }
}
}

void destroy(Voice& voice,Shared& shared,opxy::u32 owner) {
    shared.bows.release(owner,voice.lease);voice.lease=0;
}
void initialize(Voice& voice,Shared& shared,opxy::u32 owner) {
    destroy(voice,shared,owner);
    voice=Voice();
    voice.random=owner*0x9e3779b9u+1u;
    voice.trigger=1;
    voice.limiter=1<<23;
    voice.key[0]=-1;
    voice.position=-1;
}
void start(Voice& voice,Shared&,opxy::u32) { voice.trigger=1; }

void render(Voice& voice,Shared& shared,opxy::u32 owner,opxy::Output output,opxy::Parameters parameters,opxy::Pitch pitch) {
    const i32 geometry=static_cast<i32>(control(parameters[0]));
    const i32 brightness=static_cast<i32>(control(parameters[1]));
    const i32 damping=static_cast<i32>(control(parameters[2]));
    const i32 position=static_cast<i32>(control(parameters[3]));
    const u32 model=control(parameters[4])*6u>>15;
    const i32 contour=static_cast<i32>(control(parameters[5]));
    const i32 timbre=static_cast<i32>(control(parameters[6]));
    const i32 body=static_cast<i32>(control(parameters[7]));
    if(model!=voice.model) {
        voice.trigger=1;voice.model=model;
        for(unsigned i=0;i<mode_count;++i) voice.modes[i].s1=voice.modes[i].s2=0;
        for(unsigned i=0;i<8;++i) voice.bowed[i].s1=voice.bowed[i].s2=0;
        voice.bow_signal=voice.bow_velocity=0;
        voice.limiter=1<<23;
        shared.bows.release(owner,voice.lease);voice.lease=0;
    }
    const bool bowed=model==3;
    if(voice.trigger && bowed) voice.lease=shared.bows.acquire(owner,voice.lease);
    BowDelay* delays=bowed ? shared.bows.get(owner,voice.lease) : nullptr;
    const u32 increment=pitch[0]?pitch[0]:1;
    if(bowed) configure<true>(voice,increment,geometry,brightness,damping);
    else coefficients(voice,increment,geometry,brightness,damping);
    i32 excitation[opxy::block_frames];
    excite(voice,model,contour,timbre,excitation);
    i32 mixed[opxy::block_frames]={};
    const i32 coefficient=position_coefficient(position);
    i32 targets[mode_count];
    i32 y0=536870912,y1=coefficient/2;
    for(unsigned mode_index=0;mode_index<mode_count;++mode_index) {
        const i32 cosine=y0;
        y0=fmac1(coefficient,y0)*4-y1;y1=cosine;
        targets[mode_index]=(voice.active_modes[mode_index/32]>>(mode_index%32))&1u
            ? clamp(cosine+536870912,0,1073741823)*2 : 0;
    }
    if(voice.position<0)
        for(unsigned i=0;i<mode_count;++i)voice.modes[i].gain=targets[i];
    if(!bowed) {
        for(unsigned mode_index=0;mode_index<mode_count;++mode_index) {
            Filter& mode=voice.modes[mode_index];
            if(!((voice.active_modes[mode_index/32]>>(mode_index%32))&1u)) {
                mode.s1=mode.s2=mode.gain=0;continue;
            }
            const i32 target=targets[mode_index],gain_step=(target-mode.gain)/64;
            i32 gain=mode.gain,s1=mode.s1,s2=mode.s2;
            const i32 a1=mode.a1,a2=mode.a2,a3=mode.a3;
            for(unsigned i=0;i<opxy::block_frames;++i) {
                gain+=gain_step;
                const i32 bp=filter(s1,s2,a1,a2,a3,excitation[i]);
                // Scaling before the sum keeps the resonator bank below overflow.
                mixed[i]+=opxy::q31::multiply(bp,gain)/32;
            }
            mode.s1=s1;mode.s2=s2;mode.gain=target;
        }
    } else {
        for(unsigned i=0;i<8;++i)voice.modes[i].gain=targets[i];
    }
    voice.position=position;
    if(delays) {
        for(unsigned i=0;i<opxy::block_frames;++i) {
            const i32 desired=contour*512;
            voice.bow_velocity+=(desired-voice.bow_velocity)/1024;
            i32 bow_sum=0,bow_output=0;
            const i32 input=excitation[i]+voice.bow_signal;
            for(unsigned mode_index=0;mode_index<8;++mode_index) {
                if(!(voice.active_modes[0]&(1u<<mode_index))) continue;
                BowFilter& bow=voice.bowed[mode_index];
                u32& write=voice.bow_write[mode_index];
                const u32 length=voice.bow_length[mode_index];
                const u32 read=(write+1024u-length)&1023u;
                const i32 delayed=fmac1(static_cast<i32>(delays->samples[mode_index][read])*512,2126008812);
                bow_sum+=delayed;
                const i32 bp=filter(bow.s1,bow.s2,bow.a1,bow.a2,bow.a3,input+delayed);
                const i32 normalized=fmac1(bp,bow.normalization);
                delays->samples[mode_index][write]=static_cast<opxy::i16>(clamp(normalized/512,-32768,32767));
                write=(write+1)&1023u;
                bow_output+=opxy::q31::multiply(normalized,voice.modes[mode_index].gain)/8;
            }
            const i32 x=fmac1(voice.bow_velocity,279172874)-bow_sum;
            const u32 magnitude=static_cast<u32>(x<0?-x:x);
            const unsigned index=magnitude>>15;
            voice.bow_signal=fmac1(x,bow_friction[index>1024?1024:index]);
            mixed[i]=bow_output*2;
        }
    }
    voice.trigger=0;
    i32 peak=0;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        const i32 raw=excitation[i]/16;
        const i32 sample=clamp(raw+q15((mixed[i]/4)-raw,body),-(15<<20),15<<20);
        const i32 absolute=sample<0?-sample:sample;
        if(absolute>peak) peak=absolute;
        excitation[i]=sample;
    }
    peak*=128;
    if(peak>voice.limiter) voice.limiter=peak;
    else voice.limiter-=(voice.limiter-peak)/768;
    i32 gain=2147483647;
    if(voice.limiter>(1<<24)) {
        const u32 reciprocal=0x80000000u/static_cast<u32>(voice.limiter>>8);
        gain=reciprocal>=32768u?2147483647:static_cast<i32>(reciprocal*65536u);
    }
    for(unsigned i=0;i<opxy::block_frames;++i) {
        const i32 sample=opxy::q31::multiply(excitation[i],gain);
        output[i]=clamp(sample/4,-32768,32767)*16384;
    }
}
}
