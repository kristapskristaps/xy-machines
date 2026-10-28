#pragma once
#include "wide.hpp"
// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE,
// LICENSE.
namespace hkfmbell {
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
  uint32_t pc, pm;
  int32_t env;
  int16_t fb;
  uint8_t gate;
} fm_voice_t;

struct Dsp {
  const int32_t *pitch_input;
  uint8_t *scratch;
  uint8_t fx_page[3][4];
  op1_beat_t beat;
  int16_t meter;

  fm_voice_t note;
  void defaults() {}
  inline int16_t op1_sat16(int32_t x) {
    return x > 32767 ? 32767 : x < -32768 ? -32768 : (int16_t)x;
  }

  inline int32_t op1_mul16(int32_t a, int32_t b) { return (a * b) >> 15; }

  unsigned fm_ratio2(int32_t q) {
    return 1u + (((unsigned)(q < 0 ? 0 : q) >> 9) % 16u);
  }

  inline int32_t sine(uint32_t ph) {
    return sine_table[((int32_t)ph >> 19) + 4096];
  }

  void fm_note_on(void *self, const int16_t *params) {
    fm_voice_t *v = (fm_voice_t *)self;
    (void)params;
    v->pc = 0;
    v->pm = 0;
    v->env = 32767;
    v->fb = 0;
    v->gate = 1;
  }

  void fm_note_off(void *self) { ((fm_voice_t *)self)->gate = 0; }

  void fm_render(void *self, int16_t *out, const int16_t *params) {
    fm_voice_t *v = (fm_voice_t *)self;
    const int32_t *inc = pitch_input;
    uint32_t ratio2 = fm_ratio2(params[0]);
    int32_t index = params[1] >> 3;
    int32_t decay = 1 + (params[2] >> 9);
    int32_t fbamt = params[3] >> 4;
    uint32_t pc = v->pc, pm = v->pm;
    int32_t fb = v->fb;
    int32_t idx = (index * v->env) >> 15;
    int32_t vg = 32767;
    for (int n = 0; n < 128; n++) {
      uint32_t i = (uint32_t)inc[n];
      pm += (i >> 1) * ratio2;
      int32_t m = sine(pm + (uint32_t)(fb * fbamt * 32));
      fb = m;
      pc += i;
      out[n] = (int16_t)((sine(pc + (uint32_t)(m * idx * 16)) * vg) >> 16);
    }
    v->env -= (v->env * decay) >> 12;
    if (v->env < 0)
      v->env = 0;
    v->pc = pc;
    v->pm = pm;
    v->fb = (int16_t)fb;
  }
};
} // namespace port
} // namespace hkfmbell
