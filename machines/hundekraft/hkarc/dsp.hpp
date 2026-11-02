#pragma once
#include "wide.hpp"
#include "gp_table.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkarc {
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


static const int16_t ARC_GAIN[33] = {
    2048, 2049, 2050, 2051, 2053, 2054, 2056, 2057, 2058, 2059, 2059,
    2058, 2056, 2052, 2048, 2043, 2038, 2032, 2025, 2019, 2013, 2008,
    2004, 2000, 1997, 1994, 1992, 1991, 1990, 1989, 1989, 1989, 1989};

static const int16_t ARC_RW[18] = {1,     281,   3121,  3681,  4512,  5072,
                                   7592,  8792,  15683, 16523, 17972, 18812,
                                   22147, 22987, 25883, 26723, 32347, 32767};

static const uint16_t ARC_RR[18] = {8192,  8192,  10923, 10923, 12288, 12288,
                                    16384, 16384, 20480, 20480, 21845, 21845,
                                    24576, 24576, 27307, 27307, 32768, 32768};

typedef struct {
  uint32_t vtable;
  int32_t x1, v1;
  int32_t x2, v2;
  uint32_t rng;
  int32_t dc;
  int32_t lp;
  int16_t env;
  uint8_t gate;
  uint8_t live;
} arc_voice_t;

typedef char arc_voice_fits[sizeof(arc_voice_t) == 36 ? 1 : -1];

typedef struct {
  uint32_t m;
  int e;
} pitch_t;

