#include "../fx.cpp"
static halo::State state;
static opxy::i32 scratch[halo::buffer_words],left[64],right[64],parameters[8];
extern "C" {
void* memset(void* pointer,int value,unsigned long count) {
    volatile unsigned char* bytes=static_cast<volatile unsigned char*>(pointer);
    for(unsigned long i=0;i<count;++i)bytes[i]=static_cast<unsigned char>(value);
    return pointer;
}
opxy::i32* halo_left(){return left;}
opxy::i32* halo_right(){return right;}
opxy::i32* halo_parameters(){return parameters;}
void halo_reset(){halo::initialize(state);}
void halo_process(unsigned bpm,unsigned ticks,unsigned playing) {
    opxy::Buffer buffer={scratch,halo::buffer_words};
    opxy::Transport transport={bpm,ticks,playing!=0};
    halo::process(state,buffer,transport,left,right,parameters);
}
}
