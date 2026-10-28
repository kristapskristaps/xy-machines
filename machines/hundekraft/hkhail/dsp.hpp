#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkhail {
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

static const uint32_t bd_exp2[257] = {
    0,     178,   356,   535,   714,   893,   1073,  1254,  1435,  1617,  1799,
    1981,  2164,  2348,  2532,  2716,  2902,  3087,  3273,  3460,  3647,  3834,
    4022,  4211,  4400,  4590,  4780,  4971,  5162,  5353,  5546,  5738,  5932,
    6125,  6320,  6514,  6710,  6906,  7102,  7299,  7496,  7694,  7893,  8092,
    8292,  8492,  8693,  8894,  9096,  9298,  9501,  9704,  9908,  10113, 10318,
    10524, 10730, 10937, 11144, 11352, 11560, 11769, 11979, 12189, 12400, 12611,
    12823, 13036, 13249, 13462, 13676, 13891, 14106, 14322, 14539, 14756, 14974,
    15192, 15411, 15630, 15850, 16071, 16292, 16514, 16737, 16960, 17183, 17408,
    17633, 17858, 18084, 18311, 18538, 18766, 18995, 19224, 19454, 19684, 19915,
    20147, 20379, 20612, 20846, 21080, 21315, 21550, 21786, 22023, 22260, 22498,
    22737, 22977, 23216, 23457, 23698, 23940, 24183, 24426, 24670, 24915, 25160,
    25406, 25652, 25900, 26148, 26396, 26645, 26895, 27146, 27397, 27649, 27902,
    28155, 28409, 28664, 28919, 29175, 29432, 29690, 29948, 30207, 30466, 30727,
    30988, 31249, 31512, 31775, 32039, 32303, 32568, 32834, 33101, 33369, 33637,
    33906, 34175, 34446, 34717, 34988, 35261, 35534, 35808, 36083, 36359, 36635,
    36912, 37190, 37468, 37747, 38028, 38308, 38590, 38872, 39155, 39439, 39724,
    40009, 40295, 40582, 40870, 41158, 41448, 41738, 42029, 42320, 42613, 42906,
    43200, 43495, 43790, 44087, 44384, 44682, 44981, 45280, 45581, 45882, 46184,
    46487, 46791, 47095, 47401, 47707, 48014, 48322, 48631, 48940, 49251, 49562,
    49874, 50187, 50500, 50815, 51131, 51447, 51764, 52082, 52401, 52721, 53041,
    53363, 53685, 54008, 54333, 54658, 54983, 55310, 55638, 55966, 56296, 56626,
    56957, 57289, 57622, 57956, 58291, 58627, 58964, 59301, 59640, 59979, 60319,
    60661, 61003, 61346, 61690, 62035, 62381, 62727, 63075, 63424, 63774, 64124,
    64476, 64828, 65182, 65536};

static const uint16_t bd_rand_ratio[256] = {
    38169, 36823, 35565, 34385, 33276, 32233, 31250, 30321, 29442, 28610, 27820,
    27070, 26357, 25677, 25029, 24410, 23819, 23254, 22712, 22194, 21696, 21218,
    20759, 20318, 19893, 19484, 19089, 18709, 18342, 17988, 17646, 17315, 16995,
    16685, 16385, 16094, 15813, 15540, 15275, 15017, 14767, 14525, 14289, 14060,
    13836, 13619, 13408, 13202, 13002, 12807, 12616, 12431, 12250, 12073, 11900,
    11732, 11568, 11407, 11250, 11097, 10947, 10800, 10657, 10517, 10380, 10245,
    10114, 9985,  9859,  9735,  9614,  9495,  9379,  9265,  9153,  9043,  8936,
    8830,  8727,  8625,  8525,  8427,  8331,  8236,  8143,  8052,  7963,  7874,
    7788,  7703,  7619,  7537,  7456,  7376,  7298,  7221,  7145,  7070,  6997,
    6924,  6853,  6783,  6714,  6646,  6579,  6513,  6448,  6384,  6321,  6259,
    6198,  6138,  6078,  6019,  5962,  5905,  5848,  5793,  5738,  5684,  5631,
    5579,  5527,  5476,  5425,  5376,  5327,  5278,  5230,  5183,  5136,  5090,
    5045,  5000,  4956,  4912,  4869,  4826,  4784,  4742,  4701,  4660,  4620,
    4581,  4541,  4503,  4464,  4426,  4389,  4352,  4316,  4279,  4244,  4208,
    4173,  4139,  4105,  4071,  4038,  4005,  3972,  3940,  3908,  3876,  3845,
    3814,  3783,  3753,  3723,  3693,  3664,  3635,  3606,  3578,  3550,  3522,
    3495,  3467,  3440,  3414,  3387,  3361,  3335,  3309,  3284,  3259,  3234,
    3209,  3185,  3161,  3137,  3113,  3090,  3067,  3044,  3021,  2998,  2976,
    2954,  2932,  2910,  2889,  2867,  2846,  2825,  2805,  2784,  2764,  2743,
    2724,  2704,  2684,  2665,  2645,  2626,  2607,  2589,  2570,  2552,  2534,
    2516,  2498,  2480,  2462,  2445,  2428,  2410,  2393,  2377,  2360,  2343,
    2327,  2311,  2295,  2279,  2263,  2247,  2232,  2216,  2201,  2186,  2171,
    2156,  2141,  2126,  2112,  2097,  2083,  2069,  2055,  2041,  2027,  2013,
    2000,  1986,  1973};

