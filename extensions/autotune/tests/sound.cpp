#include "extension.cpp"
#include <cmath>
#include <cstdio>

namespace {
using namespace opxy;
using namespace opxy::extensions::autotune;
constexpr double tau = 6.283185307179586, sample_rate = 44100.0;
constexpr double peak = 2147483648.0;
constexpr u32 delay = 672;

bool expect(bool condition, const char* name) {
    if (!condition) std::fprintf(stderr, "Failed: %s\n", name);
    return condition;
}

i32 tone(double frequency, u32 frame) {
    const double p = tau * frequency * frame / sample_rate;
    return static_cast<i32>(peak * (0.34 * std::sin(p) + 0.13 * std::sin(2 * p) + 0.06 * std::sin(3 * p)));
}

bool tunes(double input, double target, bool opposite, u32 scale = 2, u32 retune = 0,
           u32 amount = 15, u32 mask = 0, i32 attenuation = 1) {
    reset_input();
    prepare_input(scale, retune, amount, mask);
    i32 stereo[2 * block_frames];
    double first = -1, last = 0;
    u32 crossings = 0;
    i32 previous = 0;
    i32 highest = 0;
    bool stereo_ok = true;
    for (u32 start = 0; start < 768 * block_frames; start += block_frames) {
        for (u32 i = 0; i < block_frames; ++i) {
            const i32 value = tone(input, start + i) / attenuation;
            stereo[2 * i] = opposite ? -value : 0;
            stereo[2 * i + 1] = value;
        }
        process_input(stereo);
        for (u32 i = 0; i < block_frames; ++i) {
            const i32 right = stereo[2 * i + 1];
            const i32 left = stereo[2 * i];
            stereo_ok &= opposite ? std::abs(left + right) < 16 : left == 0;
            if (start >= 16384 && previous <= 0 && right > 0) {
                const double time = start + i - 1 + double(-previous) / (right - previous);
                if (first < 0) first = time;
                last = time;
                ++crossings;
            }
            if (start >= 16384 && right > highest) highest = right;
            previous = right;
        }
    }
    const double detected = crossings > 1 ? (crossings - 1) * sample_rate / (last - first) : 0;
    const double cents = 1200.0 * std::log2(detected / target);
    if (std::abs(cents) > 5.0)
        std::fprintf(stderr, "%.1f Hz input: %.3f Hz output, expected %.3f Hz (%.2f cents)\n",
                     input, detected, target, cents);
    return expect(stereo_ok, "stereo balance and polarity") &&
        expect(highest > peak * 0.32 / attenuation, "voiced output keeps its level") &&
        expect(std::abs(cents) <= 5.0, "nearest permitted pitch");
}

double vibrato_error(u32 mode, u32 retune) {
    constexpr double c3 = 130.81278265;
    reset_input();
    prepare_input(mode, retune, 15, 0);
    i32 stereo[2 * block_frames];
    double phase = 0, last_crossing = 0, error = 0;
    i32 previous = 0;
    u32 count = 0;
    for (u32 start = 0; start < 1536 * block_frames; start += block_frames) {
        for (u32 i = 0; i < block_frames; ++i) {
            const double time = (start + i) / sample_rate;
            const double cents = 45.0 + 45.0 * std::sin(tau * 5.4 * time);
            phase += tau * c3 * std::exp2(cents / 1200.0) / sample_rate;
            const double vowel = 0.34 * std::sin(phase) + 0.13 * std::sin(2 * phase) +
                0.06 * std::sin(3 * phase);
            stereo[2 * i] = stereo[2 * i + 1] = static_cast<i32>(peak * vowel);
        }
        process_input(stereo);
        for (u32 i = 0; i < block_frames; ++i) {
            const i32 current = stereo[2 * i];
            if (start >= 8192 && previous <= 0 && current > 0) {
                const double crossing = start + i - 1.0 - double(previous) / (current - previous);
                if (last_crossing) {
                    const double frequency = sample_rate / (crossing - last_crossing);
                    error += std::abs(1200.0 * std::log2(frequency / c3));
                    ++count;
                }
                last_crossing = crossing;
            }
            previous = current;
        }
    }
    return count > 200 ? error / count : 1200.0;
}

bool hard_flattens_vibrato() {
    // The input moves between C3 and 90 cents sharp, always closer to C than D.
    const double dry = vibrato_error(0, 0);
    const double smooth = vibrato_error(3, 0);
    const double hard = vibrato_error(3, 1);
    if (dry >= 40.0 && dry <= 50.0 && hard < 12.0 && hard < smooth * 0.6) return true;
    std::fprintf(stderr, "C3 vibrato error: Off %.2f cents, Smooth %.2f, Hard %.2f\n", dry, smooth, hard);
    return false;
}

bool noise_and_reset(u32 retune, i32 attenuation) {
    reset_input();
    prepare_input(2, retune, 15, 0);
    u32 noise = 42;
    i32 ring[2048] = {};
    i32 stereo[2 * block_frames];
    for (u32 start = 0; start < 256 * block_frames; start += block_frames) {
        for (u32 i = 0; i < block_frames; ++i) {
            noise = noise * 1664525u + 1013904223u;
            const i32 value = (static_cast<i32>(noise >> 2) - 536870912) / attenuation;
            ring[(start + i) & 2047] = value;
            stereo[2 * i] = value;
            stereo[2 * i + 1] = -value;
        }
        process_input(stereo);
        for (u32 i = 0; i < block_frames; ++i) {
            const u32 frame = start + i;
            const i32 expected = frame >= delay ? ring[(frame - delay) & 2047] : 0;
            if (!expect(stereo[2 * i] == expected && stereo[2 * i + 1] == -expected, "unpitched input stays dry"))
                return false;
        }
    }
    reset_input();
    for (u32 block = 0; block < 32; ++block) {
        for (auto& sample : stereo) sample = 0;
        process_input(stereo);
        for (auto sample : stereo)
            if (!expect(sample == 0, "reset drops old audio")) return false;
    }
    return true;
}
}

