#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>
namespace acidline {
namespace {
using opxy::i32;using opxy::u32;
const i32 one=16384;
const i32 internal_peak=49152;
i32 limit(i32 x,i32 bound){return x < -bound ? -bound : x > bound ? bound : x;}
// The public fractional multiply maps to Blackfin's DSP instruction. Inputs
// here stay below 2^17 and 2^16, so both scalings fit signed 32 bits.
i32 multiply(i32 sample,i32 coefficient){return opxy::q31::multiply(sample*8192,coefficient*16);}
i32 cutoff_target(i32 base,i32 envelope,i32 amount){
    return limit(base+multiply(one-1-base,multiply(envelope,amount)),one-1);
}
i32 soft(i32 x){
    x=limit(x,internal_peak);
    // Compute the odd curve's magnitude once. Nonnegative products need no
    // signed rounding corrections; restoring the sign preserves integer truncation.
    const i32 magnitude=x<0?-x:x;
    const i32 half=magnitude/2;
    const i32 square=half*half/one;
    const i32 cubic=magnitude*square/65536;
    // |cubic| <= 27648: this reciprocal gives exact truncation by three
    // throughout that range, without Blackfin's per-sample software divide.
    const i32 third=(cubic*21846)>>16;
    return x-(x<0?-third:third);
}
i32 mix(i32 a,i32 b,i32 fraction){return a+multiply(b-a,fraction);}
i32 lookup(const opxy::i16* row,u32 phase){
    const u32 index=phase>>23;
    return row[index]+multiply(row[index+1]-row[index],static_cast<i32>((phase&0x7fffffu)>>9));
}
i32 distortion_gain(i32 amount){return 256+multiply(amount,amount)*23/64;}
i32 distort(i32 x,i32 amount,i32 gain){
    if(!amount)return x;
    const i32 boosted=x*gain/256;
    const u32 position=static_cast<u32>(boosted<0?-boosted:boosted);
    const u32 index=position>>10;
    const i32 clipped=index>=256?32767:tables::clip[index]+
        (tables::clip[index+1]-tables::clip[index])*static_cast<i32>(position&1023u)/1024;
    return mix(x,boosted<0?-clipped:clipped,amount);
}
inline i32 distort(i32 x,i32 amount){return distort(x,amount,distortion_gain(amount));}
i32 interpolate(const opxy::i16* table,i32 control){
    const u32 index=static_cast<u32>(control)>>8;
    return table[index]+(table[index+1]-table[index])*(control&255)/256;
}
i32 inverse(i32 cutoff,i32 resonance){
    const u32 row=static_cast<u32>(resonance)>>8;
    const i32 a=interpolate(tables::inverse[row],cutoff);
    return a+(interpolate(tables::inverse[row+1],cutoff)-a)*(resonance&255)/256;
}
// The 31-tap half-band has seven nonzero symmetric pairs. Keep the original
// accumulation order and rounding while eliminating the runtime tap loop.
i32 decimate(i32 (&history)[32],u32 cursor,const opxy::i16 (&kernel)[31]){
    i32 sum=history[(cursor-15)&31u]*kernel[15];
    sum+=(history[(cursor-2)&31u]+history[(cursor-28)&31u])*kernel[2];
    sum+=(history[(cursor-4)&31u]+history[(cursor-26)&31u])*kernel[4];
    sum+=(history[(cursor-6)&31u]+history[(cursor-24)&31u])*kernel[6];
    sum+=(history[(cursor-8)&31u]+history[(cursor-22)&31u])*kernel[8];
    sum+=(history[(cursor-10)&31u]+history[(cursor-20)&31u])*kernel[10];
    sum+=(history[(cursor-12)&31u]+history[(cursor-18)&31u])*kernel[12];
    sum+=(history[(cursor-14)&31u]+history[(cursor-16)&31u])*kernel[14];
    return sum/one;
}
}
inline void initialize(Voice& v,Shared&,opxy::u32){v=Voice();v.cutoff=-1;}
inline void start(Voice& v,Shared&,opxy::u32){v.envelope=one;}
inline void destroy(Voice&,Shared&,opxy::u32){}
template <int Mode> static inline void render_mode(Voice& v,opxy::Output output,opxy::Parameters p,opxy::Pitch pitch){
    const i32 base=p[0]/131072,resonance=p[1]/131072;
    const i32 drive=p[2]/131072,env=p[3]/131072,decay_control=p[4]/131072;
    const u32 choice=(static_cast<u32>(p[5])>>16)*3>>15;
    const i32 wave=Mode<0?static_cast<i32>(choice)*8192:Mode*8192;
    const i32 sub=p[6]/131072,tone=16383-p[7]/131072;
    const i32 cutoff=cutoff_target(base,v.envelope,env);
    if(v.cutoff<0){v.cutoff=cutoff;v.resonance=resonance;v.drive=drive;v.wave=wave;v.sub=sub;v.tone=tone;}
    i32 g=interpolate(tables::pole,v.cutoff);
    const i32 feedback=interpolate(tables::feedback,v.resonance);
    const i32 correction=inverse(v.cutoff,v.resonance);
    const i32 end_feedback=interpolate(tables::feedback,resonance),end_correction=inverse(cutoff,resonance);
    const i32 dg=(interpolate(tables::pole,cutoff)-g)/64;
    i32 input_gain=correction/4+multiply(correction,v.drive)/2;
    const i32 input_step=(end_correction/4+multiply(end_correction,drive)/2-input_gain)/64;
    i32 feedback_gain=multiply(correction,feedback);
    const i32 feedback_step=(multiply(end_correction,end_feedback)-feedback_gain)/64;
    i32 body=interpolate(tables::body,v.resonance);
    const i32 body_step=(interpolate(tables::body,resonance)-body)/64;
    const i32 dd=(drive-v.drive)/64,dw=(wave-v.wave)/64;
    i32 clip_gain=distortion_gain(v.drive);
    const i32 clip_step=(distortion_gain(drive)-clip_gain)/64;
    const i32 ds=(sub-v.sub)/64,dt=(tone-v.tone)/64;
    i32 tone_g=interpolate(tables::tone,v.tone);
    const i32 tone_step=(interpolate(tables::tone,tone)-tone_g)/64;
    u32 highest=0;
    for(u32 i=0;i<64;++i)if(pitch[i]>highest)highest=pitch[i];
    highest/=2;
    u32 band=0;while(band<7 && highest<(0x80000000u>>(band+1)))++band;
    for(u32 i=0;i<64;++i){
        g+=dg;v.drive+=dd;v.sub+=ds;v.tone+=dt;tone_g+=tone_step;clip_gain+=clip_step;
        if(Mode<0)v.wave+=dw;
        input_gain+=input_step;feedback_gain+=feedback_step;body+=body_step;
        i32 reduced=0;
        const u32 increment=pitch[i];
        for(u32 sub=0;sub<2;++sub){
            i32 source;
            if(Mode==1 || (Mode<0 && v.wave==8192))source=lookup(tables::triangle[band],v.phase);
            else {
                const i32 saw=lookup(tables::saw[band],v.phase);
                source=saw;
                if(Mode==2 || (Mode<0 && v.wave>8192)){
                    const i32 square=(saw-lookup(tables::saw[band],v.phase+0x80000000u))/2;
                    source=(Mode==2 || v.wave==one)?square:mix(lookup(tables::triangle[band],v.phase),square,(v.wave-8192)*2);
                }else if(Mode<0 && v.wave)source=mix(saw,lookup(tables::triangle[band],v.phase),v.wave*2);
            }
            i32 memory=0;
            memory=v.pole[0]+multiply(memory-v.pole[0],g);
            memory=v.pole[1]+multiply(memory-v.pole[1],g);
            memory=v.pole[2]+multiply(memory-v.pole[2],g);
            memory=v.pole[3]+multiply(memory-v.pole[3],g);
            i32 signal=multiply(source,input_gain)-multiply(memory,feedback_gain);
            signal=soft(limit(signal,12288)*4);
            {
                const i32 y=v.pole[0]+multiply(signal-v.pole[0],g);
                v.pole[0]=limit(y+(y-v.pole[0]),internal_peak);
                signal=y;
            }
            {
                const i32 y=v.pole[1]+multiply(signal-v.pole[1],g);
                v.pole[1]=limit(y+(y-v.pole[1]),internal_peak);
                signal=y;
            }
            {
                const i32 y=v.pole[2]+multiply(signal-v.pole[2],g);
                v.pole[2]=limit(y+(y-v.pole[2]),internal_peak);
                signal=y;
            }
            {
                const i32 y=v.pole[3]+multiply(signal-v.pole[3],g);
                v.pole[3]=limit(y+(y-v.pole[3]),internal_peak);
                signal=y;
            }
            v.fir1[v.cursor1]=distort(limit(multiply(signal,body)*2,32768),v.drive,clip_gain);
            if(sub==1)reduced=decimate(v.fir1,v.cursor1,tables::halfband31);
            v.cursor1=(v.cursor1+1)&31u;
            // Honor stock pitch modulation and portamento exactly, sample by sample.
            v.phase+=increment/2+(sub==1?increment%2:0);
        }
        const i32 dc_removed=limit(reduced-v.dc_input+multiply(v.dc_output,16376),32768);
        v.dc_input=reduced;v.dc_output=dc_removed;
        if(v.tone>=16383)v.tone_memory=dc_removed;
        else v.tone_memory+=multiply(dc_removed-v.tone_memory,tone_g);
        const i32 main=v.tone_memory;
        const i32 low=v.sub?lookup(tables::sine,v.sub_phase):0;
        output[i]=mix(main,low,v.sub/2)*32768;
        v.sub_remainder+=increment&1u;
        v.sub_phase+=increment/2+v.sub_remainder/2;
        v.sub_remainder&=1u;
    }
    v.envelope=multiply(v.envelope,interpolate(tables::decay,decay_control));
    v.cutoff=cutoff;v.resonance=resonance;v.drive=drive;v.wave=wave;v.sub=sub;v.tone=tone;
}
inline void render(Voice& v,Shared&,opxy::u32,opxy::Output output,opxy::Parameters p,opxy::Pitch pitch){
    const u32 choice=(static_cast<u32>(p[5])>>16)*3>>15;
    if(v.cutoff<0 || v.wave==static_cast<i32>(choice)*8192){
        if(choice==0){render_mode<0>(v,output,p,pitch);return;}
        if(choice==1){render_mode<1>(v,output,p,pitch);return;}
        if(choice==2){render_mode<2>(v,output,p,pitch);return;}
    }
    render_mode<-1>(v,output,p,pitch);
}

}
