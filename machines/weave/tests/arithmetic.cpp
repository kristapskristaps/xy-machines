#include "../vendor/macro.cpp"
#include <cstdint>
#include <cstdio>

static int32_t floor31(int64_t product) {
    const int64_t scale = int64_t(1) << 31;
    int64_t result = product / scale;
    if (product < 0 && product % scale) --result;
    return result>2147483647?2147483647:static_cast<int32_t>(result);
}
int main() {
    const int32_t edges[]={-2147483647-1,-2147483647,-1073741824,-1,0,1,1073741824,2147483646,2147483647};
    for(int32_t a:edges)for(int32_t x:edges)for(int32_t b:edges)for(int32_t y:edges) {
        const int32_t expected=static_cast<int32_t>(static_cast<uint32_t>(floor31(int64_t(a)*x))+
                                                  static_cast<uint32_t>(floor31(int64_t(b)*y)));
        if(weave_dsp::fmac2(a,x,b,y)!=expected || weave_dsp::fmac1(a,x)!=floor31(int64_t(a)*x)) {
            std::puts("FAIL: truncating multiply boundary");return 1;
        }
    }
    uint32_t rng = 1;
    for (unsigned i = 0; i < 1000000; ++i) {
        int32_t words[4];
        for (auto& word : words) word = static_cast<int32_t>(rng = rng * 1664525u + 1013904223u);
        const int64_t ax = int64_t(words[0]) * words[1], by = int64_t(words[2]) * words[3];
        const int32_t expected=static_cast<int32_t>(static_cast<uint32_t>(floor31(ax))+static_cast<uint32_t>(floor31(by)));
        const int32_t legacy=static_cast<int32_t>(((ax >> 23)+(by >> 23)+128)>>8);
        const int32_t rounding_change=static_cast<int32_t>(static_cast<uint32_t>(expected)-static_cast<uint32_t>(legacy));
        if (weave_dsp::fmac2(words[0],words[1],words[2],words[3]) != expected ||
            weave_dsp::fmac1(words[0],words[1]) != floor31(ax) ||
            rounding_change < -2 || rounding_change > 0 ||
            weave_dsp::fmac1t(words[0],words[1]) != floor31(ax)) {
            std::puts("FAIL: fractional multiply differs from integer reference"); return 1;
        }
    }
    const int32_t minimum = -2147483647 - 1;
    if (weave_dsp::product_hi(minimum,minimum) != 2147483647) return 1;
    std::puts("PASS: one million truncating product/reference cases; legacy sum-rounding change at most two Q31 LSBs");
}