int main() {
    bool ok = true;
    ok &= tunes(432.0, 440.0, false);
    ok &= tunes(451.0, 440.0, true);
    ok &= tunes(440.0, 440.0, true);
    ok &= tunes(81.0, 82.40689, false);
    ok &= tunes(850.0, 830.609395, true);
    ok &= tunes(313.0, 329.627557, false, 3, 1);   // Eb resolves up to E in C major.
    ok &= tunes(309.0, 293.664768, true, 24, 1);  // Eb resolves down to D in A minor.
    ok &= tunes(329.0, 329.627557, true, 21, 1);  // E belongs to F# minor.
    ok &= tunes(80.0, 82.40689, false, 3, 1);    // Near the bottom of the tracked range.
    ok &= tunes(264.0, 277.182631, true, 21, 1);  // C resolves to C# in F# minor.
    ok &= tunes(313.0, 329.627557, false, 1, 1, 15, 0xab5);  // Auto with C-major Brain mask.
    ok &= tunes(432.0, 440.0, true, 2, 1, 15, 0, 64);        // Quiet mic input.
    ok &= tunes(313.0, 329.627557, false, 3, 0, 15, 0, 512);  // Aux mixer gain uses nine bits of headroom.
    ok &= tunes(313.0, 329.627557, true, 3, 1, 15, 0, 8192);  // Quiet voice after the mixer.
    ok &= tunes(432.0, 436.266667, true, 2, 1, 8);
    ok &= hard_flattens_vibrato();
    ok &= noise_and_reset(0, 1);
    ok &= noise_and_reset(1, 512);
    i32 dry[2 * block_frames], output[2 * block_frames];
    for (u32 i = 0; i < 2 * block_frames; ++i) dry[i] = output[i] = tone(432, i);
    prepare_input(0, 1, 15, 0);
    process_input(output);
    for (u32 i = 0; i < 2 * block_frames; ++i) ok &= expect(dry[i] == output[i], "Off is undelayed bypass");
    prepare_input(2, 1, 0, 0);
    process_input(output);
    for (u32 i = 0; i < 2 * block_frames; ++i) ok &= expect(dry[i] == output[i], "Zero amount is undelayed bypass");
    return ok ? 0 : 1;
}
