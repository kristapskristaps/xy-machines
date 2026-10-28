#include "core.h"
#include "tables.h"
namespace acidline {
namespace {
using opxy::i32;using opxy::u32;
const i32 one=16384;
const i32 internal_peak=49152;
i32 limit(i32 x,i32 bound){return x < -bound ? -bound : x > bound ? bound : x;}
// Samples are Q16, coefficients Q14. Products stay in signed 32 bits.
i32 multiply(i32 sample,i32 coefficient){return sample*coefficient/one;}
i32 soft(i32 x){
    x=limit(x,internal_peak);
    const i32 half=x/2;
    const i32 square=half*half/one;
    return x-(x*square/65536)/3;
}
i32 mix(i32 a,i32 b,i32 fraction){return a+(b-a)*fraction/one;}
i32 lookup(const opxy::i16* row,u32 phase){
    const u32 index=phase>>23;
    return row[index]+(row[index+1]-row[index])*static_cast<i32>((phase&0x7fffffu)>>9)/one;
}
i32 distort_row(i32 x,const opxy::i16* row){
    const u32 position=static_cast<u32>(limit(x,32768)+32768);
    const u32 index=position>>6;
    if(index>=1024)return row[1024];
    return row[index]+(row[index+1]-row[index])*static_cast<i32>(position&63u)/64;
}
i32 distort(i32 x,i32 amount){
    if(!amount)return x;
    const u32 row=static_cast<u32>(amount)>>9;
    const i32 fraction=amount&511;
    const i32 a=distort_row(x,tables::distortion[row]);
    return a+(distort_row(x,tables::distortion[row+1])-a)*fraction/512;
}
i32 interpolate(const opxy::i16* table,i32 control){
    const u32 index=static_cast<u32>(control)>>8;
    return table[index]+(table[index+1]-table[index])*(control&255)/256;
}
i32 inverse(i32 cutoff,i32 resonance){
    const u32 row=static_cast<u32>(resonance)>>8;
    const i32 a=interpolate(tables::inverse[row],cutoff);
    return a+(interpolate(tables::inverse[row+1],cutoff)-a)*(resonance&255)/256;
}
template<unsigned Count,unsigned Size>
i32 decimate(i32 (&history)[Size],u32 cursor,const opxy::i16 (&kernel)[Count]){
    const u32 middle=Count/2;
    i32 sum=history[(cursor-middle)&(Size-1)]*kernel[middle];
    for(u32 i=0;i<middle;++i){
        if(kernel[i])sum+=(history[(cursor-i)&(Size-1)]+history[(cursor-(Count-1-i))&(Size-1)])*kernel[i];
    }
    return sum/one;
}
}
inline void initialize(Voice& v,Shared&,opxy::u32){v=Voice();v.cutoff=-1;}
inline void start(Voice& v,Shared&,opxy::u32){v.envelope=one;}
inline void destroy(Voice&,Shared&,opxy::u32){}
inline void render(Voice& v,Shared&,opxy::u32,opxy::Output output,opxy::Parameters p,opxy::Pitch pitch){
    const i32 base=p[0]/131072,resonance=p[1]/131072;
    const i32 drive=p[2]/131072,env=p[3]/131072,decay_control=p[4]/131072,wave=p[5]/131072;
    const i32 cutoff=limit(base+multiply(multiply(v.envelope,env),10650),one-1);
    if(v.cutoff<0){v.cutoff=cutoff;v.resonance=resonance;v.drive=drive;v.wave=wave;}
    i32 g=interpolate(tables::pole,v.cutoff);
    i32 feedback=interpolate(tables::feedback,v.resonance);
    i32 correction=inverse(v.cutoff,v.resonance);
    const i32 dg=(interpolate(tables::pole,cutoff)-g)/64;
    const i32 df=(interpolate(tables::feedback,resonance)-feedback)/64;
    const i32 di=(inverse(cutoff,resonance)-correction)/64;
    const i32 dd=(drive-v.drive)/64,dw=(wave-v.wave)/64;
    u32 highest=0;
    for(u32 i=0;i<64;++i)if(pitch[i]>highest)highest=pitch[i];
    highest/=2;
    u32 band=0;while(band<7 && highest<(0x80000000u>>(band+1)))++band;
    for(u32 i=0;i<64;++i){
        g+=dg;feedback+=df;correction+=di;v.drive+=dd;v.wave+=dw;
        const i32 one_minus_g=one-g;
        i32 reduced=0;
        const u32 increment=pitch[i];
        const u32 width=0x74000000u+increment/8;
        for(u32 sub=0;sub<2;++sub){
            const i32 saw=lookup(tables::saw[band],v.phase);
            i32 source=saw;
            if(v.wave){
                const i32 pulse=(saw-lookup(tables::saw[band],v.phase+width))/2;
                source=mix(saw,pulse,v.wave);
            }
            i32 memory=0;
            for(u32 stage=0;stage<4;++stage)memory=multiply(memory,g)+multiply(v.pole[stage],one_minus_g);
            i32 signal=multiply(source/4-multiply(memory,feedback),correction);
            signal=soft(limit(signal,12288)*4);
            for(u32 stage=0;stage<4;++stage){
                const i32 y=multiply(signal,g)+multiply(v.pole[stage],one_minus_g);
                v.pole[stage]=limit(y+(y-v.pole[stage]),internal_peak);
                signal=y;
            }
            v.fir1[v.cursor1]=distort(limit(signal+signal/2,32768),v.drive);
            if(sub==1)reduced=decimate(v.fir1,v.cursor1,tables::halfband31);
            v.cursor1=(v.cursor1+1)&31u;
            // Honor stock pitch modulation and portamento exactly, sample by sample.
            v.phase+=increment/2+(sub==1?increment%2:0);
        }
        const i32 dc_removed=limit(reduced-v.dc_input+multiply(v.dc_output,16376),32768);
        v.dc_input=reduced;v.dc_output=dc_removed;
        output[i]=dc_removed*32768;
    }
    v.envelope=multiply(v.envelope,interpolate(tables::decay,decay_control));
    v.cutoff=cutoff;v.resonance=resonance;v.drive=drive;v.wave=wave;
}
}
