#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkmixtur {
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

static const uint32_t DIV_D[25] = {
    0u,         0u,         2147483648u, 1431655765u, 1073741824u,
    858993459u, 715827882u, 613566756u,  536870912u,  477218588u,
    429496729u, 390451572u, 357913941u,  330382099u,  306783378u,
    286331153u, 268435456u, 252645135u,  238609294u,  226050910u,
    214748364u, 204522252u, 195225786u,  186737708u,  178956970u};

static const uint16_t DIV_R[25] = {
    0,    65535, 32768, 21845, 16384, 13107, 10922, 9362, 8192,
    7281, 6553,  5957,  5461,  5041,  4681,  4369,  4096, 3855,
    3640, 3449,  3276,  3120,  2978,  2849,  2730};

static const int16_t F1_COEF[33] = {
    210,  232,  255,  281,  310,  342,  377,  416,  458,  505,  557,
    614,  677,  746,  822,  906,  999,  1101, 1213, 1337, 1474, 1624,
    1790, 1972, 2173, 2394, 2637, 2905, 3198, 3521, 3875, 4263, 4687};

static const int16_t F2_COEF[33] = {
    583,  629,  679,  732,  790,  852,  919,  992,  1070, 1154, 1244,
    1342, 1448, 1561, 1684, 1816, 1958, 2112, 2277, 2455, 2647, 2853,
    3075, 3314, 3571, 3847, 4144, 4462, 4804, 5169, 5561, 5979, 6425};

static const int16_t RES_Q[33] = {
    11703, 7588, 6069, 5065, 4326, 3751, 3289, 2908, 2589, 2318, 2086,
    1885,  1709, 1554, 1418, 1297, 1189, 1093, 1006, 928,  857,  793,
    735,   682,  634,  589,  549,  512,  478,  446,  417,  390,  366};

static const int16_t RES_MAKEUP[33] = {
    2785, 3341, 3670, 3959, 4231,  4492,  4747,  4999,  5248,  5498,  5747,
    5997, 6249, 6503, 6758, 7016,  7277,  7540,  7807,  8077,  8350,  8627,
    8907, 9192, 9480, 9772, 10069, 10370, 10675, 10984, 11298, 11617, 11941};

static const int16_t F1_HZ[33] = {
    180,  198,  219,  241,  266,  293,  323,  356,  393,  433,  477,
    526,  580,  639,  705,  777,  856,  944,  1041, 1147, 1265, 1394,
    1537, 1694, 1867, 2059, 2269, 2502, 2758, 3040, 3352, 3695, 4073};

static const int16_t F2_HZ[33] = {
    500,  539,  582,  628,  677,  730,  788,  850,  917,  989,  1067,
    1151, 1242, 1340, 1445, 1559, 1682, 1814, 1957, 2111, 2278, 2457,
    2650, 2859, 3084, 3327, 3589, 3872, 4177, 4506, 4861, 5244, 5657};

static const int16_t RES_Q10[33] = {7,   11,  13,  16,  19,  22,  25,  28,  32,
                                    35,  39,  43,  48,  53,  58,  63,  69,  75,
                                    81,  88,  96,  103, 111, 120, 129, 139, 149,
                                    160, 172, 184, 196, 210, 224};

typedef struct {
  uint32_t vtable;
  uint32_t p;
  int32_t lp1, bp1, lp2, bp2;
  uint8_t c[4];
  uint8_t n[4];
  uint8_t g[4];
} mx_voice_t;

typedef char mx_size_check[sizeof(mx_voice_t) <= 36 ? 1 : -1];

