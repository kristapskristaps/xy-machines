#pragma once
#include <opxy/audio.hpp>
#include <opxy/q15.hpp>
#include <opxy/q31.hpp>

namespace opxy { namespace extensions { namespace autotune {

// Run a decimated YIN difference search in small, fixed pieces across audio
// blocks. Pick the first confident trough to avoid favoring octave multiples.
class PitchTracker {
    static constexpr u32 window = 96, minimum_lag = 6, maximum_lag = 81;
    static constexpr u32 snapshot_size = window + maximum_lag;
    static constexpr u32 ring_mask = 255, lags_per_block = 6;
    static constexpr u32 decimation = 8, points_per_block = block_frames / decimation;
    static constexpr u32 range_mask = (ring_mask + 1) / points_per_block - 1;
    static_assert(snapshot_size % points_per_block == 1, "Snapshot has one leading point and whole blocks");

    struct Range {
        i32 low, high;
        void include(i32 minimum, i32 maximum) {
            if (minimum < low) low = minimum;
            if (maximum > high) high = maximum;
        }
    };
    struct StereoRange { Range left, right; };

    i32 left_[ring_mask + 1], right_[ring_mask + 1];
    StereoRange ranges_[range_mask + 1];
    alignas(4) i16 snapshot_[snapshot_size + 1], shifted_snapshot_[snapshot_size + 1];
    i32 period_;
    u32 position_, filled_, lag_;
    u32 difference_sum_, previous_sum_, older_sum_;
    i32 previous_difference_, older_difference_;

    bool snapshot() {
        if (filled_ < snapshot_size) return false;
        const u32 first = position_ - snapshot_size;
        const i32 left = left_[first & ring_mask], right = right_[first & ring_mask];
        StereoRange bounds = {{left, left}, {right, right}};
        const u32 first_block = (first + 1) / points_per_block;
        for (u32 i = 0; i < snapshot_size / points_per_block; ++i) {
            const StereoRange& range = ranges_[(first_block + i) & range_mask];
            bounds.left.include(range.left.low, range.left.high);
            bounds.right.include(range.right.low, range.right.high);
        }
        const i32 left_range = bounds.left.high - bounds.left.low;
        const i32 right_range = bounds.right.high - bounds.right.low;
        const bool use_left = left_range >= right_range;
        const i32 range = use_left ? left_range : right_range;
        // Aux mixer headroom makes the scratch signal much quieter than full-scale Q31.
        if (range < 1024) return false;
        const i32* channel = use_left ? left_ : right_;
        const Range& chosen = use_left ? bounds.left : bounds.right;
        const i32 center = (chosen.high + chosen.low) / 2;
        u32 shift = 0;
        while ((range >> shift) > 4095) ++shift;
        const i32 gain = (511 << 8) / (((range >> shift) + 1) / 2);
        // Both alignments let the distance loop read two adjacent samples at a time.
        for (u32 i = 0; i < snapshot_size; ++i) {
            const i16 sample = static_cast<i16>((((channel[(first + i) & ring_mask] - center) >> shift) * gain) >> 8);
            snapshot_[i] = shifted_snapshot_[i + 1] = sample;
        }
        return true;
    }

    bool confident(u32 threshold) const {
        // Evaluate normalized YIN scores only at raw local minima. Compare the
        // same truncated quotients by multiplication where the divisor is nonzero.
        const u32 average = previous_sum_ / (lag_ - 1);
        const u32 numerator = (static_cast<u32>(previous_difference_) >> 5) << 10;
        if (!average || numerator >= threshold * average) return false;
        const u32 older_average = older_sum_ / (lag_ - 2);
        const u32 older_numerator = (static_cast<u32>(older_difference_) >> 5) << 10;
        return !older_average || numerator / average * older_average <= older_numerator;
    }

    static i32 refined_period(u32 lag, i32 before, i32 at, i32 after) {
        // Parabolic interpolation in full-rate Q12 samples.
        i32 slope = before - after;
        i32 curve = before + after - 2 * at;
        while (slope < -65535 || slope > 65535) {
            slope /= 2;
            curve /= 2;
        }
        const i32 offset = curve ? (slope * 16384) / curve : 0;
        return static_cast<i32>(lag) * 32768 + offset;
    }

public:
    void reset() {
        period_ = 0;
        position_ = filled_ = lag_ = 0;
    }

    void push_block(const i32* stereo) {
        StereoRange bounds = {{q31::maximum, -q31::maximum}, {q31::maximum, -q31::maximum}};
        for (u32 i = 0; i < points_per_block; ++i) {
            i32 left = 0, right = 0;
            // The sum of eight reduced samples is a Q29 average.
            for (u32 j = 0; j < decimation; ++j) {
                left += stereo[0] >> 5;
                right += stereo[1] >> 5;
                stereo += 2;
            }
            left_[position_ + i] = left;
            right_[position_ + i] = right;
            bounds.left.include(left, left);
            bounds.right.include(right, right);
        }
        ranges_[position_ / points_per_block] = bounds;
        position_ = (position_ + points_per_block) & ring_mask;
        if (filled_ < snapshot_size) filled_ += points_per_block;
    }

