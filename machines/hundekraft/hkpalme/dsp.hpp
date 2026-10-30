#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkpalme {
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

static const int32_t pg_bot[12][2] = {
    {146583, 136047}, {149087, 133502}, {151963, 131385}, {155136, 129749},
    {158529, 128635}, {162055, 128071}, {165625, 128071}, {169151, 128635},
    {172544, 129749}, {175717, 131385}, {178593, 133502}, {181097, 136047}};

static const int32_t pg_top[12][2] = {
    {99517, 96554},  {99099, 75813},  {105435, 56397}, {117589, 40108},
    {134212, 28398}, {153644, 22281}, {174036, 22281}, {193468, 28398},
    {210091, 40108}, {222245, 56397}, {228581, 75813}, {228163, 96554}};

static const int32_t pg_dmp[12][2] = {
    {139523, 130123}, {143052, 126537}, {147104, 123554}, {151576, 121249},
    {156356, 119679}, {161324, 118884}, {166356, 118884}, {171324, 119679},
    {176104, 121249}, {180576, 123554}, {184628, 126537}, {188157, 130123}};

static const int16_t pg_nrm[12][2] = {{658, -784}, {774, -671}, {870, -540},
                                      {944, -396}, {995, -241}, {1021, -81},
                                      {1021, 81},  {995, 241},  {944, 396},
                                      {870, 540},  {774, 671},  {658, 784}};

static const int16_t pg_dial[4][2] = {
    {260, 30}, {298, 30}, {260, 92}, {298, 92}};

static const uint8_t pg_dial_col[4][4] = {{77, 158, 255, 255},
                                          {0, 237, 149, 255},
                                          {255, 255, 255, 255},
                                          {255, 58, 93, 255}};

static const uint8_t pm_pow2z[64] = {
    86,  86,  162, 88,  89,  137, 34,  147, 37, 38, 86,  162, 200,
    100, 37,  150, 85,  89,  25,  147, 85,  86, 89, 37,  150, 88,
    98,  149, 85,  89,  37,  150, 88,  101, 85, 98, 137, 85,  89,
    149, 85,  137, 149, 88,  149, 85,  137, 85, 98, 149, 88,  37,
    86,  137, 161, 100, 152, 100, 85,  137, 85, 34, 150, 100};

static const int16_t pm_gong[12] = {0,    2429, 3749, 6043, 6264, 8157,
                                    8437, 8834, 36,   2401, 3772, 6002};

typedef struct {
  int16_t *L;
  int32_t wp;
  int32_t age;
  int32_t P[12];
  int32_t t24[12][3];
  int16_t di[12], fr[12];
  int32_t q[12][3];
  int32_t S[12];
  int32_t dgl[12];
  int32_t env[12];
  int32_t fe[12];
  int32_t tgt[12], integ[12];
  int32_t ein, lp, hq, ow;
  int32_t fb[64], fbsum;
  int16_t pw[12], kq[12];
  uint16_t czm;
  uint8_t latched, quiet, rr, clr;
} palme_t;

typedef int32_t __attribute__((may_alias)) a32_t;

static const int16_t pm_cos16[16] = {2048,  1892,  1448,  784,   0,     -784,
                                     -1448, -1892, -2048, -1892, -1448, -784,
                                     0,     784,   1448,  1892};

