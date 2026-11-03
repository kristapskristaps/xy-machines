#include "../dsp.h"
#include <cstdio>
#include <initializer_list>

bool check(int a,int b,int gain) {
    const int before=westfold::dsp::mix(a,b,gain);
    const int after=westfold::dsp::blend16(a,b,gain);
    const int minimum=a<b?a:b,maximum=a>b?a:b;
    return after>=before-1 && after<=before+1 && after>=minimum && after<=maximum;
}
int main() {
    for(int a:{-32768,-32767,-1,0,1,32766,32767})
        for(int b:{-32768,-32767,-1,0,1,32766,32767})
            for(int gain:{0,1,16383,16384,32766,32767})if(!check(a,b,gain))return 1;
    unsigned random=53519;
    for(unsigned i=0;i<1000000;++i) {
        const int a=static_cast<int>((random=random*1664525u+1013904223u)>>16)-32768;
        const int b=static_cast<int>((random=random*1664525u+1013904223u)>>16)-32768;
        const int gain=static_cast<int>((random=random*1664525u+1013904223u)>>17);
        if(!check(a,b,gain))return 1;
    }
    std::puts("PASS: each half-difference blend has at most one Q15 count error and preserves convex signed16 bounds; endpoints and one million random cases");
}
