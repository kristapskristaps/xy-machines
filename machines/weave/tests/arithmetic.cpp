#include "../vendor/macro.cpp"
#include <cstdint>
#include <cstdio>

static int32_t floor31(int64_t product) {
    const int64_t scale = int64_t(1) << 31;
    int64_t result = product / scale;
    if (product < 0 && product % scale) --result;
    return static_cast<int32_t>(result);
}
static bool normalization_matches(uint32_t value) {
    int expected = 31;
    if (value) {
        expected = 0;
        for (uint32_t bit = 0x80000000u; !(value & bit); bit >>= 1) ++expected;
    }
    return weave_dsp::clz32(value) == expected;
}
int main() {
    if (!normalization_matches(0)) return 1;
    for (unsigned bit = 0; bit < 32; ++bit) {
        const uint32_t value = uint32_t(1) << bit;
        if (!normalization_matches(value) || !normalization_matches(value - 1) ||
            !normalization_matches(value + 1)) return 1;
    }
    uint32_t rng = 1;
    for (unsigned i = 0; i < 1000000; ++i) {
        int32_t words[4];
        for (auto& word : words) {
            word = static_cast<int32_t>(rng = rng * 1664525u + 1013904223u);
            if (!normalization_matches(static_cast<uint32_t>(word))) {
                std::puts("FAIL: coefficient normalization differs from bit reference"); return 1;
            }
        }
        const int64_t ax = int64_t(words[0]) * words[1], by = int64_t(words[2]) * words[3];
        const int64_t partial = (ax >> 23) + (by >> 23) + 128;
        if (weave_dsp::fmac2(words[0],words[1],words[2],words[3]) != static_cast<int32_t>(partial >> 8) ||
            weave_dsp::fmac1(words[0],words[1]) != floor31(ax + (int64_t(1) << 30)) ||
            weave_dsp::fmac1t(words[0],words[1]) != floor31(ax)) {
            std::puts("FAIL: fractional multiply differs from integer reference"); return 1;
        }
    }
    const int32_t minimum = -2147483647 - 1;
    if (weave_dsp::product_hi(minimum,minimum) != 2147483647) return 1;
    std::puts("PASS: one million fractional multiply cases and four million coefficient normalizations");
}