static const int8_t pm_spread[12] = {0,   90,  -58,  127, -108, 38,
                                     -77, 102, -127, 64,  -26,  115};

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  palme_t st[3];
  volatile int16_t s_env[3][12];
  volatile int16_t s_in[3];
  uint16_t pm_pow2n[257];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  void pow2n_expand(void) {
    if (pm_pow2n[256])
      return;
    int32_t v = 32768, d = 89;
    for (int k = 0; k < 256; k++) {
      pm_pow2n[k] = (uint16_t)v;
      v -= d;
      d -= ((pm_pow2z[k >> 2] >> ((k & 3) * 2)) & 3) - 1;
    }
    pm_pow2n[256] = (uint16_t)v;
  }

  uint32_t pow2n(uint32_t v, int32_t n, int sh) {
    if (n < 0)
      n = 0;
    uint32_t i = (uint32_t)n >> 16, f = (uint32_t)n & 0xffff, k = f >> 8,
             t = f & 0xff;
    uint32_t m =
        pm_pow2n[k] - (((uint32_t)(pm_pow2n[k] - pm_pow2n[k + 1]) * t) >> 8);
    uint32_t r = (v * m) >> sh;
    return i < 31 ? r >> i : 0;
  }

  inline int32_t MULQ15(int16_t a, int16_t b) {
    int32_t r = (a * b) >> 15;
    return r > 32767 ? 32767 : r;
  }

  inline int32_t MUL16(int16_t a, int16_t b) { return a * b; }

  inline int32_t ADDQ(int16_t a, int16_t b) { return op1_sat16(a + b); }

  inline int32_t SUBQ(int16_t a, int16_t b) { return op1_sat16(a - b); }

  inline int32_t MULF(int16_t a, int16_t b) {
    return a == -32768 && b == -32768 ? 0x7fffffff : a * b * 2;
  }

  inline int32_t ADDF(int32_t a, int32_t b) {
    int32_t r = (int32_t)((uint32_t)a + (uint32_t)b);
    return ((a ^ r) & (b ^ r)) < 0 ? (a < 0 ? (int32_t)0x80000000 : 0x7fffffff)
                                   : r;
  }

  inline int32_t softclip16(int16_t x) {
    int32_t x2 = MULQ15(x, x), x3 = MULQ15(x2, x);
    return x - MULQ15(x3, 10923);
  }

  inline int msb(uint32_t v) {
    int b = 0;
    if (v >> 16) {
      v >>= 16;
      b += 16;
    }
    if (v >> 8) {
      v >>= 8;
      b += 8;
    }
    if (v >> 4) {
      v >>= 4;
      b += 4;
    }
    if (v >> 2) {
      v >>= 2;
      b += 2;
    }
    return b + (int)(v >> 1);
  }

  void zero16(int16_t *p, int32_t n) {
    if (n <= 0)
      return;
    if ((uintptr_t)p & 2) {
      *p++ = 0;
      n--;
    }
    a32_t *w = (a32_t *)(void *)p;
    for (int32_t k = 0; k < n >> 1; k++)
      w[k] = 0;
    if (n & 1)
      p[n - 1] = 0;
  }

  int32_t rsum(const int16_t *ls, int32_t wp, int32_t k0, int32_t k1) {
    int32_t lo = wp - k1, hi = wp - k0, s = 0;
    if (lo >= hi)
      return 0;
    if (lo < 0) {
      int32_t e = hi < 0 ? hi : 0;
      for (const int16_t *p = ls + 1024 + lo; p < ls + 1024 + e; p++)
        s += *p;
      lo = 0;
    }
    for (const int16_t *p = ls + lo; p < ls + hi; p++)
      s += *p;
    return s;
  }

  void palme_init(int pos) {
    palme_t *s = &st[pos % 3];
    pow2n_expand();
    s->L = (int16_t *)scratch;
    if (!s->L)
      return;
    s->L += 48;
    s->clr = 8;
    s->age = 0;
    s->czm = (1 << 12) - 1;
    s->wp = 0;
    s->rr = 0;
    s->ein = 0;
    s->lp = 0;
    s->hq = 0;
    s->latched = 0;
    s->quiet = 200;
    for (int i = 0; i < 12; i++) {
      s->P[i] = 0;
      s->q[i][0] = s->q[i][1] = s->q[i][2] = 0;
      s->S[i] = 0;
      s->pw[i] = 0;
      s->kq[i] = 0;
      s->dgl[i] = 0;
      s->di[i] = 2;
      s->fr[i] = 0;
      s->t24[i][0] = s->t24[i][1] = s->t24[i][2] = 0;
      s->env[i] = s->fe[i] = s->tgt[i] = s->integ[i] = 0;
      s_env[pos % 3][i] = 0;
    }
    for (int n = 0; n < 64; n++)
      s->fb[n] = 0;
    s->fbsum = 0;
    s->ow = 0;
  }

  __attribute__((noinline)) void palme_clearing(palme_t *s, int pos,
                                                int16_t *buf, int32_t k3) {
    a32_t *w =
        (a32_t *)(void *)(s->L - 48) + s->clr * (12 * (48 + 1024) / 2 / 8);
    for (int k = 0; k < (12 * (48 + 1024) / 2 / 8); k++)
      w[k] = 0;
    s->clr++;
    int16_t dryg = (int16_t)(k3 < 16384 ? 32767 : (32767 - k3) * 2);
    (void)dryg;
    (void)dryg;
    int32_t peak = 0;
    for (int n = 0; n < 64; n++) {
      int32_t o = 0;
      buf[n] = (int16_t)o;
      if (o < 0)
        o = -o;
      if (o > peak)
        peak = o;
    }
    volatile int16_t *m = (&meter);
    int32_t mv = (*m * 0xfe00) >> 16;
    *m = (int16_t)(peak > mv ? peak : mv);
  }

  void palme_render(int pos, int16_t *buf, const int16_t *p) {
    palme_t *s = &st[pos % 3];
    int16_t *L = s->L;
    if (!L)
      return;
    int32_t k0 = ((int32_t)(p)[0]), k1 = ((int32_t)(p)[1]);
    int32_t k2 = ((int32_t)(p)[2]), k3 = ((int32_t)(p)[3]);
    int32_t mg = p[5] < 0 ? 0 : p[5], damp = p[6] < 0 ? 0 : p[6],
            spread = p[7] < 0 ? 0 : p[7];
    int latch = fx_page[pos % 3][3] != 0;
    if (s->clr < 8) {
      palme_clearing(s, pos, buf, k3);
      return;
    }
    int32_t root = 36 + ((k0 * 36) >> 15);
    uint32_t a27 = pow2n(84095, (k1 * 14143) >> 10, 15);
    int32_t bq = 15729 - ((damp * 15401) >> 15);
    int32_t sp = (spread * 90) >> 15;
    int32_t ex2 = (k2 * k2) >> 15;
    int32_t lpc = 1500 + ((ex2 * 29000) >> 15);
    int32_t drive = 600 + ((ex2 * 15000) >> 15) + (k2 >> 3);
    int16_t ex[64];
    int32_t ipk = 0, lp = s->lp, hq = s->hq;
    int32_t fbg = latch || s->quiet >= 200 ? 0 : (mg * 1000) >> 15,
            fbm = s->fbsum >> 6;
    for (int n = 0; n < 64; n++) {
      int32_t in = buf[n];
      int32_t a = in < 0 ? -in : in;
      if (a > ipk)
        ipk = a;
      hq += in - (hq >> 8);
      lp += MULQ15(SUBQ(SUBQ(in, hq >> 8), lp), lpc);
      ex[n] =
          (int16_t)ADDQ(MULQ15(lp, drive), MULQ15(SUBQ(s->fb[n], fbm), fbg));
    }
    int32_t exo = 0;
    for (int n = 0; n < 64 / 2; n++)
      exo |= ((const a32_t *)(const void *)ex)[n];
    if (!ipk && lp > -64 && lp < 64)
      lp = 0;
    s->lp = lp;
    s->hq = hq;
    s->ein = ipk > s->ein ? ipk : s->ein - (s->ein >> 4);
    if (s->ein > 300)
      s->quiet = 0;
    else if (s->quiet < 200)
      s->quiet++;
    int follow = s->quiet < 200;
    if (latch && !s->latched)
      for (int i = 0; i < 12; i++) {
        s->integ[i] = 0;
        s->dgl[i] = 0;
      }
    s->latched = (uint8_t)latch;
    int32_t wp = s->wp, age = s->age;
    int rr = s->rr;
    s->rr = (uint8_t)(rr == 5 ? 0 : rr + 1);
    for (int i = 0; i < 12; i++) {
      int16_t *ls = L + i * (48 + 1024);
      if (i == rr || i == rr + 6) {
        int32_t P = s->P[i], zl = 0;
        if (P) {
          int32_t vmax = -32768, vmin = 32767, sum = 0, ca = 0, sa = 0,
                  env = s->env[i];
          for (int j = 0; j < 16; j++) {
            int32_t v = ls[(wp - 1 - ((j * (P >> 8)) >> 12)) & (1024 - 1)];
            ca += v * pm_cos16[j];
            sa += v * pm_cos16[(j + 4) & 15];
            sum += v;
            if (v > vmax)
              vmax = v;
            if (v < vmin)
              vmin = v;
          }
          int32_t mean = sum >> 4,
                  pk = vmax - mean > mean - vmin ? vmax - mean : mean - vmin;
          ca >>= 16;
          sa >>= 16;
          int32_t fe = s->fe[i] + ((ca * ca + sa * sa - s->fe[i]) >> 1);
          env += (pk - env) >> 1;
          if (env <= 12 && s->ein < 64 && !(latch && s->tgt[i])) {
            zl = s->di[i] + 8;
            if (!(s->czm & (1 << i))) {
              int32_t k0 = wp - zl;
              if (k0 >= 0)
                zero16(ls + k0, zl);
              else {
                zero16(ls + 1024 + k0, -k0);
                zero16(ls, wp);
              }
              ls[-1] = ls[1024 - 1];
              ls[-2] = ls[1024 - 2];
              s->czm |= (uint16_t)(1 << i);
            }
            env = fe = 0;
            s->S[i] = 0;
            s->pw[i] = 0;
          }
          s->fe[i] = fe;
          s->env[i] = env;
          s_env[pos % 3][i] = (int16_t)env;
          int32_t Pi = P >> 16, fr = s->fr[i], ir = 2048 - fr;
          uint32_t inv = (1u << 30) / (uint32_t)(P >> 6);
          uint32_t i16 = inv >> 4, u = ((((i16 * i16) >> 8) * 20213u) >> 10);
          uint32_t cl =
              (uint32_t)(((bq * (32768 - bq)) >> 15) + ((fr * ir) >> 7));
          int32_t lf = (int32_t)(((u >> 4) * cl) >> 11);
          int32_t kk = 24 - msb((uint32_t)lf) - 2;
          kk = kk < 3 ? 3 : (kk > 6 ? 6 : kk);
          s->kq[i] = (int16_t)(inv >> kk);
          if (!latch)
            s->integ[i] = 0;
          else {
            int32_t ff = lf + (((lf >> 9) * (lf >> 9)) >> 6);
            int32_t rel = 0;
            if (follow)
              s->tgt[i] = fe < (12 * 12)
                              ? 0
                              : (fe < (1750 * 1750) ? fe : (1750 * 1750));
            else if (s->tgt[i]) {
              int32_t t = s->tgt[i], sh = msb((uint32_t)t), d = t - fe;
              rel = sh > 14 ? d >> (sh - 14) : d * (1 << (14 - sh));
              if (rel < -32768)
                rel = -32768;
              else if (rel > 32767)
                rel = 32767;
              s->integ[i] += (rel * Pi) >> 7;
              if (s->integ[i] > 335544)
                s->integ[i] = 335544;
              else if (s->integ[i] < -335544)
                s->integ[i] = -335544;
            }
            s->dgl[i] = ff + s->integ[i] + ((rel * Pi * 13) >> 6);
          }
        }
        int32_t pc = (root - 36 + i) << 8, pg = ((root - 36) << 8) + pm_gong[i];
        int32_t pitch =
            pc + (((pg - pc) * mg) >> 15) + ((sp * pm_spread[i]) >> 7);
        if (pitch < 0)
          pitch = 0;
        int32_t Pt = (int32_t)pow2n(86303, (pitch * 21845) >> 10, 6);
        if (!P)
          P = Pt;
        else {
          int32_t d = (Pt - P) >> 1, lim = P >> 7;
          P += d > lim ? lim : (d < -lim ? -lim : d);
        }
        s->P[i] = P;
        int32_t Dl = P - (bq << 1);
        if (Dl < (2 << 16))
          Dl = 2 << 16;
        if (Dl > ((1024 - 64 - 16) << 16))
          Dl = (1024 - 64 - 16) << 16;
        int32_t fr = (Dl & 0xffff) >> 5, ir = 2048 - fr;
        s->di[i] = (int16_t)(Dl >> 16);
        s->fr[i] = (int16_t)fr;
        int32_t pw = P >> 16, ow = s->pw[i];
        if (pw > (1024 - 64 - 16))
          pw = (1024 - 64 - 16);
        if (ow < zl)
          ow = zl < pw ? zl : pw;
        s->S[i] += rsum(ls, wp, ow, pw < age ? pw : age);
        s->S[i] -= rsum(ls, wp, pw, ow < age ? ow : age);
        s->pw[i] = (int16_t)pw;
        int32_t env = s->env[i], dg;
        if (latch && (follow || s->tgt[i]))
          dg = s->dgl[i];
        else {
          int32_t x = (int32_t)(((a27 >> 3) * ((uint32_t)P >> 10)) >> 6);
          int32_t x15 = x >> 9, x2 = (x15 * x15) >> 15, x3 = (x2 * x15) >> 15,
                  x4 = (x3 * x15) >> 15;
          dg = -x +
               ((x2 >> 1) - ((x3 * 5461) >> 15) + ((x4 * 1365) >> 15)) * 512;
        }
        if (env > 14000)
          dg -= (env - 14000) << 6;
        else if (env < 256 && s->ein < 64 && !(latch && s->tgt[i]) &&
                 dg > -(1 << 21))
          dg = -(1 << 21);
        if (dg < -(1 << 23))
          dg = -(1 << 23);
        else if (dg > (1 << 20))
          dg = 1 << 20;
        int32_t G = (1 << 24) + dg;
        int32_t gb = (int32_t)((((uint32_t)G >> 8) * (uint32_t)bq) >> 7),
                ga = G - gb;
        gb >>= 5;
        ga >>= 5;
        s->t24[i][0] = (ga * ir) >> 6;
        s->t24[i][1] = (ga * fr + gb * ir) >> 6;
        s->t24[i][2] = (gb * fr) >> 6;
      }
      int16_t c[3];
      for (int k = 0; k < 3; k++) {
        int32_t v = s->t24[i][k] + s->q[i][k], cq = (v + 256) >> 9;
        if (cq > 32767)
          cq = 32767;
        s->q[i][k] = v - cq * 512;
        c[k] = (int16_t)cq;
      }
      int32_t S = s->S[i], pw = s->pw[i];
      int32_t Sx = S + ((ls[(wp - 1 - pw) & (1024 - 1)] *
                         (int32_t)((s->P[i] >> 1) & 0x7fff)) >>
                        15);
      int32_t dcs = ((Sx >> 6) * s->kq[i]) * 4;
      dcs = dcs > (1 << 30) ? (1 << 30) : (dcs < -(1 << 30) ? -(1 << 30) : dcs);
      const int32_t k0 = 0x8000 - dcs;
      int16_t *op = ls + wp;
      if (pw) {
        int32_t rb = wp - s->di[i], dd = pw - s->di[i];
        const int16_t *rp = ls + rb, *ep = ex, *const ee = ex + 64;
        const int16_t c0 = c[0], c1 = c[1], c2 = c[2];
        int n = 64;
        if (rb < 0) {
          rp += 1024;
          if (-rb < n)
            n = -rb;
        }
        for (;;) {
          const int16_t *od = rp - dd;
          for (int j = 0; j < n; j++) {
            int32_t z = ADDF(ADDF(ADDF(MULF(rp[0], c0), k0), MULF(rp[-1], c1)),
                             MULF(rp[-2], c2)) >>
                        16;
            rp++;
            int32_t y = ADDQ(z, *ep++);
            S += y - *od++;
            *op++ = (int16_t)y;
          }
          if (ep == ee)
            break;
          rp = ls;
          n = (int)(ee - ep);
        }
      } else
        for (int n = 0; n < 64; n++) {
          op[n] = ex[n];
          S += ex[n];
        }
      s->S[i] = S;
    }
    if (wp == 1024 - 64)
      for (int i = 0; i < 12; i++) {
        int16_t *ls = L + i * (48 + 1024);
        ls[-1] = ls[1024 - 1];
        ls[-2] = ls[1024 - 2];
      }
    if (exo)
      s->czm = 0;
    s->wp = (wp + 64) & (1024 - 1);
    s->age = age + 64 < 1024 ? age + 64 : 1024;
    int32_t dry = k3 < 16384 ? 32767 : (32767 - k3) * 2;
    int32_t wet = k3 > 16384 ? 32767 : k3 * 2;
    int16_t kd = (int16_t)(6999 + ((mg * 6999) >> 15));
    int16_t mk = (int16_t)(32767 - ((mg * 10923) >> 15));
    int32_t ow = s->ow;
    int16_t dryg = (int16_t)dry, wetg = (int16_t)wet;
    (void)dryg;
    int32_t peak = 0, fbsum = 0;
    const int16_t *ob = L + wp;
    for (int n = 0; n < 64; n++, ob++) {
      int32_t sum =
          ob[0] + ob[(48 + 1024)] + ob[2 * (48 + 1024)] + ob[3 * (48 + 1024)] +
          ob[4 * (48 + 1024)] + ob[5 * (48 + 1024)] + ob[6 * (48 + 1024)] +
          ob[7 * (48 + 1024)] + ob[8 * (48 + 1024)] + ob[9 * (48 + 1024)] +
          ob[10 * (48 + 1024)] + ob[11 * (48 + 1024)];
      int32_t v = (sum - ex[n] * 12) >> 5;
      int32_t d = MUL16(v, kd) >> 10;
      d = d > 32767 ? 32767 : (d < -32767 ? -32767 : d);
      int32_t c = softclip16((int16_t)d), r = c - d;
      s->fb[n] = r;
      fbsum += r;
      v = MULQ15(c + (c >> 1), mk);
      ow += v - (ow >> 10);
      v = SUBQ(v, ow >> 10);
      int32_t o = MULQ15(v, wetg);
      buf[n] = (int16_t)o;
      if (o < 0)
        o = -o;
      if (o > peak)
        peak = o;
    }
    s->fbsum = fbsum;
    s->ow = ow;
    volatile int16_t *m = (&meter);
    int32_t mv = (*m * 0xfe00) >> 16;
    *m = (int16_t)(peak > mv ? peak : mv);
    s_in[pos % 3] = (int16_t)(s->ein > 32767 ? 32767 : s->ein);
  }
};
} // namespace port
} // namespace hkpalme