static const int16_t bd_inv_sqrt[34] = {
    32767, 24576, 17378, 14189, 12288, 10991, 10033, 9289, 8689,
    8192,  7772,  7410,  7094,  6816,  6568,  6345,  6144, 5961,
    5793,  5638,  5495,  5363,  5240,  5124,  5017,  4915, 4820,
    4730,  4644,  4564,  4487,  4414,  4344,  4278};

static const uint32_t bd_recip[322] = {
    2147483647, 262144, 131072, 87381, 65536, 52429, 43691, 37449, 32768, 29127,
    26214,      23831,  21845,  20165, 18725, 17476, 16384, 15420, 14564, 13797,
    13107,      12483,  11916,  11398, 10923, 10486, 10082, 9709,  9362,  9039,
    8738,       8456,   8192,   7944,  7710,  7490,  7282,  7085,  6899,  6722,
    6554,       6394,   6242,   6096,  5958,  5825,  5699,  5578,  5461,  5350,
    5243,       5140,   5041,   4946,  4855,  4766,  4681,  4599,  4520,  4443,
    4369,       4297,   4228,   4161,  4096,  4033,  3972,  3913,  3855,  3799,
    3745,       3692,   3641,   3591,  3542,  3495,  3449,  3404,  3361,  3318,
    3277,       3236,   3197,   3158,  3121,  3084,  3048,  3013,  2979,  2945,
    2913,       2881,   2849,   2819,  2789,  2759,  2731,  2703,  2675,  2648,
    2621,       2595,   2570,   2545,  2521,  2497,  2473,  2450,  2427,  2405,
    2383,       2362,   2341,   2320,  2300,  2280,  2260,  2241,  2222,  2203,
    2185,       2166,   2149,   2131,  2114,  2097,  2081,  2064,  2048,  2032,
    2016,       2001,   1986,   1971,  1956,  1942,  1928,  1913,  1900,  1886,
    1872,       1859,   1846,   1833,  1820,  1808,  1796,  1783,  1771,  1759,
    1748,       1736,   1725,   1713,  1702,  1691,  1680,  1670,  1659,  1649,
    1638,       1628,   1618,   1608,  1598,  1589,  1579,  1570,  1560,  1551,
    1542,       1533,   1524,   1515,  1507,  1498,  1489,  1481,  1473,  1464,
    1456,       1448,   1440,   1432,  1425,  1417,  1409,  1402,  1394,  1387,
    1380,       1372,   1365,   1358,  1351,  1344,  1337,  1331,  1324,  1317,
    1311,       1304,   1298,   1291,  1285,  1279,  1273,  1266,  1260,  1254,
    1248,       1242,   1237,   1231,  1225,  1219,  1214,  1208,  1202,  1197,
    1192,       1186,   1181,   1176,  1170,  1165,  1160,  1155,  1150,  1145,
    1140,       1135,   1130,   1125,  1120,  1116,  1111,  1106,  1101,  1097,
    1092,       1088,   1083,   1079,  1074,  1070,  1066,  1061,  1057,  1053,
    1049,       1044,   1040,   1036,  1032,  1028,  1024,  1020,  1016,  1012,
    1008,       1004,   1001,   997,   993,   989,   986,   982,   978,   975,
    971,        967,    964,    960,   957,   953,   950,   946,   943,   940,
    936,        933,    930,    926,   923,   920,   917,   913,   910,   907,
    904,        901,    898,    895,   892,   889,   886,   883,   880,   877,
    874,        871,    868,    865,   862,   859,   857,   854,   851,   848,
    846,        843,    840,    838,   835,   832,   830,   827,   824,   822,
    819,        817};

typedef struct {
  uint32_t r;
  int32_t speed;
  int32_t remaining, gain;
  uint32_t ph, inc[3];
  int32_t smooth;
  int32_t vs, vsdec;
  uint8_t stage, rev;
  uint8_t pre;
  int32_t amp;
  int32_t dur;
} grain_t;

typedef struct {
  int32_t pk, lp, g, g0;
} lim_t;

