#pragma once
#include <opxy/audio.hpp>
#include <opxy/q31.hpp>

namespace opxy { namespace extensions { namespace autotune {

class Dynamics {
    // Gains use Q12 so the block detector can divide with 32-bit arithmetic.
    static constexpr i32 unity = 4096;
    // The stock Aux mixer uses nine bits of headroom. This knee is at -12 dB
    // relative to full-scale input, before the track mixes into the outputs.
    static constexpr i32 knee = 256;
    i32 low_[2], gain_, gain_target_, ess_, ess_target_;
    bool compress_, deess_;

    static i32 magnitude(i32 sample) {
        const i32 reduced = sample >> 12;
        return reduced < 0 ? -reduced : reduced;
    }

    static i32 toward(i32 current, i32 target, i32 blocks) {
        const i32 difference = target - current;
        return difference > -blocks && difference < blocks ? target : current + difference / blocks;
    }

public:
    void reset() {
        low_[0] = low_[1] = ess_ = ess_target_ = 0;
        gain_ = gain_target_ = unity;
    }

    void prepare(bool compress, bool deess) {
        if (!compress) gain_target_ = unity;
        if (!deess) ess_target_ = 0;
        compress_ = compress;
        deess_ = deess;
    }

    void process(i32* stereo) {
        if (!compress_ && !deess_ && gain_ == unity && !ess_) return;
        const i32 gain_next = toward(gain_, gain_target_, gain_target_ < gain_ ? 2 : 32);
        const i32 ess_next = toward(ess_, ess_target_, ess_target_ > ess_ ? 2 : 12);
        const i32 gain_step = (gain_next - gain_) * (1 << 13);
        const i32 ess_step = (ess_next - ess_) * (1 << 13);
        // Unsigned representation permits unity (0x80000000).
        u32 gain = static_cast<u32>(gain_) << 19;
        i32 ess = ess_ << 19;
        i32 peak = 0, high_peak = 0;
        const bool filter = deess_ || ess_ || ess_next;
        for (u32 frame = 0; frame < block_frames; ++frame) {
            gain += static_cast<u32>(gain_step);
            ess += ess_step;
            for (u32 channel = 0; channel < 2; ++channel) {
                i32 sample = *stereo;
                const i32 level = magnitude(sample);
                if (level > peak) peak = level;
                if (filter) {
                    const i32 high = q31::half(sample) - q31::half(low_[channel]);
                    low_[channel] += high;
                    const i32 high_level = magnitude(high);
                    if (high_level > high_peak) high_peak = high_level;
                    if (ess) sample = q31::add(sample, -q31::multiply(high, ess));
                }
                if (gain < 0x80000000u) sample = q31::multiply(sample, static_cast<i32>(gain));
                *stereo++ = sample;
            }
        }
        gain_ = gain_next;
        ess_ = ess_next;
        gain_target_ = compress_ && peak > knee ? (knee + (peak - knee) / 3) * unity / peak : unity;
        // Require a strong high band as well as level, so vowels and quiet hiss
        // do not trigger it. Both channels share one detector and gain.
        ess_target_ = deess_ && peak > 8 && high_peak * 3 > peak ? 2662 : 0;
    }
};

}}}
