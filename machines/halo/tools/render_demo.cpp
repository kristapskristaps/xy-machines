#include "../fx.cpp"
#include <cstdio>
#include <cmath>
static halo::State state;
static opxy::i32 memory[halo::buffer_words],left[64],right[64],parameters[8];
static void word(FILE* file,unsigned value,unsigned bytes){for(unsigned i=0;i<bytes;++i)std::fputc((value>>(i*8))&255u,file);}
int main(int argc,char** argv) {
    if(argc!=10)return 1;
    for(unsigned i=0;i<8;++i){double value;if(std::sscanf(argv[i+2],"%lf",&value)!=1)return 1;parameters[i]=static_cast<opxy::i32>(value*2147483647.);}
    FILE* file=std::fopen(argv[1],"wb");if(!file)return 1;
    const unsigned frames=44100*14;
    std::fwrite("RIFF",1,4,file);word(file,frames*4+36,4);std::fwrite("WAVEfmt ",1,8,file);word(file,16,4);word(file,1,2);word(file,2,2);word(file,44100,4);word(file,176400,4);word(file,4,2);word(file,16,2);std::fwrite("data",1,4,file);word(file,frames*4,4);
    const unsigned notes[12]={57,64,69,72,60,67,72,76,62,69,74,77};
    opxy::Buffer buffer={memory,halo::buffer_words};opxy::Transport transport={1200,0,true};double energy=0,late=0,peak=0;
    double dry[64];
    for(unsigned frame=0;frame<frames;frame+=64) {
        for(unsigned i=0;i<64;++i){const double now=(frame+i)/44100.;double sample=0;
            for(unsigned note=0;note<12;++note){const double t=now-note*.5;if(t<0||t>3)continue;const double frequency=440*std::pow(2.,(static_cast<int>(notes[note])-69)/12.);const double phase=t*frequency*6.283185307179586;
                sample+=std::fmin(1.,t/.004)*(std::sin(phase)*.075+std::sin(phase*2.003)*.023*std::exp(-t*2)+std::sin(phase*4.01)*.009*std::exp(-t*6))*std::exp(-t*2.1);}
            dry[i]=sample;left[i]=right[i]=static_cast<opxy::i32>(sample*.62*2147483647.);
        }
        halo::process(state,buffer,transport,left,right,parameters);
        for(unsigned i=0;i<64&&frame+i<frames;++i){const opxy::i32 stereo[2]={left[i],right[i]};for(auto sample:stereo){double value=(sample/2147483648.+dry[i])*.8;value=std::fmax(-1.,std::fmin(1.,value));peak=std::fmax(peak,std::fabs(value));energy+=value*value;if(frame>44100*9)late+=value*value;word(file,static_cast<unsigned short>(static_cast<short>(value*32767)),2);}}
    }
    std::fclose(file);std::printf("peak %.4f RMS %.5f late energy %.5f\n",peak,std::sqrt(energy/(frames*2)),late);
}
