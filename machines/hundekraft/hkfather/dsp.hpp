#pragma once
#include "wide.hpp"
#include <opxy/types.hpp>
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkfather {
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
  int16_t tank[8][(4096 + 16)], pre[2][4096], diff[2][2][1024];
  uint32_t head, phase[8];
  int32_t damp[8], mix, pre_samples, peak;
  int32_t gate_env, gate_gain;
  int32_t early[2];
  uint32_t gate_hold;
  uint8_t gate_enabled, gate_active, gate_armed;
  int32_t block[8][64], left[64], right[64];
} mw_state;

static const int16_t mw_decay[129][8] = {
    {18462, 14813, 11675, 9119, 6162, 4865, 2968, 1932},
    {18476, 14829, 11691, 9135, 6175, 4877, 2978, 1939},
    {18518, 14876, 11739, 9181, 6217, 4914, 3006, 1961},
    {18588, 14953, 11819, 9259, 6285, 4976, 3054, 1998},
    {18685, 15061, 11929, 9366, 6381, 5063, 3121, 2049},
    {18807, 15198, 12070, 9504, 6503, 5174, 3207, 2116},
    {18954, 15362, 12240, 9670, 6652, 5309, 3314, 2199},
    {19123, 15553, 12438, 9864, 6827, 5469, 3439, 2298},
    {19314, 15768, 12662, 10085, 7027, 5652, 3585, 2413},
    {19524, 16006, 12911, 10331, 7252, 5859, 3752, 2546},
    {19752, 16264, 13183, 10601, 7501, 6089, 3938, 2695},
    {19994, 16541, 13475, 10893, 7773, 6342, 4144, 2863},
    {20250, 16835, 13787, 11207, 8066, 6616, 4371, 3048},
    {20517, 17143, 14116, 11539, 8380, 6910, 4617, 3252},
    {20793, 17463, 14459, 11888, 8712, 7224, 4883, 3473},
    {21077, 17793, 14816, 12252, 9062, 7556, 5167, 3713},
    {21365, 18131, 15183, 12629, 9429, 7906, 5470, 3971},
    {21658, 18475, 15558, 13018, 9809, 8271, 5790, 4247},
    {21952, 18824, 15941, 13415, 10203, 8651, 6127, 4539},
    {22247, 19175, 16328, 13820, 10607, 9044, 6479, 4848},
    {22541, 19526, 16719, 14231, 11021, 9447, 6845, 5173},
    {22833, 19878, 17111, 14646, 11442, 9861, 7225, 5512},
    {23123, 20227, 17502, 15063, 11870, 10282, 7615, 5866},
    {23408, 20573, 17893, 15481, 12302, 10710, 8017, 6232},
    {23689, 20916, 18281, 15898, 12737, 11143, 8427, 6610},
    {23965, 21253, 18665, 16313, 13173, 11580, 8845, 6998},
    {24234, 21585, 19045, 16725, 13609, 12019, 9269, 7395},
    {24498, 21910, 19419, 17133, 14045, 12459, 9699, 7800},
    {24755, 22229, 19787, 17536, 14478, 12899, 10131, 8212},
    {25005, 22540, 20148, 17934, 14908, 13337, 10567, 8630},
    {25248, 22844, 20501, 18324, 15334, 13773, 11003, 9052},
    {25484, 23140, 20847, 18708, 15755, 14205, 11440, 9477},
    {25712, 23427, 21185, 19084, 16170, 14633, 11876, 9904},
    {25934, 23707, 21514, 19453, 16579, 15056, 12309, 10332},
    {26148, 23978, 21834, 19812, 16981, 15474, 12741, 10759},
    {26355, 24241, 22146, 20164, 17376, 15885, 13168, 11186},
    {26555, 24496, 22450, 20507, 17762, 16289, 13591, 11612},
    {26748, 24743, 22744, 20840, 18141, 16686, 14010, 12034},
    {26934, 24982, 23030, 21165, 18512, 17076, 14423, 12453},
    {27114, 25213, 23307, 21482, 18874, 17457, 14830, 12869},
    {27287, 25436, 23575, 21789, 19227, 17831, 15231, 13280},
    {27454, 25652, 23836, 22087, 19572, 18196, 15624, 13685},
    {27615, 25860, 24087, 22377, 19908, 18553, 16011, 14086},
    {27770, 26061, 24331, 22658, 20235, 18901, 16391, 14480},
    {27919, 26255, 24567, 22930, 20553, 19241, 16763, 14868},
    {28063, 26442, 24795, 23194, 20863, 19573, 17127, 15250},
    {28201, 26623, 25015, 23450, 21164, 19896, 17484, 15625},
    {28335, 26797, 25229, 23698, 21457, 20210, 17832, 15993},
    {28463, 26965, 25435, 23938, 21742, 20516, 18173, 16354},
    {28587, 27128, 25634, 24171, 22018, 20814, 18506, 16708},
    {28706, 27284, 25826, 24396, 22286, 21104, 18831, 17054},
    {28821, 27435, 26012, 24614, 22547, 21386, 19148, 17393},
    {28931, 27581, 26192, 24825, 22800, 21660, 19457, 17725},
    {29038, 27722, 26366, 25029, 23045, 21926, 19759, 18049},
    {29140, 27857, 26534, 25226, 23283, 22185, 20053, 18366},
    {29239, 27988, 26696, 25418, 23514, 22436, 20339, 18676},
    {29335, 28115, 26852, 25603, 23738, 22680, 20618, 18978},
    {29427, 28237, 27004, 25782, 23955, 22917, 20890, 19273},
    {29515, 28354, 27150, 25955, 24166, 23147, 21154, 19561},
    {29601, 28468, 27292, 26123, 24370, 23371, 21412, 19842},
    {29683, 28578, 27429, 26286, 24568, 23588, 21663, 20117},
    {29763, 28684, 27561, 26443, 24761, 23799, 21907, 20384},
    {29839, 28786, 27689, 26595, 24947, 24003, 22144, 20645},
    {29913, 28885, 27813, 26743, 25128, 24202, 22375, 20899},
    {29985, 28981, 27933, 26885, 25303, 24395, 22600, 21147},
    {30054, 29073, 28049, 27024, 25474, 24583, 22819, 21388},
    {30121, 29163, 28161, 27158, 25639, 24764, 23032, 21624},
    {30185, 29249, 28269, 27288, 25799, 24941, 23239, 21853},
    {30248, 29333, 28374, 27413, 25954, 25113, 23440, 22077},
    {30308, 29414, 28476, 27535, 26105, 25279, 23636, 22294},
    {30366, 29492, 28575, 27653, 26252, 25441, 23827, 22507},
    {30422, 29568, 28670, 27768, 26394, 25599, 24013, 22714},
    {30477, 29641, 28763, 27879, 26532, 25751, 24193, 22915},
    {30530, 29712, 28852, 27986, 26666, 25900, 24369, 23111},
    {30581, 29781, 28939, 28091, 26796, 26044, 24540, 23303},
    {30630, 29847, 29023, 28192, 26922, 26184, 24706, 23489},
    {30678, 29912, 29105, 28291, 27045, 26321, 24868, 23671},
    {30724, 29974, 29184, 28386, 27164, 26453, 25026, 23848},
    {30769, 30035, 29261, 28478, 27280, 26582, 25179, 24020},
    {30813, 30094, 29335, 28568, 27392, 26707, 25329, 24188},
    {30855, 30151, 29408, 28656, 27502, 26829, 25474, 24352},
    {30896, 30206, 29478, 28740, 27608, 26947, 25616, 24512},
    {30936, 30260, 29546, 28823, 27711, 27062, 25754, 24668},
    {30974, 30312, 29612, 28903, 27812, 27174, 25888, 24819},
    {31011, 30363, 29676, 28980, 27910, 27283, 26019, 24967},
    {31048, 30412, 29739, 29056, 28005, 27390, 26147, 25112},
    {31083, 30459, 29799, 29129, 28097, 27493, 26271, 25252},
    {31117, 30506, 29858, 29201, 28187, 27593, 26392, 25390},
    {31150, 30551, 29915, 29270, 28275, 27691, 26510, 25523},
    {31182, 30594, 29971, 29338, 28360, 27787, 26625, 25654},
    {31214, 30637, 30025, 29403, 28443, 27879, 26737, 25781},
    {31244, 30678, 30078, 29467, 28524, 27970, 26846, 25905},
    {31274, 30718, 30129, 29530, 28603, 28058, 26952, 26027},
    {31302, 30757, 30179, 29590, 28679, 28144, 27056, 26145},
    {31330, 30795, 30227, 29649, 28754, 28227, 27158, 26260},
    {31357, 30832, 30275, 29706, 28827, 28309, 27256, 26373},
    {31384, 30868, 30321, 29762, 28897, 28388, 27353, 26483},
    {31410, 30903, 30365, 29817, 28967, 28466, 27447, 26590},
    {31435, 30938, 30409, 29870, 29034, 28541, 27539, 26695},
    {31459, 30971, 30451, 29922, 29100, 28615, 27628, 26797},
    {31483, 31003, 30493, 29972, 29164, 28687, 27715, 26897},
    {31506, 31035, 30533, 30021, 29226, 28757, 27801, 26995},
    {31529, 31065, 30572, 30069, 29287, 28826, 27884, 27090},
    {31550, 31095, 30611, 30116, 29346, 28892, 27965, 27184},
    {31572, 31125, 30648, 30161, 29404, 28957, 28045, 27275},
    {31593, 31153, 30684, 30206, 29461, 29021, 28122, 27364},
    {31613, 31181, 30720, 30249, 29516, 29083, 28198, 27451},
    {31633, 31208, 30754, 30291, 29570, 29144, 28272, 27536},
    {31652, 31234, 30788, 30332, 29623, 29203, 28345, 27619},
    {31671, 31260, 30821, 30373, 29674, 29261, 28415, 27700},
    {31689, 31285, 30853, 30412, 29724, 29317, 28484, 27779},
    {31707, 31309, 30885, 30450, 29773, 29373, 28552, 27857},
    {31725, 31333, 30916, 30488, 29821, 29427, 28618, 27933},
    {31742, 31357, 30945, 30524, 29868, 29479, 28683, 28007},
    {31759, 31380, 30975, 30560, 29914, 29531, 28746, 28080},
    {31775, 31402, 31003, 30595, 29959, 29581, 28808, 28151},
    {31791, 31424, 31031, 30629, 30002, 29630, 28868, 28221},
    {31806, 31445, 31059, 30663, 30045, 29679, 28927, 28289},
    {31822, 31466, 31085, 30695, 30087, 29726, 28985, 28356},
    {31836, 31486, 31111, 30727, 30128, 29772, 29041, 28421},
    {31851, 31506, 31137, 30758, 30168, 29817, 29097, 28485},
    {31865, 31525, 31162, 30789, 30207, 29861, 29151, 28547},
    {31879, 31544, 31186, 30819, 30245, 29904, 29204, 28609},
    {31892, 31563, 31210, 30848, 30282, 29946, 29256, 28669},
    {31906, 31581, 31233, 30877, 30319, 29988, 29307, 28727},
    {31919, 31598, 31256, 30904, 30355, 30028, 29357, 28785},
    {31931, 31616, 31278, 30932, 30390, 30068, 29405, 28841},
    {31944, 31633, 31300, 30959, 30424, 30107, 29453, 28896},
    {31956, 31649, 31321, 30985, 30458, 30145, 29500, 28951},
};