typedef struct {
  int16_t *buf, *rev;
  uint32_t w;
  int32_t dur;
  uint8_t frozen, ok;
  grain_t g[16];
  grain_t *prio[16];
  int npool;
  int32_t makeup;
  int32_t interval;
  uint32_t tph, tinc;
  int32_t tbase, ratio;
  uint32_t rng;
  int32_t prev[64];
  int32_t dc;
  int32_t fbg, dry, wet;
  lim_t lin, lpre, lout;
  int32_t rwp, rlp1, rlp2, rlfo[2], ly0[2], ly1[2], liir[2], rwo;
  int32_t rdry, rwet, rsend, rtime;
  int32_t ridle;
  int32_t level;
  uint32_t trigs;
  struct {
    uint32_t at;
    int32_t dur;
    uint8_t rev;
  } trail[16];
} bd_t;

static const int16_t ch_rcp[64 + 1] = {
    0,    32767, 16384, 10923, 8192, 6554, 5461, 4681, 4096, 3641, 3277,
    2979, 2731,  2521,  2341,  2185, 2048, 1928, 1820, 1725, 1638, 1560,
    1489, 1425,  1365,  1311,  1260, 1214, 1170, 1130, 1092, 1057, 1024,
    993,  964,   936,   910,   886,  862,  840,  819,  799,  780,  762,
    745,  728,   712,   697,   683,  669,  655,  643,  630,  618,  607,
    596,  585,   575,   565,   555,  546,  537,  529,  520,  512};

