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

    struct Peak {
        i32 low, high;
        void observe(i32 left, i32 right) {
            const i32 minimum = left < right ? left : right;
            const i32 maximum = left > right ? left : right;
            if (minimum < low) low = minimum;
            if (maximum > high) high = maximum;
        }
        i32 level() const {
            // Shifting the extrema gives the same peak as shifting each sample.
            const i32 negative = -(low >> 12);
            const i32 positive = high >> 12;
            return negative > positive ? negative : positive;
        }
    };

    static i32 toward(i32 current, i32 target, i32 blocks) {
        const i32 difference = target - current;
        return difference > -blocks && difference < blocks ? target : current + difference / blocks;
    }

    template <bool Filter>
    void process_block(i32* stereo, i32 gain_next, i32 ess_next) {
        const i32 gain_step = (gain_next - gain_) * (1 << 13);
        const i32 ess_step = (ess_next - ess_) * (1 << 13);
        // Unsigned representation permits unity (0x80000000).
        u32 gain = static_cast<u32>(gain_) << 19;
        i32 ess = ess_ << 19;
        i32 low_left = low_[0], low_right = low_[1];
        Peak input{}, highs{};
        for (u32 frame = 0; frame < block_frames; ++frame) {
            gain += static_cast<u32>(gain_step);
            ess += ess_step;
            i32 left = stereo[0], right = stereo[1];
            input.observe(left, right);
            if (Filter) {
                const i32 high_left = q31::half(left) - q31::half(low_left);
                const i32 high_right = q31::half(right) - q31::half(low_right);
                low_left += high_left;
                low_right += high_right;
                highs.observe(high_left, high_right);
                if (ess) {
                    left = q31::add(left, -q31::multiply(high_left, ess));
                    right = q31::add(right, -q31::multiply(high_right, ess));
                }
            }
            if (gain < 0x80000000u) {
                left = q31::multiply(left, static_cast<i32>(gain));
                right = q31::multiply(right, static_cast<i32>(gain));
            }
            *stereo++ = left;
            *stereo++ = right;
        }
        if (Filter) {
            low_[0] = low_left;
            low_[1] = low_right;
        }
        const i32 peak = input.level();
        gain_target_ = compress_ && peak > knee ? (knee + (peak - knee) / 3) * unity / peak : unity;
        // Require a strong high band as well as level, so vowels and quiet hiss
        // do not trigger it. Both channels share one detector and gain.
        ess_target_ = Filter && deess_ && peak > 8 && highs.level() * 3 > peak ? 2662 : 0;
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
        if (deess_ || ess_ || ess_next) process_block<true>(stereo, gain_next, ess_next);
        else process_block<false>(stereo, gain_next, ess_next);
        gain_ = gain_next;
        ess_ = ess_next;
    }
};

}}}
