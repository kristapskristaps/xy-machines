#pragma once
// Portable two-word arithmetic for the imported DSP's coefficient calculations.
// All products and division steps use 32-bit operations; no compiler runtime is required.
namespace hkelg { namespace port {
struct Wide {
    using Signed=__INT64_TYPE__;
    using Unsigned=__UINT64_TYPE__;
    using Word=__UINT32_TYPE__;
    Signed value;
    constexpr Wide(Signed x=0):value(x){}
    template<class T> explicit operator T() const {return static_cast<T>(value);}
    static Unsigned product(Unsigned a,Unsigned b) {
        const Word al=static_cast<Word>(a),ah=static_cast<Word>(a>>32);
        const Word bl=static_cast<Word>(b),bh=static_cast<Word>(b>>32);
        const Word a0=al&65535u,a1=al>>16,b0=bl&65535u,b1=bl>>16;
        const Word t=a0*b0,w0=t&65535u;
        const Word u=a1*b0+(t>>16),w1=u&65535u,w2=u>>16;
        const Word v=a0*b1+w1;
        const Word low=(v<<16)+w0;
        const Word high=a1*b1+w2+(v>>16)+ah*bl+al*bh;
        return (static_cast<Unsigned>(high)<<32)|low;
    }
    static Unsigned magnitude(Signed x) {const Unsigned u=static_cast<Unsigned>(x);return x<0?~u+1u:u;}
    static Wide divide(Wide a,Wide b,bool remainder=false) {
        if(!b.value)return Wide(0);
        const Unsigned n=magnitude(a.value),d=magnitude(b.value);
        Unsigned q=0,r=0;
        for(unsigned bit=64;bit;--bit) {
            r=(r<<1)|((n>>(bit-1))&1u);
            if(r>=d){r-=d;q|=static_cast<Unsigned>(1)<<(bit-1);}
        }
        const Unsigned v=remainder?r:q;
        const bool negative=remainder?a.value<0:(a.value<0)!=(b.value<0);
        return Wide(static_cast<Signed>(negative?~v+1u:v));
    }
    friend Wide operator+(Wide a,Wide b){return Wide(static_cast<Signed>(static_cast<Unsigned>(a.value)+static_cast<Unsigned>(b.value)));}
    friend Wide operator-(Wide a,Wide b){return Wide(static_cast<Signed>(static_cast<Unsigned>(a.value)-static_cast<Unsigned>(b.value)));}
    friend Wide operator-(Wide a){return Wide(static_cast<Signed>(~static_cast<Unsigned>(a.value)+1u));}
    friend Wide operator*(Wide a,Wide b){return Wide(static_cast<Signed>(product(static_cast<Unsigned>(a.value),static_cast<Unsigned>(b.value))));}
    friend Wide operator/(Wide a,Wide b){return divide(a,b);}
    friend Wide operator%(Wide a,Wide b){return divide(a,b,true);}
    friend Wide operator<<(Wide a,unsigned b){return Wide(static_cast<Signed>(static_cast<Unsigned>(a.value)<<b));}
    friend Wide operator>>(Wide a,unsigned b){return Wide(a.value>>b);}
    friend Wide operator&(Wide a,Wide b){return Wide(a.value&b.value);}
    friend Wide operator|(Wide a,Wide b){return Wide(a.value|b.value);}
    friend Wide operator^(Wide a,Wide b){return Wide(a.value^b.value);}
    friend bool operator==(Wide a,Wide b){return a.value==b.value;}
    friend bool operator!=(Wide a,Wide b){return a.value!=b.value;}
    friend bool operator<(Wide a,Wide b){return a.value<b.value;}
    friend bool operator>(Wide a,Wide b){return a.value>b.value;}
    friend bool operator<=(Wide a,Wide b){return a.value<=b.value;}
    friend bool operator>=(Wide a,Wide b){return a.value>=b.value;}
    Wide& operator+=(Wide b){return *this=*this+b;}
    Wide& operator-=(Wide b){return *this=*this-b;}
    Wide& operator*=(Wide b){return *this=*this*b;}
    Wide& operator/=(Wide b){return *this=*this/b;}
    Wide& operator<<=(unsigned b){return *this=*this<<b;}
    Wide& operator>>=(unsigned b){return *this=*this>>b;}
    Wide& operator++(){return *this+=1;}
    Wide& operator--(){return *this-=1;}
};
inline int leading_zeroes(__UINT32_TYPE__ x) {
    if(!x)return 32;
    int n=0;
    if(!(x&0xffff0000u)){n+=16;x<<=16;}
    if(!(x&0xff000000u)){n+=8;x<<=8;}
    if(!(x&0xf0000000u)){n+=4;x<<=4;}
    if(!(x&0xc0000000u)){n+=2;x<<=2;}
    return n+((x&0x80000000u)?0:1);
}
}}
