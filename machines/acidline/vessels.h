#pragma once
namespace acidline {
inline opxy::i32 vessel_width(opxy::u32 kind,opxy::i32 y){
    using opxy::i32;
    if(kind==0)return 8+opxy::draw::clamp(y-112,0,72)*28/72;
    if(kind==2)return 14;
    static const i32 widths[71]={0,8,11,14,16,18,19,21,22,23,24,25,26,27,28,28,29,30,30,31,31,32,32,32,33,33,33,34,34,34,34,34,34,34,34,35,34,34,34,34,34,34,34,34,33,33,33,32,32,32,31,31,30,30,29,28,28,27,26,25,24,23,22,21,19,18,16,14,11,8,0};
    return widths[opxy::draw::clamp(y-120,0,70)];
}
}
