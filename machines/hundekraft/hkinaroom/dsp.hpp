#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkinaroom {
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

enum { S_IDLE = 0, S_REC, S_ERODE };

static const uint8_t sync_lq[6] = {1, 2, 4, 8, 16, 32};

typedef struct {
  int32_t dim[3];
  int32_t f[6];
  uint8_t ax[6];
  uint8_t n[6];
  uint8_t deg[6];
  uint8_t shape;
} room_t;

static const int32_t shape_ratio[4][2] = {
    {32767, 32767},
    {20252, 12517},
    {25967, 20608},
    {23098, 13966},
};

typedef struct {
  int32_t lp, bp, f, g;
} res_t;

typedef struct {
  res_t res[6];
  int32_t q15, wroom, wdir, a15;
  int32_t tap[3];
  int32_t dcx, dcy, z, dcs;
  int32_t len, i, gen, mode, latch;
  int32_t gq12, gm, rs;
  int32_t gmo, rso, gk;
  int32_t gap;
  int32_t peak, ipeak, cpeak;
  int32_t pw, x, ph, dsh, lw;
  int32_t e1, e2, e3, f1, f2, f3;
  int32_t fade, fmul;
  int32_t xf, xfw;
  int32_t xl, xs;
  int32_t rgw, r1, r2, r3;
  int32_t goff;
  int16_t inb[64 + 3];
  int32_t erw, erc;
  int32_t dkv, dkn;
  int32_t rb, ba, cb, bhi, blo;
  int32_t kroom, kero, kabs, kdsh;
  int32_t env[6];
  int32_t lq, qleft, armed;
  int32_t bp, blen, tape, tbeat;
  int32_t qbar;
  int32_t plen;
  room_t room;
} sit_t;

typedef struct {
  int32_t t0, t1, t2, q, wd, wr, ab, rb, len, dcs, fr;
} cst_t;

