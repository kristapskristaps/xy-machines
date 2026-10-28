#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkplate {
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
  uint32_t head, phase;
  int32_t input, damp[2], feedback[2], pre, dry, wet, peak;
  int16_t ring[65536];
} plate_state;

static const int16_t plate_sin[257] = {
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

static_assert(sizeof(plate_state) <= 176460, "Plate scratch overflow");

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  volatile uint32_t plate_blocks[3];
  volatile int32_t plate_peak[3];
  void defaults() {}
  int32_t sat(int32_t x) { return x > 32767 ? 32767 : x < -32768 ? -32768 : x; }

  int32_t mul(int32_t a, int32_t b) { return (a * b) >> 15; }

  int32_t plate_sine(uint32_t p) {
    unsigned i = p >> 24;
    int32_t a = plate_sin[i];
    return a + (((plate_sin[i + 1] - a) * (int32_t)((p >> 8) & 65535)) >> 16);
  }

  void plate_init(plate_state *s) {
    s->head = s->phase = 0;
    s->input = s->damp[0] = s->damp[1] = 0;
    s->feedback[0] = s->feedback[1] = s->peak = 0;
    s->pre = s->dry = -1;
    s->wet = 0;
    for (unsigned i = 0; i < 65536; i++)
      s->ring[i] = 0;
  }

  int32_t read_at(plate_state *s, unsigned h, unsigned off) {
    return s->ring[(h + off) & 65535];
  }

  void write_at(plate_state *s, unsigned h, unsigned off, int32_t v) {
    s->ring[(h + off) & 65535] = (int16_t)v;
  }

  int32_t ap(plate_state *s, unsigned h, unsigned wr, unsigned rd, int32_t x,
             int32_t g) {
    int32_t d = read_at(s, h, rd), v = sat(x - mul(d, g));
    write_at(s, h, wr, v);
    return sat(d + mul(v, g));
  }

  int32_t modap(plate_state *s, unsigned h, unsigned wr, unsigned delay,
                int32_t x) {
    unsigned off = wr + (delay >> 8);
    int32_t d = read_at(s, h, off);
    d += ((read_at(s, h, off + 1) - d) * (int32_t)(delay & 255)) >> 8;
    int32_t v = sat(x + mul(d, 22938));
    write_at(s, h, wr, v);
    return sat(d - mul(v, 22938));
  }

  void plate_process(plate_state *s, int16_t *l, int16_t *r,
                     const int32_t q[4]) {
    int32_t dec = mul(q[0], 27853), gain = dec + 1638, diff = dec + 6553;
    if (diff < 8192)
      diff = 8192;
    if (diff > 16384)
      diff = 16384;
    int32_t damp = 32767 - mul(32767 - q[1], 26214),
            input = 32767 - (((32767 - q[1]) * 26214) >> 16);
    int32_t pre = mul(mul(q[2], q[2]), 8191);
    if (s->pre < 0)
      s->pre = pre;
    else if (pre > s->pre + 16)
      s->pre += 16;
    else if (pre < s->pre - 16)
      s->pre -= 16;
    else
      s->pre = pre;
    int32_t dry = q[3] <= 16384 ? 32768 : (32768 - q[3]) * 2,
            wet = q[3] <= 16384 ? q[3] * 2 : 32768;
    if (s->dry < 0) {
      s->dry = dry;
      s->wet = wet;
    }
    int32_t dr = s->dry * 64, we = s->wet * 64, dd = dry - s->dry,
            dw = wet - s->wet;
    unsigned dl = (unsigned)(((plate_sine(s->phase) * 24) >> 7) + 254976);
    unsigned rr =
        (unsigned)(((plate_sine(s->phase + 0x40000000u) * 24) >> 7) + 344320);
    s->peak = 0;
    for (unsigned n = 0; n < 64; n++) {
      unsigned h = s->head;
      int32_t a = l[n], b = r ? r[n] : a;
      write_at(s, h, 0, (a + b) >> 1);
      s->input +=
          (((read_at(s, h, (unsigned)s->pre) - s->input) >> 1) * input) >> 14;
      int32_t x = ap(s, h, 8192, 8402, s->input, 24576);
      x = ap(s, h, 8403, 8562, x, 24576);
      x = ap(s, h, 8563, 9125, x, 20480);
      x = ap(s, h, 9126, 9536, x, 20480);
      int32_t y = modap(s, h, 9537, dl, sat(x + mul(s->feedback[1], gain)));
      write_at(s, h, 10559, y);
      s->damp[0] += (((read_at(s, h, 17157) - s->damp[0]) >> 1) * damp) >> 14;
      y = ap(s, h, 17158, 19825, mul(s->damp[0], gain), diff);
      write_at(s, h, 19826, y);
      y = modap(s, h, 25339, rr, sat(x + mul(s->feedback[0], gain)));
      write_at(s, h, 26710, y);
      s->damp[1] += (((read_at(s, h, 32959) - s->damp[1]) >> 1) * damp) >> 14;
      y = ap(s, h, 32960, 36896, mul(s->damp[1], gain), diff);
      write_at(s, h, 36897, y);
      s->feedback[0] = read_at(s, h, 25338);
      s->feedback[1] = read_at(s, h, 41584);
      int32_t wl = read_at(s, h, 27104) + read_at(s, h, 31117) -
                   read_at(s, h, 35795) + read_at(s, h, 39855) -
                   read_at(s, h, 13508) - read_at(s, h, 17435) -
                   read_at(s, h, 21406);
      int32_t wr = read_at(s, h, 11082) + read_at(s, h, 15934) -
                   read_at(s, h, 18978) + read_at(s, h, 23787) -
                   read_at(s, h, 29838) - read_at(s, h, 33456) -
                   read_at(s, h, 37076);
      wl = sat(((wl >> 3) * 19661) >> 12);
      wr = sat(((wr >> 3) * 19661) >> 12);
      int32_t peak = wl < 0 ? -wl : wl;
      if (peak > s->peak)
        s->peak = peak;
      dr += dd;
      we += dw;
      l[n] = (int16_t)sat(
          (a * (dr >> 6) + (r ? wl : ((wl + wr) >> 1)) * (we >> 6)) >> 15);
      if (r)
        r[n] = (int16_t)sat((b * (dr >> 6) + wr * (we >> 6)) >> 15);
      s->head = (h - 1) & 65535;
    }
    s->dry = dry;
    s->wet = wet;
    s->phase += 0x60ae00u;
  }

  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  plate_state *state(int p) { return (plate_state *)scratch; }

  void init(int p) {
    plate_state *s = state(p);
    if (s)
      plate_init(s);
  }

  void stereo(int p, int16_t *l, int16_t *r, const int16_t *params) {
    plate_state *s = state(p);
    if (!s)
      return;
    int32_t q[4];
    for (int i = 0; i < 4; i++)
      q[i] = ((int32_t)(params)[i]);
    plate_process(s, l, r, q);
    plate_peak[p] = s->peak;
    plate_blocks[p]++;
  }

  void mono(int p, int16_t *b, const int16_t *q) { stereo(p, b, 0, q); }
};
} // namespace port
} // namespace hkplate
