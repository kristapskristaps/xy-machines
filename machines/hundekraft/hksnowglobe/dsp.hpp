#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hksnowglobe {
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

enum { AP0 = 71, AP1 = 53, AP2 = 189, AP3 = 138 };

static const int16_t line_len[4] = {3557, 3617, 3491, 3422};

typedef struct {
  int32_t ap[4][2 * 256 + 16 / 2];
  int16_t line[5][2 * 2048 + 16];
  int32_t lp[4], dc[4];
  int32_t wet;
  uint32_t w;
  uint32_t e, eblocks;
} state_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  state_t st[1];
  volatile uint32_t snowglobe_blocks;
  volatile int32_t snowglobe_level;
  volatile int32_t snowglobe_freeze;
  volatile int16_t snowglobe_knobs[1][4];
  volatile uint32_t snowglobe_kblocks[1];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline void touch_lines(state_t *s, uint32_t api, uint32_t wi) {
    for (int k = 0; k < 4; k++)
      for (int i = 0; i < 32; i += 8) {
        (void)((const volatile int32_t *)s->ap[k])[api + i];
        (void)((const volatile int32_t *)s->ap[k])[api + 256 + i];
      }
    for (int k = 0; k < 5; k++)
      for (int i = 0; i < 32; i += 16)
        (void)((const volatile int16_t *)s->line[k])[wi + i];
    for (int i = 0; i < 32; i += 16)
      (void)((const volatile int16_t *)s->line[4])[wi + 2048 + i];
  }

  void fx_reset(int inst) {
    state_t *s = &st[inst];
    uint8_t *p = reinterpret_cast<uint8_t *>(s);
    for (unsigned i = 0; i < sizeof(state_t); i++)
      p[i] = 0;
  }

  inline int32_t sg_sat(int32_t v) {
    v = v > 32767 ? 32767 : v;
    return v < -32768 ? -32768 : v;
  }

  void fx_process(int inst, int16_t *io, int n, const int16_t *knobs) {
    for (int i = 0; i < 4; i++)
      snowglobe_knobs[inst][i] = knobs[i];
    snowglobe_kblocks[inst]++;
    state_t *s = &st[inst];
    const int32_t k0 = knobs[0] < 0 ? 0 : knobs[0],
                  k1 = knobs[1] < 0 ? 0 : knobs[1];
    const int32_t k2 = knobs[2] < 0 ? 0 : knobs[2],
                  k3 = knobs[3] < 0 ? 0 : knobs[3];
    const int freeze = fx_page[0][3] != 0;
    const int32_t fb = freeze ? 32767 : 19661 + ((k0 * 12616) >> 15);
    const int32_t damp = freeze ? 32767 : 27525;
    const int32_t shimmer = freeze ? 0 : (k1 * 16384) >> 15;
    const int32_t fb_o = (fb * (32768 - shimmer)) >> 15,
                  fb_up = (fb * shimmer) >> 15;
    const int32_t size = 13107 + ((k2 * 19660) >> 15);
    const int32_t in_gain = freeze ? 0 : 9830;
    uint32_t e = s->e;
    int32_t lp0 = s->lp[0], lp1 = s->lp[1], lp2 = s->lp[2], lp3 = s->lp[3];
    int32_t dc0 = s->dc[0], dc1 = s->dc[1], dc2 = s->dc[2], dc3 = s->dc[3];
    const int32_t len0 = (line_len[0] * size) >> 16,
                  len1 = (line_len[1] * size) >> 16;
    const int32_t len2 = (line_len[2] * size) >> 16,
                  len3 = (line_len[3] * size) >> 16;
    int32_t sc[6][32];
    uint32_t w = s->w;
    int32_t wprev = s->wet;
    const int steps = n >> 1;
    for (int c = 0; c < steps;) {
      const int cl = (int)(32 - (w & (32 - 1))) < steps - c
                         ? (int)(32 - (w & (32 - 1)))
                         : steps - c;
      const uint32_t api = w & (256 - 1), wi = w & (2048 - 1);
      touch_lines(s, api, wi);
      int16_t *const io2 = io + 2 * c;
      int32_t *a0 = &s->ap[0][api], *a1 = &s->ap[1][api], *a2 = &s->ap[2][api],
              *a3 = &s->ap[3][api];
      for (int j = 0; j < cl; j++) {
        int32_t d = ((io2[2 * j] + io2[2 * j + 1]) * in_gain) >> 16;
        do {
          const int32_t dl = (a0)[256 - (AP0)];
          const int32_t v = (d) - (dl >> 1);
          (a0)[0] = (a0)[256] = v;
          (d) = dl + (v >> 1);
        } while (0);
        do {
          const int32_t dl = (a1)[256 - (AP1)];
          const int32_t v = (d) - (dl >> 1);
          (a1)[0] = (a1)[256] = v;
          (d) = dl + (v >> 1);
        } while (0);
        do {
          const int32_t dl = (a2)[256 - (AP2)];
          const int32_t v = (d) - (dl >> 1);
          (a2)[0] = (a2)[256] = v;
          (d) = dl + (v >> 1);
        } while (0);
        do {
          const int32_t dl = (a3)[256 - (AP3)];
          const int32_t v = (d) - (dl >> 1);
          (a3)[0] = (a3)[256] = v;
          (d) = dl + (v >> 1);
        } while (0);
        a0++;
        a1++;
        a2++;
        a3++;
        sc[4][j] = d;
      }
      do {
        const int16_t *r = &s->line[0][(wi - (uint32_t)(len0)) & (2048 - 1)];
        int32_t *const y = sc[0];
        for (int j = 0; j < cl; j++) {
          (lp0) += ((r[j] - (lp0)) * damp) >> 15;
          (dc0) += ((lp0) * 32 - (dc0)) >> 8;
          y[j] = (lp0) - ((dc0) >> 5);
        }
      } while (0);
      do {
        const int16_t *r = &s->line[1][(wi - (uint32_t)(len1)) & (2048 - 1)];
        int32_t *const y = sc[1];
        for (int j = 0; j < cl; j++) {
          (lp1) += ((r[j] - (lp1)) * damp) >> 15;
          (dc1) += ((lp1) * 32 - (dc1)) >> 8;
          y[j] = (lp1) - ((dc1) >> 5);
        }
      } while (0);
      do {
        const int16_t *r = &s->line[2][(wi - (uint32_t)(len2)) & (2048 - 1)];
        int32_t *const y = sc[2];
        for (int j = 0; j < cl; j++) {
          (lp2) += ((r[j] - (lp2)) * damp) >> 15;
          (dc2) += ((lp2) * 32 - (dc2)) >> 8;
          y[j] = (lp2) - ((dc2) >> 5);
        }
      } while (0);
      do {
        const int16_t *r = &s->line[3][(wi - (uint32_t)(len3)) & (2048 - 1)];
        int32_t *const y = sc[3];
        for (int j = 0; j < cl; j++) {
          (lp3) += ((r[j] - (lp3)) * damp) >> 15;
          (dc3) += ((lp3) * 32 - (dc3)) >> 8;
          y[j] = (lp3) - ((dc3) >> 5);
        }
      } while (0);
      const uint32_t ta = w & (1024 - 1), tb = (w + 1024 / 2) & (1024 - 1);
      int32_t ga = ta < 1024 / 2 ? (int32_t)ta : (int32_t)(1024 - ta);
      const int32_t dga = ta < 1024 / 2 ? 1 : -1;
      const int16_t *ra = &s->line[4][wi + 2048 - 1024 + ta],
                    *rb = &s->line[4][wi + 2048 - 1024 + tb];
      int16_t *l0 = &s->line[0][wi];
      for (int j = 0; j < cl; j++) {
        const int32_t y0 = sc[0][j], y1 = sc[1][j], y2 = sc[2][j],
                      y3 = sc[3][j], d = sc[4][j];
        const int32_t a = y0 + y1, b = y0 - y1, cc = y2 + y3, e2 = y2 - y3;
        const int32_t o0 = (a + cc) >> 1, o1 = (b + e2) >> 1,
                      o2 = (a - cc) >> 1, o3 = (b - e2) >> 1;
        const int16_t t16 = (int16_t)sg_sat(o0);
        l0[4 * (2 * 2048 + 16)] = l0[4 * (2 * 2048 + 16) + 2048] = t16;
        const int32_t sb = rb[0], up = sb + (((ra[0] - sb) * ga) >> 9);
        ra += 2;
        rb += 2;
        ga += dga;
        l0[0] = (int16_t)sg_sat(d + ((o0 * fb_o + up * fb_up) >> 15));
        l0[1 * (2 * 2048 + 16)] = (int16_t)sg_sat(d + ((o1 * fb) >> 15));
        l0[2 * (2 * 2048 + 16)] =
            (int16_t)sg_sat(d + ((o2 * fb_o + up * fb_up) >> 15));
        l0[3 * (2 * 2048 + 16)] = (int16_t)sg_sat(d + ((o3 * fb) >> 15));
        l0++;
        sc[5][j] = t16;
      }
      if (wi < 32)
        for (int k = 0; k < 4; k++)
          for (uint32_t i = wi; i < wi + (uint32_t)cl && i < 32; i++)
            s->line[k][2048 + i] = s->line[k][i];
      for (int j = 0; j < cl; j++) {
        const int32_t wk = (sc[5][j] * k3) >> 15;
        io2[2 * j] = (int16_t)sg_sat(((wprev + wk) >> 1));
        io2[2 * j + 1] = (int16_t)sg_sat(wk);
        wprev = wk;
        const int32_t q = sc[5][j] >> 3;
        e += (uint32_t)(q * q);
      }
      w += (uint32_t)cl;
      c += cl;
    }
    s->lp[0] = lp0;
    s->lp[1] = lp1;
    s->lp[2] = lp2;
    s->lp[3] = lp3;
    s->dc[0] = dc0;
    s->dc[1] = dc1;
    s->dc[2] = dc2;
    s->dc[3] = dc3;
    s->w = w;
    s->wet = wprev;
    s->e = e;
    s->eblocks += (uint32_t)steps;
    if (inst != 2) {
      snowglobe_blocks++;
      snowglobe_freeze = freeze;
    }
    if (s->eblocks >= 128) {
      const uint32_t nst = s->eblocks;
      uint32_t m = (nst == 128 ? e >> 7 : e / nst) << 6;
      s->e = s->eblocks = 0;
      if (inst != 2) {
        int32_t r = 0;
        for (uint32_t bit = 1u << 30; bit; bit >>= 2) {
          if (m >= (uint32_t)r + bit) {
            m -= (uint32_t)r + bit;
            r = (r >> 1) + (int32_t)bit;
          } else
            r >>= 1;
        }
        snowglobe_level = r;
      }
    }
  }
};
} // namespace port
} // namespace hksnowglobe
