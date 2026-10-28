#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkeva {
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

static const int16_t ev_sin[1025] = {
    0,      201,    402,    603,    804,    1005,   1206,   1407,   1608,
    1809,   2009,   2210,   2410,   2611,   2811,   3012,   3212,   3412,
    3612,   3811,   4011,   4210,   4410,   4609,   4808,   5007,   5205,
    5404,   5602,   5800,   5998,   6195,   6393,   6590,   6786,   6983,
    7179,   7375,   7571,   7767,   7962,   8157,   8351,   8545,   8739,
    8933,   9126,   9319,   9512,   9704,   9896,   10087,  10278,  10469,
    10659,  10849,  11039,  11228,  11417,  11605,  11793,  11980,  12167,
    12353,  12539,  12725,  12910,  13094,  13279,  13462,  13645,  13828,
    14010,  14191,  14372,  14553,  14732,  14912,  15090,  15269,  15446,
    15623,  15800,  15976,  16151,  16325,  16499,  16673,  16846,  17018,
    17189,  17360,  17530,  17700,  17869,  18037,  18204,  18371,  18537,
    18703,  18868,  19032,  19195,  19357,  19519,  19680,  19841,  20000,
    20159,  20317,  20475,  20631,  20787,  20942,  21096,  21250,  21403,
    21554,  21705,  21856,  22005,  22154,  22301,  22448,  22594,  22739,
    22884,  23027,  23170,  23311,  23452,  23592,  23731,  23870,  24007,
    24143,  24279,  24413,  24547,  24680,  24811,  24942,  25072,  25201,
    25329,  25456,  25582,  25708,  25832,  25955,  26077,  26198,  26319,
    26438,  26556,  26674,  26790,  26905,  27019,  27133,  27245,  27356,
    27466,  27575,  27683,  27790,  27896,  28001,  28105,  28208,  28310,
    28411,  28510,  28609,  28706,  28803,  28898,  28992,  29085,  29177,
    29268,  29358,  29447,  29534,  29621,  29706,  29791,  29874,  29956,
    30037,  30117,  30195,  30273,  30349,  30424,  30498,  30571,  30643,
    30714,  30783,  30852,  30919,  30985,  31050,  31113,  31176,  31237,
    31297,  31356,  31414,  31470,  31526,  31580,  31633,  31685,  31736,
    31785,  31833,  31880,  31926,  31971,  32014,  32057,  32098,  32137,
    32176,  32213,  32250,  32285,  32318,  32351,  32382,  32412,  32441,
    32469,  32495,  32521,  32545,  32567,  32589,  32609,  32628,  32646,
    32663,  32678,  32692,  32705,  32717,  32728,  32737,  32745,  32752,
    32757,  32761,  32765,  32766,  32767,  32766,  32765,  32761,  32757,
    32752,  32745,  32737,  32728,  32717,  32705,  32692,  32678,  32663,
    32646,  32628,  32609,  32589,  32567,  32545,  32521,  32495,  32469,
    32441,  32412,  32382,  32351,  32318,  32285,  32250,  32213,  32176,
    32137,  32098,  32057,  32014,  31971,  31926,  31880,  31833,  31785,
    31736,  31685,  31633,  31580,  31526,  31470,  31414,  31356,  31297,
    31237,  31176,  31113,  31050,  30985,  30919,  30852,  30783,  30714,
    30643,  30571,  30498,  30424,  30349,  30273,  30195,  30117,  30037,
    29956,  29874,  29791,  29706,  29621,  29534,  29447,  29358,  29268,
    29177,  29085,  28992,  28898,  28803,  28706,  28609,  28510,  28411,
    28310,  28208,  28105,  28001,  27896,  27790,  27683,  27575,  27466,
    27356,  27245,  27133,  27019,  26905,  26790,  26674,  26556,  26438,
    26319,  26198,  26077,  25955,  25832,  25708,  25582,  25456,  25329,
    25201,  25072,  24942,  24811,  24680,  24547,  24413,  24279,  24143,
    24007,  23870,  23731,  23592,  23452,  23311,  23170,  23027,  22884,
    22739,  22594,  22448,  22301,  22154,  22005,  21856,  21705,  21554,
    21403,  21250,  21096,  20942,  20787,  20631,  20475,  20317,  20159,
    20000,  19841,  19680,  19519,  19357,  19195,  19032,  18868,  18703,
    18537,  18371,  18204,  18037,  17869,  17700,  17530,  17360,  17189,
    17018,  16846,  16673,  16499,  16325,  16151,  15976,  15800,  15623,
    15446,  15269,  15090,  14912,  14732,  14553,  14372,  14191,  14010,
    13828,  13645,  13462,  13279,  13094,  12910,  12725,  12539,  12353,
    12167,  11980,  11793,  11605,  11417,  11228,  11039,  10849,  10659,
    10469,  10278,  10087,  9896,   9704,   9512,   9319,   9126,   8933,
    8739,   8545,   8351,   8157,   7962,   7767,   7571,   7375,   7179,
    6983,   6786,   6590,   6393,   6195,   5998,   5800,   5602,   5404,
    5205,   5007,   4808,   4609,   4410,   4210,   4011,   3811,   3612,
    3412,   3212,   3012,   2811,   2611,   2410,   2210,   2009,   1809,
    1608,   1407,   1206,   1005,   804,    603,    402,    201,    0,
    -201,   -402,   -603,   -804,   -1005,  -1206,  -1407,  -1608,  -1809,
    -2009,  -2210,  -2410,  -2611,  -2811,  -3012,  -3212,  -3412,  -3612,
    -3811,  -4011,  -4210,  -4410,  -4609,  -4808,  -5007,  -5205,  -5404,
    -5602,  -5800,  -5998,  -6195,  -6393,  -6590,  -6786,  -6983,  -7179,
    -7375,  -7571,  -7767,  -7962,  -8157,  -8351,  -8545,  -8739,  -8933,
    -9126,  -9319,  -9512,  -9704,  -9896,  -10087, -10278, -10469, -10659,
    -10849, -11039, -11228, -11417, -11605, -11793, -11980, -12167, -12353,
    -12539, -12725, -12910, -13094, -13279, -13462, -13645, -13828, -14010,
    -14191, -14372, -14553, -14732, -14912, -15090, -15269, -15446, -15623,
    -15800, -15976, -16151, -16325, -16499, -16673, -16846, -17018, -17189,
    -17360, -17530, -17700, -17869, -18037, -18204, -18371, -18537, -18703,
    -18868, -19032, -19195, -19357, -19519, -19680, -19841, -20000, -20159,
    -20317, -20475, -20631, -20787, -20942, -21096, -21250, -21403, -21554,
    -21705, -21856, -22005, -22154, -22301, -22448, -22594, -22739, -22884,
    -23027, -23170, -23311, -23452, -23592, -23731, -23870, -24007, -24143,
    -24279, -24413, -24547, -24680, -24811, -24942, -25072, -25201, -25329,
    -25456, -25582, -25708, -25832, -25955, -26077, -26198, -26319, -26438,
    -26556, -26674, -26790, -26905, -27019, -27133, -27245, -27356, -27466,
    -27575, -27683, -27790, -27896, -28001, -28105, -28208, -28310, -28411,
    -28510, -28609, -28706, -28803, -28898, -28992, -29085, -29177, -29268,
    -29358, -29447, -29534, -29621, -29706, -29791, -29874, -29956, -30037,
    -30117, -30195, -30273, -30349, -30424, -30498, -30571, -30643, -30714,
    -30783, -30852, -30919, -30985, -31050, -31113, -31176, -31237, -31297,
    -31356, -31414, -31470, -31526, -31580, -31633, -31685, -31736, -31785,
    -31833, -31880, -31926, -31971, -32014, -32057, -32098, -32137, -32176,
    -32213, -32250, -32285, -32318, -32351, -32382, -32412, -32441, -32469,
    -32495, -32521, -32545, -32567, -32589, -32609, -32628, -32646, -32663,
    -32678, -32692, -32705, -32717, -32728, -32737, -32745, -32752, -32757,
    -32761, -32765, -32766, -32767, -32766, -32765, -32761, -32757, -32752,
    -32745, -32737, -32728, -32717, -32705, -32692, -32678, -32663, -32646,
    -32628, -32609, -32589, -32567, -32545, -32521, -32495, -32469, -32441,
    -32412, -32382, -32351, -32318, -32285, -32250, -32213, -32176, -32137,
    -32098, -32057, -32014, -31971, -31926, -31880, -31833, -31785, -31736,
    -31685, -31633, -31580, -31526, -31470, -31414, -31356, -31297, -31237,
    -31176, -31113, -31050, -30985, -30919, -30852, -30783, -30714, -30643,
    -30571, -30498, -30424, -30349, -30273, -30195, -30117, -30037, -29956,
    -29874, -29791, -29706, -29621, -29534, -29447, -29358, -29268, -29177,
    -29085, -28992, -28898, -28803, -28706, -28609, -28510, -28411, -28310,
    -28208, -28105, -28001, -27896, -27790, -27683, -27575, -27466, -27356,
    -27245, -27133, -27019, -26905, -26790, -26674, -26556, -26438, -26319,
    -26198, -26077, -25955, -25832, -25708, -25582, -25456, -25329, -25201,
    -25072, -24942, -24811, -24680, -24547, -24413, -24279, -24143, -24007,
    -23870, -23731, -23592, -23452, -23311, -23170, -23027, -22884, -22739,
    -22594, -22448, -22301, -22154, -22005, -21856, -21705, -21554, -21403,
    -21250, -21096, -20942, -20787, -20631, -20475, -20317, -20159, -20000,
    -19841, -19680, -19519, -19357, -19195, -19032, -18868, -18703, -18537,
    -18371, -18204, -18037, -17869, -17700, -17530, -17360, -17189, -17018,
    -16846, -16673, -16499, -16325, -16151, -15976, -15800, -15623, -15446,
    -15269, -15090, -14912, -14732, -14553, -14372, -14191, -14010, -13828,
    -13645, -13462, -13279, -13094, -12910, -12725, -12539, -12353, -12167,
    -11980, -11793, -11605, -11417, -11228, -11039, -10849, -10659, -10469,
    -10278, -10087, -9896,  -9704,  -9512,  -9319,  -9126,  -8933,  -8739,
    -8545,  -8351,  -8157,  -7962,  -7767,  -7571,  -7375,  -7179,  -6983,
    -6786,  -6590,  -6393,  -6195,  -5998,  -5800,  -5602,  -5404,  -5205,
    -5007,  -4808,  -4609,  -4410,  -4210,  -4011,  -3811,  -3612,  -3412,
    -3212,  -3012,  -2811,  -2611,  -2410,  -2210,  -2009,  -1809,  -1608,
    -1407,  -1206,  -1005,  -804,   -603,   -402,   -201,   0,
};

