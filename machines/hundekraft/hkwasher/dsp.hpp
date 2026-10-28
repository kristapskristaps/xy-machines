#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkwasher {
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

typedef struct {
  int16_t buf[2048];
  uint32_t w, ph;
} state_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  state_t st[1];
  volatile uint32_t washer_phase;
  volatile uint32_t washer_blocks;
  volatile int32_t washer_level;
  volatile int16_t washer_knobs[1][4];
  volatile uint32_t washer_kblocks[1];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  void fx_reset(int inst) {
    state_t *s = &st[inst];
    for (int i = 0; i < 2048; i++)
      s->buf[i] = 0;
    s->w = 0;
    s->ph = 0;
  }

  int32_t exp2q(int32_t x) {
    const int32_t e = x >> 12, f = x & 4095;
    return (4096 + ((f * (2839 + ((f * 1257) >> 12))) >> 12)) << e;
  }

  inline int32_t tap(const int16_t *buf, uint32_t w, int32_t d8) {
    const uint32_t pos = (w << 8) - (uint32_t)d8;
    const uint32_t i = (pos >> 8) & (2048 - 1);
    const int32_t f = (int32_t)(pos & 255);
    const int32_t a = buf[i], b = buf[(i + 1) & (2048 - 1)];
    return a + (((b - a) * f) >> 8);
  }

  inline int32_t div3(int32_t x) {
    const uint32_t a = (uint32_t)(x < 0 ? -x : x);
    const int32_t q =
        (int32_t)(((a >> 8) * 43691u + (((a & 255) * 43691u) >> 8)) >> 9);
    return x < 0 ? -q : q;
  }

  void fx_process(int inst, int16_t *io, int n, const int16_t *knobs) {
    for (int i = 0; i < 4; i++)
      washer_knobs[inst][i] = knobs[i];
    washer_kblocks[inst]++;
    state_t *s = &st[inst];
    const int32_t k0 = knobs[0] < 0 ? 0 : knobs[0],
                  k1 = knobs[1] < 0 ? 0 : knobs[1];
    const int32_t k2 = knobs[2] < 0 ? 0 : knobs[2];
    const uint32_t inc =
        (uint32_t)((4870 * (int64_t)exp2q((k0 * 29900) >> 15)) >> 12);
    const int32_t base = (529 << 8) - ((k2 * 485) >> 7);
    int32_t depth = (k1 * (176 << 8)) >> 15;
    if (depth > base - (2 << 8))
      depth = base - (2 << 8);
    const int32_t fb = (k2 * 27853) >> 15;
    int32_t wet_sum = 0;
    uint32_t w = s->w, ph = s->ph;
    for (int i = 0; i < n; i++) {
      int32_t wet = 0, first = 0;
      for (int t = 0; t < 3; t++) {
        const uint32_t p = ph + (uint32_t)t * 0x55555555u;
        const int32_t tri = (int32_t)(p >> 16) - 32768;
        const int32_t lfo = (tri < 0 ? -tri : tri) * 2 - 32768;
        const int32_t y = tap(s->buf, w, base + ((depth * lfo) >> 15));
        if (!t)
          first = y;
        wet += y;
      }
      wet = div3(wet);
      const int32_t x = io[i];
      s->buf[w & (2048 - 1)] = op1_sat16(x + ((first * fb) >> 15));
      w++;
      ph += inc;
      io[i] = op1_sat16(wet);
      wet_sum += wet < 0 ? -wet : wet;
    }
    s->w = w;
    s->ph = ph;
    if (inst != 2) {
      washer_phase = ph;
      washer_blocks++;
      washer_level = wet_sum / (n ? n : 1);
    }
  }
};
} // namespace port
} // namespace hkwasher
