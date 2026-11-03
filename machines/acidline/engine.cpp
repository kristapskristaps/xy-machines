#include "core.h"
#include "tables.h"
#include "ladder_tables.h"
#include <opxy/q31.hpp>
// Open303 TB_303 ladder/coefficient adaptation via Acid Rain; see NOTICE.md.
namespace acidline {
namespace {
using opxy::i32;using opxy::u32;
using opxy::q31::multiply;
const i32 one=16384;
i32 limit(i32 x,i32 bound){return x < -bound ? -bound : x > bound ? bound : x;}
// Callers bound these short products below 2^31; retain their Q14 floor.
i32 gain14(i32 sample,i32 coefficient){return sample*coefficient>>14;}
i32 mix(i32 a,i32 b,i32 fraction){return a+gain14(b-a,fraction);}
i32 interpolate(const opxy::i16* table,i32 control){
    const u32 index=static_cast<u32>(control)>>8;
    return table[index]+(table[index+1]-table[index])*(control&255)/256;
}
i32 interpolate31(const i32* table,i32 control){
    const u32 index=static_cast<u32>(control)>>8;
    return table[index]+multiply(table[index+1]-table[index],(control&255)*8388608);
}
i32 cutoff_target(i32 base,i32 envelope,i32 amount){
    return limit(base+gain14(one-1-base,gain14(envelope,amount)),one-1);
}
i32 lookup(const opxy::i16* row,u32 phase){
    const u32 index=phase>>23;
    return row[index]+multiply(row[index+1]-row[index],static_cast<i32>((phase&0x7ffe00u)<<8));
}
i32 distortion_gain(i32 amount){return 256+gain14(amount,amount)*23/64;}
i32 distort(i32 x,i32 amount,i32 gain){
    if(!amount)return x;
    const i32 boosted=x*gain/256;
    const u32 position=static_cast<u32>(boosted<0?-boosted:boosted),index=position>>10;
    const i32 clipped=index>=256?32767:tables::clip[index]+
        (tables::clip[index+1]-tables::clip[index])*static_cast<i32>(position&1023u)/1024;
    return mix(x,boosted<0?-clipped:clipped,amount);
}
inline i32 distort(i32 x,i32 amount){return distort(x,amount,distortion_gain(amount));}
template<int Mode>
__attribute__((noinline)) void oscillators(Voice& v,opxy::Output output,opxy::Pitch pitch,u32 band,i32 wave,i32 drive){
    const i32 dw=(wave-v.wave)/64;
    i32 gain=(4096+v.drive/2)*131072,input_low=v.input_low;
    const i32 dg=((4096+drive/2)*131072-gain)/64;
    u32 phase=v.phase;
    for(unsigned i=0;i<opxy::block_frames;++i){
        if(Mode<0)v.wave+=dw;
        i32 source;
        if(Mode==1 || (Mode<0 && v.wave==8192))source=lookup(tables::triangle[band],phase);
        else{
            const i32 saw=lookup(tables::saw[band],phase);source=saw;
            if(Mode==2 || (Mode<0 && v.wave>8192)){
                const i32 square=(saw-lookup(tables::saw[band],phase+0x80000000u))/2;
                source=(Mode==2 || v.wave==one)?square:mix(lookup(tables::triangle[band],phase),square,(v.wave-8192)*2);
            }else if(Mode<0 && v.wave)source=mix(saw,lookup(tables::triangle[band],phase),v.wave*2);
        }
        gain+=dg;input_low+=multiply(source-input_low,ladder_tables::input_highpass);
        output[i]=multiply(source-input_low,gain)*4;phase+=pitch[i];
    }
    v.phase=phase;v.input_low=input_low;
}
__attribute__((noinline)) void ladder(Voice& v,opxy::Output output,i32 coefficient,i32 feedback){
    i32 y1=v.pole[0],y2=v.pole[1],y3=v.pole[2],y4=v.pole[3];
    i32 feedback_low=v.feedback_low;
    i32 b2=v.filter_coefficient,fb=v.filter_feedback;
    const i32 db=(coefficient-b2)/64,df=(feedback-fb)/64;
    for(unsigned i=0;i<opxy::block_frames;++i){
        b2+=db;fb+=df;
        const i32 b0=b2/2;
        const i32 source=output[i];
        const i32 fed=multiply(y4,fb)*128;
        feedback_low+=multiply(fed-feedback_low,ladder_tables::feedback_highpass);
        const i32 y0=source-(fed-feedback_low);
        y1+=multiply(b2,y0-y1+y2);
        y2+=multiply(b0,y1-2*y2+y3);
        y3+=multiply(b0,y2-2*y3+y4);
        y4+=multiply(b0,y3-2*y4);
        output[i]=y4;
    }
    v.pole[0]=y1;v.pole[1]=y2;v.pole[2]=y3;v.pole[3]=y4;
    v.feedback_low=feedback_low;
    v.filter_coefficient=coefficient;v.filter_feedback=feedback;
}
__attribute__((noinline)) void finish(Voice& v,opxy::Output output,opxy::Pitch pitch,
        i32 resonance,i32 drive,i32 sub,i32 tone){
    i32 amount=v.drive,blend_sub=v.sub,damping=v.tone;
    const i32 dd=(drive-amount)/64,ds=(sub-blend_sub)/64,dt=(tone-damping)/64;
    const i32 body_start=interpolate(tables::body,v.resonance);
    i32 body=body_start*65536,clip_gain=distortion_gain(v.drive);
    const i32 db=(interpolate(tables::body,resonance)-body_start)/64*65536;
    const i32 dc=(distortion_gain(drive)-clip_gain)/64;
    const i32 start_tone=interpolate(tables::tone,v.tone);
    i32 tone_g=start_tone*131072;
    const i32 dg=(interpolate(tables::tone,tone)-start_tone)/64*131072;
    for(unsigned i=0;i<opxy::block_frames;++i){
        amount+=dd;blend_sub+=ds;damping+=dt;body+=db;clip_gain+=dc;tone_g+=dg;
        const i32 shaped=distort(limit(multiply(output[i],body)*6,32768),amount,clip_gain);
        const i32 dc_removed=limit(shaped-v.dc_input+multiply(v.dc_output,16376*131072),32768);
        v.dc_input=shaped;v.dc_output=dc_removed;
        if(damping>=16383)v.tone_memory=dc_removed;
        else v.tone_memory+=multiply(dc_removed-v.tone_memory,tone_g);
        const i32 low=blend_sub?lookup(tables::sine,v.sub_phase):0;
        output[i]=mix(v.tone_memory,low,blend_sub/2)*32768;
        const u32 increment=pitch[i];v.sub_remainder+=increment&1u;
        v.sub_phase+=increment/2+v.sub_remainder/2;v.sub_remainder&=1u;
    }
}
}
inline void initialize(Voice& v,Shared&,u32){v=Voice();v.cutoff=-1;}
inline void start(Voice& v,Shared&,u32){v.envelope=one;}
inline void destroy(Voice&,Shared&,u32){}
inline void render(Voice& v,Shared&,u32,opxy::Output output,opxy::Parameters p,opxy::Pitch pitch){
    const i32 base=p[0]/131072,resonance=p[1]/131072,drive=p[2]/131072;
    const i32 env=p[3]/131072,decay=p[4]/131072;
    const u32 choice=(static_cast<u32>(p[5])>>16)*3>>15;
    const i32 wave=static_cast<i32>(choice)*8192,sub=p[6]/131072,tone=16383-p[7]/131072;
    const i32 cutoff=cutoff_target(base,v.envelope,env);
    const i32 coefficient=interpolate31(ladder_tables::b0x2,cutoff);
    const i32 k=interpolate31(ladder_tables::feedback,cutoff);
    const i32 feedback=multiply(k,interpolate(tables::feedback,resonance)*131072)*256;
    const bool first=v.cutoff<0;
    if(first){v.cutoff=cutoff;v.resonance=resonance;v.drive=drive;v.wave=wave;v.sub=sub;v.tone=tone;
        v.filter_coefficient=coefficient;v.filter_feedback=feedback;}
    u32 highest=0;for(unsigned i=0;i<opxy::block_frames;++i)if(pitch[i]>highest)highest=pitch[i];
    u32 band=0;while(band<7 && highest<(0x80000000u>>(band+1)))++band;
    if(v.wave==wave){
        if(choice==0)oscillators<0>(v,output,pitch,band,wave,drive);
        else if(choice==1)oscillators<1>(v,output,pitch,band,wave,drive);
        else oscillators<2>(v,output,pitch,band,wave,drive);
    }else oscillators<-1>(v,output,pitch,band,wave,drive);
    ladder(v,output,coefficient,feedback);
    finish(v,output,pitch,resonance,drive,sub,tone);
    v.envelope=gain14(v.envelope,interpolate(tables::decay,decay));
    v.cutoff=cutoff;v.resonance=resonance;v.drive=drive;v.wave=wave;v.sub=sub;v.tone=tone;
}
}
