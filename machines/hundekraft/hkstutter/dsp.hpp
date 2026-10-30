#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkstutter {
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

enum { M_REPEAT, M_BRAKE, M_REVERSE, M_GATE };

static const uint8_t DIVS[6] = {1, 2, 3, 4, 6, 8};

static const uint8_t REPS[6] = {2, 3, 4, 6, 8, 16};

typedef struct {
  uint32_t w;
  int32_t bp;
  int32_t pos, len, sub;
  int32_t ps, part;
  uint32_t start;
  uint8_t stut, mode, faded;
  int32_t fade, finc, duty;
  int32_t rd, rate;
  int32_t gin;
  uint8_t latch, frozen;
  int32_t rec;
  uint32_t rng;
  int32_t slice;
  uint32_t hist;
  uint8_t ndiv, reps;
} st_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  st_t st[3];
  volatile uint32_t sx_blocks[3];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  unsigned sx_range(int32_t q, unsigned count) {
    unsigned v = (unsigned)(q < 0 ? 0 : q);
    unsigned n = v * count >> 15;
    return n < count ? n : count - 1;
  }

  unsigned sx_select(int32_t q, unsigned count) {
    return ((unsigned)(q < 0 ? 0 : q) >> 9) % count;
  }

  inline int32_t q15(const int16_t *p, int i) { return ((int32_t)(p)[i]); }

  inline uint32_t lcg(uint32_t *s) { return *s = *s * 1664525u + 1013904223u; }

  void sx_init(int pos) {
    pos %= 3;
    st_t *s = &st[pos];
    s->w = 0;
    s->bp = -1;
    s->pos = 0;
    s->len = 0;
    s->sub = 1;
    s->start = 0;
    s->stut = 0;
    s->faded = 0;
    s->gin = 0;
    s->latch = fx_page[pos][3];
    s->frozen = 0;
    s->rng = 0x2545F491u + (uint32_t)pos * 0x9E3779B9u;
    s->slice = 0;
    s->hist = 0;
    int16_t *buf = (int16_t *)scratch;
    if (buf)
      for (int i = 0; i < 65536; i++)
        buf[i] = 0;
  }

  int grid(st_t *s, int div, int32_t *slen) {
    op1_beat_t c;
    (*(&c) = beat);
    int32_t len = c.len, p, step;
    const int moving = len >= (256 << 14) && c.step > 0 && c.step <= 0x400000 &&
                       c.next >= 0 && c.next <= len;
    if (len < (256 << 14))
      len = 22050 << 14;
    if (moving) {
      p = c.next;
      step = c.step;
    } else {
      p = s->bp < 0 || s->bp > len ? len : s->bp;
      step = 0x100000;
    }
    const int32_t np = p - step;
    s->bp = np < 0 ? np + len : np;
    const int32_t sq = step >> 14;
    const int32_t bl = sq > 0 && sq != 64 ? len / sq * 64 : len >> 14;
    *slen = bl / div;
    const int32_t q = len / div;
    for (int j = div - 1; j >= 0; j--) {
      const int32_t t = p - j * q;
      if (t < 0 || t >= step)
        continue;
      s->slice = j == 0 ? 0 : div - j;
      return step == 0x100000 ? t >> 14 : (int)(((int64_t)t * 64) / step);
    }
    return -1;
  }

  void slice_start(st_t *s, int32_t slen, const int16_t *p, int pos) {
    const int32_t chance = q15(p, 3);
    const int32_t shape = q15(p, 2);
    const int32_t w5 = p[5] < 0 ? 0 : p[5];
    const int reps = REPS[sx_select(w5, 6)];
    const int frz = fx_page[pos][3] != 0;
    if (slen > 65536 - 64)
      slen = 65536 - 64;
    if (slen < 64)
      slen = 64;
    s->len = slen;
    s->pos = 0;
    s->ps = 0;
    s->part = 0;
    s->mode = (uint8_t)(sx_range(q15(p, 1), 4));
    const uint8_t was = s->stut;
    if (frz) {
      s->stut = 1;
      if (!s->frozen) {
        s->frozen = 1;
        s->start = s->w;
        s->sub = slen / reps;
        s->rec = s->sub;
      }
    } else {
      s->frozen = 0;
      s->stut = (uint8_t)((int32_t)((lcg(&s->rng) >> 17) & 0x7FFF) < chance);
      s->start = s->w;
      s->sub = slen / reps;
    }
    if (s->sub < 32)
      s->sub = 32;
    int32_t f = 88 + ((shape * 441) >> 15);
    if (f > s->sub / 4)
      f = s->sub / 4;
    s->fade = f < 1 ? 1 : f;
    s->finc = 32768 / s->fade;
    s->duty =
        (int32_t)(((int64_t)s->sub * (3277 + ((shape * 26214) >> 15))) >> 15);
    s->rd = 0;
    s->rate = 65536;
    s->gin = was && !s->stut ? s->fade : 0;
    s->faded = was && s->stut && s->mode != M_GATE;
    s->hist = (s->hist << 1) | s->stut;
    s->reps = (uint8_t)reps;
  }

  void run(st_t *s, int16_t *buf, int16_t *ring, int a, int b) {
    for (int i = a; i < b; i++) {
      const int32_t x = buf[i];
      if (!s->frozen || s->rec > 0) {
        ring[s->w & (65536 - 1)] = (int16_t)x;
        if (s->rec > 0)
          s->rec--;
      }
      int32_t y = x;
      if (s->stut && s->len) {
        const int32_t sub = s->sub, ps = s->ps, part = s->part, f = s->fade,
                      fi = s->finc;
        int32_t e = 32768;
        if (s->mode == M_GATE) {
          if (ps >= s->duty)
            e = ps < s->duty + f ? 32768 - (ps - s->duty) * fi : 0;
          else if (part && ps < f)
            e = ps * fi;
        } else {
          if (part == 0 && !s->frozen) {
            if (s->faded && ps < f)
              e = ps * fi;
          } else {
            int32_t r;
            if (ps == 0 && s->mode == M_BRAKE) {
              s->rate = 65536 - part * 7864;
              if (s->rate < 19661)
                s->rate = 19661;
              s->rd = 0;
            }
            if (s->mode == M_BRAKE) {
              r = s->rd >> 16;
              s->rd += s->rate;
            } else if (s->mode == M_REVERSE)
              r = sub - 1 - ps;
            else
              r = ps;
            y = ring[(s->start + (uint32_t)r) & (65536 - 1)];
            if (ps < f)
              e = ps * fi;
          }
          if (ps >= sub - f) {
            const int32_t d = (sub - ps) * fi;
            if (d < e)
              e = d;
          }
        }
        y = (y * e) >> 15;
      } else if (s->gin > 0) {
        y = (y * (32768 - s->gin * s->finc)) >> 15;
        s->gin--;
      }
      buf[i] = (int16_t)y;
      s->w++;
      s->pos++;
      if (++s->ps >= s->sub) {
        s->ps = 0;
        s->part++;
      }
    }
  }

  void sx_render(int pos, int16_t *buf, const int16_t *p) {
    pos %= 3;
    st_t *s = &st[pos];
    int16_t *ring = (int16_t *)scratch;
    if (!ring)
      return;
    const int div = DIVS[sx_range(q15(p, 0), 6)];
    s->ndiv = (uint8_t)div;
    int32_t slen;
    const int off = grid(s, div, &slen);
    if (s->len == 0)
      slice_start(s, slen, p, pos);
    if (off < 0) {
      run(s, buf, ring, 0, 64);
      if (s->pos >= s->len + s->len / 4)
        slice_start(s, slen, p, pos);
    } else {
      run(s, buf, ring, 0, off);
      slice_start(s, slen, p, pos);
      run(s, buf, ring, off, 64);
    }
    int32_t pk = 0;
    for (int i = 0; i < 64; i++) {
      const int32_t a = buf[i] < 0 ? -buf[i] : buf[i];
      if (a > pk)
        pk = a;
    }
    volatile int16_t *m = (&meter);
    const int32_t mv = (*m * 0xfe00) >> 16;
    *m = (int16_t)(pk > mv ? pk : mv);
    sx_blocks[pos]++;
  }
};
} // namespace port
} // namespace hkstutter
