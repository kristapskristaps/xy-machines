#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hktape {
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
  uint32_t w;
  uint32_t filled;
  int32_t d;
  uint32_t wow, flt;
  int32_t lp;
  int32_t rng;
} tape_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  tape_t st[4];
  volatile int16_t in_level[3];
  volatile int16_t fb_level[3];
  volatile uint32_t samples[3];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t sine(uint32_t ph) {
    return sine_table[((int32_t)ph >> 19) + 4096];
  }

  inline int32_t softclip(int32_t x) {
    if (x > 32767)
      x = 32767;
    else if (x < -32767)
      x = -32767;
    int32_t x3 = (((x * x) >> 15) * x) >> 15;
    return x - ((x3 * 10923) >> 15);
  }

  void tape_init(int pos) {
    tape_t *t = &st[pos % 3];
    t->w = 0;
    t->filled = 0;
    t->lp = 0;
    t->wow = 0;
    t->flt = 0x40000000u;
    t->d = -1;
  }

  void tape_render(int pos, int16_t *buf, const int16_t *p) {
    tape_t *t = &st[pos % 3];
    int16_t *ring = (int16_t *)scratch;
    if (!ring)
      return;
    int32_t k0 = ((int32_t)(p)[0]), k1 = ((int32_t)(p)[1]);
    int32_t k2 = ((int32_t)(p)[2]), k3 = ((int32_t)(p)[3]);
    int32_t target = 1323 + (int32_t)(((uint32_t)k2 * (64000 - 1323)) >> 15);
    int32_t fb = (k1 * 36045) >> 15;
    int32_t wowd = (k3 * k3) >> 15;
    int32_t age = k0;
    int32_t lpk = 32767 - ((age * 29000) >> 15);
    int32_t drive = 32768 + (age << 1);
    if (t->d < 0)
      t->d = target << 10;
    int32_t peak = 0, inpk = 0, fbpk = 0;
    for (int n = 0; n < 64; n++) {
      t->d += ((target << 10) - t->d) >> 12;
      t->wow += 58436u;
      t->flt += 681788u;
      int32_t mod = (((sine(t->wow) * wowd) >> 15) * 175 >> 5) +
                    (((sine(t->flt) * wowd) >> 15) * 4 >> 5);
      int32_t dd = t->d + mod;
      if (dd < (2 << 10))
        dd = 2 << 10;
      uint32_t di = (uint32_t)dd >> 10, fr = (uint32_t)dd & 0x3ff;
      int32_t wet = 0;
      if (di + 1 < t->filled) {
        uint32_t r = (t->w - di) & (65536u - 1);
        int32_t a = ring[r], b = ring[(r - 1) & (65536u - 1)];
        wet = a + (((b - a) * (int32_t)fr) >> 10);
      }
      t->lp += ((wet - t->lp) * lpk) >> 15;
      int32_t loop = softclip((t->lp * drive) >> 15);
      int32_t in = buf[n];
      if (in > inpk)
        inpk = in;
      else if (-in > inpk)
        inpk = -in;
      int32_t fbs = (loop * fb) >> 15;
      if (fbs > fbpk)
        fbpk = fbs;
      else if (-fbs > fbpk)
        fbpk = -fbs;
      ring[t->w] = (int16_t)softclip(in + fbs);
      t->w = (t->w + 1) & (65536u - 1);
      if (t->filled < 65536u)
        t->filled++;
      int32_t o = ((t->lp * 23000) >> 15);
      o = op1_sat16(o);
      buf[n] = (int16_t)o;
      if (o < 0)
        o = -o;
      if (o > peak)
        peak = o;
    }
    volatile int16_t *m = (&meter);
    int32_t mv = (*m * 0xfe00) >> 16;
    *m = (int16_t)(peak > mv ? peak : mv);
    int32_t il = (in_level[pos % 3] * 0xfc00) >> 16;
    in_level[pos % 3] = (int16_t)(inpk > il ? inpk : il);
    int32_t fl = (fb_level[pos % 3] * 0xfc00) >> 16;
    fb_level[pos % 3] = (int16_t)(fbpk > fl ? fbpk : fl);
    samples[pos % 3] += 64;
  }
};
} // namespace port
} // namespace hktape
