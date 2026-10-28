#include "../engine.cpp"
#include <cmath>
#include <cstdio>

int main() {
    // Independent double-precision version of Elements' modal coefficient laws.
    const double notes[]={27.5,55,110,440,1760,7040};
    const int geometries[]={0,8192,9830,19661,29491,32767};
    const int controls[]={0,9830,22938,32767};
    matter::Voice voice={};voice.key[0]=-1;
    double worst=0;
    double worst_hz=0;int worst_geometry=0,worst_brightness=0,worst_damping=0;unsigned worst_mode=0;
    for(double hz:notes)for(int geometry:geometries)for(int brightness:controls)for(int damping:controls) {
        const opxy::u32 increment=static_cast<opxy::u32>(hz/44100.0*4294967296.0+.5);
        matter::coefficients(voice,increment,geometry,brightness,damping);
        const double g=geometry/32768.0,b=brightness/32768.0,d=damping/32768.0;
        const unsigned index=static_cast<unsigned>(geometry)>>7;
        double stiffness=(matter_dsp::stiffness[index]+(matter_dsp::stiffness[index+1]-matter_dsp::stiffness[index])*(geometry&127)/128.0)/16777216.0;
        const double brightness_adjusted=b*(1-.2*std::pow(1-g,8));
        double q=500*std::pow(10,3.2*d);
        double qloss=brightness_adjusted*(2-brightness_adjusted)*.85+.15;
        const double qrate=g*(2-g)*.1;
        double stretch=1;
        for(unsigned mode=0;mode<matter::mode_count;++mode) {
            const double f=std::fmin(.49,increment/4294967296.0*(mode+1)*stretch);
            const double pi=3.14159265358979323846;
            const double tangent=f*(pi+f*f*(.3260*pi*pi*pi+.1823*pi*pi*pi*pi*pi*f*f));
            const double quality=1+f*q;
            const double a1=1/(1+tangent*(tangent+1/quality)),a2=tangent*a1,a3=tangent*a2;
            const matter::Filter& actual=voice.modes[mode];
            const double error=std::fmax(std::fabs(actual.a1/2147483648.0-a1),std::fmax(std::fabs(actual.a2/2147483648.0-a2),std::fabs(actual.a3/2147483648.0-a3)));
            if(error>worst) {worst_hz=hz;worst_geometry=geometry;worst_brightness=brightness;worst_damping=damping;worst_mode=mode;}
            worst=std::fmax(worst,std::fabs(actual.a1/2147483648.0-a1));
            worst=std::fmax(worst,std::fabs(actual.a2/2147483648.0-a2));
            worst=std::fmax(worst,std::fabs(actual.a3/2147483648.0-a3));
            stretch+=stiffness;stiffness*=stiffness<0?.93:.98;
            qloss+=qrate*(1-qloss);q*=qloss;
        }
    }
    std::printf("Modal coefficient maximum absolute error: %.9f\n",worst);
    if(worst>.00003){std::printf("case %.1f %d %d %d mode %u\n",worst_hz,worst_geometry,worst_brightness,worst_damping,worst_mode);return 1;}
    // Pickup position uses the original recurrence, bounded over all 52 modes.
    for(int position=0;position<32768;position+=31) {
        const opxy::i32 coefficient=matter::position_coefficient(position);
        opxy::i32 y0=536870912,y1=coefficient/2;
        for(unsigned mode=0;mode<matter::mode_count;++mode) {
            const opxy::i32 previous=y0;y0=matter_dsp::fmac1(coefficient,y0)*4-y1;y1=previous;
            if(y0<-537000000||y0>537000000)return 1;
        }
    }
    std::puts("PASS: 29,952 double-reference modal coefficient sets and pickup recurrence");
}