    i32 period() const { return period_; }

    void advance(bool hard) {
        if (!lag_) {
            if (!snapshot()) {
                period_ = 0;
                return;
            }
            lag_ = 1;
            difference_sum_ = older_sum_ = previous_sum_ = 0;
            older_difference_ = previous_difference_ = 0;
        }

        const u32 budget = hard ? lags_per_block * 2 : lags_per_block;
        const u32 threshold = hard ? 245 : 180;
        for (u32 i = 0; i < budget && lag_ <= maximum_lag; ++i, ++lag_) {
            const u32 offset = maximum_lag - lag_;
            const i16* aligned = (offset & 1) ? shifted_snapshot_ + offset + 1 : snapshot_ + offset;
            const i32 difference = static_cast<i32>(q15::squared_distance<window>(
                shifted_snapshot_ + maximum_lag + 1, aligned));
            difference_sum_ += static_cast<u32>(difference) >> 5;
            if (lag_ > minimum_lag && previous_difference_ <= older_difference_ &&
                    previous_difference_ < difference && confident(threshold)) {
                period_ = refined_period(lag_ - 1, older_difference_, previous_difference_, difference);
                lag_ = 0;
                return;
            }
            older_sum_ = previous_sum_;
            previous_sum_ = difference_sum_;
            older_difference_ = previous_difference_;
            previous_difference_ = difference;
        }
        if (lag_ > maximum_lag) {
            period_ = 0;
            lag_ = 0;
        }
    }
};

class Tuner {
    struct Stereo { i32 left, right; };
    static constexpr u32 ring_mask = 2047;
    static constexpr u32 delay = 672;  // 15.24 ms; longer than the largest tracked period.
    static constexpr i32 fade_frames = 384;

    PitchTracker tracker_;
    Stereo history_[ring_mask + 2];
    u32 cursor_, filled_, phase_;
    i32 period_, speed_, wet_;
    u32 pitch_classes_, amount_;
    bool hard_;

    i32 correction(i32 period) const {
        // Full-rate Q12 periods of MIDI 36 through 84, referenced to A4 = 440 Hz.
        static constexpr i32 notes[] = {
        2761712, 2606709, 2460405, 2322314, 2191972, 2068946, 1952825,
        1843221, 1739769, 1642124, 1549958, 1462966, 1380856, 1303354,
        1230203, 1161157, 1095986, 1034473, 976413, 921611, 869885,
        821062, 774979, 731483, 690428, 651677, 615101, 580578,
        547993, 517237, 488206, 460805, 434942, 410531, 387490,
        365741, 345214, 325839, 307551, 290289, 273997, 258618,
        244103, 230403, 217471, 205265, 193745, 182871, 172607,
        };
        u32 low = 0, high = sizeof(notes) / sizeof(*notes) - 1;
        while (high - low > 1) {
            const u32 mid = (low + high) / 2;
            if (period >= notes[mid]) high = mid;
            else low = mid;
        }
        // MIDI 36 is C. Find the allowed note on each side of the input.
        const u32 last = sizeof(notes) / sizeof(*notes) - 1;
        while (!(pitch_classes_ & (1u << (low % 12))) && low > 0) --low;
        while (!(pitch_classes_ & (1u << (high % 12))) && high < last) ++high;
        const bool has_low = (pitch_classes_ & (1u << (low % 12))) != 0;
        const bool has_high = (pitch_classes_ & (1u << (high % 12))) != 0;
        // Approximate the geometric midpoint, the boundary in cents.
        const i32 span = (notes[low] - notes[high]) / 16;
        const i32 center = (notes[low] + notes[high]) / 2;
        const i32 midpoint = center - (span * span) / (center / 32);
        const i32 target = has_low && (!has_high || period >= midpoint) ? notes[low] : notes[high];
        return (period - target) * 4096 / (target >> 4);  // Shift ratio minus one, Q16.
    }

    static Stereo blend(Stereo a, Stereo b, i32 weight) {
        if (!weight) return a;
        if (weight == q31::maximum) return b;
        const i32 inverse = q31::maximum - weight;
        return {q31::add(q31::multiply(a.left, inverse), q31::multiply(b.left, weight)),
                q31::add(q31::multiply(a.right, inverse), q31::multiply(b.right, weight))};
    }

    Stereo read(u32 cursor, i32 delay_q16) const {
        const u32 newer = cursor - (static_cast<u32>(delay_q16) >> 16);
        const i32 weight = (delay_q16 & 65535) << 15;
        const Stereo* samples = history_ + (newer & ring_mask);
        return blend(samples[1], samples[0], weight);
    }

