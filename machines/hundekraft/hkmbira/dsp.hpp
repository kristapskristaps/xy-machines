#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkmbira {
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
  uint32_t ph[5];
  uint32_t pinc[5];
  int fresh;
  int coeff_valid;
  int32_t amp[5];
  int32_t click;
  uint32_t noise;
  int32_t clp, nprev, zlp;
  int32_t blo, bbd, bo, bp;
  int32_t wlo, wbd, wo, wp;
  uint32_t wph;
  int32_t box, buzz, wah, dmix;
  int32_t level;
  int32_t rat[5], dk[5];
  int32_t kt, kring, kdbl, khz, knew;
  uint32_t inc[5], kinc0;
} voice_t;

static const int32_t RATIO[3][4] = {
    {65536, 262144, 642253, 1114112},
    {65536, 131072, 406323, 1022362},
    {65536, 180879, 353894, 585236},
};

static const int32_t LEVEL[3][4] = {
    {32767, 9830, 4915, 1638},
    {32767, 5243, 13107, 4915},
    {32767, 16384, 9830, 6554},
};

typedef struct {
  uint32_t *ph, c;
  int32_t d, a, st;
} part_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  voice_t voices[1];
  volatile int32_t mb_click[1], mb_level[1];
  volatile uint32_t mb_pitch[1], mb_blocks;
  struct {
    int valid;
    int32_t t, ring, dbl, hz, rat[5], dk[5];
  } coeff_cache[16];
  unsigned coeff_next;
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t clampi(int32_t x, int32_t lo, int32_t hi) {
    return x < lo ? lo : x > hi ? hi : x;
  }

  inline int16_t sat16(int32_t x) { return (int16_t)clampi(x, -32768, 32767); }

  inline int32_t knob(const int16_t *p, int i) { return p[i] < 0 ? 0 : p[i]; }

  inline int32_t sine(uint32_t ph) { return sine_table[ph >> 19]; }

  inline uint32_t lcg(uint32_t *s) { return *s = *s * 1664525u + 1013904223u; }

  void tine(int32_t t, int k, int32_t *ratio, int32_t *level) {
    const int a = t < 16384 ? 0 : 1;
    const int32_t f = t < 16384 ? t * 2 : (t - 16384) * 2;
    const int32_t d = RATIO[a + 1][k] - RATIO[a][k];
    *ratio = RATIO[a][k] + (d >> 15) * f + (((d & 32767) * f) >> 15);
    *level = LEVEL[a][k] + (((LEVEL[a + 1][k] - LEVEL[a][k]) * f) >> 15);
  }

  inline int32_t decay(int32_t tau_ms) {
    if (tau_ms < 20)
      tau_ms = 20;
    return 32768 - 95108 / tau_ms;
  }

  void mb_note_on(void *self, const int16_t *p) {
    const int vi = 0;
    voice_t *v = &voices[vi];
    const int32_t strike = knob(p, 0), t = knob(p, 1);
    const int32_t bright = 6554 + ((strike * 26214) >> 15);
    for (int k = 0; k < 4; k++) {
      const int bank = t < 16384 ? 0 : 1;
      const int32_t f = t < 16384 ? t * 2 : (t - 16384) * 2;
      int32_t a =
          LEVEL[bank][k] + (((LEVEL[bank + 1][k] - LEVEL[bank][k]) * f) >> 15);
      for (int j = 0; j < k; j++)
        a = (a * bright) >> 15;
      v->amp[k] = a << 9;
      v->ph[k] = 0;
    }
    v->amp[4] = (v->amp[0] >> 4) * 11;
    v->ph[4] = 0x40000000u;
    v->fresh = 1;
    v->click = 32767;
    v->noise = 0x9E3779B9u ^ ((uint32_t)vi * 0x85EBCA6Bu);
    v->clp = v->nprev = v->zlp = 0;
    v->blo = v->bbd = v->bo = v->bp = v->wlo = v->wbd = v->wo = v->wp = 0;
    v->box = knob(p, 2);
    v->buzz = knob(p, 3);
    v->wah = p[7] < 0 ? 0 : p[7];
    v->dmix = p[6] > 0 ? 11469 + ((p[6] * 4000) >> 15) : 0;
  }

  void mb_note_off(void *self) { (void)self; }

  void coeffs(voice_t *v, int32_t t, int32_t ring, int32_t dbl, int32_t hz) {
    const int32_t hk = hz > 262 ? hz & ~7 : 0;
    if (v->coeff_valid && t == v->kt && ring == v->kring && dbl == v->kdbl &&
        hk == v->khz)
      return;
    v->coeff_valid = 1;
    v->kt = t;
    v->kring = ring;
    v->kdbl = dbl;
    v->khz = hk;
    v->knew = 1;
    for (unsigned i = 0; i < 16; i++) {
      if (coeff_cache[i].valid && coeff_cache[i].t == t &&
          coeff_cache[i].ring == ring && coeff_cache[i].dbl == dbl &&
          coeff_cache[i].hz == hk) {
        for (int k = 0; k < 5; k++) {
          v->rat[k] = coeff_cache[i].rat[k];
          v->dk[k] = coeff_cache[i].dk[k];
        }
        return;
      }
    }
    int32_t tau0 = 1600 + ((ring * 6400) >> 15);
    if (hk)
      tau0 = (tau0 * 262) / hk + tau0 / 4;
    for (int k = 0; k < 5; k++) {
      int32_t r, l;
      tine(t, k < 4 ? k : 0, &r, &l);
      if (k == 4)
        r += (r >> 10) * ((dbl * 7) >> 15);
      v->rat[k] = r;
      const int32_t tau =
          k == 0 || k == 4 ? tau0 : (tau0 * 131072) / (r + 65536) + 40;
      v->dk[k] = decay(tau);
    }
    unsigned slot = coeff_next++ & (16 - 1);
    coeff_cache[slot].t = t;
    coeff_cache[slot].ring = ring;
    coeff_cache[slot].dbl = dbl;
    coeff_cache[slot].hz = hk;
    for (int k = 0; k < 5; k++) {
      coeff_cache[slot].rat[k] = v->rat[k];
      coeff_cache[slot].dk[k] = v->dk[k];
    }
    coeff_cache[slot].valid = 1;
  }

  template<bool First, bool Glide>
  inline void pass2(int32_t *acc, part_t *x, part_t *y) {
    uint32_t pa = *x->ph, ca = x->c, pb = *y->ph, cb = y->c;
    int32_t aa = x->a, ab = y->a;
    const int32_t da = x->d, db = y->d, sa = x->st, sb = y->st;
    for (int i = 0; i < 128; i += 8) {
      const int32_t ga = aa >> 10, gb = ab >> 10;
      for (int j = i; j < i + 8; j++) {
        pa += ca;
        pb += cb;
        if (Glide) {
          ca += (uint32_t)da;
          cb += (uint32_t)db;
        }
        const int32_t s = (sine(pa) * ga + sine(pb) * gb) >> 15;
        acc[j] = First ? s : acc[j] + s;
      }
      aa += sa * 8;
      ab += sb * 8;
    }
    *x->ph = pa;
    *y->ph = pb;
  }

  template<bool First, bool Glide>
  inline void pass1(int32_t *acc, part_t *x) {
    uint32_t pa = *x->ph, ca = x->c;
    int32_t aa = x->a;
    const int32_t da = x->d, sa = x->st;
    for (int i = 0; i < 128; i += 8) {
      const int32_t ga = aa >> 9;
      for (int j = i; j < i + 8; j++) {
        pa += ca;
        if (Glide)
          ca += (uint32_t)da;
        const int32_t s = (sine(pa) * ga) >> 16;
        acc[j] = First ? s : acc[j] + s;
      }
      aa += sa * 8;
    }
    *x->ph = pa;
  }

  void partials(int32_t *acc, part_t *l, int n, int glide) {
    for (int i = 0; i < n; i += 2) {
      const int first = i == 0;
      if (i + 1 < n) {
        if (glide) {
          if (first)
            pass2<true, true>(acc, &l[i], &l[i + 1]);
          else
            pass2<false, true>(acc, &l[i], &l[i + 1]);
        } else {
          if (first)
            pass2<true, false>(acc, &l[i], &l[i + 1]);
          else
            pass2<false, false>(acc, &l[i], &l[i + 1]);
        }
      } else {
        if (glide) {
          if (first)
            pass1<true, true>(acc, &l[i]);
          else
            pass1<false, true>(acc, &l[i]);
        } else {
          if (first)
            pass1<true, false>(acc, &l[i]);
          else
            pass1<false, false>(acc, &l[i]);
        }
      }
    }
  }

  void mb_render(void *self, int16_t *out, const int16_t *p) {
    const int vi = 0;
    voice_t *v = &voices[vi];
    const int32_t strike = knob(p, 0), t = knob(p, 1), box = knob(p, 2),
                  buzz = knob(p, 3);
    const int32_t ring = p[5] < 0 ? 0 : p[5], dbl = p[6] < 0 ? 0 : p[6],
                  wah = p[7] < 0 ? 0 : p[7];
    const uint32_t inc0 = (uint32_t)pitch_input[0];
    coeffs(v, t, ring, dbl, (int32_t)(((inc0 >> 16) * 44100u) >> 16));
    uint32_t inc[5];
    int32_t end[5], step[5], dinc[5];
    if (inc0 != v->kinc0 || v->knew) {
      v->kinc0 = inc0;
      v->knew = 0;
      for (int k = 0; k < 5; k++) {
        const uint64_t f = ((uint64_t)inc0 * (uint32_t)v->rat[k]) >> 16;
        v->inc[k] = f > 0x70000000u ? 0 : (uint32_t)f;
      }
    }
    for (int k = 0; k < 5; k++) {
      inc[k] = v->inc[k];
      if (!inc[k])
        v->amp[k] = 0;
      const int32_t a = v->amp[k];
      const int32_t e =
          (int32_t)((((uint32_t)a >> 8) * (uint32_t)v->dk[k]) >> 7);
      step[k] = (e - a) >> 7;
      end[k] = a + step[k] * 128;
      if (v->fresh)
        v->pinc[k] = inc[k];
      dinc[k] = (int32_t)(inc[k] - v->pinc[k]) >> 7;
    }
    v->fresh = 0;
    const int32_t dmix1 = dbl > 0 ? 11469 + ((dbl * 4000) >> 15) : 0;
    const int32_t dmix0 = v->dmix;
    const int32_t box0 = v->box, dbox = (box - box0) >> 7, buzz0 = v->buzz,
                  dbuzz = (buzz - buzz0) >> 7;
    const int32_t wah0 = v->wah, dwah = (wah - wah0) >> 7;
    const int32_t vg = 32767;
    int32_t acc[128];
    const uint32_t ph0 = v->ph[0], c0 = v->pinc[0];
    part_t live[5];
    int n = 0, glide = 0;
    for (int k = 0; k < 4; k++)
      if (!k || v->amp[k]) {
        live[n++] =
            (part_t){&v->ph[k], v->pinc[k], dinc[k], v->amp[k], step[k]};
        glide |= dinc[k];
      }
    if ((v->amp[4] | end[4]) && (dmix0 | dmix1)) {
      const int32_t a0 = ((v->amp[4] >> 8) * dmix0) >> 7,
                    a1 = ((end[4] >> 8) * dmix1) >> 7;
      live[n++] = (part_t){&v->ph[4], v->pinc[4], dinc[4], a0, (a1 - a0) >> 7};
      glide |= dinc[4];
    } else
      v->ph[4] += v->pinc[4] * 128 + (uint32_t)dinc[4] * (128 * (128 - 1) / 2);
    partials(acc, live, n, glide != 0);
    int32_t click = v->click, clp = v->clp;
    uint32_t noise = v->noise;
    if (click > 8 || clp) {
      const int32_t cf = 32034 + (((32767 - strike) * 586) >> 15);
      const int32_t clp_a = 3000 + ((strike * 24000) >> 15);
      for (int i = 0; i < 128; i++) {
        const int32_t n = (int32_t)lcg(&noise) >> 16;
        if (click > 8) {
          clp += ((((n * click) >> 15) - clp) * clp_a) >> 15;
          click = (click * cf) >> 15;
        } else {
          clp -= clp >> 4;
        }
        acc[i] += clp >> 1;
      }
      if (click <= 8 && clp > -16 && clp < 16)
        clp = 0;
    }
    int32_t blo = v->blo, bbd = v->bbd, bo = v->bo, bp = v->bp;
    if (box0 | box) {
      int32_t bx = box0;
      for (int i = 0; i < 128; i += 8) {
        int32_t *q = acc + i, in = 0;
        for (int j = 0; j < 8; j++)
          in += q[j];
        const int32_t d = (bo - bp) >> 3;
        int32_t o = bp;
        for (int j = 0; j < 8; j++) {
          o += d;
          q[j] += o;
        }
        bp = bo;
        blo = clampi(blo + (bbd >> 2), -131071, 131071);
        bbd = clampi(bbd + (((in >> 3) - blo - (bbd >> 2)) >> 2), -131071,
                     131071);
        bo = (((bbd * (bx >> 2)) >> 13) * 3) >> 2;
        bx += dbox * 8;
      }
    } else
      bo = bp = 0;
    int32_t nprev = v->nprev, zlp = v->zlp;
    if (buzz0 | buzz) {
      uint32_t ph = ph0, c = c0;
      int32_t a = v->amp[0], bz = buzz0;
      for (int i = 0; i < 128; i += 8) {
        const int32_t s = sine(ph + c), as = s < 0 ? -s : s;
        const int32_t g =
            as > 22937 ? ((as - 22937) * 3) + ((as - 22937) >> 3) : 0;
        const int32_t gz = (((((a + step[0]) >> 10) * g) >> 15) * bz) >> 15;
        ph += c << 3;
        c += (uint32_t)dinc[0] << 3;
        a += step[0] * 8;
        bz += dbuzz * 8;
        if (!gz)
          continue;
        for (int j = i; j < i + 8; j++) {
          const int32_t n = (int32_t)lcg(&noise) >> 16;
          zlp += (((n - nprev) >> 1) - zlp) >> 1;
          acc[j] += (zlp * gz) >> 15;
          nprev = n;
        }
      }
    }
    int32_t wlo = v->wlo, wbd = v->wbd, wo = v->wo, wp = v->wp;
    const int32_t g = vg >> 2;
    const int32_t wf = 933 + (((sine(v->wph) + 32768) * 2562) >> 16);
    v->wph += 62144000u;
    if (wah0 | wah) {
      int32_t w = wah0;
      for (int i = 0; i < 128; i += 2) {
        const int32_t y0 = acc[i], y1 = acc[i + 1], wm = w >> 3;
        out[i] =
            sat16(((y0 + (((((wp + wo) >> 1) - y0) * wm) >> 12)) * g) >> 15);
        out[i + 1] = sat16(((y1 + (((wo - y1) * wm) >> 12)) * g) >> 15);
        wlo = clampi(wlo + ((wf * wbd) >> 13), -131071, 131071);
        wbd = clampi(
            wbd +
                ((wf * (((y0 + y1) >> 2) - wlo - ((2500 * wbd) >> 13))) >> 13),
            -131071, 131071);
        wp = wo;
        wo = wbd * 3;
        w += dwah * 2;
      }
    } else {
      wo = wp = 0;
      for (int i = 0; i < 128; i++)
        out[i] = sat16((acc[i] * g) >> 15);
    }
    for (int k = 0; k < 5; k++) {
      v->pinc[k] = inc[k];
      v->amp[k] =
          end[k] < (4 << 9) || (k && end[k] < (end[0] >> 10)) ? 0 : end[k];
    }
    v->click = click;
    v->clp = clp;
    v->nprev = nprev;
    v->zlp = zlp;
    v->noise = noise;
    v->blo = blo;
    v->bbd = bbd;
    v->bo = bo;
    v->bp = bp;
    v->wlo = wlo;
    v->wbd = wbd;
    v->wo = wo;
    v->wp = wp;
    v->box = box;
    v->buzz = buzz;
    v->wah = wah;
    v->dmix = dmix1;
    mb_level[vi] = v->amp[0] >> 9;
    mb_click[vi] = v->click;
    mb_pitch[vi] = inc0;
    mb_blocks++;
  }
};
} // namespace port
} // namespace hkmbira
