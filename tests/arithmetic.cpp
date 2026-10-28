#include "../sample.cpp"
#include <cstdint>

int main() {
    std::uint32_t state = 27;
    for (unsigned i = 0; i < 100000; ++i) {
        state = state * 1664525u + 1013904223u;
        const std::uint32_t a = state;
        state = state * 1664525u + 1013904223u;
        const std::uint32_t b = state;
        const std::uint64_t product = std::uint64_t(a) * b;
        if (granular::high_product(a, b) != (product >> 32)) return 1;
        const std::uint64_t shifted = product >> 16;
        const std::uint32_t expected = shifted > (256u << 16) ? (256u << 16) : std::uint32_t(shifted);
        if (granular::multiply_rate(a, b) != expected) return 2;
        if (granular::divide_fraction(a, 25480370u) != (std::uint64_t(a) << 16) / 25480370u) return 3;
    }
    return 0;
}