static const unsigned delays[8] = {641,  887,  1153, 1429,
                                   1867, 2131, 2683, 3163};

typedef struct {
  int32_t len, next, step, tape_pos;
  uint8_t follows_tape;
} op1_beat_t;

static_assert(sizeof(mw_state) <= 176460, "Father scratch overflow");

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;
  volatile uint32_t father_blocks[3];
  volatile int16_t father_live[3][4];
  volatile int32_t father_peak[3];
  volatile int16_t father_gate[3];
  int16_t raw_params[3][4];
  int32_t normalized[3][4];
  uint8_t params_valid[3];
  void defaults() {}
  int32_t sat(int32_t v) { return v > 32767 ? 32767 : v < -32768 ? -32768 : v; }

  int32_t mul(int32_t a, int32_t b) {
    int32_t v = a * b;
    return (v + ((v >> 31) & 32767)) >> 15;
  }

  void mw_init(mw_state *s) {
    uint8_t *p = (uint8_t *)s;
    for (unsigned i = 0; i < sizeof *s; i++)
      p[i] = 0;
    s->mix = -1;
    s->pre_samples = -1;
    s->gate_gain = 32767;
    for (unsigned i = 0; i < 8; i++)
      s->phase[i] = 0x9E3779B9u * (i + 1u);
  }

  int32_t half(int32_t v) { return (v + (int32_t)((uint32_t)v >> 31)) >> 1; }

  void diffuse_block(int16_t ring[1024], unsigned start, unsigned delay,
                     int32_t samples[64]) {
    for (unsigned n = 0; n < 64; n++) {
      unsigned h = (start + n) & 1023;
      int32_t d = ring[(h - delay) & 1023];
      int32_t v = sat(samples[n] - half(d));
      ring[h] = (int16_t)v;
      samples[n] = sat(d + half(v));
    }
  }

  __attribute__((noinline)) void tank_read(int16_t *tank, int32_t *output,
                                           uint32_t *phasep, int32_t *dampp,
                                           unsigned start, unsigned delay_base,
                                           int32_t depth, uint32_t speed,
                                           int32_t damping) {
    uint32_t phase = *phasep;
    int32_t damp = *dampp;
    for (unsigned group = 0; group < 64; group += 8) {
      uint32_t ph = phase + speed;
      int32_t tri0 =
          (int32_t)(((ph >> 15) ^ (0u - (ph >> 31))) & 65535) - 32768;
      ph += speed * 8;
      int32_t tri8 =
          (int32_t)(((ph >> 15) ^ (0u - (ph >> 31))) & 65535) - 32768;
      int32_t mod0 = mul(tri0, depth), mod8 = mul(tri8, depth);
      int32_t delay_acc = ((int32_t)(delay_base * 256) + mod0) * 8;
      int32_t delta = mod8 - mod0;
      unsigned integer_delay = (unsigned)(delay_acc >> 11);
      unsigned rd = (start + group - integer_delay) & 4095;
      if (integer_delay == (unsigned)((delay_acc + 7 * delta) >> 11) &&
          rd >= 1 && rd <= 4088) {
        const int16_t *p = tank + rd;
        int32_t fraction = delay_acc & 2047;
        for (unsigned j = 0; j < 8; j++) {
          int32_t v = p[j];
          v += ((p[(int)j - 1] - v) * (fraction >> 3)) >> 8;
          damp = v + mul(damp - v, 32768 - damping);
          output[group + j] = damp;
          fraction += delta;
        }
      } else {
        for (unsigned j = 0; j < 8; j++) {
          unsigned n = group + j, h = (start + n) & 4095,
                   delay = (unsigned)(delay_acc >> 3);
          unsigned rd = (h - (delay >> 8)) & 4095;
          int32_t v = tank[rd];
          v += ((tank[(rd - 1) & 4095] - v) * (int32_t)(delay & 255)) >> 8;
          damp = v + mul(damp - v, 32768 - damping);
          output[n] = damp;
          delay_acc += delta;
        }
      }
      phase += speed * 8;
    }
    *phasep = phase;
    *dampp = damp;
  }

  void mw_process(mw_state *s, int16_t *l, int16_t *r, const int32_t q[4]) {
    unsigned ix = ((unsigned)q[0] * 128 + 16383) >> 15;
    if (ix > 128)
      ix = 128;
    int32_t damping = 1800 + mul(q[2], 20000), target_pre = mul(q[0], 662);
    const int gated = q[1] != 0;
    int32_t free = 32767 - q[1];
    const uint32_t hold_samples = 1764u + (uint32_t)mul(mul(free, free), 37926);
    const int32_t close_rate = 64 + mul(q[1], 192);
    int32_t target_mix = q[3];
    if (s->mix < 0)
      s->mix = target_mix;
    if (s->pre_samples < 0)
      s->pre_samples = target_pre;
    if (s->pre_samples < target_pre)
      s->pre_samples++;
    else if (s->pre_samples > target_pre)
      s->pre_samples--;
    int32_t mix = s->mix * 64, step = target_mix - s->mix;
    s->peak = 0;
    int32_t (*block)[64] = s->block, *left = s->left, *right = s->right;
    unsigned start = s->head;
    int32_t input_peak = 0;
    for (unsigned n = 0; n < 64; n++) {
      unsigned h = (start + n) & 4095;
      int32_t a = l[n], b = r ? r[n] : a;
      (void)b;
      s->pre[0][h] = (int16_t)a;
      s->pre[1][h] = (int16_t)b;
      int32_t pa = a < 0 ? -a : a, pb = b < 0 ? -b : b;
      if (pa > input_peak)
        input_peak = pa;
      if (pb > input_peak)
        input_peak = pb;
    }
    int32_t previous_env = s->gate_env;
    int was_active = s->gate_active;
    if (input_peak > s->gate_env)
      s->gate_env = input_peak;
    else
      s->gate_env -= (s->gate_env + 3) >> 2;
    if (!s->gate_active && s->gate_env >= 32)
      s->gate_active = 1;
    else if (s->gate_active && s->gate_env <= 16)
      s->gate_active = 0;
    int onset = (!was_active && s->gate_active) ||
                (input_peak >= 32 &&
                 input_peak > previous_env + (previous_env >> 1) + 32);
    if (!gated) {
      s->gate_enabled = 0;
      s->gate_armed = 0;
      s->gate_hold = 0;
    } else {
      if (!s->gate_enabled) {
        s->gate_enabled = 1;
        s->gate_armed = 0;
        s->gate_hold = 0;
      }
      if (onset)
        s->gate_armed = 1;
      if (s->gate_armed) {
        if (s->gate_active)
          s->gate_hold = hold_samples + (unsigned)s->pre_samples;
        else
          s->gate_hold = s->gate_hold > 64 ? s->gate_hold - 64 : 0;
      }
    }
    const int32_t gate_target =
        (!gated || !s->gate_armed || s->gate_hold) ? 32767 : 0;
    const int apply_gate = gate_target != 32767 || s->gate_gain != 32767;
    for (unsigned n = 0; n < 64; n++) {
      unsigned ph = (start + n - (unsigned)s->pre_samples) & 4095;
      left[n] = s->pre[0][ph];
      right[n] = s->pre[1][ph];
    }
    diffuse_block(s->diff[0][0], start, 227, left);
    diffuse_block(s->diff[0][1], start, 541, left);
    diffuse_block(s->diff[1][0], start, 337, right);
    diffuse_block(s->diff[1][1], start, 719, right);
    s->head += 64;
    for (unsigned i = 0; i < 8; i++)
      tank_read(s->tank[i], block[i], s->phase + i, s->damp + i, start,
                delays[i], 1024 + (int32_t)i * 73, 17000u + i * 2473u, damping);
    for (unsigned n = 0; n < 64; n++) {
      int32_t a = l[n], b = r ? r[n] : a;
      (void)b;
      if (apply_gate) {
        if (gate_target > s->gate_gain)
          s->gate_gain += (32767 - s->gate_gain + 31) >> 5;
        else if (gate_target < s->gate_gain)
          s->gate_gain -= (s->gate_gain * close_rate + 32767) >> 15;
      }
      unsigned eh = start + n - (unsigned)s->pre_samples;
      int32_t el = (s->pre[0][(eh - 149) & 4095] >> 2) +
                   (s->pre[1][(eh - 263) & 4095] >> 3) -
                   (s->pre[0][(eh - 421) & 4095] >> 4) +
                   (s->pre[1][(eh - 631) & 4095] >> 4);
      int32_t er = (s->pre[1][(eh - 173) & 4095] >> 2) +
                   (s->pre[0][(eh - 307) & 4095] >> 3) -
                   (s->pre[1][(eh - 487) & 4095] >> 4) +
                   (s->pre[0][(eh - 733) & 4095] >> 4);
      s->early[0] += mul(el - s->early[0], damping);
      s->early[1] += mul(er - s->early[1], damping);
      int32_t tl = block[0][n] + block[2][n] - block[4][n] - block[6][n];
      int32_t tr = block[1][n] + block[3][n] - block[5][n] - block[7][n];
      int32_t wl = sat(half(tl + half(tl)) + s->early[0]),
              wr = sat(half(tr + half(tr)) + s->early[1]);
      if (apply_gate) {
        wl = mul(wl, s->gate_gain);
        wr = mul(wr, s->gate_gain);
      }
      mix += step;
      int32_t wet = mix >> 6;
      int32_t out_l = sat(mul(wl, wet));
      int32_t out_r = sat(mul(wr, wet));
      l[n] = (int16_t)out_l;
      if (r)
        r[n] = (int16_t)out_r;
      int32_t pk = out_l < 0 ? -out_l : out_l;
      if (pk > s->peak)
        s->peak = pk;
      pk = out_r < 0 ? -out_r : out_r;
      if (pk > s->peak)
        s->peak = pk;
    }
    for (unsigned group = 0; group < 2; group++) {
      int32_t *input = group ? right : left;
      for (unsigned n = 0; n < 64; n++) {
        int32_t d0 = block[group * 4][n], d1 = block[group * 4 + 1][n];
        int32_t d2 = block[group * 4 + 2][n], d3 = block[group * 4 + 3][n];
        int32_t a0 = d0 + d1, a1 = d0 - d1, a2 = d2 + d3, a3 = d2 - d3;
        unsigned h = (start + n) & 4095;
        int32_t in = input[n] >> 2;
        s->tank[group][h] =
            (int16_t)sat(mul(sat(half(a0 + a2)), mw_decay[ix][group]) + in);
        s->tank[group + 2][h] =
            (int16_t)sat(mul(sat(half(a1 + a3)), mw_decay[ix][group + 2]) +
                         ((-input[n]) >> 2));
        s->tank[group + 4][h] =
            (int16_t)sat(mul(sat(half(a0 - a2)), mw_decay[ix][group + 4]) + in);
        s->tank[group + 6][h] =
            (int16_t)sat(mul(sat(half(a1 - a3)), mw_decay[ix][group + 6]) +
                         ((-input[n]) >> 2));
      }
    }
    s->mix = target_mix;
  }

  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  mw_state *state(int p) { return (mw_state *)scratch; }

  void init(int p) {
    mw_state *s = state(p);
    if (s)
      mw_init(s);
    params_valid[p] = 0;
  }

  void stereo(int p, int16_t *l, int16_t *r, const int16_t *params) {
    mw_state *s = state(p);
    if (!s)
      return;
    int32_t q[4];
    for (unsigned i = 0; i < 4; i++) {
      if (!params_valid[p] || raw_params[p][i] != params[i]) {
        raw_params[p][i] = params[i];
        normalized[p][i] = ((int32_t)(params)[i]);
      }
      father_live[p][i] = (int16_t)(q[i] = normalized[p][i]);
    }
    params_valid[p] = 1;
    mw_process(s, l, r, q);
    father_peak[p] = s->peak;
    father_gate[p] = (int16_t)s->gate_gain;
    father_blocks[p]++;
  }

  void mono(int p, int16_t *b, const int16_t *q) { stereo(p, b, 0, q); }
};
} // namespace port
} // namespace hkfather