typedef struct {
  int32_t h, sh, he, hk, sg, g;
} lamp_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  int16_t arc_trace[256][4];
  volatile uint16_t arc_tpos;
  volatile uint8_t arc_tvoice;
  volatile int16_t arc_level[1];
  arc_voice_t note;
  void defaults() { arc_tvoice = 0xff; }
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t SADD(int32_t a, int32_t b) {
    int32_t s = (int32_t)((uint32_t)a + (uint32_t)b);
    return ((a ^ s) & (b ^ s)) < 0 ? (a < 0 ? (int32_t)0x80000000 : 0x7fffffff)
                                   : s;
  }

  inline int32_t SHL3(int32_t a) {
    return a > 0x0FFFFFFF ? 0x7fffffff
                          : (a < -0x10000000 ? (int32_t)0x80000000 : a * 8);
  }

  int msb32(uint32_t v) {
    int e = 0;
    while (v > 1) {
      v >>= 1;
      e++;
    }
    return e;
  }

  uint32_t isqrt32(uint32_t v) {
    uint32_t r = 0, b = 1u << 30;
    while (b > v)
      b >>= 2;
    while (b) {
      if (v >= r + b) {
        v -= r + b;
        r = (r >> 1) + b;
      } else
        r >>= 1;
      b >>= 2;
    }
    return r;
  }

  pitch_t pitch_of(uint32_t inc, int32_t r14) {
    pitch_t p;
    if (inc < (1u << 18))
      inc = 1u << 18;
    p.e = msb32(inc);
    p.m = inc >> (p.e - 15);
    p.m = (p.m * (uint32_t)r14) >> 14;
    if (p.m >= 1u << 16) {
      p.m >>= 1;
      p.e++;
    } else if (p.m < 1u << 15) {
      p.m <<= 1;
      p.e--;
    }
    return p;
  }

  uint32_t gp_of(int ki, int32_t kf, pitch_t p) {
    int o = p.e - 19, pi;
    int32_t pf;
    if (o < 0) {
      pi = 0;
      pf = 0;
    } else if (o >= 12) {
      pi = 97 - 2;
      pf = 32767;
    } else {
      pi = o * 8 + (int)((p.m >> 12) & 7);
      pf = (int32_t)(p.m & 4095) << 3;
    }
    const uint16_t *a = ARC_GP[ki], *b = ARC_GP[ki + 1];
    int32_t g0 = a[pi] + ((((int32_t)a[pi + 1] - a[pi]) * pf) >> 15);
    int32_t g1 = b[pi] + ((((int32_t)b[pi + 1] - b[pi]) * pf) >> 15);
    return (uint32_t)(g0 + (((g1 - g0) * kf) >> 15));
  }

  int32_t shl(uint32_t v, int k) {
    return (int32_t)(k >= 0 ? v << k : (k > -32 ? v >> -k : 0));
  }

  void lamp_setup(lamp_t *L, uint32_t inc, int32_t r14, int oct, int32_t sag,
                  int ki, int32_t kf, int32_t eps12, int32_t k12,
                  int32_t sigc) {
    pitch_t p = pitch_of(inc, r14);
    p.e += oct;
    uint32_t P = ((p.m * gp_of(ki, kf, p)) >> 15) * (uint32_t)sag;
    if (!P)
      P = 1;
    int s = 58 - (msb32(P) + p.e), h;
    if (s < 0) {
      s = 0;
      h = 29491;
    } else {
      if (s > 24)
        s = 24;
      h = shl(P, p.e - 44 + s);
      if (s == 0 && h > 29491)
        h = 29491;
    }
    L->h = h;
    L->sh = s;
    int32_t he = ((h * eps12) >> 12) >> s;
    L->he = he > 9830 ? 9830 : he;
    L->hk = (h * k12) >> 10;
    uint32_t rt = isqrt32((uint32_t)h << 15) >> (s >> 1);
    if (s & 1)
      rt = (rt * 23170u) >> 15;
    L->sg = (int32_t)(((uint32_t)sigc * rt) >> 18);
    int32_t g = s >= 1 ? h >> (s - 1) : 2 * h;
    L->g = g > 29491 ? 29491 : g;
  }

  int32_t ratio_of(int32_t w) {
    if (w <= 0)
      return 16384;
    for (int i = 1; i < 18; i++)
      if (w <= ARC_RW[i]) {
        int32_t w0 = ARC_RW[i - 1], w1 = ARC_RW[i], r0 = ARC_RR[i - 1],
                r1 = ARC_RR[i];
        return w1 == w0 ? r1 : r0 + (r1 - r0) * (w - w0) / (w1 - w0);
      }
    return ARC_RR[18 - 1];
  }

  inline int32_t clampk(int32_t k) {
    return k < 0 ? 0 : (k > 32767 ? 32767 : k);
  }

  int16_t lienard(int32_t xs, int32_t v, int32_t eps12, int32_t vmul) {
    if (xs > 12288)
      xs = 12288;
    else if (xs < -12288)
      xs = -12288;
    int32_t x3 = (((xs * xs) >> 12) * xs) >> 12, F = xs - ((x3 * 10923) >> 15);
    int32_t y = (((v >> 15) - ((eps12 * F) >> 12)) * vmul) >> 12;
    return (int16_t)(y > 32767 ? 32767 : (y < -32767 ? -32767 : y));
  }

  void arc_note_on(void *self, const int16_t *params) {
    arc_voice_t *v = (arc_voice_t *)self;
    v->x1 = 0;
    v->v1 = 2 << 27;
    v->x2 = 1 << 30;
    v->v2 = 0;
    if (!v->rng)
      v->rng = 0x9E3779B9u ^ (uint32_t)(uintptr_t)self;
    v->dc = (((int32_t)(int16_t)(-8192) * (int16_t)(clampk(params[1]))) >> 16) *
            256;
    v->lp = 0;
    v->env = 32767;
    v->gate = 1;
    v->live = 1;
    arc_tvoice = (uint8_t)0;
  }

  void arc_note_off(void *self) { ((arc_voice_t *)self)->gate = 0; }

  void arc_render(void *self, int16_t *out, const int16_t *params) {
    arc_voice_t *v = (arc_voice_t *)self;
    const int32_t *inc = pitch_input;
    int32_t mu = clampk(params[0]), carbon = clampk(params[1]),
            hiss = clampk(params[2]), strike = clampk(params[3]);
    int32_t wr = clampk(params[5]), wc = clampk(params[6]),
            wb = clampk(params[7]);
    int vi = 0;
    int32_t u2 = (mu * mu) >> 15;
    int32_t eps12 = 164 + ((u2 * 28508) >> 15);
    int ki = (int)(mu >> 10);
    int32_t kf = (mu & 1023) << 5;
    int32_t sq = 32767 - strike, env0 = v->env,
            depth = (32767 - ((sq * sq) >> 15)) >> 1;
    int32_t dec = 1927 - ((strike * 1832) >> 15);
    int sput = v->live & 2;
    if (sput)
      dec <<= 2;
    int32_t env1 = env0 - ((env0 * dec) >> 15);
    if (env1 < 16)
      env1 = 0, sput = 0;
    if (v->gate && strike) {
      uint32_t q = v->rng;
      q ^= q << 13;
      q ^= q >> 17;
      q ^= q << 5;
      v->rng = q;
      if ((q & 0xffff) < (uint32_t)((((strike * strike) >> 15) * 1141) >> 15)) {
        int32_t amp = 11469 + (int32_t)(((q >> 16) * 18022) >> 16);
        if (amp > env1)
          env1 = amp, sput = 2;
      }
    }
    v->env = (int16_t)env1;
    v->live = (uint8_t)((v->live & 1) | sput);
    int32_t sag = 32767 - ((depth * ((env0 + env1) >> 1)) >> 15);
    int32_t dip0 = sput ? 32767 - ((env0 * 27853) >> 15) : 32767,
            dip1 = sput ? 32767 - ((env1 * 27853) >> 15) : 32767;
    int32_t h2k = (hiss * hiss) >> 15;
    int32_t sigc = ((h2k * 14745) >> 15) + ((((depth * env1) >> 15) * 3) >> 1);
    if (sigc > 32767)
      sigc = 32767;
    int32_t r14 = ratio_of(wr);
    int32_t k12 = ((wc * wc) >> 15) >> 5;
    if (k12 > eps12 >> 2)
      k12 = eps12 >> 2;
    int32_t mix = wc << 2;
    if (mix > 16384)
      mix = 16384;
    int dual = wc > 0;
    int32_t wbb = wb ? wb : 8192;
    int32_t bt = wbb <= 8192 ? 1024 + ((wbb * 3072) >> 13)
                             : 4096 + (((wbb - 8192) * 6144) / 24575);
    uint32_t im = (uint32_t)inc[128 / 2];
    lamp_t A, B;
    lamp_setup(&A, im, 16384, 0, sag, ki, kf, eps12, k12, sigc);
    if (dual)
      lamp_setup(&B, im, r14, 1, sag, ki, kf, eps12, k12, sigc);
    else
      B = A;
    int32_t gn =
        ARC_GAIN[ki] + (((ARC_GAIN[ki + 1] - ARC_GAIN[ki]) * kf) >> 15);
    int32_t gv = (32767 * gn) >> 12;
    gv = (gv * (4096 + (mix >> 4))) /
         (4096 + ((carbon * 1229) >> 15) + ((h2k * 1638) >> 15) +
          (bt > 4096 ? bt - 4096 : 0));
    if (gv > 32767)
      gv = 32767;
    int traced = vi == arc_tvoice, chunk = 128;
    if (traced) {
      int k = 32 - msb32(im | 1) - 6;
      chunk = k < 1 ? 2 : (k > 7 ? 128 : 1 << k);
    }
    int32_t epsd = A.he >= 9830 && (A.h >> A.sh) > 0
                       ? (9830 * 4096) / (A.h >> A.sh)
                       : eps12;
    if (epsd > eps12)
      epsd = eps12;
    int32_t vmul = (4096 * 4096) / (4096 + epsd / 3);
    int32_t x1 = v->x1, v1 = v->v1, x2 = v->x2, v2 = v->v2;
    uint32_t r = v->rng;
    const int32_t
        K1 = ((int32_t)(((uint32_t)(A.he) << 16) | ((uint32_t)(A.h) & 0xffff))),
        N1 =
            ((int32_t)(((uint32_t)(A.hk) << 16) | ((uint32_t)(A.sg) & 0xffff))),
        K2 = ((int32_t)(((uint32_t)(B.he) << 16) | ((uint32_t)(B.h) & 0xffff))),
        N2 =
            ((int32_t)(((uint32_t)(B.hk) << 16) | ((uint32_t)(B.sg) & 0xffff)));
    const int32_t O =
        ((int32_t)(((uint32_t)(mix) << 16) | ((uint32_t)(h2k >> 2) & 0xffff)));
    const int sh1 = A.sh, sh2 = B.sh;
    for (int16_t *o = out; o < out + 128;) {
      int32_t xs = 0, xt = 0;
      int16_t *e = o + chunk;
      if (!dual)
        for (; o < e; o++) {
          r ^= r << 13;
          r ^= r >> 17;
          r ^= r << 5;
          xs = x1 >> 17;
          do {
            int32_t f_ =
                2048 - (((int32_t)(int16_t)(xs) * (int16_t)(xs)) >> 13);
            int32_t a_ =
                (((int32_t)(int16_t)(f_) * (int16_t)((uint32_t)(K1) >> 16)) +
                 1024) >>
                11;
            if (a_ < (-24576))
              a_ = (-24576);
            (v1) = SADD((v1),
                        ((int32_t)(int16_t)((v1) >> 16) * (int16_t)(a_)) * 2);
            (v1) =
                SADD((v1), (((0) - ((int32_t)(int16_t)(xs) * (int16_t)(K1))) >>
                            (sh1)) +
                               ((int32_t)(int16_t)(r >> 16) * (int16_t)(N1)));
            (x1) = SADD((x1),
                        SHL3(((int32_t)(int16_t)((v1) >> 16) * (int16_t)(K1)) >>
                             (sh1)));
          } while (0);
          *o = (int16_t)(xs + (((int32_t)(int16_t)(((int32_t)(int16_t)(r) *
                                                    (int16_t)(O)) >>
                                                   15) *
                                (int16_t)(xs < 0 ? -xs : xs)) >>
                               13));
        }
      else
        for (; o < e; o += 2) {
          r ^= r << 13;
          r ^= r >> 17;
          r ^= r << 5;
          int32_t xp = x2 >> 17, dv = (v2 >> 17) - (v1 >> 17);
          xt = xp;
          do {
            int32_t f_ =
                2048 - (((int32_t)(int16_t)(xt) * (int16_t)(xt)) >> 13);
            int32_t a_ =
                (((int32_t)(int16_t)(f_) * (int16_t)((uint32_t)(K2) >> 16)) +
                 1024) >>
                11;
            if (a_ < (-24576))
              a_ = (-24576);
            (v2) = SADD((v2),
                        ((int32_t)(int16_t)((v2) >> 16) * (int16_t)(a_)) * 2);
            (v2) = SADD((v2), (((-((int32_t)(int16_t)(dv) *
                                   (int16_t)((uint32_t)(N2) >> 16))) -
                                ((int32_t)(int16_t)(xt) * (int16_t)(K2))) >>
                               (sh2)) +
                                  ((int32_t)(int16_t)(r >> 8) * (int16_t)(N2)));
            (x2) = SADD((x2),
                        SHL3(((int32_t)(int16_t)((v2) >> 16) * (int16_t)(K2)) >>
                             (sh2)));
          } while (0);
          xt = x2 >> 17;
          xp = (xp + xt) >> 1;
          xs = x1 >> 17;
          dv = ((int32_t)(int16_t)((v2 >> 17) - (v1 >> 17)) *
                (int16_t)((uint32_t)(N1) >> 16));
          do {
            int32_t f_ =
                2048 - (((int32_t)(int16_t)(xs) * (int16_t)(xs)) >> 13);
            int32_t a_ =
                (((int32_t)(int16_t)(f_) * (int16_t)((uint32_t)(K1) >> 16)) +
                 1024) >>
                11;
            if (a_ < (-24576))
              a_ = (-24576);
            (v1) = SADD((v1),
                        ((int32_t)(int16_t)((v1) >> 16) * (int16_t)(a_)) * 2);
            (v1) =
                SADD((v1), (((dv) - ((int32_t)(int16_t)(xs) * (int16_t)(K1))) >>
                            (sh1)) +
                               ((int32_t)(int16_t)(r >> 16) * (int16_t)(N1)));
            (x1) = SADD((x1),
                        SHL3(((int32_t)(int16_t)((v1) >> 16) * (int16_t)(K1)) >>
                             (sh1)));
          } while (0);
          o[0] = (int16_t)(xs +
                           (((int32_t)(int16_t)((xp - xs) >> 1) *
                             (int16_t)((uint32_t)(O) >> 16)) >>
                            14) +
                           (((int32_t)(int16_t)(((int32_t)(int16_t)(r) *
                                                 (int16_t)(O)) >>
                                                15) *
                             (int16_t)(xs < 0 ? -xs : xs)) >>
                            13));
          xs = x1 >> 17;
          do {
            int32_t f_ =
                2048 - (((int32_t)(int16_t)(xs) * (int16_t)(xs)) >> 13);
            int32_t a_ =
                (((int32_t)(int16_t)(f_) * (int16_t)((uint32_t)(K1) >> 16)) +
                 1024) >>
                11;
            if (a_ < (-24576))
              a_ = (-24576);
            (v1) = SADD((v1),
                        ((int32_t)(int16_t)((v1) >> 16) * (int16_t)(a_)) * 2);
            (v1) =
                SADD((v1), (((dv) - ((int32_t)(int16_t)(xs) * (int16_t)(K1))) >>
                            (sh1)) +
                               ((int32_t)(int16_t)(r) * (int16_t)(N1)));
            (x1) = SADD((x1),
                        SHL3(((int32_t)(int16_t)((v1) >> 16) * (int16_t)(K1)) >>
                             (sh1)));
          } while (0);
          o[1] = (int16_t)(xs +
                           (((int32_t)(int16_t)((xt - xs) >> 1) *
                             (int16_t)((uint32_t)(O) >> 16)) >>
                            14) +
                           (((int32_t)(int16_t)(((int32_t)(int16_t)(r >> 16) *
                                                 (int16_t)(O)) >>
                                                15) *
                             (int16_t)(xs < 0 ? -xs : xs)) >>
                            13));
        }
      if (traced) {
        int t = arc_tpos;
        arc_trace[t][0] = (int16_t)xs;
        arc_trace[t][1] = lienard(xs, v1, epsd, vmul);
        arc_trace[t][2] = (int16_t)(dual ? xt : xs);
        arc_trace[t][3] = dual ? lienard(xt, v2, epsd, vmul) : arc_trace[t][1];
        arc_tpos = (uint16_t)((t + 1) & (256 - 1));
      }
    }
    v->x1 = x1;
    v->v1 = v1;
    v->x2 = x2;
    v->v2 = v2;
    v->rng = r;
    int32_t dc = v->dc, lp = v->lp, pk = 0;
    const int16_t cb = (int16_t)carbon, g = (int16_t)A.g, btq = (int16_t)bt;
    int32_t ga = ((gv * dip0) >> 15) << 16,
            dg = ((((gv * dip1) >> 15) << 16) - ga) >> 7;
    for (int n = 0; n < 128; n++) {
      const int16_t gq = (int16_t)(ga >> 16);
      ga += dg;
      int32_t y = out[n];
      int32_t q = (((int32_t)(int16_t)(y) * (int16_t)(y)) >> 12) - 8192;
      if (q > 32767)
        q = 32767;
      y += ((int32_t)(int16_t)(q) * (int16_t)(cb)) >> 16;
      dc += (y * 256 - dc) >> 10;
      y -= dc >> 8;
      if (y > 16383)
        y = 16383;
      else if (y < -16383)
        y = -16383;
      lp += ((int32_t)(int16_t)(y - (lp >> 8)) * (int16_t)(g)) >> 7;
      int32_t l = lp >> 8,
              o = l + (((int32_t)(int16_t)(y - l) * (int16_t)(btq)) >> 12);
      if (o > 32767)
        o = 32767;
      else if (o < -32767)
        o = -32767;
      out[n] = (int16_t)(SHL3(((int32_t)(int16_t)(o) * (int16_t)(gq))) >> 16);
    }
    v->dc = dc;
    v->lp = lp;
    for (int n = 0; n < 128; n += 8) {
      int32_t a = out[n] < 0 ? -out[n] : out[n];
      if (a > pk)
        pk = a;
    }
    arc_level[vi] = (int16_t)pk;
  }
};
} // namespace port
} // namespace hkarc