enum {
  B0 = 0,
  B1 = 29,
  B2 = 70,
  B3 = 131,
  B4 = 231,
  B5 = 641,
  B6 = 1147,
  B7 = 1992,
  B8 = 2467,
  B9 = 2880,
  T0 = 27,
  T1 = 68,
  T2 = 129,
  T3 = 229,
  T4 = 639,
  T5 = 1145,
  T6 = 1990,
  T7 = 2465,
  T8 = 2878
};

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  bd_t bd[3];
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t clip16(int32_t x) {
    return x < -32768 ? -32768 : (x > 32767 ? 32767 : x);
  }

  inline uint32_t rnd(bd_t *s) {
    return s->rng = s->rng * 1664525u + 1013904223u;
  }

  uint32_t pow2_q16(int32_t e) {
    const int32_t i = e >> 16;
    const uint32_t f = (uint32_t)e & 65535u, k = f >> 8, fr = f & 255u;
    const uint32_t m =
        65536u + bd_exp2[k] + (((bd_exp2[k + 1] - bd_exp2[k]) * fr) >> 8);
    return i >= 0 ? m << i : (i > -31 ? m >> -i : 0);
  }

  int lim_update(lim_t *l, int32_t pk, int32_t maxq15, int sh) {
    const int32_t p8 = pk << 8, m8 = maxq15 << 8;
    l->pk = p8 > l->pk ? p8 : l->pk - (((l->pk - p8) >> sh) << 6);
    l->lp = l->pk > l->lp ? l->pk : l->lp - (((l->lp - l->pk) >> sh) << 6);
    int32_t g = 32768;
    if (l->lp > m8) {
      const int32_t pq = l->lp >> 8, k = pq >> 12, f = pq & 4095;
      const uint32_t r =
          bd_recip[k] - (((bd_recip[k] - bd_recip[k + 1]) * (uint32_t)f) >> 12);
      g = (int32_t)(((uint32_t)maxq15 * r) >> 15);
    }
    l->g0 = l->g ? l->g : 32768;
    l->g = g;
    return !(l->g0 == 32768 && g == 32768);
  }

  void __attribute__((noinline)) lim_apply(const lim_t *l, int32_t *x) {
    const int32_t dg = (l->g - l->g0) >> 6;
    int32_t c = l->g0;
    for (int i = 0; i < 64; i++) {
      int32_t v = x[i];
      if (v > (1 << 18))
        v = 1 << 18;
      if (v < -(1 << 18))
        v = -(1 << 18);
      c += dg;
      x[i] = ((v >> 2) * c) >> 13;
    }
  }

  void __attribute__((noinline)) limit_block(lim_t *l, int32_t *x,
                                             int32_t maxq15, int sh) {
    int32_t pk = 0;
    for (int i = 0; i < 64; i++) {
      int32_t v = x[i];
      if (v > (1 << 18))
        v = 1 << 18;
      if (v < -(1 << 18))
        v = -(1 << 18);
      x[i] = v;
      if (v < 0)
        v = -v;
      if (v > pk)
        pk = v;
    }
    if (lim_update(l, pk, maxq15, sh))
      lim_apply(l, x);
  }

  inline int grain_done(const grain_t *g) {
    return g->stage >= 3 || g->vs <= 0;
  }

  void fade_out(grain_t *g, int32_t d) {
    if (d < 1)
      d = 1;
    if (g->remaining > d) {
      g->vsdec = (1 << 24) / d;
      if (d > (1 << 19))
        d = 1 << 19;
      g->remaining = ((g->vs >> 12) * d + (((g->vs & 4095) * d) >> 12)) >> 12;
    }
  }

  void env_start(grain_t *g, int32_t dur, int32_t shape) {
    int32_t sus = 32768 - 3 * shape;
    if (sus < 0)
      sus = 0;
    int32_t sh = ((shape - 10923) * 3) >> 1;
    if (sh < 0)
      sh = 0;
    else if (sh < 16384)
      sh = (((sh * sh) >> 15) * sh) >> 13;
    const int32_t atk = sh, rel = ((32768 - atk) * (32768 - sus)) >> 15;
    int32_t ad = (((dur) >> 15) * (atk) + ((((dur) & 32767) * (atk)) >> 15)),
            sd = (((dur) >> 15) * (sus) + ((((dur) & 32767) * (sus)) >> 15)),
            rd = (((dur) >> 15) * (rel) + ((((dur) & 32767) * (rel)) >> 15));
    const int32_t amin = (22 * (32768 + 3 * sus)) >> 15;
    if (ad < amin)
      ad = amin;
    if (sd < 32)
      sd = 32;
    if (rd < 441)
      rd = 441;
    g->inc[0] = (1u << 30) / (uint32_t)ad;
    g->inc[1] = (1u << 30) / (uint32_t)sd;
    g->inc[2] = (1u << 30) / (uint32_t)rd;
    g->smooth = atk < 16384 ? 2 * atk : 65536 - 2 * atk;
    g->stage = 0;
    g->ph = 0;
  }

  void grain_start(bd_t *s, grain_t *g, int32_t delay, int32_t speed,
                   int32_t dur, int32_t shape, int32_t gain, uint32_t hq,
                   int pre) {
    const int32_t cons = speed - (s->frozen ? 0 : 4096),
                  ac = cons < 0 ? -cons : cons;
    if (ac) {
      const int32_t lim = ((65536 - 256) << 12) / ac;
      if (dur > lim)
        dur = lim;
    }
    const int32_t need = (dur >> 12) * ac + (((dur & 4095) * ac) >> 12);
    if (cons > 0 && delay < need)
      delay = need;
    else if (cons < 0 && delay > (65536 - 256) - need)
      delay = (65536 - 256) - need;
    if (delay < 0)
      delay = 0;
    g->r = hq - ((uint32_t)delay << 12);
    g->speed = speed;
    g->gain = gain;
    g->rev = speed < 0;
    g->dur = dur;
    env_start(g, dur, shape);
    g->remaining = dur;
    g->vs = 1 << 24;
    g->vsdec = 0;
    g->amp = 0;
    g->pre = (uint8_t)pre;
  }

  inline int32_t env_step(grain_t *g, int n) {
    if (g->stage >= 3)
      return 0;
    uint32_t ph = g->ph + g->inc[g->stage] * (uint32_t)n;
    if (ph >= (1u << 30)) {
      ph = 0;
      if (++g->stage == 3) {
        g->ph = 0;
        return 0;
      }
    }
    g->ph = (uint32_t)ph;
    if (g->stage == 1)
      return 32768;
    int32_t t = (int32_t)(g->ph >> 15);
    if (g->stage == 2)
      t = 32768 - t;
    const int32_t t2 = (t * t) >> 15,
                  base = g->stage == 0 ? t : (t2 * t2) >> 15;
    const int32_t ss = (t2 * (49152 - t)) >> 14;
    return base + (((ss - base) * g->smooth) >> 15);
  }

  inline int16_t MUL16(int32_t a, int32_t b) {
    const int32_t v = ((int16_t)a * (int16_t)b) >> 15;
    return (int16_t)(v > 32767 ? 32767 : v);
  }

  void __attribute__((noinline)) grain_run(const int16_t *buf, grain_t *g,
                                           int32_t *out, uint32_t hq,
                                           uint32_t hstep) {
    const int32_t tail = (65536 - 256) << 12;
    uint32_t r = g->r;
    const int32_t speed = g->speed;
    int32_t amp = g->amp;
    int i = g->pre;
    g->pre = 0;
    hq += (uint32_t)i * hstep;
    while (i < 64) {
      int n = 64 - i;
      if (n > 64)
        n = 64;
      hq += (uint32_t)n * hstep;
      const int32_t d = (int32_t)(hq - (r + (uint32_t)(n * speed)));
      int32_t target = 0, stop = 0;
      if (d < 0 || d >= tail)
        stop = 1;
      else {
        const int32_t e = env_step(g, n);
        g->vs -= g->vsdec * n;
        if (g->vs <= 0) {
          g->vs = 0;
          stop = 1;
        } else {
          target = (((e * (g->vs >> 9)) >> 15) * g->gain) >> 15;
          const int32_t dd = d >= (tail >> 1) ? tail - d : d;
          if (dd < (256 << 12)) {
            const int32_t t = dd >> 5, t2 = (t * t) >> 15;
            target = (target * ((t2 * (49152 - t)) >> 14)) >> 15;
          }
        }
        if (g->stage >= 3)
          stop = 1;
      }
      const int32_t damp = ((target - amp) * ch_rcp[n]) >> 15;
      int32_t *o = out + i;
      i += n;
      if (((uint32_t)speed & 4095u) == 0 && (r & 4095u) == 0) {
        const int32_t st = speed >> 12,
                      k0 = (int32_t)(((r + (uint32_t)speed) >> 12) &
                                     (65536 - 1)),
                      k1 = k0 + (n - 1) * st;
        if (k1 >= 0 && k1 < 65536) {
          const int16_t *p = buf + k0;
          for (int j = 0; j < n; j++, p += st) {
            amp += damp;
            *o++ += MUL16(*p, amp);
          }
          r += (uint32_t)(n * speed);
        } else {
          for (int j = 0; j < n; j++) {
            r += (uint32_t)speed;
            amp += damp;
            *o++ += MUL16(buf[(r >> 12) & (65536 - 1)], amp);
          }
        }
      } else {
        for (int j = 0; j < n; j++) {
          r += (uint32_t)speed;
          const int16_t *q = buf + ((r >> 12) & (65536 - 1));
          const int32_t x0 = q[0],
                        x = x0 + (((q[1] - x0) * (int32_t)(r & 4095u)) >> 12);
          amp += damp;
          *o++ += (x * amp) >> 15;
        }
      }
      amp = target;
      g->remaining -= n;
      if (stop) {
        g->stage = 3;
        break;
      }
    }
    g->r = r;
    g->amp = amp;
    if (grain_done(g)) {
      g->remaining = 0;
      g->amp = 0;
    }
  }

  void sort_grains(bd_t *s) {
    grain_t **p = s->prio;
    int32_t ft[4];
    const int ns = s->npool >> 2 < 1 ? 1 : s->npool >> 2;
    const int32_t iv = s->interval;
    if (iv < 1102) {
      grain_t *f = p[0];
      for (int i = 0; i < s->npool - 1; i++)
        p[i] = p[i + 1];
      p[s->npool - 1] = f;
      ft[0] = iv + 1;
      ft[1] = 220;
      ft[2] = 220;
      ft[3] = 4410;
      if (iv > 220) {
        const int32_t t = ft[0];
        ft[0] = ft[2];
        ft[2] = t;
      }
    } else {
      for (int i = 0; i < ns; i++) {
        int m = i;
        for (int j = i + 1; j < s->npool; j++)
          if (p[j]->remaining < p[m]->remaining)
            m = j;
        grain_t *t = p[i];
        p[i] = p[m];
        p[m] = t;
      }
      ft[0] = 220;
      ft[1] = 220;
      ft[2] = iv;
      ft[3] = 4410;
      if (iv > 4410) {
        const int32_t t = ft[2];
        ft[2] = ft[3];
        ft[3] = t;
      }
    }
    for (int i = 0; i < ns; i++)
      fade_out(p[i], ft[i]);
    s->interval = 0;
  }

  inline int32_t sat(int32_t x) {
    return x < -(1 << 17) ? -(1 << 17) : (x > (1 << 17) ? (1 << 17) : x);
  }

  inline int32_t rv_interp(const int16_t *q, int32_t base, int32_t off_q16) {
    const int16_t *r = q + base + (off_q16 >> 16);
    const int32_t f = (off_q16 & 65535) >> 1, a = r[0], b = r[1];
    return a + (((b - a) * f) >> 15);
  }

  void __attribute__((noinline)) reverb(bd_t *s, int32_t *io, int32_t amount) {
    const int32_t r2 = (amount * amount) >> 15, mix = (r2 * 29491) >> 15;
    const int32_t time = 9830 + ((((r2 * amount) >> 15) * 19661) >> 15);
    const int32_t ig = (6554 * (32768 - ((time * 10912) >> 15))) >> 15;
    const int32_t og = (13107 << 15) / ig;
    const int32_t ssq = (mix * (65536 - mix)) >> 15,
                  send = (((ssq * (65536 - ssq)) >> 15) * ig) >> 15;
    const int32_t dry = ((32768 - mix) * (32768 + mix)) >> 15;
    if (amount == 0 && s->rdry == 32768 && s->ridle > 32)
      return;
    const int32_t kap = 20480, klp = 27525;
    const int32_t dd = (dry - s->rdry) >> 4, dw = (og - s->rwet) >> 4,
                  dsn = (send - s->rsend) >> 4;
    int32_t cd = s->rdry, cw = s->rwet, cs = s->rsend;
    int16_t *const rb = s->rev;
    int32_t wp = s->rwp, lp1 = s->rlp1, lp2 = s->rlp2, l0 = s->rlfo[0],
            l1 = s->rlfo[1], wo = s->rwo, wpk = 0;
    const int32_t rt = s->rtime;
    for (int t = 0; t < 64; t += 4) {
      cd += dd;
      cw += dw;
      cs += dsn;
      if (--wp < 0)
        wp += 8192;
      if ((wp & 31) == 0) {
        for (int k = 0; k < 2; k++) {
          const int32_t y = s->ly0[k];
          s->ly0[k] = (int32_t)(((int64_t)s->liir[k] * y) >> 29) - s->ly1[k];
          s->ly1[k] = y;
          s->rlfo[k] = (y + (1 << 28)) >> 14;
        }
        l0 = s->rlfo[0];
        l1 = s->rlfo[1];
      }
      const int16_t *const q = rb + wp;
      int32_t acc, prev, apout, wl;
      int16_t v;
      acc = rv_interp(q, B0, (15 << 13) + 30 * l0);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B0 + 18)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = ((((sat((io[t] + io[t + 1] + io[t + 2] + io[t + 3]) >> 1)) >> 2) *
              (cs)) >>
             13);
      prev = q[T0];
      acc += (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B0)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, -kap) + prev;
      prev = q[T1];
      acc += (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B1)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, -kap) + prev;
      prev = q[T2];
      acc += (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B2)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, -kap) + prev;
      prev = q[T3];
      acc += (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B3)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, -kap) + prev;
      apout = acc;
      acc = apout + MUL16(rv_interp(q, B9, (1173 << 16) + (75 * l1 >> 2)), rt);
      lp1 += ((((acc - lp1) >> 2) * (klp)) >> 13);
      acc = lp1;
      prev = q[T4];
      acc -= (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B4)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, kap) + prev;
      prev = q[T5];
      acc += (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B5)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, -kap) + prev;
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B6)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      wl = acc;
      acc = apout + MUL16(rv_interp(q, B6, (835 << 16) + 15 * l0), rt);
      lp2 += ((((acc - lp2) >> 2) * (klp)) >> 13);
      acc = lp2;
      prev = q[T7];
      acc += (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B7)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, -kap) + prev;
      prev = q[T8];
      acc -= (((int32_t)(int16_t)(prev) * (int16_t)(kap)) >> 15);
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B8)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      acc = MUL16(v, kap) + prev;
      v = (int16_t)clip16(acc);
      do {
        const int32_t i_ = (wp + (B9)) & (8192 - 1);
        rb[i_] = (v);
        rb[i_ + 8192] = (v);
      } while (0);
      const int32_t w = ((((sat((wl + acc) >> 1)) >> 2) * (cw >> 1)) >> 13) * 2;
      const int32_t dwo = (w - wo) >> 2;
      io[t] = ((((sat(io[t])) >> 2) * (cd)) >> 13) + wo + dwo;
      io[t + 1] = ((((sat(io[t + 1])) >> 2) * (cd)) >> 13) + wo + 2 * dwo;
      io[t + 2] = ((((sat(io[t + 2])) >> 2) * (cd)) >> 13) + w - dwo;
      io[t + 3] = ((((sat(io[t + 3])) >> 2) * (cd)) >> 13) + w;
      wo = w;
      const int32_t aw = w < 0 ? -w : w;
      if (aw > wpk)
        wpk = aw;
    }
    s->ridle = amount == 0 && wpk < 8 ? s->ridle + 1 : 0;
    s->rwp = wp;
    s->rlp1 = lp1;
    s->rlp2 = lp2;
    s->rwo = wo;
    s->rdry = dry;
    s->rwet = og;
    s->rsend = send;
    s->rtime = time;
  }

  void bd_init(int pos) {
    pos %= 3;
    bd_t *s = &bd[pos];
    uint8_t *m = (uint8_t *)scratch;
    for (unsigned i = 0; i < sizeof(*s); i++)
      ((uint8_t *)s)[i] = 0;
    if (!m)
      return;
    s->buf = (int16_t *)m;
    s->rev = (int16_t *)(m + 65536 * 2 + 4);
    for (int i = 0; i <= 65536; i++)
      s->buf[i] = 0;
    for (int i = 0; i < 2 * 8192; i++)
      s->rev[i] = 0;
    for (int i = 0; i < 16; i++) {
      s->g[i].stage = 3;
      s->prio[i] = &s->g[i];
    }
    s->npool = 16;
    s->makeup = 4096;
    for (int k = 0; k < 2; k++) {
      const int64_t f = ((int64_t)(k ? 3 : 5) * 32 << 29) / (Wide(10) * 11025);
      const int64_t fr = (Wide(1) << 27) - f;
      s->liir[k] = (int32_t)((16 * fr * ((Wide(1) << 29) - 2 * fr)) >> 29);
      s->ly1[k] = s->liir[k] / 4;
      s->ly0[k] = 1 << 28;
      s->rlfo[k] = (s->ly1[k] + (1 << 28)) >> 14;
    }
    s->rdry = 32768;
    s->rtime = 9830;
    s->ridle = 1000;
    s->lin.pk = s->lin.lp = s->lpre.pk = s->lpre.lp = s->lout.pk = s->lout.lp =
        16384 << 8;
    s->ratio = 4096;
    s->rng = 0x2545F491u + (uint32_t)pos * 0x9E3779B9u;
    s->interval = 1;
    s->ok = 1;
  }

  inline int32_t pword(const int16_t *p, int i) { return p[i] < 0 ? 0 : p[i]; }

  void bd_render(int pos, int16_t *io, const int16_t *p) {
    pos %= 3;
    bd_t *s = &bd[pos];
    if (!s->ok)
      return;
    const int32_t time = ((int32_t)(p)[0]), size = ((int32_t)(p)[1]),
                  shape = ((int32_t)(p)[2]), dens = ((int32_t)(p)[3]);
    const int32_t w5 = pword(p, 5), w6 = pword(p, 6), w7 = pword(p, 7);
    s->frozen = fx_page[pos][3] != 0;
    int32_t pv = w5 - 16384;
    pv = pv > 600 ? pv - 600 : (pv < -600 ? pv + 600 : 0);
    const int32_t st8 = pv * (24 * 256) / (16384 - 600);
    int32_t speed = (int32_t)(pow2_q16(st8 * 256 / 12) >> 4);
    const int reverse = size < 10923;
    if (reverse)
      speed = -speed;
    int32_t cs = reverse ? ((3 * size) * (65536 - 3 * size)) >> 15
                         : 32768 - ((3 * (size - 10923)) >> 1);
    cs = (cs * 33587) >> 15;
    if (cs > 32768)
      cs = 32768;
    const int32_t px2 = (int32_t)pow2_q16(-(cs * 9) * 2);
    const int32_t gdur = (2756 * px2 + (px2 >> 2)) >> 10;
    const int32_t rv = w7 < 16384 ? (w7 * 3) >> 1 : 24576;
    const int32_t fb = w7 > 16384 ? (w7 - 16384) * 2 : 0;
    const int32_t fbt = ((fb * fb) >> 15) * 3;
    const uint32_t w0 = s->w;
    int32_t fbg = s->fbg;
    const int32_t fstep = (fbt - fbg) >> 6;
    int32_t ipk = 0;
    if (fbg == 0 && fbt == 0)
      for (int i = 0; i < 64; i++) {
        const int32_t v = io[i];
        s->prev[i] = v;
        const int32_t a = v < 0 ? -v : v;
        if (a > ipk)
          ipk = a;
      }
    else
      for (int i = 0; i < 64; i++) {
        fbg += fstep;
        const int32_t v = io[i] + ((clip16(s->prev[i]) * (fbg >> 1)) >> 14);
        s->prev[i] = v;
        const int32_t a = v < 0 ? -v : v;
        if (a > ipk)
          ipk = a;
      }
    if (lim_update(&s->lin, ipk, 31130, 15))
      lim_apply(&s->lin, s->prev);
    if (!s->frozen) {
      const int k0 = (int)(w0 & (65536 - 1)),
                n1 = 65536 - k0 < 64 ? 65536 - k0 : 64;
      int16_t *d = s->buf + k0;
      for (int i = 0; i < n1; i++)
        d[i] = (int16_t)clip16(s->prev[i]);
      for (int i = n1; i < 64; i++)
        s->buf[i - n1] = (int16_t)clip16(s->prev[i]);
      if (k0 == 0)
        s->buf[65536] = s->buf[0];
      else if (n1 < 64)
        s->buf[65536] = s->buf[0];
    }
    s->fbg = fbt;
    if (!s->frozen) {
      s->w = w0 + 64;
      s->dur = s->dur + 64 > (65536 - 256) ? (65536 - 256) : s->dur + 64;
    }
    int ntr = 0;
    uint8_t tr[64];
    if (dens < 15729 || dens > 17039) {
      const int rnd_mode = dens > 17039;
      const uint32_t px =
          pow2_q16(rnd_mode ? -(32767 - dens) * 32 : -dens * 32);
      s->tbase = rnd_mode ? (int32_t)(48 * px + ((39154u * px) >> 16))
                          : (int32_t)(97 * px + ((12772u * px) >> 16));
      if (!rnd_mode)
        s->ratio = 4096;
      int32_t f =
          (s->tbase >> 12) * s->ratio + (((s->tbase & 4095) * s->ratio) >> 12);
      s->tinc = (uint32_t)(f < 2147 ? 2147 : (f > (1 << 29) ? (1 << 29) : f));
      for (int i = 0; i < 64; i++) {
        s->tph += s->tinc;
        if (s->tph >= 0x80000000u) {
          s->tph -= 0x80000000u;
          tr[ntr++] = (uint8_t)i;
          s->trigs++;
          if (rnd_mode) {
            s->ratio = bd_rand_ratio[rnd(s) >> 24];
            f = (s->tbase >> 12) * s->ratio +
                (((s->tbase & 4095) * s->ratio) >> 12);
            s->tinc =
                (uint32_t)(f < 2147 ? 2147 : (f > (1 << 29) ? (1 << 29) : f));
          }
        }
      }
    }
    int32_t wet[64];
    for (int i = 0; i < 64; i++)
      wet[i] = 0;
    const uint32_t hstep = s->frozen ? 0 : 4096;
    const uint32_t hq0 = s->frozen ? w0 << 12 : (w0 + 1) << 12;
    {
      const int np = (rv || s->ridle <= 32) ? 8 : 12;
      if (np != s->npool) {
        for (int i = 0; i < np; i++)
          s->prio[i] = &s->g[i];
        s->npool = np;
        s->makeup = np >= 16   ? 4096
                    : np >= 12 ? 4730
                    : np >= 9  ? 5461
                    : np >= 8  ? 5793
                               : 6214;
      }
    }
    const int pool_small = s->npool < 16;
    int a = 0;
    for (int n = 0; n < ntr; n++) {
      const int b = tr[n];
      s->interval += b - a;
      a = b;
      int active = 0;
      for (int k = 0; k < 16; k++)
        active += !grain_done(&s->g[k]);
      const int32_t mk = pool_small && active >= s->npool - (s->npool >> 2) - 1
                             ? s->makeup
                             : 4096;
      int32_t md = s->dur - 2 * 257;
      if (md < 0)
        md = 0;
      const int32_t delay =
          (((md) >> 15) * (time) + ((((md) & 32767) * (time)) >> 15)) + 257;
      grain_start(s, s->prio[0], delay, speed, gdur, shape,
                  (bd_inv_sqrt[active + 1] * mk) >> 12,
                  hq0 + (uint32_t)b * hstep, b);
      {
        const grain_t *g = s->prio[0];
        const int t = (int)(s->trigs & 15u);
        s->trail[t].at = g->r >> 12;
        s->trail[t].dur = g->dur;
        s->trail[t].rev = g->rev;
      }
      sort_grains(s);
    }
    s->interval += 64 - a;
    for (int k = 0; k < 16; k++)
      if (!grain_done(&s->g[k]))
        grain_run(s->buf, &s->g[k], wet, hq0, hstep);
    const int32_t dry = ((((32768 - w6) * (32768 + w6)) >> 15) * 24576) >> 15;
    const int32_t wt = ((((w6 * (65536 - w6)) >> 15) * 49152) >> 15);
    const int32_t ddr = (dry - s->dry) >> 6, dwt = (wt - s->wet) >> 6;
    int32_t cd = s->dry, cw = s->wet, out[64], dc = s->dc, mpk = 0;
    for (int i = 0; i < 64; i++) {
      const int32_t w = wet[i];
      const int32_t x = w > 131071 ? 131071 : (w < -131071 ? -131071 : w);
      dc += (x * 256 - dc) >> 8;
      s->prev[i] = x - (dc >> 8);
      cd += ddr;
      (void)cd;
      cw += dwt;
      const int32_t xm = w > 65535 ? 65535 : (w < -65535 ? -65535 : w);
      const int32_t o = ((xm * (cw >> 1)) >> 14);
      out[i] = o;
      const int32_t a = o < 0 ? -o : o;
      if (a > mpk)
        mpk = a;
    }
    s->dc = dc;
    if (lim_update(&s->lpre, mpk > (1 << 18) ? 1 << 18 : mpk, 29491, 17))
      lim_apply(&s->lpre, out);
    s->dry = dry;
    s->wet = wt;
    reverb(s, out, rv);
    limit_block(&s->lout, out, 29491, 18);
    int32_t pk = 0;
    for (int i = 0; i < 64; i++) {
      const int32_t y = clip16(out[i]);
      io[i] = (int16_t)y;
      const int32_t ab = y < 0 ? -y : y;
      if (ab > pk)
        pk = ab;
    }
    volatile int16_t *mt = (&meter);
    const int32_t mv = (*mt * 0xfe00) >> 16;
    *mt = (int16_t)(pk > mv ? pk : mv);
    s->level = pk;
  }
};
} // namespace port
} // namespace hkhail
