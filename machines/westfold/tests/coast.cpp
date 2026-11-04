#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

static void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

int main() {
    double maximum_error=0;
    // Compare the runtime recurrence with the independent trigonometric definition
    // of each polynomial at every quantized oscillator value, including both rails.
    const int zero[9] = {};
    for (unsigned family = 0; family < 2; ++family) for (unsigned frame = 0; frame < 17; ++frame) {
        int weights[9];
        for (unsigned h = 0; h < 9; ++h) weights[h] = westfold::coast::frames[family][frame][h];
        for (int value = -32768; value <= 32767; ++value) {
            const double angle = std::acos(value / 32768.0);
            double expected = 0;
            for (unsigned h = 0; h < 9; ++h) expected += weights[h] * std::cos((h + 1) * angle);
            const int actual = westfold::harmonic_sample(value, weights, zero);
            maximum_error=std::fmax(maximum_error,std::abs(actual-expected));
            require(std::abs(actual - expected) < 256, "Q13 Clenshaw output agrees with the independent trigonometric series within its fixed-point error bound");
            require(actual >= -30000 && actual <= 30000, "harmonic series stays inside its headroom");
        }
    }
    for (unsigned family = 0; family < 2; ++family) {
        for (unsigned frame = 0; frame < 17; ++frame) {
            int sum = 0, weighted = 0;
            for (unsigned h = 0; h < 9; ++h) {
                const int weight = westfold::coast::frames[family][frame][h];
                sum += std::abs(weight);
                weighted += (h + 1) * std::abs(weight);
                require((h % 2 == family) || weight == 0, "odd/even families do not leak the opposite parity");
            }
            require(weighted < 214000, "Q13 recurrence products fit i32 at the oscillator rails");
            require(sum > 28000 && sum < 30000, "each folding frame has bounded useful energy");
        }
    }
    for (unsigned h = 1; h <= 9; ++h) {
        int previous = 32768;
        for (unsigned step = 0; step <= 1024; ++step) {
            const int gain = westfold::harmonic_gain(step * 0x200000u, h);
            require(gain >= 0 && gain <= 32768 && gain <= previous, "partial attenuation is bounded and monotonic");
            previous = gain;
        }
        require(previous == 0, "partials stop before Nyquist");
    }
    std::printf("Maximum recurrence error %.3f Q15 units.\n",maximum_error);
    std::puts("PASS: all 65,536 oscillator values, bounded harmonic frames and monotonic partial guards");
}
