#pragma once
#include <opxy/q31.hpp>
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkelg {
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
  int32_t len, next, step, tape_pos;
  uint8_t follows_tape;
} op1_beat_t;

static const int16_t elg_level_trim[85] = {
    5786, 5786, 5786, 3298, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
    2048, 2048, 2048, 2048, 2048, 2048, 5786, 5786, 5786, 5786, 3319,
    2251, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
    2048, 5786, 5786, 5786, 5786, 3676, 2814, 2048, 2048, 2048, 2048,
    2048, 2048, 2048, 2048, 2048, 2048, 2048, 5786, 5786, 5786, 5786,
    4098, 3669, 2398, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
    2048, 2048, 5786, 5786, 5786, 5786, 4034, 3718, 2584, 2258, 2048,
    2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
};

typedef struct {
  uint32_t ph[3], lfo;
  int32_t low, band;
  int32_t drift, drift_v;
  uint32_t rnd;
  int32_t control[5];
  int control_ready;
} voice_t;

static const int32_t ratio[5] = {4096, 6144, 8192, 12288, 16384};

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  voice_t voices[1];
  volatile uint32_t elg_blocks;
  volatile int32_t elg_level, elg_lfo;
  volatile int16_t elg_knobs[4];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  int elg_interval(int32_t value) {
    return ((unsigned)(value < 0 ? 0 : value) >> 9) % 5u;
  }

  int32_t exp2q(int32_t x) {
    if (x < 0)
      x = 0;
    const int32_t e = x >> 12, f = x & 4095;
    return (4096 + ((f * (2839 + ((f * 1257) >> 12))) >> 12)) << e;
  }

  inline uint32_t mul_u32_s(uint32_t u, int32_t m, int s) {
    const int32_t hi = (int32_t)(u >> 16) * m, lo = (int32_t)(u & 0xFFFF) * m;
    return (uint32_t)(hi * (1 << (16 - s)) + (lo >> s));
  }

  inline int32_t mul_q15(int32_t a, int32_t b) {
    // Filter frequency and gain stay in [0, 32767]. Scaling that operand
    // makes the SDK's Q31 multiply exactly the original Q15 product.
    return opxy::q31::multiply(a, b * 65536);
  }

  inline int32_t div3(int32_t x) {
    const uint32_t a = (uint32_t)(x < 0 ? -x : x);
    // The sum of three signed 16-bit saws has magnitude at most 98304.
    const int32_t q = opxy::q31::multiply(a, 715827883);
    return x < 0 ? -q : q;
  }

  uint32_t lfo_increment(int32_t motion) {
    const uint32_t rate = exp2q((motion * 24800) >> 15);
    // Split at the final shift: the low product fits in 31 bits.
    return (rate >> 12) * 373000u + (((rate & 4095u) * 373000u) >> 12);
  }

  inline int32_t sine_lfo(uint32_t ph) {
    const int32_t x = (int32_t)(ph >> 16) - 32768;
    const int32_t ax = x < 0 ? -x : x;
    const int32_t y = (x * (32768 - ax)) >> 13;
    return y > 32767 ? 32767 : y;
  }

  void engine_note_on(void *self, const int16_t *knobs) {
    const int vi = 0;
    voice_t *v = &voices[vi];
    (void)knobs;
    v->control_ready = 0;
    v->rnd = v->rnd * 1664525u + 1013904223u + (uint32_t)vi * 0x9E3779B9u;
    v->ph[0] = 0;
    v->ph[1] = v->rnd;
    v->ph[2] = v->rnd << 7;
    v->lfo = v->rnd << 3;
    v->low = v->band = 0;
  }

  void engine_note_off(void *self, const int16_t *knobs) {
    (void)self;
    (void)knobs;
  }

  void engine_render(void *self, int16_t *out, const int16_t *knobs) {
    for (int i = 0; i < 4; i++)
      elg_knobs[i] = knobs[i];
    const int vi = 0;
    voice_t *v = &voices[vi];
    int vel = 127;
    if (vel < 1 || vel > 127)
      vel = 100;
    const int32_t k0 = knobs[0] < 0 ? 0 : knobs[0],
                  k1 = knobs[1] < 0 ? 0 : knobs[1];
    const int32_t k2 = knobs[2] < 0 ? 0 : knobs[2],
                  k3 = knobs[3] < 0 ? 0 : knobs[3];
    v->lfo += lfo_increment(k3);
    const int32_t lfo = sine_lfo(v->lfo);
    v->rnd = v->rnd * 1664525u + 1013904223u;
    v->drift_v += ((int32_t)(v->rnd >> 20) - 2048) - (v->drift >> 6);
    v->drift_v -= v->drift_v >> 4;
    v->drift += v->drift_v >> 4;
    if (v->drift > 32767)
      v->drift = 32767;
    if (v->drift < -32767)
      v->drift = -32767;
    const int32_t det = (k1 * 393) >> 15;
    const int32_t dr = ((v->drift * k3) >> 15) * 115 >> 15;
    int step = elg_interval(k0);
    if (step > 4)
      step = 4;
    int32_t cx = (k2 * 29500) >> 15;
    cx += ((lfo * k3) >> 15) * 6144 >> 15;
    int32_t f = (187 * exp2q(cx)) >> 12;
    if (f > 27000)
      f = 27000;
    if (f < 120)
      f = 120;
    const int32_t q = 20000;
    const int32_t drive = 4096 + ((k2 * 12288) >> 15);
    const int32_t swell =
        32767 -
        (((((lfo + 32767) >> 1) * k3) >> 15) * 9830 >> 15);
    const int32_t trim_pos = k2 * 16, trim_index = trim_pos >> 15;
    const int32_t trim_a = elg_level_trim[step * 17 + trim_index];
    const int32_t trim =
        trim_a + (((elg_level_trim[step * 17 + trim_index + 1] - trim_a) *
                   (trim_pos & 32767)) >>
                  15);
    const int32_t voice_gain =
        (swell * ((11000 * vel) >> 7)) >> 15;
    const int32_t gain = (voice_gain * trim) >> 12;
    const int32_t target[5] = {det, dr, f, drive, gain};
    int32_t control[5], delta[5];
    for (int j = 0; j < 5; j++) {
      const int32_t previous = v->control_ready ? v->control[j] : target[j];
      control[j] = previous * 128;
      delta[j] = target[j] - previous;
      v->control[j] = target[j];
    }
    v->control_ready = 1;
    uint32_t p0 = v->ph[0], p1 = v->ph[1], p2 = v->ph[2];
    int32_t low = v->low, band = v->band, peak = 0;
    for (int i = 0; i < 128; i++) {
      const int32_t det = control[0] >> 7, dr = control[1] >> 7,
                    f = control[2] >> 7;
      const int32_t drive = control[3] >> 7, gain = control[4] >> 7;
      const uint32_t inc = pitch_input[i];
      const uint32_t base = inc + mul_u32_s(inc, dr, 15);
      p0 += base;
      p1 += base + mul_u32_s(base, det, 15);
      p2 += ((base >> 1) * (uint32_t)(ratio[step] >> 11) +
             ((base & 1) * (uint32_t)(ratio[step] >> 11) >> 1)) -
            mul_u32_s(base, det, 16);
      int32_t x =
          div3(((int32_t)(p0 >> 16) - 32768) + ((int32_t)(p1 >> 16) - 32768) +
               ((int32_t)(p2 >> 16) - 32768));
      x = (x * drive) >> 12;
      if (x > 32767)
        x = 32767;
      if (x < -32767)
        x = -32767;
      x = (x * (98304 - ((x * x) >> 15))) >> 16;
      low += (f * band) >> 15;
      const int32_t high = x - low - ((q * band) >> 15);
      band += mul_q15(high, f);
      if (band > 65535)
        band = 65535;
      if (band < -65535)
        band = -65535;
      const int32_t y = op1_sat16(mul_q15(low, gain));
      out[i] = (int16_t)y;
      control[0] += delta[0];
      control[1] += delta[1];
      control[2] += delta[2];
      control[3] += delta[3];
      control[4] += delta[4];
      peak = y > peak ? y : -y > peak ? -y : peak;
    }
    v->ph[0] = p0;
    v->ph[1] = p1;
    v->ph[2] = p2;
    v->low = low;
    v->band = band;
    elg_blocks++;
    elg_level = peak;
    elg_lfo = (lfo * k3) >> 15;
  }
};
} // namespace port
} // namespace hkelg