typedef struct {
  int32_t i, erw, dcx, dcy, z, pk, ipk, ba, cb, bhi, blo, gm, rsh, gmo, rso, gk,
      gap, xl, xs;
} hot_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  sit_t st[3];
  int8_t wave_hi[3][128], wave_lo[3][128];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int sync_sel(int32_t w5) {
    return w5 < 13107 ? -1 : (int)(((unsigned)(w5 - 13107) >> 9) % 6);
  }

  inline int32_t free_len(int32_t w5) {
    int32_t n = 11025 + ((w5 * 46876) >> 13);
    return n > 84096 ? 84096 : n;
  }

  int32_t exp2_q15(int32_t x) {
    int32_t i = x >> 15, f = x & 32767;
    int32_t m = 32768 + ((f * 21512) >> 15) + ((((f * f) >> 15) * 11256) >> 15);
    return m << i;
  }

  unsigned room_shape(int32_t q) {
    return ((unsigned)(q < 0 ? 0 : q) >> 9) % 4u;
  }

  void room_calc(room_t *r, int32_t ksize, int32_t kshape) {
    int sh = (int)room_shape(kshape);
    r->shape = (uint8_t)sh;
    int32_t L = (800 * exp2_q15((ksize * 54426) >> 14)) >> 15;
    r->dim[0] = L;
    r->dim[1] = (L * shape_ratio[sh][0]) >> 15;
    r->dim[2] = (L * shape_ratio[sh][1]) >> 15;
    int32_t f1[3], n[3], cur[3];
    for (int d = 0; d < 3; d++) {
      f1[d] = 43904000 / r->dim[d];
      n[d] = ((110 << 8) + f1[d] - 1) / f1[d];
      cur[d] = n[d] * f1[d];
    }
    int cnt = 0;
    for (int it = 0; it < 24 && cnt < 6; it++) {
      int d = cur[0] <= cur[1] ? 0 : 1;
      if (cur[2] < cur[d])
        d = 2;
      int32_t f = cur[d];
      if (cnt && f <= r->f[cnt - 1] + (r->f[cnt - 1] >> 7)) {
        r->deg[cnt - 1]++;
        r->ax[cnt - 1] |= (uint8_t)(1 << d);
      } else {
        r->f[cnt] = f;
        r->ax[cnt] = (uint8_t)(1 << d);
        r->n[cnt] = (uint8_t)n[d];
        r->deg[cnt] = 1;
        cnt++;
      }
      n[d]++;
      cur[d] += f1[d];
    }
  }

  int32_t svf_coef(int32_t f_q8) {
    if (f_q8 > (3400 << 8))
      f_q8 = 3400 << 8;
    int64_t t = ((int64_t)f_q8 * 2447717) >> 12;
    int64_t t2 = (t * t) >> 30, t3 = (t2 * t) >> 30, t5 = (t3 * t2) >> 30;
    int32_t s = (int32_t)t - (int32_t)t3 / 6 + (int32_t)t5 / 120;
    return (s + 8192) >> 14;
  }

  inline int32_t m16(int32_t a, int32_t b) {
    return (int32_t)(int16_t)a * (int32_t)(int16_t)b;
  }

  inline int32_t mq15(int32_t a, int32_t b) {
    return m16(a >> 15, b) + (m16(a & 0x7fff, b) >> 15);
  }

  inline int32_t clampi(int32_t x, int32_t m) {
    return x > m ? m : (x < -m ? -m : x);
  }

  void set_gain(sit_t *s, int32_t g) {
    if (g < 256)
      g = 256;
    else if (g > 262144)
      g = 262144;
    s->gq12 = g;
    int32_t m = g << 3, e = 0;
    while (m > 32767) {
      m >>= 1;
      e++;
    }
    while (m < 16384) {
      m <<= 1;
      e--;
    }
    s->gm = m;
    s->rs = 11 - e;
  }

  void wave_clear(int pos) {
    for (int b = 0; b < 128; b++)
      wave_hi[pos][b] = wave_lo[pos][b] = 0;
  }

  void sit_init(int pos) {
    sit_t *s = &st[pos % 3];
    s->mode = S_IDLE;
    s->latch = fx_page[pos % 3][3];
    s->gen = 0;
    s->i = 0;
    s->len = s->plen = 11025;
    s->dkn = 0;
    s->pw = s->x = s->ph = s->lw = 0;
    s->dsh = 1;
    s->e1 = s->e2 = s->e3 = s->f1 = s->f2 = s->f3 = 0;
    s->xf = s->xl = 0;
    s->rgw = s->r1 = s->r2 = s->r3 = 0;
    s->lq = s->armed = 0;
    s->qbar = -1;
    s->bp = 0;
    s->blen = 22050 << 14;
    s->tape = 0;
    s->tbeat = -1;
    s->kroom = s->kero = s->kabs = s->kdsh = -1;
    for (int k = 0; k < 6; k++)
      s->env[k] = 0;
    set_gain(s, 4096);
    s->gk = 0;
    wave_clear(pos % 3);
  }

  int32_t sync_fit(int32_t blen, int32_t *lq, int32_t *dsh) {
    int32_t spb = blen >> 14, lout = (spb * *lq) >> 2;
    while (*lq > 1 && (lout >> 2) + (lout >> 6) + 64 > 84096) {
      *lq >>= 1;
      lout = (spb * *lq) >> 2;
    }
    *dsh = (lout >> 1) + (lout >> 5) + 64 <= 84096 ? 1 : 2;
    return lout;
  }

  void start_capture(sit_t *s, int pos, int32_t w5, int32_t lq, int32_t w7) {
    s->dkv = s->lw;
    s->dkn = 256;
    s->mode = S_REC;
    s->i = 0;
    s->gen = 0;
    s->cpeak = 0;
    s->ph = 0;
    s->pw = s->x = 0;
    s->erc = 0;
    if (!lq) {
      s->dsh = 1;
      s->len = s->plen = free_len(w5);
      s->fade = 220;
      s->fmul = 149;
    } else {
      int32_t dsh, lout = sync_fit(s->blen, &lq, &dsh);
      int32_t cap = (lout >> dsh) + (lout >> (dsh + 4)) + 64;
      s->dsh = dsh;
      s->len = cap > 84096 ? 84096 : cap;
      s->plen = lout >> dsh;
      s->fade = 32 >> (dsh - 1);
      s->fmul = 32768 / s->fade;
      s->qleft = lq;
    }
    s->lq = lq;
    s->xf = ((w7 < 0 ? 0 : w7) * (2048 - 1) >> 15) >> (s->dsh - 1);
    s->xfw = (s->rgw - ((64 - s->goff) >> 1)) & (2048 - 1);
    s->e1 = s->inb[s->goff + 2];
    s->e2 = s->inb[s->goff + 1];
    s->e3 = s->inb[s->goff];
    const int16_t *ring = (const int16_t *)scratch + 84096 + 2048;
    s->f1 = ring[(s->xfw - 1) & (2048 - 1)];
    s->f2 = ring[(s->xfw - 2) & (2048 - 1)];
    s->f3 = ring[(s->xfw - 3) & (2048 - 1)];
    if (s->xf)
      s->fade = 0;
    s->rb = (128 << 16) / s->plen;
    s->cb = 0;
    s->ba = 0;
    s->bhi = s->blo = 0;
    wave_clear(pos);
  }

  void start_erode(sit_t *s, int16_t *er) {
    for (int k = s->erc; k < 2048; k++)
      er[k] = 0;
    s->erc = 2048;
    for (int k = 0; k < 6; k++)
      s->res[k].lp = s->res[k].bp = 0;
    s->dcx = s->dcy = s->z = 0;
    s->erw = 0;
    s->mode = S_ERODE;
    s->i = 0;
    s->gen = 0;
    s->peak = 0;
    s->ipeak = 0;
    s->gk = 0;
    s->gap = 0;
    if (s->xf == 0)
      s->xl = 0;
    s->ph = 0;
    s->pw = s->x;
    s->cb = 0;
    s->ba = 0;
    s->bhi = s->blo = 0;
    int32_t e2 =
        (s->kero < 0 ? 16384 : s->kero) * (s->kero < 0 ? 16384 : s->kero) >> 15;
    set_gain(s, s->cpeak < 64 ? 4096
                              : ((((16384 << 12) / s->cpeak) >> 4) *
                                 (4096 + (e2 * 3 >> 3))) >>
                                    8);
  }

  void next_generation(sit_t *s) {
    s->gen++;
    if (s->peak >= 16 && s->ipeak >= 16) {
      int32_t pk = s->peak > 0x7ffff ? 0x7ffff : s->peak;
      int32_t r1 = (16384 << 12) / pk, r2 = (s->ipeak << 12) / pk;
      r1 = r1 < 1024 ? 1024 : (r1 > 16384 ? 16384 : r1);
      r2 = r2 < 1024 ? 1024 : (r2 > 16384 ? 16384 : r2);
      int32_t g = ((s->gq12 >> 2) * r1) >> 10;
      g = g > 262144 ? 262144 : g;
      set_gain(s, ((g >> 2) * r2) >> 10);
    }
    s->peak = 0;
    s->ipeak = 0;
  }

  void update_coefs(sit_t *s, int32_t kroom, int32_t kshape, int32_t kero,
                    int32_t kabs) {
    const int32_t sh = s->dsh - 1;
    if (s->kdsh != s->dsh) {
      s->kdsh = s->dsh;
      s->kroom = s->kabs = -1;
      s->dcs = 7 - sh;
    }
    int32_t key = kroom | (room_shape(kshape) << 16);
    if (key != s->kroom) {
      s->kroom = key;
      room_calc(&s->room, kroom, kshape);
      for (int k = 0; k < 6; k++) {
        s->res[k].f = svf_coef(s->room.f[k] << sh);
        s->res[k].g = 19661 + 6553 * (s->room.deg[k] - 1);
      }
      for (int d = 0; d < 3; d++) {
        int32_t t = (s->room.dim[d] * 8436) >> (16 + sh);
        s->tap[d] = t < 1 ? 1 : (t > 2048 - 1 ? 2048 - 1 : t);
      }
    }
    if (kero != s->kero) {
      s->kero = kero;
      int32_t qq15 = 8 * exp2_q15((kero * 54426) >> 14);
      s->q15 = (int32_t)(0x40000000u / (uint32_t)qq15);
      s->wroom = 3932 + ((kero * 28835) >> 15);
      s->wdir = 32767 - s->wroom;
    }
    if (kabs != s->kabs) {
      s->kabs = kabs;
      s->a15 = 32767 - ((kabs * 22500) >> 15);
      if (sh) {
        int32_t a = 2 * s->a15 - ((s->a15 * s->a15) >> 15);
        s->a15 = a > 32767 ? 32767 : a;
      }
    }
  }

  inline void bucket_flush(int pos, int32_t cb, int32_t hi, int32_t lo) {
    if (cb < 128) {
      wave_hi[pos][cb] = (int8_t)(hi >> 8);
      wave_lo[pos][cb] = (int8_t)(lo >> 8);
    }
  }

  inline __attribute__((always_inline)) void hot_load(sit_t *s, cst_t *c,
                                                      hot_t *h) {
    c->t0 = s->tap[0];
    c->t1 = s->tap[1];
    c->t2 = s->tap[2];
    c->q = s->q15;
    c->wd = s->wdir;
    c->wr = s->wroom;
    c->ab = s->a15;
    c->rb = s->rb;
    c->len = s->len;
    c->dcs = s->dcs;
    c->fr = !s->lq;
    h->i = s->i;
    h->erw = s->erw;
    h->dcx = s->dcx;
    h->dcy = s->dcy;
    h->z = s->z;
    h->pk = s->peak;
    h->ipk = s->ipeak;
    h->ba = s->ba;
    h->cb = s->cb;
    h->bhi = s->bhi;
    h->blo = s->blo;
    h->gm = s->gm;
    h->rsh = s->rs;
    h->gmo = s->gmo;
    h->rso = s->rso;
    h->gk = s->gk;
    h->gap = s->gap;
    h->xl = s->xl;
    h->xs = s->xs;
  }

  inline __attribute__((always_inline)) void hot_store(sit_t *s,
                                                       const hot_t *h) {
    s->i = h->i;
    s->erw = h->erw;
    s->dcx = h->dcx;
    s->dcy = h->dcy;
    s->z = h->z;
    s->peak = h->pk;
    s->ipeak = h->ipk;
    s->ba = h->ba;
    s->cb = h->cb;
    s->bhi = h->bhi;
    s->blo = h->blo;
    s->gk = h->gk;
    s->gap = h->gap;
    s->xl = h->xl;
  }

  inline __attribute__((always_inline)) int32_t erode1(sit_t *s, int pos,
                                                       int16_t *loop,
                                                       int16_t *er,
                                                       const cst_t *c, hot_t *h,
                                                       int32_t prev) {
    if (h->i >= c->len) {
      h->gap = 1;
      return (prev * 3) >> 2;
    }
    int32_t x = loop[h->i];
    int32_t ax = x < 0 ? -x : x;
    if (ax > h->ipk)
      h->ipk = ax;
    int32_t erw = h->erw;
    er[erw] = (int16_t)x;
    int32_t t = er[(erw - c->t0) & (2048 - 1)] +
                er[(erw - c->t1) & (2048 - 1)] + er[(erw - c->t2) & (2048 - 1)];
    h->erw = (erw + 1) & (2048 - 1);
    int32_t e = x * 8 + t * 2 + (t >> 1) + (t >> 2) + (t >> 3);
    int32_t room = 0;
    res_t *r = s->res;
    for (int k = 0; k < 6; k++, r++) {
      int32_t bp = r->bp, f = r->f, lp = r->lp + mq15(bp, f);
      int32_t qb = mq15(bp, c->q);
      r->lp = lp;
      r->bp = clampi(bp + mq15(e - lp - qb, f), 1 << 26);
      room += mq15(qb, r->g);
    }
    int32_t y = mq15(e, c->wd) + mq15(clampi(room, 1 << 22), c->wr);
    h->dcy = y - h->dcx + h->dcy - (h->dcy >> c->dcs);
    h->dcx = y;
    h->z += mq15(h->dcy - h->z, c->ab);
    int32_t zc = clampi(h->z, (1 << 21) - 1);
    int32_t o = (mq15(zc, h->gm) * 256) >> h->rsh;
    if (h->gk) {
      int32_t oo = (mq15(zc, h->gmo) * 256) >> h->rso;
      o += (clampi(oo - o, (1 << 22) - 1) * h->gk) >> 8;
      h->gk--;
    }
    if (h->xl > 0) {
      int32_t t = (h->i + 1) * h->xs;
      o = (x * (32768 - t) + clampi(o, 65535) * t) >> 15;
      h->xl--;
    }
    int32_t a = o < 0 ? -o : o;
    if (a > h->pk)
      h->pk = a;
    if (a > 24576) {
      int32_t d = a - 24576;
      a = d < 16384 ? 24576 + (d >> 2) : 28672 + ((d - 16384) >> 4);
      o = o < 0 ? -a : a;
    }
    o = op1_sat16(o);
    loop[h->i] = (int16_t)o;
    int32_t b = h->ba >> 16;
    h->ba += c->rb;
    if (b != h->cb) {
      bucket_flush(pos, h->cb, h->bhi, h->blo);
      h->cb = b;
      h->bhi = h->blo = 0;
    }
    if (o > h->bhi)
      h->bhi = o;
    else if (o < h->blo)
      h->blo = o;
    if (++h->i >= c->len && c->fr) {
      bucket_flush(pos, h->cb, h->bhi, h->blo);
      h->i = 0;
      h->ba = 0;
      h->cb = 0;
      h->bhi = h->blo = 0;
      s->peak = h->pk;
      s->ipeak = h->ipk;
      h->gmo = h->gm;
      h->rso = h->rsh;
      h->gk = (1 << 8);
      next_generation(s);
      h->pk = h->ipk = 0;
      h->gm = s->gm;
      h->rsh = s->rs;
    }
    return x;
  }

  inline int32_t wet(int16_t *buf, int n, int32_t v, int32_t mix) {
    int32_t in = buf[n];
    (void)in;
    int32_t o = op1_sat16(v);
    buf[n] = (int16_t)o;
    return o < 0 ? -o : o;
  }

  int32_t erode_pairs(sit_t *s, int pos, int16_t *loop, int16_t *er,
                      int16_t *buf, int *pn, int end, int32_t mix) {
    cst_t c;
    hot_t h;
    hot_load(s, &c, &h);
    int32_t pw = s->pw, x = s->x, dkv = s->dkv, dkn = s->dkn, opk = 0;
    int n = *pn;
    for (; n + 1 < end; n += 2) {
      x = erode1(s, pos, loop, er, &c, &h, x);
      int32_t v0 = (pw + x) >> 1, v1 = x;
      pw = x;
      if (dkn) {
        v0 += m16(dkv, dkn) >> 8;
        dkn--;
        v1 += m16(dkv, dkn) >> 8;
        if (dkn)
          dkn--;
      }
      int32_t o0 = wet(buf, n, v0, mix), o1 = wet(buf, n + 1, v1, mix);
      if (o0 > opk)
        opk = o0;
      if (o1 > opk)
        opk = o1;
    }
    hot_store(s, &h);
    s->pw = s->x = s->lw = x;
    s->dkn = dkn;
    *pn = n;
    return opk;
  }

  int32_t erode_run(sit_t *s, int pos, int16_t *loop, int16_t *er, int16_t *buf,
                    int n, int end, int32_t mix) {
    int32_t opk = 0;
    if (s->dsh == 1 && !s->ph)
      opk = erode_pairs(s, pos, loop, er, buf, &n, end, mix);
    if (n >= end)
      return opk;
    cst_t c;
    hot_t h;
    hot_load(s, &c, &h);
    const int32_t dsh = s->dsh, dm = (1 << dsh) - 1;
    int32_t pw = s->pw, x = s->x, ph = s->ph, dkv = s->dkv, dkn = s->dkn,
            w = s->lw;
    for (; n < end; n++) {
      if (!ph)
        x = erode1(s, pos, loop, er, &c, &h, x);
      w = pw + (((x - pw) * (ph + 1)) >> dsh);
      if (ph == dm) {
        ph = 0;
        pw = x;
      } else
        ph++;
      int32_t v = w;
      if (dkn) {
        v += m16(dkv, dkn) >> 8;
        dkn--;
      }
      int32_t o = wet(buf, n, v, mix);
      if (o > opk)
        opk = o;
    }
    hot_store(s, &h);
    s->pw = pw;
    s->x = x;
    s->ph = ph;
    s->dkn = dkn;
    s->lw = w;
    return opk;
  }

  void xfade_bake(sit_t *s, int16_t *loop, const int16_t *ring, int32_t len) {
    int32_t x = s->xf < len / 2 ? s->xf : len / 2;
    s->xl = 0;
    if (x <= 0)
      return;
    int32_t step = 32768 / (x + 1), d = s->dsh - 1;
    s->xl = x;
    s->xs = step;
    for (int32_t k = 0; k < x; k++) {
      int32_t t = (k + 1) * step, i = len - x + k;
      int32_t r = (s->xfw - ((x - k) << d)) & (2048 - 1);
      int32_t pre = d ? (ring[r] + ring[(r + 1) & (2048 - 1)]) >> 1 : ring[r];
      loop[i] = (int16_t)((loop[i] * (32768 - t) + pre * t) >> 15);
    }
  }

  void capture_sample(sit_t *s, int pos, int16_t *loop, int16_t *er,
                      int32_t x) {
    int32_t a = x < 0 ? -x : x, i = s->i;
    if (a > s->cpeak)
      s->cpeak = a;
    if (s->erc < 2048) {
      int16_t *e = er + s->erc;
      e[0] = e[1] = e[2] = e[3] = 0;
      s->erc += 4;
    }
    if (i < s->len) {
      int32_t j = s->len - 1 - i;
      if (i < s->fade)
        x = (x * i * s->fmul) >> 15;
      else if (!s->lq && j < s->fade)
        x = (x * j * s->fmul) >> 15;
      loop[i] = (int16_t)x;
      int32_t b = s->ba >> 16;
      s->ba += s->rb;
      if (b != s->cb) {
        bucket_flush(pos, s->cb, s->bhi, s->blo);
        s->cb = b;
        s->bhi = s->blo = 0;
      }
      if (x > s->bhi)
        s->bhi = x;
      else if (x < s->blo)
        s->blo = x;
      s->i = i + 1;
    } else
      x = 0;
    s->pw = s->x;
    s->x = x;
    if (s->i >= s->len && !s->lq) {
      bucket_flush(pos, s->cb, s->bhi, s->blo);
      xfade_bake(s, loop, er + 2048, s->len);
      start_erode(s, er);
    }
  }

  int32_t rec_run(sit_t *s, int pos, int16_t *loop, int16_t *er, int16_t *buf,
                  int n, int end, int32_t mix) {
    int32_t opk = 0;
    for (; n < end; n++) {
      if (s->mode == S_ERODE) {
        int32_t pk = erode_run(s, pos, loop, er, buf, n, end, mix);
        return pk > opk ? pk : opk;
      }
      int32_t in = buf[n], ph = s->ph;
      int32_t w = s->pw + (((s->x - s->pw) * (ph + 1)) >> s->dsh);
      s->lw = w;
      if (s->mode == S_REC) {
        if (ph & 1) {
          int32_t y = (s->e3 + 3 * s->e2 + 3 * s->e1 + in) >> 3;
          if (s->dsh == 1)
            capture_sample(s, pos, loop, er, y);
          else {
            if (ph == 3)
              capture_sample(s, pos, loop, er,
                             (s->f3 + 3 * s->f2 + 3 * s->f1 + y) >> 3);
            s->f3 = s->f2;
            s->f2 = s->f1;
            s->f1 = y;
          }
        }
        s->e3 = s->e2;
        s->e2 = s->e1;
        s->e1 = in;
        if (s->mode == S_REC)
          s->ph = ph == (1 << s->dsh) - 1 ? 0 : ph + 1;
      }
      if (s->dkn > 0) {
        w += m16(s->dkv, s->dkn) >> 8;
        s->dkn--;
      }
      int32_t o = op1_sat16(w);
      buf[n] = (int16_t)o;
      if (o < 0)
        o = -o;
      if (o > opk)
        opk = o;
    }
    return opk;
  }

  int32_t run(sit_t *s, int pos, int16_t *loop, int16_t *er, int16_t *buf,
              int n, int end, int32_t mix) {
    if (n >= end)
      return 0;
    return s->mode == S_ERODE ? erode_run(s, pos, loop, er, buf, n, end, mix)
                              : rec_run(s, pos, loop, er, buf, n, end, mix);
  }

  int beat_grid(sit_t *s, int32_t *qpos) {
    op1_beat_t c;
    (*(&c) = beat);
    int32_t len = c.len, p, step;
    int moving = len >= (256 << 14) && c.step > 0 && c.step <= 0x400000 &&
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
    int32_t np = p - step;
    s->bp = np < 0 ? np + len : np;
    int32_t sq = step >> 14, bl = sq > 0 && sq != 64 ? len / sq : len >> 6;
    s->blen = bl > 0x1ffffff ? 0x7fffffff : bl << 6;
    s->tape = moving && c.follows_tape;
    int32_t qq = len >> 2;
    for (int j = 3; j >= 0; j--) {
      int32_t t = p - j * qq;
      if (t < 0 || t >= step)
        continue;
      int off = step == 0x100000 ? t >> 14 : (t * 64) / step;
      *qpos = (4 - j) & 3;
      s->tbeat = -1;
      if (!*qpos && s->tape) {
        int32_t spb = len >> 14;
        s->tbeat = (c.tape_pos + off + (spb >> 1)) / spb;
      }
      return off;
    }
    return -1;
  }

  void grid_event(sit_t *s, int pos, int16_t *loop, int16_t *er, int32_t qpos,
                  int32_t w5, int32_t w7) {
    if (!qpos && s->tbeat >= 0)
      s->qbar = (s->tbeat & 3) << 2;
    else if (s->qbar >= 0)
      s->qbar = ((s->qbar & 12) + (qpos ? qpos : 4)) & 15;
    if (s->armed) {
      int sel = sync_sel(w5);
      int32_t lq = sel < 0 ? 0 : sync_lq[sel], a = lq > 16 ? 16 : lq;
      if (lq && s->qbar < 0 && !qpos)
        s->qbar = 0;
      if (!lq || (a <= 4 ? (qpos & (a - 1)) == 0
                         : s->qbar >= 0 && (s->qbar & (a - 1)) == 0)) {
        s->armed = 0;
        start_capture(s, pos, w5, lq, w7);
        return;
      }
    }
    if (!s->lq || --s->qleft > 0)
      return;
    s->qleft = s->lq;
    if (s->mode == S_REC) {
      int32_t n = s->i;
      if (n < 64) {
        for (int32_t k = n; k < 64; k++)
          loop[k] = 0;
        n = 64;
      }
      s->len = s->plen = n;
      int32_t f = s->fade < n / 2 ? s->fade : n / 2;
      for (int32_t j = 0; j < f; j++)
        loop[n - 1 - j] = (int16_t)((loop[n - 1 - j] * j * s->fmul) >> 15);
      xfade_bake(s, loop, er + 2048, n);
      bucket_flush(pos, s->cb, s->bhi, s->blo);
      s->rb = (128 << 16) / n;
      start_erode(s, er);
    } else if (s->mode == S_ERODE) {
      bucket_flush(pos, s->cb, s->bhi, s->blo);
      s->gmo = s->gm;
      s->rso = s->rs;
      s->gk = (1 << 8);
      next_generation(s);
      int exact = s->i >= s->len && !s->gap;
      s->gap = 0;
      s->i = 0;
      s->ph = 0;
      s->ba = 0;
      s->cb = 0;
      s->bhi = s->blo = 0;
      if (exact)
        s->pw = s->x;
      else {
        int32_t x0 = loop[0];
        s->dkv = clampi(s->lw - x0, 32767);
        s->dkn = 256;
        s->pw = s->x = x0;
      }
    }
  }

  void sit_render(int pos, int16_t *buf, const int16_t *p) {
    pos %= 3;
    sit_t *s = &st[pos];
    int16_t *loop = (int16_t *)scratch;
    if (!loop)
      return;
    int16_t *er = loop + 84096;
    int32_t mix = ((int32_t)(p)[3]), w5 = p[5] < 0 ? 0 : p[5];
    update_coefs(s, ((int32_t)(p)[0]), ((int32_t)(p)[1]), ((int32_t)(p)[2]),
                 p[6] < 0 ? 0 : p[6]);
    int l = fx_page[pos][3] != 0;
    if (l != s->latch) {
      s->latch = l;
      if (s->armed)
        s->armed = 0;
      else if (sync_sel(w5) < 0) {
        s->goff = 64;
        start_capture(s, pos, w5, 0, p[7]);
      } else
        s->armed = 1;
    }
    s->inb[0] = s->inb[64];
    s->inb[1] = s->inb[64 + 1];
    s->inb[2] = s->inb[64 + 2];
    for (int n = 0; n < 64; n++)
      s->inb[n + 3] = buf[n];
    if (s->mode != S_REC) {
      int16_t *ring = er + 2048;
      for (int n = 1; n < 64; n += 2) {
        int32_t a = buf[n - 1], b = buf[n];
        ring[s->rgw] = (int16_t)((s->r2 + 3 * s->r1 + 3 * a + b) >> 3);
        s->rgw = (s->rgw + 1) & (2048 - 1);
        s->r2 = a;
        s->r1 = b;
      }
    }
    int32_t qpos = 0;
    int off = beat_grid(s, &qpos);
    int32_t peak = run(s, pos, loop, er, buf, 0, off < 0 ? 64 : off, mix);
    if (off >= 0) {
      s->goff = off;
      grid_event(s, pos, loop, er, qpos, w5, p[7]);
      int32_t pk = run(s, pos, loop, er, buf, off, 64, mix);
      if (pk > peak)
        peak = pk;
    }
    if (s->mode == S_ERODE)
      for (int k = 0; k < 6; k++) {
        int32_t bp = s->res[k].bp, lp = s->res[k].lp;
        int32_t a =
            mq15((bp < 0 ? -bp : bp) + (lp < 0 ? -lp : lp), s->q15) >> 3;
        int32_t d = s->env[k] - (s->env[k] >> 5);
        s->env[k] = a > d ? a : d;
      }
    volatile int16_t *m = (&meter);
    int32_t mv = (*m * 0xfe00) >> 16;
    *m = (int16_t)(peak > mv ? peak : mv);
  }
};
} // namespace port
} // namespace hkinaroom
