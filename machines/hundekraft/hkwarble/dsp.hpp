#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkwarble {
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
  uint32_t phase[2], write;
  int32_t lp[2], mix, depth;
  int16_t ring[2][2048];
} wb_state;

static const int32_t wb_exp[33] = {
    65536,  66971,  68438,  69936,  71468,  73032,  74632,  76266,  77936,
    79642,  81386,  83169,  84990,  86851,  88752,  90696,  92682,  94711,
    96785,  98905,  101070, 103283, 105545, 107856, 110218, 112631, 115098,
    117618, 120194, 122825, 125515, 128263, 131072};

static const int16_t wb_sin[257] = {
    0,      804,    1608,   2410,   3212,   4011,   4808,   5602,   6393,
    7179,   7962,   8739,   9512,   10278,  11039,  11793,  12539,  13279,
    14010,  14732,  15446,  16151,  16846,  17530,  18204,  18868,  19519,
    20159,  20787,  21403,  22005,  22594,  23170,  23731,  24279,  24811,
    25329,  25832,  26319,  26790,  27245,  27683,  28105,  28510,  28898,
    29268,  29621,  29956,  30273,  30571,  30852,  31113,  31356,  31580,
    31785,  31971,  32137,  32285,  32412,  32521,  32609,  32678,  32728,
    32757,  32767,  32757,  32728,  32678,  32609,  32521,  32412,  32285,
    32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,  30273,
    29956,  29621,  29268,  28898,  28510,  28105,  27683,  27245,  26790,
    26319,  25832,  25329,  24811,  24279,  23731,  23170,  22594,  22005,
    21403,  20787,  20159,  19519,  18868,  18204,  17530,  16846,  16151,
    15446,  14732,  14010,  13279,  12539,  11793,  11039,  10278,  9512,
    8739,   7962,   7179,   6393,   5602,   4808,   4011,   3212,   2410,
    1608,   804,    0,      -804,   -1608,  -2410,  -3212,  -4011,  -4808,
    -5602,  -6393,  -7179,  -7962,  -8739,  -9512,  -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204,
    -18868, -19519, -20159, -20787, -21403, -22005, -22594, -23170, -23731,
    -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105,
    -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113,
    -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609,
    -32678, -32728, -32757, -32767, -32757, -32728, -32678, -32609, -32521,
    -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852,
    -30571, -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
    -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731, -23170,
    -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530,
    -16846, -16151, -15446, -14732, -14010, -13279, -12539, -11793, -11039,
    -10278, -9512,  -8739,  -7962,  -7179,  -6393,  -5602,  -4808,  -4011,
    -3212,  -2410,  -1608,  -804,   0};

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
  volatile uint32_t warble_blocks[3];
  void defaults() {}
  int32_t wb_sine(uint32_t p) {
    unsigned i = p >> 24;
    int32_t a = wb_sin[i];
    return a + (((wb_sin[i + 1] - a) * (int32_t)((p >> 8) & 65535)) >> 16);
  }

  uint32_t wb_rate(int32_t q) {
    uint32_t x = (uint32_t)((q * 15655) >> 15), i = (x >> 6) & 31;
    uint32_t e =
        (uint32_t)(wb_exp[i] +
                   (((wb_exp[i + 1] - wb_exp[i]) * (int32_t)(x & 63)) >> 6));
    e <<= x >> 11;
    return ((e >> 6) * 4870u) >> 10;
  }

  void wb_init(wb_state *s) {
    s->phase[0] = 0;
    s->phase[1] = 0x40000000u;
    s->write = 0;
    s->lp[0] = s->lp[1] = 0;
    s->mix = -1;
    s->depth = 0;
    for (unsigned c = 0; c < 2; c++)
      for (unsigned i = 0; i < 2048; i++)
        s->ring[c][i] = 0;
  }

  int16_t clip(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  void wb_process(wb_state *s, int16_t *l, int16_t *r, const int32_t q[4]) {
    uint32_t inc = wb_rate(q[0]), inc2 = inc + ((inc * 21u) >> 4);
    int32_t dep = ((q[1] * q[1]) >> 15) * 25 >> 3, mix = q[3];
    int32_t tone = 3000 + (((q[2] * q[2]) >> 15) * 29767 >> 15);
    if (s->mix < 0) {
      s->mix = mix;
      s->depth = dep;
    }
    int32_t dm = mix - s->mix, dd = dep - s->depth;
    for (unsigned c = 0; c < (r ? 2u : 1u); c++) {
      int16_t *buf = c ? r : l;
      uint32_t a = s->phase[0] + (c ? 0x40000000u : 0),
               b = s->phase[1] + (c ? 0x40000000u : 0), w = s->write;
      int32_t m = s->mix * 64, d = s->depth * 64, lp = s->lp[c];
      for (unsigned n = 0; n < 64; n++) {
        int32_t dry = buf[n];
        s->ring[c][w] = buf[n];
        m += dm;
        d += dd;
        int32_t wave = (3 * wb_sine(a) + wb_sine(b)) >> 2;
        uint32_t delay = (uint32_t)((wave * (d >> 7) >> 14) + (d >> 6) + 56320);
        unsigned rd = (w - (delay >> 8)) & (2048 - 1);
        int32_t v = s->ring[c][rd];
        v += ((s->ring[c][(rd - 1) & (2048 - 1)] - v) *
              (int32_t)(delay & 255)) >>
             8;
        lp += (((v - lp) >> 1) * tone) >> 14;
        buf[n] = clip(dry + ((((lp - dry) >> 1) * (m >> 6)) >> 14));
        w = (w + 1) & (2048 - 1);
        a += inc;
        b += inc2;
      }
      s->lp[c] = lp;
    }
    s->write = (s->write + 64) & (2048 - 1);
    s->phase[0] += inc * 64;
    s->phase[1] += inc2 * 64;
    s->mix = mix;
    s->depth = dep;
  }

  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  wb_state *state(int pos) { return (wb_state *)scratch; }

  void init(int pos) {
    wb_state *s = state(pos);
    if (s)
      wb_init(s);
  }

  void render_stereo(int pos, int16_t *l, int16_t *r, const int16_t *p) {
    wb_state *s = state(pos);
    if (!s)
      return;
    int32_t q[4];
    for (int i = 0; i < 4; i++)
      q[i] = ((int32_t)(p)[i]);
    wb_process(s, l, r, q);
    warble_blocks[pos % 3]++;
  }

  void render(int pos, int16_t *b, const int16_t *p) {
    render_stereo(pos, b, 0, p);
  }
};
} // namespace port
} // namespace hkwarble