    void render_shifted(i32* stereo, u32 count, i32 period_step) {
        i32 period = period_;
        u32 phase = phase_;
        for (u32 frame = 0; frame < count; ++frame) {
            // The two reads stay one period apart and exchange roles at the phase wrap.
            period += period_step;
            const i32 weight = static_cast<i32>(phase >> 1);
            const i32 tap = (delay << 16) + q31::multiply(weight, period);
            const u32 cursor = cursor_ + frame;
            const Stereo output = blend(read(cursor, tap), read(cursor, tap - period), weight);
            *stereo++ = output.left;
            *stereo++ = output.right;
            phase -= static_cast<u32>(speed_);
        }
        phase_ = phase;
        period_ = period;
    }

public:
    void reset() {
        tracker_.reset();
        cursor_ = filled_ = phase_ = 0;
        period_ = speed_ = wet_ = 0;
    }

    void prepare(u32 mode, u32 retune, u32 amount, u32 automatic_mask) {
        constexpr u32 chromatic = 0xfff;
        constexpr u32 major = 0xab5, minor = 0x5ad;
        const bool active = mode && amount;
        if (!active && amount_) reset();
        amount_ = active ? amount : 0;
        hard_ = retune == 1;
        if (mode >= 3 && mode <= 26) {
            const u32 root = (mode - 3) % 12;
            const u32 intervals = mode <= 14 ? major : minor;
            pitch_classes_ = ((intervals << root) | (intervals >> (12 - root))) & chromatic;
        } else {
            const u32 mask = automatic_mask & chromatic;
            pitch_classes_ = mode == 1 && mask ? mask : chromatic;
        }
    }

    void process(i32* stereo) {
        if (!amount_) return;
        tracker_.push_block(stereo);
        const i32 detected = tracker_.period();
        const i32 desired_period = detected << 4;
        const i32 distance = desired_period - period_;
        const bool nearby = period_ && distance >= -period_ / 8 && distance <= period_ / 8;
        if (!wet_ && desired_period && !nearby) {
            period_ = desired_period;
            phase_ = 0;
            speed_ = 0;
        }
        const bool voiced = detected && (nearby || !wet_);
        const i32 period_step = voiced ? (desired_period - period_) / 256 : 0;
        const i32 shift = voiced ? correction(detected) * static_cast<i32>(amount_) / 15 : 0;
        const i32 speed = voiced ? shift * static_cast<i32>(0x80000000u / (period_ >> 8)) / 128 : 0;
        speed_ = hard_ ? speed : speed_ + (speed - speed_) / 4;
        // The cursor is block-aligned. All taps look backward and the ring is
        // longer than the furthest tap plus a block, so writing the block first
        // preserves the same history for every read.
        Stereo* write = history_ + cursor_ + 1;
        for (u32 frame = 0; frame < block_frames; ++frame) {
            write[frame] = {stereo[frame * 2], stereo[frame * 2 + 1]};
        }
        const u32 next = (cursor_ + block_frames) & ring_mask;
        if (!next) history_[0] = history_[ring_mask + 1];

        const u32 fade_step = hard_ ? 4 : 1;
        const i32 to_edge = voiced ? fade_frames - wet_ : wet_;
        const u32 fade_length = to_edge ? (static_cast<u32>(to_edge) - 1) / fade_step : 0;
        const u32 mixed = fade_length < block_frames ? fade_length : block_frames;
        const u32 shifted = voiced ? block_frames : mixed;
        if (shifted) render_shifted(stereo, shifted, period_step);

        const i32 wet_step = voiced ? static_cast<i32>(fade_step) : -static_cast<i32>(fade_step);
        i32 wet = wet_;
        for (u32 frame = 0; frame < mixed; ++frame) {
            wet += wet_step;
            const Stereo dry = history_[((cursor_ + frame - delay) & ring_mask) + 1];
            const Stereo tuned = {stereo[frame * 2], stereo[frame * 2 + 1]};
            const Stereo output = blend(dry, tuned, wet * (q31::maximum / fade_frames));
            stereo[frame * 2] = output.left;
            stereo[frame * 2 + 1] = output.right;
        }
        if (!voiced) {
            const u32 dry_start = delay - filled_;
            for (u32 frame = shifted; frame < block_frames; ++frame) {
                const Stereo dry = frame < dry_start ? Stereo{0, 0} : history_[((cursor_ + frame - delay) & ring_mask) + 1];
                stereo[frame * 2] = dry.left;
                stereo[frame * 2 + 1] = dry.right;
            }
        }
        wet_ = mixed == block_frames ? wet : voiced ? fade_frames : 0;
        cursor_ = next;
        filled_ += block_frames;
        if (filled_ > delay) filled_ = delay;
        tracker_.advance(hard_);
    }
};

}}}
