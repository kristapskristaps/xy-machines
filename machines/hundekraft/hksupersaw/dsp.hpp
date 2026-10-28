#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hksupersaw {
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
  uint32_t vtable;
  uint32_t ph1, ph2;
  uint32_t phs;
  int32_t lp;
  uint8_t gate;
} voice_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;

  voice_t note;
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  void ss_note_on(void *self, const int16_t *params) {
    voice_t *v = (voice_t *)self;
    (void)params;
    v->ph1 = 0;
    v->ph2 = 0x40000000u;
    v->phs = 0;
    v->lp = 0;
    v->gate = 1;
  }

  void ss_note_off(void *self) { ((voice_t *)self)->gate = 0; }

  void ss_render(void *self, int16_t *out, const int16_t *params) {
    voice_t *v = (voice_t *)self;
    const int32_t *inc = pitch_input;
    const int16_t *sine = sine_table;
    int32_t detune = params[0] >> 7;
    int32_t morph = params[1];
    int32_t sublvl = params[2];
    int32_t drive = 32768 + (params[3] << 2);
    uint32_t p1 = v->ph1, p2 = v->ph2, ps = v->phs;
    int32_t lp = v->lp;
    int32_t vg = 32767;
    for (int n = 0; n < 128; n++) {
      uint32_t i = (uint32_t)inc[n];
      uint32_t d = (i >> 14) * (uint32_t)detune;
      p1 += i + d;
      p2 += i - d;
      ps += i >> 1;
      int32_t s1 = (int32_t)p1 >> 16;
      int32_t s2 = (int32_t)p2 >> 16;
      int32_t s2m = s2 - op1_mul16(2 * s2, morph);
      int32_t x = (s1 + s2m) >> 1;
      x += op1_mul16(sine[((int32_t)ps >> 19) + 4096], sublvl) >> 1;
      x = (x * (drive >> 4)) >> 11;
      if (x > 32767)
        x = 32767;
      else if (x < -32767)
        x = -32767;
      {
        int32_t x2 = (x * x) >> 15;
        int32_t x3 = (x2 * x) >> 15;
        x -= (x3 * 10923) >> 15;
      }
      lp += (x - lp) >> 1;
      out[n] = op1_sat16(((lp - (lp >> 2)) * vg) >> 15);
    }
    v->ph1 = p1;
    v->ph2 = p2;
    v->phs = ps;
    v->lp = lp;
  }
};
} // namespace port
} // namespace hksupersaw