static const int16_t ev_hz[6] = {
    380, 800, 720, 1150, 330, 1900,
};

static const int16_t ev_bw[6] = {
    160, 220, 180, 240, 150, 300,
};

static const int16_t ev_level[6] = {
    32767, 20675, 32767, 29204, 32767, 32767,
};

enum {
  P_LPA,
  P_F1,
  P_F2,
  P_D1,
  P_D2,
  P_G1,
  P_G2,
  P_NOISE,
  P_AMP,
  P_CHO,
  P_HP,
  P_RLP,
  P_SAT,
  P_HISS,
  P_CRUSH,
  P_GATE,
  NP
};

typedef struct {
  uint32_t ph1, ph2, vph, noise, c1ph, c2ph;
  int32_t vib, drift, dtarget, env;
  int32_t a1, a2;
  int32_t lo1, bd1, lo2, bd2;
  int16_t dl[2 * 512];
  uint32_t dw;
  int32_t hs1, hs2, l1, l2;
  int32_t last;
  int32_t held_s, cnt, hold_n, step;
  int32_t p[NP];
  int32_t com;
  int32_t kf, vc[6];
  uint32_t kinc;
  int32_t inv;
} voice_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  voice_t voices[1];
  uint8_t held[1];
  int last_voice;
  uint32_t rng;
  uint32_t br_ph, br_inc;
  int32_t drop_left;
  int32_t gate_s;
  int q_owner;
  uint32_t q_ph, q_inc;
  int32_t q_left, q_level, sq_left, sq_level;
  volatile uint32_t eva_blocks;
  volatile int32_t eva_breath, eva_gate, eva_beep;
  int32_t kc, kdrop, com_t[5];
  void defaults() {
    last_voice = 1;
    rng = 0x2545F491u;
    br_inc = 1495000u;
    gate_s = 32768 - 1;
    q_owner = -1;
    eva_gate = 32768 - 1;
    kc = -1;
  }
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  inline int32_t com_warm(int32_t c) {
    return c < 13107 ? (c * 5) >> 1 : 32767;
  }

  inline int32_t com_radio(int32_t c) {
    return c > 13107 ? ((c - 13107) * 27307) >> 14 : 0;
  }

  inline int32_t beep_level(int32_t c) {
    return c > 13107 ? 1500 + (((c - 13107) * 15834) >> 15) : 0;
  }

  inline int32_t clamp(int32_t x, int32_t lo, int32_t hi) {
    return x < lo ? lo : x > hi ? hi : x;
  }

  unsigned beep_octave(const int16_t *k) {
    return k[7] > 0 ? ((unsigned)k[7] >> 9) % 3 : 1;
  }

  uint32_t beep_increment(uint32_t inc, unsigned oct) {
    const uint32_t ceiling = 0x60000000u;
    return inc > (ceiling >> oct) ? ceiling : inc << oct;
  }

  inline int32_t knob(const int16_t *k, int i) { return k[i] < 0 ? 0 : k[i]; }

  inline int32_t sinq(uint32_t ph) {
    const uint32_t i = ph >> 22;
    const int32_t a = ev_sin[i];
    return a + (((ev_sin[i + 1] - a) * (int32_t)((ph >> 7) & 0x7FFF)) >> 15);
  }

  inline uint32_t lcg(uint32_t *s) { return *s = *s * 1664525u + 1013904223u; }

  inline int32_t sclip(int32_t x) {
    x = clamp(x, -49151, 49151);
    const int32_t h = x >> 1;
    const int32_t x2 = (h * h) >> 13;
    const int32_t x3 = ((x2 >> 1) * h) >> 13;
    return x - ((x3 * 4855) >> 15);
  }

  inline int32_t blep_saw(uint32_t ph, int32_t dt, int32_t inv8) {
    const int32_t t = (int32_t)(ph >> 17);
    int32_t s = (int32_t)(ph >> 16) - 32768;
    if (t < dt) {
      const int32_t x = (t * inv8) >> 7;
      s -= x + x - ((x * x) >> 15) - 32768;
    } else if (t > 32768 - dt) {
      const int32_t x = ((t - 32768) * inv8) >> 7;
      s -= ((x * x) >> 15) + x + x + 32768;
    }
    return s;
  }

  inline int32_t lp_coef(int32_t b) {
    return 2600 + ((((((b * b) >> 15) + b) >> 1) * 23600) >> 15);
  }

  void targets(voice_t *v, const int16_t *knobs, int32_t *t) {
    const int32_t p = knob(knobs, 0), f = knob(knobs, 1), o = knob(knobs, 2),
                  c = knob(knobs, 3);
    const int32_t bright = 8192 + ((p * 24575) >> 15);
    const int32_t bloom = 13107 + ((v->env * 19660) >> 15);
    t[P_LPA] = lp_coef((bright * bloom) >> 15);
    if (f != v->kf) {
      v->kf = f;
      const int32_t pos = f * 2;
      int vi = pos >> 15;
      int32_t fr = pos & 0x7FFF;
      if (vi >= 2) {
        vi = 1;
        fr = 32768 - 1;
      }
      for (int k = 0; k < 2; k++) {
        const int a = vi * 2 + k, b = a + 2;
        const int32_t hz = ev_hz[a] + (((ev_hz[b] - ev_hz[a]) * fr) >> 15);
        const int32_t bw = ev_bw[a] + (((ev_bw[b] - ev_bw[a]) * fr) >> 15);
        const int32_t x = (hz * 4781) >> 10;
        const int32_t x3 = (((x * x) >> 15) * x) >> 15;
        v->vc[k] = x - x3 / 6;
        v->vc[2 + k] = (bw << 14) / hz;
        v->vc[4 + k] =
            ((ev_level[a] + (((ev_level[b] - ev_level[a]) * fr) >> 15)) *
             v->vc[2 + k]) >>
            14;
      }
    }
    for (int k = 0; k < 2; k++) {
      t[P_F1 + k] = v->vc[k];
      t[P_D1 + k] = v->vc[2 + k];
      t[P_G1 + k] = v->vc[4 + k];
    }
    t[P_NOISE] = (o * (1600 + (((32767 - v->env) * 9000) >> 15))) >> 15;
    t[P_CHO] = 13107 + ((o * 19660) >> 15);
    const int32_t sw = 16384 + (sinq(br_ph) >> 1);
    t[P_AMP] = (v->env * (32768 - 1 - ((((o * sw) >> 15) * 8192) >> 15))) >> 15;

    if (c != kc || (drop_left != 0) != kdrop) {
      kc = c;
      kdrop = drop_left != 0;
      const int32_t w = com_warm(c), r = com_radio(c);
      com_t[0] = ((30 + ((w * 130) >> 15) + ((r * 170) >> 15)) * 4781) >> 10;
      com_t[1] = 32767 - ((w * 7800) >> 15) - ((r * 11000) >> 15);
      com_t[2] = 4096 + ((w * 3000) >> 15) + ((r * 3500) >> 15);
      com_t[3] =
          r ? 200 + ((r * 1600) >> 15) + (drop_left ? (r * 2500) >> 15 : 0) : 0;
      com_t[4] = c > 24576 ? (c - 24576) * 4 : 0;
    }
    t[P_HP] = com_t[0];
    t[P_RLP] = com_t[1];
    t[P_SAT] = com_t[2];
    t[P_HISS] = com_t[3];
    t[P_CRUSH] = com_t[4];
    t[P_GATE] = gate_s;
    v->com = c;
  }

  void eva_note_on(void *self, const int16_t *knobs) {
    const int vi = 0;
    voice_t *v = &voices[vi];
    int alone = 1;
    for (int i = 0; i < 1; i++)
      if (i != vi && held[i])
        alone = 0;
    held[vi] = 1;
    v->noise = 0x9E3779B9u ^ ((uint32_t)vi * 0x85EBCA6Bu);
    v->ph1 = lcg(&v->noise);
    v->ph2 = lcg(&v->noise);
    v->c1ph = (uint32_t)vi * 0x2AAAAAAAu;
    v->c2ph = (uint32_t)vi * 0x15555555u + 0x40000000u;
    v->vph = 0;
    v->vib = 0;
    v->drift = v->dtarget = 0;
    v->env = 0;
    v->a1 = v->a2 = v->lo1 = v->bd1 = v->lo2 = v->bd2 = 0;
    for (int i = 0; i < 2 * 512; i++)
      v->dl[i] = 0;
    v->dw = 0;
    v->hs1 = v->hs2 = v->l1 = v->l2 = v->last = 0;
    v->held_s = v->cnt = 0;
    v->kf = -1;
    v->kinc = 0;
    v->inv = 0;
    targets(v, knobs, v->p);
    v->hold_n = 1 + ((v->p[P_CRUSH] * 3) >> 15);
    v->step = 1 << (3 + ((v->p[P_CRUSH] * 6) >> 15));
    if (alone && beep_level(v->com)) {
      q_owner = vi;
      q_ph = 0;
      q_inc = 0;
      q_left = 5512;
      q_level = (beep_level(v->com) * 7) / 5;
      sq_left = 0;
    }
  }

  void eva_note_off(void *self) {
    const int vi = 0;
    held[vi] = 0;
    for (int i = 0; i < 1; i++)
      if (held[i])
        return;
    const int32_t c = voices[vi].com;
    if (!beep_level(c))
      return;
    q_owner = vi;
    q_ph = 0;
    q_inc = 0;
    q_left = 5512;
    q_level = (beep_level(c) * 7) / 5;
    sq_left = 2048;
    sq_level = (beep_level(c) * 2) / 3;
  }

  inline void radio(voice_t *v, int32_t *x, const int32_t *p0, const int32_t *d,
                    int fin, int crush, int16_t *out, int32_t gain) {
    int32_t hs1 = v->hs1, hs2 = v->hs2, l1 = v->l1, l2 = v->l2, last = v->last,
            cnt = v->cnt, held_s = v->held_s;
    const int32_t hold_n = v->hold_n, step = v->step;
    for (int sb = 0; sb < 4; sb++) {
      const int32_t hp = p0[P_HP] + ((d[P_HP] * sb) >> 2),
                    rlp = p0[P_RLP] + ((d[P_RLP] * sb) >> 2);
      const int32_t sat = p0[P_SAT] + ((d[P_SAT] * sb) >> 2),
                    cr = p0[P_CRUSH] + ((d[P_CRUSH] * sb) >> 2);
      for (int i = sb * 16; i < sb * 16 + 16; i++) {
        int32_t y = clamp(x[i], -65535, 65535);
        hs1 += ((y - hs1) * hp) >> 14;
        y -= hs1;
        hs2 += ((y - hs2) * hp) >> 14;
        l1 += ((y - hs2 - l1) * rlp) >> 15;
        l2 += ((l1 - l2) * rlp) >> 15;
        y = sclip((l2 * sat) >> 12);
        if (fin) {
          if (crush) {
            if (++cnt >= hold_n) {
              cnt = 0;
              held_s = y & ~(step - 1);
            }
            y += ((held_s - y) * cr) >> 15;
          }
          y = (y * gain) >> 15;
          out[2 * i] = (int16_t)((last + y) >> 1);
          out[2 * i + 1] = (int16_t)y;
          last = y;
        } else
          x[i] = y;
      }
    }
    v->hs1 = hs1;
    v->hs2 = hs2;
    v->l1 = l1;
    v->l2 = l2;
    if (fin)
      v->last = last;
    if (crush) {
      v->cnt = cnt;
      v->held_s = held_s;
    }
  }

  void eva_render(void *self, int16_t *out, const int16_t *knobs) {
    const int vi = 0;
    voice_t *v = &voices[vi];
    if (vi <= last_voice) {
      br_ph += br_inc;
      const int32_t o = knob(knobs, 2);
      br_inc = 1495000u + (uint32_t)((o * 1500) >> 15) * 1024u;
      const int32_t c = knob(knobs, 3);
      const int32_t r = com_radio(c);
      if (drop_left)
        drop_left--;
      else if (r && (int32_t)((lcg(&rng) >> 17) & 0x7FFF) <
                        ((((r * r) >> 15) * 120) >> 15))
        drop_left = 6 + (int32_t)((lcg(&rng) >> 27) + (lcg(&rng) >> 28));
      gate_s += ((drop_left ? 32768 - 1 - ((r * 29000) >> 15) : 32768 - 1) -
                 gate_s) >>
                2;
    }
    last_voice = vi;
    const int32_t pr = knob(knobs, 0),
                  ka = 158 + ((((pr * pr) >> 15) * 6200) >> 15);
    v->env += ((32768 - 1 - v->env) * ka) >> 15;
    int32_t t[NP], d[NP];
    targets(v, knobs, t);
    const int32_t *p0 = v->p;
    for (int k = 0; k < NP; k++)
      d[k] = t[k] - p0[k];
    eva_breath = knob(knobs, 2);
    eva_gate = t[P_GATE];
    eva_beep = q_left > 0 && q_owner >= 0;
    if (v->vib < 20480)
      v->vib += 120;
    if ((lcg(&v->noise) >> 24) < 2)
      v->dtarget = (int32_t)(lcg(&v->noise) >> 17) - 16384;
    v->drift += (v->dtarget - v->drift) >> 6;
    const int32_t m =
        ((sinq(v->vph) * v->vib) >> 15) + ((v->drift * 9830) >> 15);
    v->vph += 62300000u;
    v->c1ph += 4600000u;
    v->c2ph += 6600000u;
    const int32_t cm1 = sinq(v->c1ph), cm2 = sinq(v->c2ph);
    const int32_t dd1 = (221 << 8) + ((cm1 * (55 << 8)) >> 15),
                  dd2 = (309 << 8) + ((cm2 * (66 << 8)) >> 15);
    const uint32_t inc0 = (uint32_t)pitch_input[0] + (uint32_t)pitch_input[1];
    if (vi == q_owner && q_left > 0)
      q_inc = beep_increment(inc0, beep_octave(knobs));
    const uint32_t i1 = inc0 + (uint32_t)(((int32_t)(inc0 >> 16) * m) >> 7);
    const uint32_t i2 = i1 + (i1 >> 9) + (i1 >> 11);
    const int32_t dt1 = (int32_t)(i1 >> 17), dt2 = (int32_t)(i2 >> 17);
    if (inc0 != v->kinc) {
      v->kinc = inc0;
      const int32_t dt0 = (int32_t)(inc0 >> 17);
      v->inv = dt0 > 0 ? ((1 << 30) / dt0) >> 8 : 0;
    }
    const int32_t inv1 = v->inv, inv2 = inv1 - (inv1 >> 9) - (inv1 >> 11);
    int32_t x[(128 / 2)];
    uint32_t noise = v->noise;
    {
      uint32_t ph1 = v->ph1, ph2 = v->ph2;
      if (i2 < 0x40000000u)
        for (int i = 0; i < (128 / 2); i++) {
          ph1 += i1;
          ph2 += i2;
          const int32_t saw = ph1 + i1 < i1 + i1 ? blep_saw(ph1, dt1, inv1)
                                                 : (int32_t)(ph1 >> 16) - 32768;
          const uint32_t q = ph2 << 1;
          if (q + i2 + i2 < (i2 << 2)) {
            const int32_t pul =
                ph2 + i2 < i2 + i2
                    ? (blep_saw(ph2, dt2, inv2) -
                       ((int32_t)((ph2 + 0x80000000u) >> 16) - 32768)) >>
                          1
                    : (((int32_t)(ph2 >> 16) - 32768) -
                       blep_saw(ph2 + 0x80000000u, dt2, inv2)) >>
                          1;
            x[i] = (saw * 19000 + pul * 12000) >> 15;
          } else
            x[i] = ((saw * 19000) >> 15) + (ph2 & 0x80000000u ? 6000 : -6000);
        }
      else
        for (int i = 0; i < (128 / 2); i++) {
          ph1 += i1;
          ph2 += i2;
          const int32_t saw = blep_saw(ph1, dt1, inv1);
          const int32_t pul = (blep_saw(ph2, dt2, inv2) -
                               blep_saw(ph2 + 0x80000000u, dt2, inv2)) >>
                              1;
          x[i] = (saw * 19000 + pul * 12000) >> 15;
        }
      v->ph1 = ph1;
      v->ph2 = ph2;
    }
    int32_t pb[(128 / 2)];
    {
      int32_t a1 = v->a1, a2 = v->a2;
      if (p0[P_NOISE] | t[P_NOISE])
        for (int sb = 0; sb < 8; sb++) {
          const int32_t lpa = (p0[P_LPA] + ((d[P_LPA] * (sb)) >> 3)),
                        nz = (p0[P_NOISE] + ((d[P_NOISE] * (sb)) >> 3));
          for (int i = sb * 8; i < sb * 8 + 8; i++) {
            const int32_t in =
                x[i] + ((((int32_t)lcg(&noise) >> 16) * nz) >> 15);
            x[i] = in;
            a1 += ((in - a1) * lpa) >> 15;
            a2 += ((a1 - a2) * lpa) >> 15;
            pb[i] = a2;
          }
        }
      else
        for (int sb = 0; sb < 8; sb++) {
          const int32_t lpa = (p0[P_LPA] + ((d[P_LPA] * (sb)) >> 3));
          for (int i = sb * 8; i < sb * 8 + 8; i++) {
            a1 += ((x[i] - a1) * lpa) >> 15;
            a2 += ((a1 - a2) * lpa) >> 15;
            pb[i] = a2;
          }
        }
      v->a1 = a1;
      v->a2 = a2;
      for (int k = 0; k < 2; k++) {
        int32_t lo = k ? v->lo2 : v->lo1, bd = k ? v->bd2 : v->bd1;
        for (int sb = 0; sb < 4; sb++) {
          const int32_t f = (p0[P_F1 + k] + ((d[P_F1 + k] * (sb)) >> 2)),
                        q = (p0[P_D1 + k] + ((d[P_D1 + k] * (sb)) >> 2));
          const int32_t g =
              ((p0[P_G1 + k] + ((d[P_G1 + k] * (sb)) >> 2)) * 26000) >> 15;
          for (int i = sb * 16; i < sb * 16 + 16; i++) {
            lo += (f * bd) >> 14;
            bd += (f * ((x[i] >> 2) - lo - ((q * bd) >> 14))) >> 14;
            pb[i] += (bd * g) >> 13;
          }
        }
        if (k) {
          v->lo2 = lo;
          v->bd2 = bd;
        } else {
          v->lo1 = lo;
          v->bd1 = bd;
        }
      }
    }
    {
      uint32_t dw = v->dw;
      int16_t *dl = v->dl;
      const int32_t fa = dd1 & 255, fb = dd2 & 255;
      const int16_t *ra = dl + 512 + (dw & (512 - 1)) - (dd1 >> 8),
                    *rb = dl + 512 + (dw & (512 - 1)) - (dd2 >> 8);
      for (int sb = 0; sb < 8; sb++) {
        const int32_t cho = (p0[P_CHO] + ((d[P_CHO] * (sb)) >> 3)) >> 1,
                      ag = ((p0[P_AMP] + ((d[P_AMP] * (sb)) >> 3)) *
                            (p0[P_GATE] + ((d[P_GATE] * (sb)) >> 3))) >>
                           15;
        for (int i = sb * 8; i < sb * 8 + 8; i++) {
          int32_t y = pb[i];
          const int16_t w = (int16_t)clamp(y, -32768, 32767);
          const uint32_t wi = dw & (512 - 1);
          dl[wi] = w;
          dl[wi + 512] = w;
          dw++;
          const int32_t a = *ra, b = *rb;
          const int32_t ta = a + (((ra[-1] - a) * fa) >> 8);
          const int32_t tb = b + (((rb[-1] - b) * fb) >> 8);
          ra++;
          rb++;
          y += ((((ta + tb) >> 1) - y) * cho) >> 15;
          pb[i] = (y * ag) >> 15;
        }
      }
      v->dw = dw;
    }
    if (p0[P_HISS] | t[P_HISS])
      for (int sb = 0; sb < 4; sb++) {
        const int32_t hiss = (p0[P_HISS] + ((d[P_HISS] * (sb)) >> 2));
        for (int i = sb * 16; i < sb * 16 + 16; i++)
          pb[i] += (((int32_t)lcg(&noise) >> 16) * hiss) >> 15;
      }
    const int crushing = (p0[P_CRUSH] | t[P_CRUSH]) != 0,
              beeping = vi == q_owner && (q_left > 0 || sq_left > 0);
    const int32_t gain = (15000 * 32767) >> 15;
    if (beeping)
      radio(v, pb, p0, d, 0, 0, out, gain);
    else if (crushing)
      radio(v, pb, p0, d, 1, 1, out, gain);
    else
      radio(v, pb, p0, d, 1, 0, out, gain);
    if (beeping && crushing) {
      int32_t cnt = v->cnt, held_s = v->held_s;
      const int32_t hold_n = v->hold_n, step = v->step;
      for (int sb = 0; sb < 8; sb++) {
        const int32_t crush = (p0[P_CRUSH] + ((d[P_CRUSH] * (sb)) >> 3));
        for (int i = sb * 8; i < sb * 8 + 8; i++) {
          if (++cnt >= hold_n) {
            cnt = 0;
            held_s = pb[i] & ~(step - 1);
          }
          pb[i] += ((held_s - pb[i]) * crush) >> 15;
        }
      }
      v->cnt = cnt;
      v->held_s = held_s;
    }
    if (vi == q_owner && (q_left > 0 || sq_left > 0))
      for (int i = 0; i < (128 / 2); i++) {
        if (q_left > 0) {
          const int32_t env = q_left < 32          ? q_left
                              : 5512 - q_left < 32 ? 5512 - q_left
                                                   : 32;
          pb[i] += (((sinq(q_ph) * q_level) >> 15) * env) >> 5;
          q_ph += q_inc;
          q_left--;
        } else if (sq_left > 0) {
          pb[i] +=
              ((((int32_t)lcg(&noise) >> 16) * sq_level >> 15) * sq_left) >> 11;
          sq_left--;
        }
      }
    if (beeping) {
      int32_t last = v->last;
      for (int i = 0; i < (128 / 2); i++) {
        const int32_t y = (pb[i] * gain) >> 15;
        out[2 * i] = (int16_t)clamp((last + y) >> 1, -32768, 32767);
        out[2 * i + 1] = (int16_t)clamp(y, -32768, 32767);
        last = y;
      }
      v->last = last;
    }
    for (int k = 0; k < NP; k++)
      v->p[k] = t[k];
    v->hold_n = 1 + ((t[P_CRUSH] * 3) >> 15);
    v->step = 1 << (3 + ((t[P_CRUSH] * 6) >> 15));
    v->noise = noise;
    eva_blocks++;
  }
};
} // namespace port
} // namespace hkeva
