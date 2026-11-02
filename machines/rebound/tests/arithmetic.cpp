#include "../fx.cpp"
#include <cstdio>
#include <cstdlib>

using opxy::i32;
using opxy::i64;
using opxy::u32;

static void require(bool ok, const char* message, i32 a, i32 b) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s (%d, %d)\n", message, a, b);
        std::exit(1);
    }
}

static i32 reference(i32 injection, i32 loop) {
    const i64 value = static_cast<i64>(injection) + 4LL * loop;
    const i64 magnitude = value < 0 ? -value : value;
    const i32 knee = 536870912, ceiling = 1073741823;
    i32 result;
    if (magnitude <= knee) result = static_cast<i32>(magnitude);
    else if (magnitude >= 3LL * knee) result = ceiling;
    else {
        const i32 x = static_cast<i32>(magnitude - knee);
        result = knee + x - opxy::q31::multiply(x, x);
        if (result > ceiling) result = ceiling;
    }
    return value < 0 ? -result : result;
}

static void saturation(i32 injection, i32 loop) {
    require(rebound::saturate_sum(injection, loop) == reference(injection, loop),
            "wide-reference saturation", injection, loop);
}

static void interpolation(i32 difference, i32 fraction) {
    require(rebound::fraction_of(difference, fraction) == static_cast<i64>(difference) * fraction / 256,
            "signed interpolation", difference, fraction);
}

int main() {
    const i32 injections[] = {-1073741824, -1073741823, -536870912, -4, -3, -2, -1,
                              0, 1, 2, 3, 4, 536870912, 1073741822, 1073741823};
    const i32 loops[] = {-805306368, -536870912, -402653184, -134217728, -1, 0,
                         1, 134217728, 402653184, 536870911, 805306367};
    for (auto input : injections) for (auto loop : loops) saturation(input, loop);
    const i32 edges[] = {-1610612736, -536870912, 536870912, 1610612736};
    for (auto input : injections) for (auto edge : edges) {
        for (i32 offset = -1024; offset <= 1024; ++offset) {
            const i64 loop = (static_cast<i64>(edge) - input) / 4 + offset;
            if (loop >= -805306368 && loop <= 805306367) saturation(input, static_cast<i32>(loop));
        }
    }
    for (i32 difference = -65536; difference <= 65536; ++difference)
        for (i32 fraction = 0; fraction < 256; ++fraction) interpolation(difference, fraction);
    const i32 differences[] = {-2147483646, -2147483645, -1073741823, -257, -256, -255, -1,
                               0, 1, 255, 256, 257, 1073741823, 2147483645, 2147483646};
    for (auto difference : differences)
        for (i32 fraction = 0; fraction < 256; ++fraction) interpolation(difference, fraction);
    u32 random = 7139;
    for (unsigned n = 0; n < 1000000; ++n) {
        const i32 input = static_cast<i32>((random = random * 1664525u + 1013904223u) & 0x7fffffffu) - 1073741824;
        const i32 loop = static_cast<i32>((random = random * 1664525u + 1013904223u) % 1610612736u) - 805306368;
        saturation(input, loop);
        const i32 a = static_cast<i32>((random = random * 1664525u + 1013904223u) % 2147483647u) - 1073741823;
        const i32 b = static_cast<i32>((random = random * 1664525u + 1013904223u) % 2147483647u) - 1073741823;
        const i32 fraction = static_cast<i32>((random = random * 1664525u + 1013904223u) >> 24);
        interpolation(b - a, fraction);
    }
    std::puts("PASS: wide-reference saturation/cancellation and exact signed interpolation across boundaries and random inputs");
}
