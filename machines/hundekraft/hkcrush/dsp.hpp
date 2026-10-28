#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkcrush {
namespace port {
typedef int int32_t;

typedef unsigned int uint32_t;

typedef short int16_t;

typedef unsigned short uint16_t;

typedef signed char int8_t;

typedef unsigned char uint8_t;

typedef Wide int64_t;

typedef Wide uint64_t;

typedef long unsigned int uintptr_t;

typedef struct {
  uint32_t phase;
  int32_t held[2], gain, mix, peak;
} crush_state;

static const int32_t exp2_table[33] = {
    65536,  66971,  68438,  69936,  71468,  73032,  74632,  76266,  77936,
    79642,  81386,  83169,  84990,  86851,  88752,  90696,  92682,  94711,
    96785,  98905,  101070, 103283, 105545, 107856, 110218, 112631, 115098,
    117618, 120194, 122825, 125515, 128263, 131072};

typedef struct {
  int32_t len, next, step, tape_pos;
  uint8_t follows_tape;
} op1_beat_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  crush_state state[3];
  volatile uint32_t crush_blocks[3];
  void defaults() {}
  int32_t crush_exp2(int32_t x) {
    unsigned i = ((uint32_t)x >> 6) & 31, f = (uint32_t)x & 63;
    uint32_t y = exp2_table[i] + ((exp2_table[i + 1] - exp2_table[i]) * f >> 6);
    int oct = x >> 11;
    return (int32_t)(oct < 0 ? y >> -oct : y << oct);
  }

  int crush_bits(int q) { return 1 + ((q * 15 + 16384) >> 15); }

  void crush_reset(crush_state *s) {
    s->phase = 65535;
    s->held[0] = s->held[1] = 0;
    s->gain = -1;
    s->mix = s->peak = 0;
  }

  int32_t sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : x;
  }

  int32_t driven(int32_t x, int32_t gain, int bits) {
    x = (x * gain) >> 8;
    int sign = x < 0, mag = sign ? -x : x;
    if (mag > 24576) {
      int t = mag - 24576;
      if (t > 16384)
        t = 16384;
      mag = 24576 + t - ((t * t) >> 15);
      if (mag > 32767)
        mag = 32767;
      x = sign ? -mag : mag;
    }
    if (bits < 16) {
      int shift = 16 - bits, unit = 1 << shift;
      x = sat16(((x + (unit >> 1)) >> shift) * unit);
    }
    return x;
  }

  void crush_process(crush_state *s, int16_t *left, int16_t *right,
                     const int16_t q[4]) {
    int bits = crush_bits(q[0]);
    uint32_t increment = (uint32_t)crush_exp2(((q[1] - 32767) * 14336) >> 15);
    int gain = crush_exp2(q[2] >> 2) >> 8, mix = q[3];
    if (s->gain < 0) {
      s->gain = gain;
      s->mix = mix;
    }
    int dg = gain - s->gain, dm = mix - s->mix, peak = 0;
    uint32_t end_phase = s->phase;
    for (int channel = 0; channel < (right ? 2 : 1); channel++) {
      int16_t *buffer = channel ? right : left;
      uint32_t phase = s->phase;
      int held = s->held[channel];
      int gain64 = s->gain * 64, mix64 = s->mix * 64;
      for (int n = 0; n < 64; n++) {
        int dry = buffer[n];
        gain64 += dg;
        mix64 += dm;
        phase += increment;
        if (phase >= 65536) {
          phase -= 65536;
          held = driven(dry, gain64 >> 6, bits);
        }
        int magnitude = held < 0 ? -held : held;
        if (magnitude > peak)
          peak = magnitude;
        int difference = (held - dry) >> 1;
        buffer[n] = (int16_t)sat16(dry + ((difference * (mix64 >> 6)) >> 14));
      }
      s->held[channel] = held;
      end_phase = phase;
    }
    s->phase = end_phase;
    s->gain = gain;
    s->mix = mix;
    s->peak = peak;
  }

  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  void init(int pos) { crush_reset(&state[pos % 3]); }

  void render(int pos, int16_t *left, int16_t *right, const int16_t *p) {
    int16_t q[4];
    for (int i = 0; i < 4; i++)
      q[i] = (int16_t)((int32_t)(p)[i]);
    crush_process(&state[pos % 3], left, right, q);
    crush_blocks[pos % 3]++;
  }

  void mono(int pos, int16_t *buffer, const int16_t *p) {
    render(pos, buffer, 0, p);
  }

  void stereo(int pos, int16_t *left, int16_t *right, const int16_t *p) {
    render(pos, left, right, p);
  }
};
} // namespace port
} // namespace hkcrush