static const uint8_t NORM[5] = {0, 196, 132, 108, 94};

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  uint32_t own[1];
  mx_voice_t note;
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline uint32_t sig(const void *v_) {
    const uint32_t *w = (const uint32_t *)v_;
    return (w[1] ^ 0x6D697874u) + w[3] * 0x9E3779B1u + w[5] * 0x85EBCA6Bu +
           w[6];
  }

  inline int32_t mul_ll(int32_t a, int32_t b) {
    return (int16_t)a * (int16_t)b;
  }

  inline int32_t mul_lh(int32_t a, int32_t b) {
    return (int16_t)a * (int16_t)((uint32_t)b >> 16);
  }

  inline uint32_t umul_ll(uint32_t a, uint32_t b) {
    return (uint16_t)a * (uint32_t)(uint16_t)b;
  }

  inline uint32_t umul_lh(uint32_t a, uint32_t b) {
    return (uint16_t)a * (b >> 16);
  }

  inline int32_t clamp(int32_t x, int32_t m) {
    return x > m ? m : (x < -m ? -m : x);
  }

  inline int knob_n(int32_t k) {
    return (int)(((unsigned)(k < 0 ? 0 : k) >> 9) % 25u);
  }

  inline int32_t page_word(const int16_t *params, int i) {
    return params[i] > 0 ? params[i] : 8192;
  }

  inline int32_t curve(const int16_t *t, int32_t w) {
    int i = w >> 10;
    return t[i] + (((t[i + 1] - t[i]) * (w & 1023)) >> 10);
  }

  void targets(const int16_t *params, uint32_t inc, int nt[4], int32_t gt[4]) {
    int act = 0;
    for (int k = 0; k < 4; k++) {
      nt[k] = knob_n(params[k]);
      gt[k] = nt[k] != 0 ? 256 : 0;
      for (int j = 0; j < k; j++)
        if (nt[j] == nt[k])
          gt[k] = 0;
      if (gt[k] && inc) {
        uint32_t s = inc / (uint32_t)nt[k];
        gt[k] = s >= 3895834u   ? 256
                : s <= 2434896u ? 0
                                : (int32_t)(((s - 2434896u) >> 4) * 256 /
                                            ((3895834u - 2434896u) >> 4));
      }
      act += gt[k] >= 128;
    }
    if (!act)
      act = 1;
    for (int k = 0; k < 4; k++)
      gt[k] = (NORM[act] * gt[k]) >> 8;
  }

  int adopt(mx_voice_t *v) {
    int vi = 0;
    if (vi < 0 || vi >= 1)
      return 0;
    if (own[vi] != sig(v)) {
      v->p = 0;
      v->lp1 = v->bp1 = v->lp2 = v->bp2 = 0;
      for (int k = 0; k < 4; k++)
        v->c[k] = v->n[k] = v->g[k] = 0;
    }
    return vi;
  }

  void mx_note_on(void *self, const int16_t *params) {
    mx_voice_t *v = (mx_voice_t *)self;
    int vi = adopt(v), n[4];
    int32_t gt[4];
    targets(params, 0, n, gt);
    for (int k = 0; k < 4; k++) {
      if (v->c[k] >= n[k])
        v->c[k] = 0;
      v->n[k] = (uint8_t)n[k];
      if (n[k] == 1)
        v->g[k] = (uint8_t)gt[k];
    }
    v->n[0] |= 0x80;
    own[vi] = sig(v);
  }

  void mx_note_off(void *self) { ((mx_voice_t *)self)->n[0] &= (uint8_t)~0x80; }

  void mx_render(void *self, int16_t *out, const int16_t *params) {
    mx_voice_t *v = (mx_voice_t *)self;
    const int32_t *inc = pitch_input;
    int vi = adopt(v), nt[4];
    int32_t gt[4];
    targets(params, (uint32_t)inc[0], nt, gt);
    uint32_t base[4], R[4], c[4], n[4];
    int32_t gq[4], dg[4];
    for (int k = 0; k < 4; k++) {
      uint32_t nk = v->n[k] & 31u, ck = v->c[k];
      int32_t g = v->g[k];
      if (nk > 24)
        nk = 0;
      if (nk != (uint32_t)nt[k] && g == 0) {
        nk = (uint32_t)nt[k];
        if (ck >= 24)
          ck = 0;
        while (nk && ck >= nk)
          ck -= nk;
      }
      if (!nk || ck >= nk)
        ck = 0;
      int32_t tgt = nk == (uint32_t)nt[k] ? gt[k] : 0, gn = g;
      gn += clamp(tgt - g, 64);
      n[k] = nk;
      c[k] = ck;
      R[k] = DIV_R[nk];
      base[k] = ck * DIV_D[nk];
      gq[k] = g << 7;
      dg[k] = (gn - g) * 16;
      gq[k] -= dg[k];
      v->g[k] = (uint8_t)gn;
    }
    int32_t f1 = curve(F1_COEF, page_word(params, 5)),
            f2 = curve(F2_COEF, page_word(params, 6));
    int32_t wr = page_word(params, 7), q = curve(RES_Q, wr);
    int32_t gain = (((curve(RES_MAKEUP, wr) * 32767) >> 15) * 5793) >> 12;
    if (gain > 32767)
      gain = 32767;
    uint32_t inc0 = (uint32_t)inc[0] | 1u;
    int sh = 0;
    while ((inc0 >> sh) >= 32768u)
      sh++;
    uint32_t rinv = (1u << 29) / (inc0 >> sh);
    uint32_t p = v->p, A = 0, B = 0, off = 0;
    int32_t gm = dg[0] | dg[1] | dg[2] | dg[3] ? 15 : -1;
    int16_t xbuf[128];
    for (int i = 0; i < 128; i++) {
      uint32_t di = (uint32_t)inc[i], pn = p + di;
      int32_t blep = 0, upd = (i & gm) == 0;
      if (upd) {
        for (int k = 0; k < 4; k++)
          gq[k] += dg[k];
        off = (uint32_t)(gq[0] + gq[1] + gq[2] + gq[3]) << 14;
        B = 0;
        for (int k = 0; k < 4; k++)
          B += ((uint32_t)gq[k] * R[k]) >> 17;
      }
      if (pn < p) {
        int32_t hg = 0;
        for (int k = 0; k < 4; k++) {
          if (++c[k] >= n[k]) {
            c[k] = 0;
            hg += gq[k];
          }
          base[k] = c[k] * DIV_D[n[k]];
        }
        uint32_t d = ((pn >> sh) * rinv) >> 14;
        if (d > 32767)
          d = 32767;
        int32_t e = 32768 - (int32_t)d, sq = (e * e) >> 15;
        blep += ((hg >> 2) * sq) >> 14;
        upd = 1;
      }
      if (upd) {
        A = 0;
        for (int k = 0; k < 4; k++)
          A += (uint32_t)gq[k] * (base[k] >> 17);
      }
      p = pn;
      if (p + di < p) {
        int32_t hg = 0;
        for (int k = 0; k < 4; k++)
          if (c[k] + 1 >= n[k])
            hg += gq[k];
        uint32_t d = (((p + di) >> sh) * rinv) >> 14;
        if (d > 32767)
          d = 32767;
        int32_t sq = ((int32_t)d * (int32_t)d) >> 15;
        blep -= ((hg >> 2) * sq) >> 14;
      }
      xbuf[i] =
          (int16_t)((((int32_t)(A + umul_ll(p >> 16, B) - off) >> 15) + blep) >>
                    2);
    }
    uint32_t fq = (uint32_t)q << 16 | (uint32_t)f1,
             f2g = (uint32_t)gain << 16 | (uint32_t)f2;
    int32_t lp1 = clamp(v->lp1, 131071), bp1 = clamp(v->bp1, 131071),
            lp2 = clamp(v->lp2, 131071), bp2 = clamp(v->bp2, 131071);
    for (int i = 0; i < 128; i++) {
      int32_t in = mul_lh(xbuf[i], fq) >> 12, hp, b;
      b = bp1 >> 2;
      lp1 += mul_ll(b, fq) >> 11;
      hp = in - lp1 - (mul_lh(b, fq) >> 11);
      bp1 += mul_ll(hp >> 3, fq) >> 10;
      b = bp2 >> 2;
      lp2 += mul_ll(b, f2g) >> 11;
      hp = in - lp2 - (mul_lh(b, fq) >> 11);
      bp2 += mul_ll(hp >> 3, f2g) >> 10;
      int32_t y = (bp1 >> 2) + (mul_ll(bp2 >> 2, 22938) >> 15);
      y = clamp(mul_lh(y >> 1, f2g) >> 8, 49152);
      int32_t ay = y < 0 ? -y : y;
      if (ay > 22938) {
        int32_t t = ay - 22938;
        if (t > 19658)
          t = 19658;
        ay = 22938 + t - ((((t * t) >> 15) * 27312) >> 15);
        y = y < 0 ? -ay : ay;
      }
      out[i] = op1_sat16(y);
    }
    v->p = p;
    v->lp1 = lp1;
    v->bp1 = bp1;
    v->lp2 = lp2;
    v->bp2 = bp2;
    for (int k = 0; k < 4; k++)
      v->c[k] = (uint8_t)c[k];
    v->n[0] = (uint8_t)(n[0] | (v->n[0] & 0x80));
    v->n[1] = (uint8_t)n[1];
    v->n[2] = (uint8_t)n[2];
    v->n[3] = (uint8_t)n[3];
    own[vi] = sig(v);
  }
};
} // namespace port
} // namespace hkmixtur
