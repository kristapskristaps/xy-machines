#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkreverb {
namespace port {
typedef opxy::i32 int32_t;

typedef opxy::u32 uint32_t;

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

static const int16_t rv_len[10] = {207,  295,  440,  726,  3007,
                                   3707, 6203, 3480, 3028, 8698};

typedef struct {
  int16_t *buf;
  int32_t wp, base[10];
  int32_t lp1, lp2;
  int32_t lfo_iir[2], lfo_y0[2], lfo_y1[2], lfo[2];
} reverb_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  reverb_t rv[3];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t c17(int32_t x) {
    return x < -65535 ? -65535 : (x > 65535 ? 65535 : x);
  }

  inline int32_t m15(int32_t a, int32_t b) {
    int32_t x = c17(a) * b;
    return x >= 0 ? x >> 15 : -(-x >> 15);
  }

  inline int16_t sat(int32_t x) {
    return (int16_t)(x < -32768 ? -32768 : (x > 32767 ? 32767 : x));
  }

  inline int32_t interp(reverb_t *r, int i, int32_t off, int32_t amp,
                        int32_t lfo) {
    int32_t pos = (off << 15) + amp * lfo;
    int32_t k = pos >> 15, f = pos & 32767;
    int32_t a = r->buf[(r->wp + r->base[i] + (k)) & (32768 - 1)],
            b = r->buf[(r->wp + r->base[i] + (k + 1)) & (32768 - 1)];
    return a + m15(b - a, f);
  }

  inline int32_t allpass(reverb_t *r, int i, int32_t acc, int32_t g) {
    int32_t prev = r->buf[(r->wp + r->base[i] + (rv_len[i] - 1)) & (32768 - 1)];
    acc += m15(prev, g);
    r->buf[(r->wp + r->base[i] + (0)) & (32768 - 1)] = sat(acc);
    return m15(acc, -g) + prev;
  }

  void reverb_init(int pos) {
    reverb_t *r = &rv[pos % 3];
    r->buf = (int16_t *)scratch;
    if (!r->buf)
      return;
    for (int i = 0, b = 0; i < 10; i++) {
      r->base[i] = b;
      b += rv_len[i] + 1;
    }
    r->wp = 0;
    r->lp1 = r->lp2 = 0;
    for (int k = 0; k < 2; k++) {
      int64_t f = ((int64_t)(k ? 3 : 5) * 32 << 29) / (Wide(10) * 44100);
      int64_t fr = (Wide(1) << 27) - f;
      r->lfo_iir[k] = (int32_t)((16 * fr * ((Wide(1) << 29) - 2 * fr)) >> 29);
      r->lfo_y1[k] = r->lfo_iir[k] / 4;
      r->lfo_y0[k] = 1 << 28;
      r->lfo[k] = (r->lfo_y1[k] + (1 << 28)) >> 14;
    }
  }

  void reverb_render(int pos, int16_t *buf, const int16_t *p) {
    reverb_t *r = &rv[pos % 3];
    if (!r->buf)
      return;
    int32_t krt = 11469 + (((int32_t)(p)[0]) * 20644 >> 15);
    int32_t amount = ((int32_t)(p)[3]);
    int32_t kap = 16384 + (((int32_t)(p)[2]) * 8192 >> 15);
    int32_t klp = 9830 + (((int32_t)(p)[1]) * 21299 >> 15);
    const int32_t gain = 6554;
    int32_t peak = 0;
    for (int t = 0; t < 64; t++) {
      if (--r->wp < 0)
        r->wp += 32768;
      if ((r->wp & 31) == 0)
        for (int k = 0; k < 2; k++) {
          int32_t y = r->lfo_y0[k];
          r->lfo_y0[k] =
              (int32_t)(((int64_t)r->lfo_iir[k] * y) >> 29) - r->lfo_y1[k];
          r->lfo_y1[k] = y;
          r->lfo[k] = (y + (1 << 28)) >> 14;
        }
      int32_t in = buf[t], acc, apout, wet;
      r->buf[(r->wp + r->base[0] + (138)) & (32768 - 1)] =
          sat(interp(r, 0, 14, 110, r->lfo[0]));
      acc = m15(in, gain);
      acc = allpass(r, 0, acc, kap);
      acc = allpass(r, 1, acc, kap);
      acc = allpass(r, 2, acc, kap);
      acc = allpass(r, 3, acc, kap);
      apout = acc;
      acc = apout + m15(interp(r, 9, 8550, 138, r->lfo[1]), krt);
      r->lp1 += m15(acc - r->lp1, klp);
      acc = r->lp1;
      acc = allpass(r, 4, acc, -kap);
      acc = allpass(r, 5, acc, kap);
      r->buf[(r->wp + r->base[6] + (0)) & (32768 - 1)] = sat(acc);
      wet = acc;
      acc = apout +
            m15(r->buf[(r->wp + r->base[6] + (rv_len[6] - 1)) & (32768 - 1)],
                krt);
      r->lp2 += m15(acc - r->lp2, klp);
      acc = r->lp2;
      acc = allpass(r, 7, acc, kap);
      acc = allpass(r, 8, acc, -kap);
      r->buf[(r->wp + r->base[9] + (0)) & (32768 - 1)] = sat(acc);
      wet = (wet + acc) * 5;
      int32_t o = (((wet >> 3) * amount) >> 12);
      int32_t ao = o < 0 ? -o : o;
      if (ao > 24576) {
        int32_t d = ao - 24576;
        ao = d < 16384 ? 24576 + (d >> 2) : 28672 + ((d - 16384) >> 4);
        o = sat(o < 0 ? -ao : ao);
      }
      buf[t] = (int16_t)o;
      if (o < 0)
        o = -o;
      if (o > peak)
        peak = o;
    }
    volatile int16_t *m = (&meter);
    int32_t mv = (*m * 0xfe00) >> 16;
    *m = (int16_t)(peak > mv ? peak : mv);
  }
};
} // namespace port
} // namespace hkreverb
