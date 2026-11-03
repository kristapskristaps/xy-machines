#include "../engine.cpp"
#include "reference/engine.cpp"
#include <cmath>
#include <cstdio>
int main(){
    // Quantify the explicit 1x tradeoff rather than claiming the removed FIR
    // still rejects aliases. Same 6 kHz/14.1 kHz clipping fixture as the legacy test.
    const double pi=3.14159265358979323846;
    double old_real=0,old_imag=0,new_real=0,new_imag=0;
    acidline::Voice cached={};do{acidline::prepare_shape(cached,11141,11469);}while(!cached.shape_ready);
    const int body=acidline::interpolate(acidline::tables::body,11141)*65536;
    const int drive_gain=acidline::distortion_gain(11469);
    double cached_real=0,cached_imag=0,direct_real=0,direct_imag=0,shape_energy=0,shape_error=0;
    opxy::i32 history[64]={};opxy::u32 cursor=0;
    for(unsigned n=0;n<44100;++n){
        const auto x=static_cast<opxy::i32>(.3*std::sin(2*pi*6000*n/44100)*65536);
        const double direct=acidline::distort(x,11469)/65536.;
        const double shaped=acidline::distort(acidline::limit(opxy::q31::multiply(x,body)*6,32768),11469,drive_gain)/65536.;
        const double approximate=acidline::cached_shape(cached,x)/65536.;opxy::i32 filtered=0;
        for(unsigned sub=0;sub<2;++sub){
            const auto source=static_cast<opxy::i32>(.3*std::sin(2*pi*6000*(n*2+sub)/88200)*65536);
            history[cursor]=history[cursor+32]=acidline_legacy::distort(source,11469);
            if(sub==1)filtered=acidline_legacy::decimate(history,cursor,acidline_legacy::tables::halfband31);
            cursor=(cursor+1)&31;
        }
        if(n>=4410){
            const double angle=2*pi*14100*n/44100;
            new_real+=direct*std::cos(angle);new_imag+=direct*std::sin(angle);
            old_real+=filtered/65536.*std::cos(angle);old_imag+=filtered/65536.*std::sin(angle);
            direct_real+=shaped*std::cos(angle);direct_imag+=shaped*std::sin(angle);
            cached_real+=approximate*std::cos(angle);cached_imag+=approximate*std::sin(angle);
            shape_energy+=shaped*shaped;shape_error+=(shaped-approximate)*(shaped-approximate);
        }
    }
    const double difference=10*std::log10((new_real*new_real+new_imag*new_imag)/(old_real*old_real+old_imag*old_imag));
    std::printf("1x tradeoff: nonlinear 6 kHz fixture has %.1f dB more 14.1 kHz alias energy than the frozen 2x/FIR reference\n",difference);
    const double cache_delta=10*std::log10((cached_real*cached_real+cached_imag*cached_imag)/(direct_real*direct_real+direct_imag*direct_imag));
    const double rms_error=std::sqrt(shape_error/shape_energy);
    std::printf("Cached/direct1x shaper: 14.1kHz spur delta %.4f dB, relative RMS error %.6f\n",cache_delta,rms_error);
    if(!std::isfinite(cache_delta)||std::abs(cache_delta)>.25||rms_error>.005)return 1;
    if(!std::isfinite(difference) || difference<20 || difference>80)return 1;
    return 0;
}
