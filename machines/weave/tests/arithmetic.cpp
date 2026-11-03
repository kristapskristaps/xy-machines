#include "../vendor/macro.cpp"
#include <cstdint>
#include <cstdio>

static int32_t floor31(int64_t product) {
    const int64_t scale = int64_t(1) << 31;
    int64_t result = product / scale;
    if (product < 0 && product % scale) --result;
    return static_cast<int32_t>(result);
}
int main() {
    uint32_t rng = 1;
    for (unsigned i = 0; i < 1000000; ++i) {
        int32_t words[4];
        for (auto& word : words) word = static_cast<int32_t>(rng = rng * 1664525u + 1013904223u);
        const int64_t ax = int64_t(words[0]) * words[1], by = int64_t(words[2]) * words[3];
        const int64_t partial = (ax >> 23) + (by >> 23) + 128;
        if (weave_dsp::fmac2(words[0],words[1],words[2],words[3]) != static_cast<int32_t>(partial >> 8) ||
            weave_dsp::fmac1(words[0],words[1]) != floor31(ax + (int64_t(1) << 30)) ||
            weave_dsp::fmac1t(words[0],words[1]) != floor31(ax)) {
            std::puts("FAIL: fractional multiply differs from integer reference"); return 1;
        }
        const int32_t a = static_cast<int32_t>(static_cast<uint32_t>(words[0]) >> 1);
        const int32_t b = static_cast<int32_t>(static_cast<uint32_t>(words[2]) >> 1);
        if (weave_dsp::modal_mac(a,words[1],b,words[3]) != weave_dsp::fmac2(a,words[1],b,words[3]) ||
            weave_dsp::modal_multiply(a,words[1]) != weave_dsp::fmac1(a,words[1])) {
            std::puts("FAIL: full-product modal arithmetic differs"); return 1;
        }
    }
    const int32_t minimum = -2147483647 - 1;
    if (weave_dsp::product_hi(minimum,minimum) != 2147483647) return 1;
    const int32_t boundaries[] = {minimum,minimum+1,-1073741824,-1,0,1,1073741824,2147483646,2147483647};
    for (int32_t a : boundaries) for (int32_t x : boundaries) {
        const int64_t ax=int64_t(a)*x;
        const bool saturated=ax==(int64_t(1)<<62);
        const uint32_t high=saturated?2147483647u:static_cast<uint32_t>(floor31(ax));
        const uint32_t low=static_cast<uint32_t>(ax);
        const int32_t single=static_cast<int32_t>(high+((low>>30)&1u));
        if (weave_dsp::fmac1(a,x)!=single) return 1;
        for (int32_t b : boundaries) for (int32_t y : boundaries) {
            const int64_t by=int64_t(b)*y;
            const uint32_t second=by==(int64_t(1)<<62)?2147483647u:static_cast<uint32_t>(floor31(by));
            const uint32_t tail=((low>>23)&255u)+((static_cast<uint32_t>(by)>>23)&255u)+128u;
            if (weave_dsp::fmac2(a,x,b,y)!=static_cast<int32_t>(high+second+(tail>>8))) return 1;
        }
    }
    const int32_t coefficients[] = {0,1,32767,1073741824,2147483647};
    const int32_t signals[] = {minimum,minimum+1,-1,0,1,2147483646,2147483647};
    for (int32_t a : coefficients) for (int32_t x : signals) {
        if (weave_dsp::modal_multiply(a,x) != weave_dsp::fmac1(a,x)) return 1;
        for (int32_t b : coefficients) for (int32_t y : signals)
            if (weave_dsp::modal_mac(a,x,b,y) != weave_dsp::fmac2(a,x,b,y)) return 1;
    }
    std::puts("PASS: one million fractional multiply/reference cases");
}
