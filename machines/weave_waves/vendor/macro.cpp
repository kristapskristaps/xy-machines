/* Digi Mono's MACRO machine: engines ported from Plaits, in 32-bit integer arithmetic.
 *
 * The synthesis is Plaits' (github.com/pichenettes/eurorack, plaits/dsp), by Emilie Gillet, under the MIT
 * licence below; this file restates it in fixed point for a CPU without an FPU. Each engine says which
 * Plaits file it follows. Numbers: a phase is a 32-bit fraction of a cycle; a signal is Q15 (32768 = 1)
 * unless named Q16; a knob is 0..127 as the SRC page gives it.
 *
 * Copyright 2016 Emilie Gillet (the synthesis); the fixed-point port, the digi1_mods authors.
 * Model-TG changes: MACRO_SEL, the engines built in and their knob order (macro_engine_of, HAS()).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "macro.h"
#include <opxy/q31.hpp>
namespace weave_waves_dsp {
// Defined two's-complement shifts for signed audio samples.
constexpr int32_t defined_shift(int32_t value, unsigned bits) {
    return static_cast<int32_t>(static_cast<uint32_t>(value) << bits);
}

#include "macro_tables.h"
#include "macro_division.h"


// Excitation presets. Slot zero preserves every original fixed coefficient.




/* code run once a block or less (coefficients, starts): built for size, the mod's RAM is short */
#define COLD

/* ---- the multiply-accumulate unit ------------------------------------------------------------------- *
 * The filters multiply on the ColdFire's EMAC: signed fractional, truncating (MACSR 0x20), ACC0 only. Two
 * 32-bit products summed come out as (floor(a x / 2^23) + floor(b y / 2^23)) >> 8, which a PC build
 * computes in 64-bit C (the tests compare the two). A third product, 2^15 x 2^15, adds half the last bit:
 * rounded, not truncated (truncation's bias builds up in a resonant filter's states). macro_render() saves MACSR, ACC0, ACC1
 * (MODAL's filters use it too) and ACCEXT01 and puts them back, as Digi EQ does: the firmware's own audio code uses the unit. */
// OP-XY CPU variant: public Q31 truncating products. The original EMAC
// rounding described above is intentionally omitted: one product changes by
// at most one Q31 LSB, and the two-product sum by at most two. Sound-quality
// tests compare the resulting model behavior with the unchanged prior source.
static inline int32_t product_hi(int32_t a, int32_t b) {
    return opxy::q31::multiply(a, b);
}

static inline int32_t fmac1(int32_t a, int32_t x) {
    return product_hi(a,x);
}
static inline int32_t fmac1t(int32_t a, int32_t x) { return product_hi(a,x); }
struct emac_save { int32_t unused; };
static inline void emac_enter(emac_save*) {}
static inline void emac_leave(const emac_save*) {}

#define INC_MAX 0x40000000u                 /* Plaits' kMaxFrequency, 0.25 of the sample rate */
#define INC_MIN 4295u                       /* kMinFrequency, 1e-6                            */

/* ---- shared pieces ------------------------------------------------------------------------------- */

/* 0..127 -> Q16 (127 -> 65535) and Q15 */
static inline int32_t k16(int x) { return (x * 33026) >> 6; }




/* stmlib's integrated polyBLEP, Q16 in and out: NextIntegratedBlepSample(t) */


/* t = d / inc as a Q16 fraction of a sample, d < inc */


/* stmlib's InterpolateHermite over a Q15 table, index Q15 in 0..1 scaled by 512 (Plaits' fold tables) */


/* Plaits' Sine(phase) over lut_sine (512 points a cycle), phase Q15 in 0..1.25 */


/* waveshaping_engine.cc's Tame(): how much of a control to keep as the fundamental rises, Q15 */


/* A linear ramp from a block's start value to its target (stmlib's ParameterInterpolator). Q23.
 * RAMP_NEW in *prev (an engine just started): no ramp, the block starts at the target. */
#define RAMP_NEW ((int32_t)0x80000000)
struct ramp { int32_t v, d; };



/* 2^t for t in 0..1 (Q16), as Q16 (65536..131071): a quartic, within 5e-6 (0.01 cent) */
static inline uint32_t exp2_frac(uint32_t t)
{
    return 65536 + ((t * (45416 + ((t * (15831 + ((t * (3392 + ((t * 897) >> 16))) >> 16))) >> 16))) >> 16);
}

/* 2^x, x Q16 (any sign), as Q16 */
static uint32_t exp2_q16(int32_t x)
{
    int32_t e = x >> 16;
    uint32_t y = exp2_frac((uint32_t)x & 0xffff);
    if (e >= 0)
        return e > 14 ? 0x7fffffffu : y << e;
    return e < -16 ? 0 : y >> -e;
}

/* log2(x), x > 0, as Q16; a quartic on the mantissa, within 0.00015 */


/* The MIDI note of a phase increment, Q8 (note 69 = 440 Hz at 48 kHz) */


/* a x b / 65536 for a phase increment a and a ratio b (Q16, up to 2^19) */


/* The phase increment whose log2 is x (Q16): 2^x, saturating at 0xffffffff (a frequency of 1) */
static uint32_t inc_of_log2(int32_t x)
{
    int32_t e = x >> 16;
    uint32_t y;
    if (x >= (defined_shift(32, 16)))
        return 0xffffffffu;
    y = exp2_frac((uint32_t)x & 0xffff);
    if (e >= 16)
        return y << (e - 16);
    return e < -1 ? 0 : y >> (16 - e);
}

/* ---- coefficients and the state-variable filter ----------------------------------------------------- *
 * A filter coefficient is a float of sorts: m x 2^-s, the mantissa m in 16384..32767, 11 <= s <= 31 (a value
 * from about 2^-17 to 16, coef_fit()). cmul() multiplies a signal (|x| < 2^27) by it to full precision, in two 16 x 16
 * products: no 64-bit arithmetic. */
struct coef { int32_t m, s; };

/* leading zeros of v > 0 (in C, not the CPU's FF1, which the emulators the tests use do not know) */














/* stmlib's Svf, in A. Simper's form of the same trapezoidal filter, on the EMAC (Q31 coefficients):
 *   v3 = in - ic2;  bp = v1 = a1 ic1 + a2 v3;  lp = v2 = ic2 + a2 ic1 + a3 v3;  ic1 = 2 v1 - ic1;
 *   ic2 = 2 v2 - ic2;  hp = in - k bp - lp
 * a1 = 1 / (1 + g (g + k)), a2 = g a1, a3 = g a2 (k = r = 1/q). Signals Q24 (|x| < 128); the states ic1,
 * ic2 are struct macro_svf's s1, s2. */
struct svf_c { int32_t a1, a2, a3, k2; };                           /* Q31; k2 = k / 2 */



/* a1 = 1/D to 31 bits (a hardware division to 16, one Newton step on the EMAC), a2 = g a1, a3 = g a2 with
 * the same g: a filter as stable as the float one up to q 512 at the top of the band (where 1 - |pole|^2
 * is 5e-4 and a1, a2, a3 rounded to 15 bits each were not). */


/* g = tan(pi f), FREQUENCY_ACCURATE (Plaits' polynomial, tabulated as tan(pi f) / f), f up to 0.5 */


/* g = tan(pi f), FREQUENCY_DIRTY: f (pi + 3.736e-1 pi^3 f^2), f up to 0.25 */


/* stmlib's Svf::set_f_q<FREQUENCY_ACCURATE>: the frequency as a phase increment, the damping r = 1/q */


/* One sample: BP and LP (Q24) */
#define SVF_STEP(F, K, IN, BP, LP) do {                                                             \
        int32_t v3_ = (IN) - (F).s2;                                                               \
        BP = fmac2((K).a1, (F).s1, (K).a2, v3_);                                                   \
        LP = (F).s2 + fmac2((K).a2, (F).s1, (K).a3, v3_);                                          \
        (F).s1 = BP + BP - (F).s1;                                                                 \
        (F).s2 = LP + LP - (F).s2;                                                                 \
    } while (0)
#define SVF_HP(K, IN, BP, LP) ((IN) - 2 * fmac1((K).k2, BP) - (LP))


/* After a block: the states kept within +-32 (Q24), so nothing runs away out of range (Plaits' float filter
 * never needs it at the levels its engines feed it) */


/* stmlib's Limiter (as Plaits' voice applies it to the engines registered with a negative gain): a peak
 * follower (attack 0.05, release 0.00002 a sample) and 1/peak above 1. x: Q17 in, Q15 out (the 0.8 after
 * it is in the machine's gain). The peak is Q24: in Q17 the release step rounded to 15 % too fast. */


/* a random 32-bit word (stmlib's Random::GetWord()) */


/* a random Q15 value, -1..1 (stmlib's Random::GetFloat() x 2 - 1) */


/* Plaits' SinePM: the sine at a 32-bit phase (512 points a cycle, linear), Q15 */


/* ---- the slope oscillator: plaits/dsp/oscillator/oscillator.h, OSCILLATOR_SHAPE_SLOPE ------------ */



/* pw: Q16, already kept in 2f..1-2f. out: Q15. */


/* ---- WSH: plaits/dsp/engine/waveshaping_engine.cc ----------------------------------------------- */







/* Keep a slope oscillator's phase running over n samples without rendering them (the AUX path of an
 * engine while the AUX knob is at 0): the next sample starts from the plain slope, without a blep. */


struct wsh_coordinate { unsigned index, bits; int16_t fraction; };






/* ---- FM: single-rate, bandwidth-aware two-operator phase modulation ------------------------------ *
 * HARM keeps the frequency-ratio map; TIMBRE controls the phase index and MORPH
 * chooses negative frequency feedback or positive phase feedback.
 * Both operators run once per source frame. Index and feedback are smoothly
 * limited to the available sideband budget instead of generating four samples
 * and discarding three. The old oversampled renderer lives only in tests.
 */





struct fm_block {
    struct ramp amount,feedback,taming;
    uint32_t mod_current,mod_target;
    int32_t mod_step;
};




/* ---- NOISE: plaits/dsp/engine/noise_engine.cc ------------------------------------------------------- *
 * Two clocked noises (plaits/dsp/noise/clocked_noise.h) at TIMBRE's clock, the second's clock HARMONICS
 * apart; OUT: the first through a filter at the note's pitch, LP (HARMONICS 0) to BP to HP (1), MORPH its
 * resonance; AUX: two band-passes, at the note and HARMONICS (-2..+2 octaves) from it. A note start is
 * Plaits' trigger: both clocks restart, TIMBRE spans its patched range (-24..128). The filters' controls
 * are the block's (Plaits glides them over the block). */







/* ---- PARTICLE: plaits/dsp/engine/particle_engine.cc, plaits/dsp/noise/particle.h ---------------------- *
 * Six particles; each draws an impulse a sample with the probability TIMBRE's density sets, its height
 * random (0..1), into its own resonant band-pass (stmlib's Svf, FREQUENCY_DIRTY) at a frequency HARMONICS
 * spreads at random around the note (+-4 octaves at most), drawn again at the first impulse of a block.
 * OUT: their sum through a low-pass at the note; AUX: the impulses themselves. A note start makes every
 * particle fire at once (Plaits' trigger). MORPH: above its middle the band-passes' resonance; below it
 * Plaits adds a reverb-like diffuser (16 KB of delay a voice), which this port leaves out: there the
 * band-passes keep the resonance at the middle.
 *
 * Here the impulses come from exponential waiting times (the same Bernoulli process, drawn once an
 * impulse instead of every sample), and a particle whose band-pass has rung out and has no impulse in
 * the block is skipped. */



/* an exponential waiting time, Q26: -ln(u), u uniform in (0, 1] */


/* a particle's band-pass at f (log2, Q16, 2^-16..0.25), damping k = 1/q; its input's gain 2^lpre. The
 * filter is linear, so an impulse x adds to the step without it: bp and lp by a2 x and a3 x, s1 and s2 by
 * twice those (c1, c2: pre_gain a2, pre_gain a3). Pre_gain itself reaches the thousands at low
 * frequencies, c1 and c2 stay small. */






/* ---- shared by the drums ------------------------------------------------------------------------------ */

#define ONE_POLE(X, IN, C) ((X) += fmac1t((IN) - (X), (C)))        /* C: Q31 (truncated: a smoother) */
#define Q31(x) ((int32_t)((x) * 2147483648.0 + 0.5))                 /* a constant 0..1 as Q31 (compile time) */
#define Q24(x) ((int32_t)((x) * 16777216.0 + ((x) < 0 ? -0.5 : 0.5)))

/* y / (1 + |y|), Q24 in and out: tables (MACRO_RSAT_*), a division past |y| = 16 (rsat_far). Inline: it
 * runs a sample in the drums' loops. */




/* stmlib's SoftClip, Q24 (inline: a sample in the drums' loops) */


/* A drum's sound has died away when every state that feeds its output is below -96 dB (Q24): from then on
 * until the next trig its output is silence and nothing of it is computed. */
#define TINY(x) ((uint32_t)((x) + 256) < 512u)
/* (a resonator rounding to a fixed point a few hundred LSBs off zero at a low frequency: -72 dB counts) */
#define TINYR(x) ((uint32_t)((x) + 4096) < 8192u)

/* AnalogBassDrum's Diode(): x, or 0.7 x 2x / (1 + |2x|) below 0 (Q24) */


/* g = tan(pi f) for f in Q31 (<= 0.5), as Q27: FREQUENCY_DIRTY f (pi + 3.736e-1 pi^3 f^2) and FREQUENCY_FAST
 * f (pi + f^2 (3.260e-1 pi^3 + 1.823e-1 pi^5 f^2)) */




/* 1/x for x >= 1 (Qq, q 16..27) as Q31: a hardware division to 16 bits and a Newton step to 31 */

#define recip27(x) recip_q((x), 27)

/* An Svf's coefficients (struct svf_c, k2 left out) from g and g k (Q27): a1 = 1/D to 31 bits (the margin to instability, 2 g k / D, is 4e-5 at the longest
 * decays), a2 = g a1, a3 = g a2. */


/* k = 1 / (1 + q f) (qf: q f, Q8), Q31 */


/* A resonator: FREQUENCY_DIRTY at f (Q31, <= 0.4), q = 1 + q f */


/* stmlib's OnePole (FREQUENCY_FAST at f): G = g / (1 + g), Q31; lp = s + G (in - s), s = 2 lp - s */

#define ONEPOLE_LP(S, G, IN, LP) do {                                                              \
        LP = (S) + fmac1((IN) - (S), (G));                                                         \
        (S) = LP + LP - (S);                                                                       \
    } while (0)

/* ---- BD: plaits/dsp/engine/bass_drum_engine.cc, drums/analog_bass_drum.h, drums/synthetic_bass_drum.h --- *
 * OUT: the analog-style drum (a pulse into a resonator that its own output and an attack pulse bend in
 * pitch, then the overdrive); AUX: the synthetic one (a distorted sine with a pitch envelope, a click and
 * noise). HARM: attack FM, self FM, drive; TIMB: tone; MORP: decay. Accent is Plaits' unpatched 0.8; the
 * drums are triggered (Plaits' patched trigger), never free-running. The resonator's pitch is updated
 * every sample during the attack's pitch sweep (the first 7 ms), then every 8 samples (Plaits: every
 * sample; measured: the decay's level within 0.3 dB); its other arithmetic is Plaits', in Q24. */



#define BD_EVERY 7                                    /* the resonator's pitch: every 8 samples after the attack */


/* SyntheticBassDrum's DistortedSine: a triangle bent by t / (1 + |t|), towards a clean sine as
 * dirtiness falls; the phase jittered by the phase noise. Q24 */




/* Plaits' Overdrive (on OUT): drive 0.5 + 0.5 max(2h - 1, 0) max(1 - 16 f0, 0) */




/* ---- SD: plaits/dsp/engine/snare_drum_engine.cc, drums/analog_snare_drum.h, drums/synthetic_snare_drum.h ---- *
 * OUT: the analog-style snare (a pulse into five resonant modes, 808-like up to TIMBRE 2/3 then more modes,
 * soft-clipped, plus band-passed noise); AUX: the synthetic one (two coupled distorted sines and filtered
 * noise with their own envelopes). HARM: snappy (noise against shell); TIMB: tone (OUT), FM (AUX); MORP:
 * decay. Accent 0.8, triggered. */



static const int32_t sd_ratio28[5] = {268435456, 536870912, 853624750, 1116691497, 1508608262};   /* 1, 2, 3.18, 4.16, 5.62 */



/* SyntheticSnareDrum's DistortedSine: t = 4 p - 1.3 (p mirrored past 0.5), 2 t / (1 + |t|); p Q28 */






/* ---- HH: plaits/dsp/engine/hi_hat_engine.cc, drums/hi_hat.h ------------------------------------------- *
 * OUT: 808-style metallic noise (six square oscillators), a resonant band-pass, clocked noise mixed in by
 * HARMONICS, a "swing" VCA and a high-pass; AUX: three ring-modulated square x saw pairs, a band-pass, a
 * linear VCA with a two-stage envelope, a high-pass. TIMB: tone (the filters' frequency); MORP: decay. */



static const uint32_t hh_sq_ratio28[6] = {268435456, 350039835, 393526378, 479694160, 518617301, 680752316};
static const uint32_t hh_rm_inc[6] = {17895697, 673772995, 45634028, 722538769, 65319294, 939524096};

/* Plaits' Oscillator, SQUARE (pw 0.5) or SAW, one sample, Q15 (band-limited steps: polyBLEP) */




/* ---- GRAIN: plaits/dsp/engine/grain_engine.cc, oscillator/grainlet_oscillator.h, z_oscillator.h ------ *
 * OUT: two "grainlets" (a sine formant hard-synced to a shaped carrier), the second's formant HARMONICS
 * away (-2..+2 octaves), summed, DC-blocked; AUX: the Z oscillator (a formant with a sine discontinuity).
 * TIMB: formant frequency; MORP: carrier shape; HARM: formant ratio and carrier bleed (OUT), the
 * discontinuity's shape (AUX). Controls are the block's (Plaits glides them over the block). */



/* a sine of a Q32 phase, Q24 */


struct carrier_c { int32_t seg, m27, bp31, a22, b31; };

/* GrainletOscillator::Carrier for a block's shape: the warped phase's parameters */


/* the carrier, Q24: (Sine(warped phase) + 1) / 4; p: Q31, 0..1 inclusive */




/* ---- the machine ---------------------------------------------------------------------------------- */

/* ---- CHORD: direct periodic-table adaptation of Plaits' voicings ------------------------------- *
 * Eleven chord banks, four notes crossfaded through five oscillator slots as TIMBRE rotates inversions.
 * MORPH moves through eight organ registrations and then fifteen differentiated wavetable colors.
 * Offline harmonic mips replace the divide-down BLEPs and integrated-wave reconstruction. OUT is the
 * complete voicing; AUX contains only root slots, x3. All five phases keep advancing. */





static const int32_t chord_fade[MACRO_CHORD_VOICES] = { 36045, 30802, 32113, 33423, 34734 };   /* Q16 */

/* One-pole smoothing of MORPH and TIMBRE: Plaits' 0.1 a CHORD_BLOCK-sample block. The tests set CHORD_BLOCK to
 * their own block size, and CHORD_FROM_ZERO to start both at 0 as Plaits' Init does; built for the Cycles, they
 * start at the knobs, so a p-lock that switches to CHORDS does not sweep through the inversions first. */
#define CHORD_BLOCK 12
#define CHORD_LP_NEW (-1)



/* the high 32 bits of a x b, exactly */
static inline uint32_t mulhi(uint32_t a, uint32_t b)
{
    uint32_t ah = a >> 16, al = a & 0xffff, bh = b >> 16, bl = b & 0xffff;
    uint32_t m1 = ah * bl, m2 = al * bh, lo = al * bl;
    return ah * bh + (m1 >> 16) + (m2 >> 16) + (((m1 & 0xffff) + (m2 & 0xffff) + (lo >> 16)) >> 16);
}

/* x << s, saturating at 0xffffffff */


/* Direct periodic keys replace divide-down BLEPs and differentiated integrated waves.
 * The chord bank/inversion mapping below remains independent of the oscillator. */


// A packed periodic point holds its signed16 value and signed16 slope.
// Playback needs one aligned word and one signed16 interpolation product.


static inline int32_t chord_output_blend(int32_t main,int32_t auxiliary,int16_t weight) {
    const int32_t gain=defined_shift(26214,16);
    int32_t x=fmac1t(main,gain),y=fmac1t(auxiliary,gain);
    x=x>32767?32767:x< -32768? -32768:x;
    y=y>32767?32767:y< -32768? -32768:y;
    const int16_t half=static_cast<int16_t>((y-x)/2);
    return x+fmac1t(defined_shift(half,16),static_cast<int32_t>(weight)*2);
}













/* ---- SWARM (Model-TG): plaits/dsp/engine/swarm_engine.cc ------------------------------------------- *
 * Eight voices, each a grain envelope (re-randomised as its phase wraps, once a block) shaping a saw (OUT)
 * and a sine (AUX) at the note spread over +-4 octaves by HARM^3 and the voice's rank. TIMB: grain density
 * (NoteToFrequency(120 TIMBRE) x 0.025 a sample); MORP: grain size (below a size of 1 the grains glide, from
 * one random pitch to the next). A trig is Plaits' trigger, patched: a burst. OUT goes through the limiter at
 * x 3, as Plaits registers the engine with -3. Accumulated in Q24. */

#define NOTE_L0 1653765                     /* log2 of NoteToFrequency(69) x 2^32, at Plaits' 47872.34 Hz, Q16 */

/* rank (rank + 0.01) / 4, Q30, for the four selected ranks -1, -1/3, 1/3, 1. */
static const int32_t swarm_lin[MACRO_SWARM_VOICES] = {
    265751101, 28931377, 30720947, 271119811
};



/* GrainEnvelope::Step (rate Q30), burst mode (the trigger is patched) */


/* the voice's sine shaped amplitude and its pitch offset (GrainEnvelope::amplitude, frequency); sr Q16 */




/* AdditiveSawOscillator::Render, gain Q26, adds into out (Q24) */




/* FastSineOscillator::Render, ADDITIVE: amp Q25 (before 1 - 4f), adds into out (Q24) */




/* ---- WAVETABLE (Model-TG): plaits/dsp/engine/wavetable_engine.cc ----------------------------------- *
 * 192 integrated waves in 4 banks of 8 x 8 (the fourth Plaits' reshuffle of the others). TIMB: X, MORP: Y,
 * HARM: the bank (Z, mirrored 0 1 2 3 3 2 1 0 over its range; above 3/7 the positions snap towards whole waves).
 * Eight source waves are blended trilinearly into a double-buffered Q8 period, rebuilt in bounded tiles.
 * Playback reads one Hermite-interpolated cached wave (two during a 64-sample crossfade), differentiated
 * (the waves are stored integrated), scaled by 1 / f once a block and low-passed at 128 f. AUX: OUT truncated to steps of 1/32.
 * X Y Z go through Plaits' one-poles (0.2, 0.2, 0.05 a 12-sample block, rescaled; WT_BLOCK for the tests), then
 * a per-sample one-pole at 2 f (4 - 3 quantization), 0.01..0.1. On the box they start at the knobs. */

#include "macro_wavetable.h"

#define WT_BLOCK 12
#define WT_NEW (-1)

static COLD void wt_init(struct macro_wavetable *w)
{
    int32_t *p = (int32_t *)w, *end = (int32_t *)(w + 1);
    while (p < end)
        *p++ = 0;
    w->f = 4934440u;                                    /* previous_f0_ = a0 */
    w->pre[0] = WT_NEW;
}

/* InterpolateWaveHermite over a wave (Q8 out), at sample i (0..127) and fraction f (Q31) */
static inline int32_t wt_hermite(const int32_t *t, int32_t i, int32_t f)
{
    int32_t xm1=t[i],x0=t[i+1],x1=t[i+2],x2=t[i+3];
    int32_t c=(x1-xm1)>>1,v=x0-x1,w=c+v,a=w+v+((x2-x0)>>1),bn=w+a;
    return opxy::q31::multiply(opxy::q31::multiply(opxy::q31::multiply(a,f)-bn,f)+c,f)+x0;
}

static inline const int16_t *wt_wave(int x, int y, int z)
{
    int i = x + y * 8;
    return wt_waves + 132 * (z < 3 ? z * 64 + i : wt_bank3[i]);
}

/* Clamp(): a fraction (Q24) pushed towards 0 or 1, (f - 0.5) x 16 + 0.5 within 0..1 */
static inline int32_t wt_snap(int32_t f)
{
    int32_t v = (defined_shift((f - (defined_shift(1, 23))), 4)) + (defined_shift(1, 23));
    return v < 0 ? 0 : v > (defined_shift(1, 24)) ? defined_shift(1, 24) : v;
}

// Blend fixed coordinates once per tile, before reading the oscillator phase.
// A fixed quota bounds work during continuous XYZ modulation as well as at rest.
template<unsigned Step>
static __attribute__((noinline)) void wt_blend_tile(const int32_t* key,int32_t* table,unsigned first,unsigned end)
{
    int32_t cell[3],fraction[3];
    for(unsigned k=0;k<3;++k) {
        int32_t value=key[k];
        if(value<0)value=0;
        if(value>117440511)value=117440511;
        cell[k]=value>>24;fraction[k]=(value&0xffffff)<<7;
    }
    const int x=cell[0],y=cell[1],z=cell[2];
    const int z0=z>=4?7-z:z,z1=z+1>=4?6-z:z+1;
    const int16_t* waves[8]={wt_wave(x,y,z0),wt_wave(x+1,y,z0),wt_wave(x,y+1,z0),wt_wave(x+1,y+1,z0),
                             wt_wave(x,y,z1),wt_wave(x+1,y,z1),wt_wave(x,y+1,z1),wt_wave(x+1,y+1,z1)};
    const int32_t xf=fraction[0],yf=fraction[1],zf=fraction[2];
    for(unsigned i=first;i<end;i+=Step) {
        int32_t a0=defined_shift(waves[0][i],8),a1=defined_shift(waves[1][i],8);
        int32_t b0=defined_shift(waves[2][i],8),b1=defined_shift(waves[3][i],8);
        a0+=fmac1(a1-a0,xf);b0+=fmac1(b1-b0,xf);
        const int32_t m0=a0+fmac1(b0-a0,yf);
        a0=defined_shift(waves[4][i],8);a1=defined_shift(waves[5][i],8);
        b0=defined_shift(waves[6][i],8);b1=defined_shift(waves[7][i],8);
        a0+=fmac1(a1-a0,xf);b0+=fmac1(b1-b0,xf);
        const int32_t m1=a0+fmac1(b0-a0,yf);
        table[i]=m0+fmac1(m1-m0,zf);
    }
}

static void wt_cache(struct macro_wavetable* w,unsigned n)
{
    if(!w->valid) {
        // Start from a 32-point approximation of the requested wave, then refine
        // its full 128-point period through the same bounded tile path.
        wt_blend_tile<4>(w->lp,w->blend[0],1,129);
        for(unsigned i=1;i<129;i+=4) {
            const int32_t from=w->blend[0][i],to=w->blend[0][i==125?1:i+4];
            const int32_t step=(to-from)/4;
            w->blend[0][i+1]=from+step;
            w->blend[0][i+2]=from+2*step;
            w->blend[0][i+3]=from+3*step;
        }
        w->blend[0][0]=w->blend[0][128];
        w->blend[0][129]=w->blend[0][1];w->blend[0][130]=w->blend[0][2];w->blend[0][131]=w->blend[0][3];
        w->active=0;w->valid=1;w->building=1;w->build_at=0;
        for(unsigned k=0;k<3;++k)w->build_key[k]=w->lp[k];
    }
    if(!w->building && !w->fade) {
        bool changed=false;
        for(unsigned k=0;k<3;++k)if(w->cache_key[k]!=w->lp[k])changed=true;
        if(changed) {
            w->building=1;w->build_at=0;
            for(unsigned k=0;k<3;++k)w->build_key[k]=w->lp[k];
        }
    }
    if(w->building) {
        const unsigned quota=n/2?n/2:1;
        unsigned end=w->build_at+quota;if(end>132)end=132;
        wt_blend_tile<1>(w->build_key,w->blend[w->active^1],w->build_at,end);
        w->build_at=end;
        if(end==132) {
            w->building=0;w->fade=64;
            for(unsigned k=0;k<3;++k)w->cache_key[k]=w->build_key[k];
        }
    }
}

static void wt_render(struct macro_wavetable *__restrict w, const uint8_t *p, uint32_t inc, int32_t *__restrict out,
                      int32_t *__restrict aux, int n)
{
    static const int32_t lg[3] = { -21098, -21098, -4850 };          /* log2 (1 - 0.2), (1 - 0.2), (1 - 0.05), Q16 */
    int32_t knob[3], tg[3], d[3], cur[3], q, lpc, i, k, sh, z;
    uint32_t f = w->f, ph = w->phase, in, r16;
    int32_t fd, prev = w->prev, dlp = w->dlp;
    knob[0] = defined_shift(k16(p[MACRO_P_TIMB]) * 7, 8);                       /* x 6.9999, Q24 */
    knob[1] = defined_shift(k16(p[MACRO_P_MORPH]) * 7, 8);
    knob[2] = defined_shift(k16(p[MACRO_P_HARM]) * 7, 8);
    for (k = 0; k < 3; k++)
        knob[k] -= knob[k] >> 16;                                   /* (7 - 0.0001) */
    if (w->pre[0] == WT_NEW)                                        /* the engine just started: no sweep */
        for (k = 0; k < 3; k++)
            w->pre[k] = w->tgt[k] = w->lp[k] = knob[k];
    for (k = 0; k < 3; k++) {
        int32_t cp = 65536 - (int32_t)exp2_q16(lg[k] * n / WT_BLOCK);
        // OP-XY: preserve the full smoother product before its six-bit shift.
        const int32_t delta=(knob[k]-w->pre[k])>>8, coefficient=cp>>2;
        w->pre[k] += (delta>>6)*coefficient+((delta&63)*coefficient>>6);
    }
    q = w->pre[2] - (defined_shift(3, 24));                                      /* quantization: Z - 3 in 0..1, Q24 */
    q = q < 0 ? 0 : q > (defined_shift(1, 24)) ? defined_shift(1, 24) : q;
    {   /* 2 f0 (4 - 3 quantization) in 0.01 .. 0.1, Q31; f0 = inc / 2^32 */
        uint32_t m = (4u << 24) - 3u * (uint32_t)q;                 /* Q24, 1..4 */
        uint32_t c = mulhi(inc, m << 5) << 3;                       /* f0 m x 2, Q31 (m << 6 would wrap at 4) */
        lpc = c < 21474836u ? 21474836 : c > 214748365u ? 214748365 : (int32_t)c;
    }
    for (k = 0; k < 3; k++) {                                       /* the target, its fraction snapped */
        int32_t ip = w->pre[k] & ~0xffffff, fr = w->pre[k] & 0xffffff;
        fr += fmac1(wt_snap(fr) - fr, q >= (defined_shift(1, 24)) ? 0x7fffffff : defined_shift(q, 7));   /* (1.0 << 7 would wrap) */
        tg[k] = ip + fr;
        d[k] = divide_block(tg[k] - w->tgt[k], n);
        cur[k] = w->tgt[k];
        w->tgt[k] = tg[k];
    }
    wt_cache(w,static_cast<unsigned>(n));
    fd = divide_block((int32_t)inc - (int32_t)f, n);
    /* y = d / (128 f), table units a table step, Q8: d x 2^25 / inc (as wtv_render) */
    for (z = 0, in = inc; !(in & 0x80000000u); z++)
        in <<= 1;
    r16 = 0xffffffffu / (in >> 16);
    sh = z - 12;
    {
        int32_t g31 = 2040109466 - (int32_t)(inc >> 1);             /* 0.95 - f0, Q31 */
        for (i = 0; i < n; i++) {
            int32_t pi,pf,mix,dd,y,c;
            f += (uint32_t)fd;
            for (k = 0; k < 3; k++) {
                cur[k] += d[k];
                w->lp[k] += fmac1(cur[k] - w->lp[k], lpc);
            }
            ph += f;
            pi=static_cast<int32_t>(ph>>25);
            pf=static_cast<int32_t>((ph<<7)>>1);
            mix=wt_hermite(w->blend[w->active],pi,pf);
            if(w->fade) {
                const int32_t next=wt_hermite(w->blend[w->active^1],pi,pf);
                const int32_t alpha=static_cast<int32_t>((64-w->fade)*33554432u);
                mix+=fmac1(next-mix,alpha);
                if(!--w->fade)w->active^=1;
            }
            dd = mix - prev;
            prev = mix;
            y = fmac1(defined_shift(dd, 6), (int32_t)(r16 << 14));
            y = sh >= 0 ? defined_shift(y, sh) : y >> -sh;
            c = f >= (1u << 25) ? 0x7fffffff : (int32_t)(f << 6);   /* min(128 f, 1) */
            dlp += fmac1(y - dlp, c);
            out[i] = fmac1(defined_shift(dlp, 2), g31) >> 5;                     /* x (0.95 - f) / 1024, Q15 */
            aux[i] = (out[i] / 1024) * 1024;                        /* int(x 32) / 32 */
        }
    }
    w->f = inc;
    w->phase = ph;
    w->prev = prev;
    w->dlp = dlp;
}

/* Model-TG / Modded-Cycles: the phase increment (at 48 kHz) of a MIDI note, Q16 (note 69 = 440 Hz) */
uint32_t macro_inc_of_note(int32_t note_q16)
{
    return inc_of_log2(1653513 + (note_q16 - (defined_shift(69, 16))) / 12);
}

/* ---- VA (Model-TG): plaits/dsp/engine/virtual_analog_engine.cc (VA_VARIANT 2),
 * oscillator/variable_shape_oscillator.h, variable_saw_oscillator.h ------------------------------------- *
 * OUT: a variable-width square (TIMBRE: width, then hard sync above the middle) plus a variable saw (MORPH: notch
 * to triangle), detuned by HARMONICS (unison, fifth, octave, ... both ways); AUX: two variable-shape oscillators
 * (MORPH: triangle, saw, square, width) in "monster sync" (TIMBRE). Signals Q28 inside, the edges corrected with
 * polyBLEP and integrated polyBLEP as in Plaits; the slopes' reciprocals are the block's. */

          /* Q28 x Q28 */
   /* x a, a Q16 */






/* 1 / pw and 1 / (1 - pw), Q15, for a pulse width pw (Q31) */


/* the naive variable shape (saw, square, triangle) at phase ph, Q28; sq, tri: Q16 amounts */




/* VariableShapeOscillator::Render (sync: master mf, slave sf); writes Q28 into out */


#define NOTCH (53687091)                                     /* kVariableSawNotchDepth 0.2, Q28 */



/* VariableSawOscillator::Render: writes Q28 into out */




/* SemitonesToRatio's tables, Q31: 2^(k / 12) and 2^(k / 256 / 12) */
static const uint32_t pitch_semi[12] = {
    2147483648u, 2275179671u, 2410468894u, 2553802834u, 2705659852u, 2866546760u,
    3037000500u, 3217589947u, 3408917802u, 3611622603u, 3826380858u, 4053909305u,
};
static const uint32_t pitch_fine[256] = {
    2147483648u, 2147968248u, 2148452957u, 2148937775u, 2149422703u, 2149907740u,
    2150392887u, 2150878143u, 2151363509u, 2151848984u, 2152334569u, 2152820263u,
    2153306067u, 2153791980u, 2154278004u, 2154764136u, 2155250379u, 2155736731u,
    2156223193u, 2156709765u, 2157196447u, 2157683238u, 2158170140u, 2158657151u,
    2159144272u, 2159631503u, 2160118844u, 2160606295u, 2161093856u, 2161581527u,
    2162069308u, 2162557199u, 2163045200u, 2163533311u, 2164021532u, 2164509864u,
    2164998306u, 2165486858u, 2165975520u, 2166464293u, 2166953175u, 2167442169u,
    2167931272u, 2168420486u, 2168909810u, 2169399245u, 2169888790u, 2170378446u,
    2170868212u, 2171358088u, 2171848076u, 2172338173u, 2172828382u, 2173318700u,
    2173809130u, 2174299670u, 2174790321u, 2175281083u, 2175771955u, 2176262939u,
    2176754033u, 2177245237u, 2177736553u, 2178227980u, 2178719517u, 2179211165u,
    2179702925u, 2180194795u, 2180686776u, 2181178868u, 2181671072u, 2182163386u,
    2182655811u, 2183148348u, 2183640996u, 2184133755u, 2184626625u, 2185119606u,
    2185612699u, 2186105903u, 2186599218u, 2187092644u, 2187586182u, 2188079831u,
    2188573592u, 2189067464u, 2189561447u, 2190055542u, 2190549748u, 2191044066u,
    2191538496u, 2192033037u, 2192527690u, 2193022454u, 2193517330u, 2194012317u,
    2194507417u, 2195002628u, 2195497950u, 2195993385u, 2196488931u, 2196984590u,
    2197480360u, 2197976241u, 2198472235u, 2198968341u, 2199464559u, 2199960888u,
    2200457330u, 2200953884u, 2201450549u, 2201947327u, 2202444217u, 2202941219u,
    2203438333u, 2203935560u, 2204432898u, 2204930349u, 2205427912u, 2205925587u,
    2206423375u, 2206921275u, 2207419287u, 2207917412u, 2208415649u, 2208913999u,
    2209412461u, 2209911035u, 2210409722u, 2210908522u, 2211407434u, 2211906458u,
    2212405596u, 2212904845u, 2213404208u, 2213903683u, 2214403271u, 2214902972u,
    2215402785u, 2215902712u, 2216402751u, 2216902903u, 2217403167u, 2217903545u,
    2218404036u, 2218904639u, 2219405356u, 2219906185u, 2220407128u, 2220908183u,
    2221409352u, 2221910633u, 2222412028u, 2222913536u, 2223415157u, 2223916892u,
    2224418739u, 2224920700u, 2225422774u, 2225924961u, 2226427262u, 2226929676u,
    2227432204u, 2227934844u, 2228437599u, 2228940466u, 2229443447u, 2229946542u,
    2230449750u, 2230953072u, 2231456507u, 2231960056u, 2232463719u, 2232967495u,
    2233471385u, 2233975388u, 2234479506u, 2234983737u, 2235488082u, 2235992540u,
    2236497113u, 2237001799u, 2237506600u, 2238011514u, 2238516542u, 2239021684u,
    2239526940u, 2240032310u, 2240537794u, 2241043393u, 2241549105u, 2242054931u,
    2242560872u, 2243066927u, 2243573095u, 2244079379u, 2244585776u, 2245092288u,
    2245598914u, 2246105654u, 2246612509u, 2247119478u, 2247626561u, 2248133759u,
    2248641071u, 2249148498u, 2249656039u, 2250163695u, 2250671465u, 2251179350u,
    2251687350u, 2252195464u, 2252703693u, 2253212037u, 2253720495u, 2254229068u,
    2254737756u, 2255246558u, 2255755475u, 2256264508u, 2256773655u, 2257282917u,
    2257792294u, 2258301786u, 2258811392u, 2259321114u, 2259830951u, 2260340903u,
    2260850970u, 2261361152u, 2261871449u, 2262381861u, 2262892389u, 2263403032u,
    2263913790u, 2264424663u, 2264935651u, 2265446755u, 2265957974u, 2266469309u,
    2266980759u, 2267492324u, 2268004005u, 2268515801u, 2269027713u, 2269539740u,
    2270051883u, 2270564141u, 2271076515u, 2271589004u, 2272101610u, 2272614330u,
    2273127167u, 2273640119u, 2274153187u, 2274666371u,
};

/* inc x 2^(semitones / 12), semitones Q16, read in steps of 1/256 semitone as Plaits' NoteToFrequency does */




/* ---- MODAL (Model-TG): plaits/dsp/engine/modal_engine.cc, physical_modelling/modal_voice.cc, resonator.cc ---------- *
 * A struck resonator: 24 band-pass modes, their spacing set by HARMONICS (stiffness: from bell-like through the
 * harmonic series to stretched), excited on each trig by a click filtered by TIMBRE (brightness); MORPH is the
 * damping. OUT: the modes (Plaits limits it); AUX: the click. The modes' Q goes into the tens of thousands, where the
 * Q23 denominator of svf_coefs_g cannot tell the damping apart, so their coefficients are worked out with a 32-bit
 * mantissa and an exponent (struct uf), and the filters run in svf_hq's form of the same trapezoidal SVF. */

#define MODAL_BLOCK 12
#define MODAL_LP_NEW (-1)

/* x = m 2^(e - 30), m in [2^30, 2^31), or m = 0 for 0: 31-bit mantissas, so a product is one fmac1 on the EMAC */







/* 1 / a: a 16-bit quotient, then two Newton steps */




/* 2^(semitones / 12), semitones Q16, floored to 1/256 semitone as SemitonesToRatio reads its tables */


/* OnePole::tan<FREQUENCY_FAST>: f (pi + f^2 (a + b f^2)), f a phase increment up to 0.499 (below 2^31) */


/* a trapezoidal SVF's a1 = 1 / D, a2 = g / D, a3 = g^2 / D, D = 1 + g (g + 1/q), as q / (q (1 + g^2) + g) */


static const int32_t modal_stiffness[65] = {                       /* lut_stiffness, Q24 */
    -1048576, -983040, -917504, -851968, -786432, -720896, -655360, -589824,
    -524288, -458752, -393216, -327680, -262144, -196608, -131072, -65536,
    0, 0, 0, 0, 16938, 40535, 67147, 97158,
    131003, 169172, 212218, 260762, 315508, 377248, 446875, 525398,
    613951, 713818, 826443, 953456, 1096695, 1258232, 1440407, 1645855,
    1877549, 2138843, 2433517, 2765837, 3140610, 3563262, 4039909, 4577448,
    5183658, 5867313, 6638307, 7507796, 8488364, 9594201, 10841311, 12247741,
    13833846, 15622578, 16777848, 16871825, 17582612, 19864067, 24446104, 33554432,
    33554432,
};
static const int32_t modal_amp[24] = {               /* Resonator::Init(0.015): cosine x 0.25, Q31 */
    536870912, 535904512, 533012352, 528215168, 521547521, 513057472,
    502806079, 490867199, 477326785, 462282304, 445842112, 428124608,
    409257280, 389376031, 368623999, 347150593, 325110431, 302662207,
    279967553, 257189855, 234493120, 212040784, 189994496, 168513008,
};

/* one mode over the block: SVF_STEP, its band-pass x gain added into out. On the ColdFire, by hand: the two
 * sums on ACC0 and ACC1 side by side, the same products and rounding as fmac2 / fmac1 (so the same samples). */






/* ---- STRING (Model-TG): plaits/dsp/engine/string_engine.cc, physical_modelling/string_voice.cc, string.cc ------ *
 * Three Karplus-Strong strings in turn (each trig plucks the next, the others ring on): a burst of filtered noise one
 * period long (TIMBRE: brightness), a delay line with a damping low-pass (MORPH: decay, endless at the top) and, from
 * HARMONICS, a curved bridge (below the middle) or dispersion through an allpass (above). OUT: the strings (Plaits
 * limits it); AUX: the bursts. Signals Q24, delays Q20 (samples; up to 2048, as dispersion and the curved bridge stretch them past 1020). */

static const int32_t string_shift[129] = {                         /* lut_svf_shift[0..128], Q31 */
    1610612736, 1630343906, 1650009481, 1669544948, 1688887922, 1707979097,
    1726763091, 1745189139, 1763211629, 1780790470, 1797891299, 1814485525,
    1830550242, 1846068024, 1861026615, 1875418564, 1889240793, 1902494149,
    1915182937, 1927314462, 1938898579, 1949947278, 1960474287, 1970494717,
    1980024741, 1989081306, 1997681885, 2005844259, 2013586331, 2020925971,
    2027880884, 2034468504, 2040705906, 2046609740, 2052196174, 2057480857,
    2062478892, 2067204814, 2071672582, 2075895575, 2079886593, 2083657863,
    2087221051, 2090587270, 2093767101, 2096770602, 2099607332, 2102286367,
    2104816317, 2107205347, 2109461196, 2111591196, 2113602289, 2115501047,
    2117293690, 2118986101, 2120583846, 2122092186, 2123516097, 2124860281,
    2126129183, 2127327001, 2128457705, 2129525044, 2130532557, 2131483591,
    2132381305, 2133228681, 2134028538, 2134783536, 2135496188, 2136168866,
    2136803810, 2137403133, 2137968834, 2138502796, 2139006799, 2139482524,
    2139931555, 2140355389, 2140755441, 2141133044, 2141489457, 2141825870,
    2142143403, 2142443117, 2142726011, 2142993028, 2143245061, 2143482949,
    2143707486, 2143919422, 2144119463, 2144308278, 2144486495, 2144654710,
    2144813485, 2144963348, 2145104800, 2145238314, 2145364334, 2145483281,
    2145595553, 2145701523, 2145801545, 2145895954, 2145985064, 2146069173,
    2146148561, 2146223494, 2146294221, 2146360978, 2146423989, 2146483462,
    2146539599, 2146592584, 2146642595, 2146689800, 2146734355, 2146776410,
    2146816104, 2146853570, 2146888934, 2146922313, 2146953818, 2146983555,
    2147011623, 2147038116, 2147063122,
};

/* OnePole::tan<FREQUENCY_DIRTY>: f (pi + a f^2) */






/* the delay line read between taps (Read), and with Hermite interpolation (ReadHermite); d Q20 */




/* one string over the block: StringVoice::Render (no sustain) and String::Process */




/* ---- 6-OP (Model-TG): plaits/dsp/engine2/six_op_engine.cc, plaits/dsp/fm/ (voice.h, operator.h, envelope.h,
 * lfo.h, dx_units.h, algorithms.cc) ---------------------------------------------------------------------------- *
 * Plaits' DX7 engine with its three factory banks as one: HARMONICS picks one of 96 patches, TIMBRE brightens the
 * modulators, MORPH scales the envelope times. Two voices take the trigs in turn, rendered one per block over two
 * blocks (Plaits' staggered rendering). The gate is a fixed pulse from each trig (DX7_GATE_BLOCKS).
 * Operator outputs are phase offsets in turns, Q26 (Plaits quantizes them to 2^-26 too); amplitudes Q27; envelope
 * levels Q24 (log2 units of 1/8); envelope phases Q30. */



#define DX7_GATE_BLOCKS 188                                         /* 125 ms: a 16th at 120 BPM (10 ms left the slow */
#define DX7_PREVIOUS (-0x7fffffff)                                  /* Envelope's PREVIOUS_LEVEL */
#define DX7_LEVEL_67 112407347                                      /* 6.7, Q24 */

/* Pow2Fast<1> and Pow2Fast<2> (the float tricks Plaits' FM code uses), x Q16 */








/* Envelope::Init(scale): the levels and increments Voice::Init leaves before a patch is set up */




/* Envelope::Render(gate, rate, ad_scale, release_scale) */


/* PitchEnvelopeIncrement(rate): (1 + 192 r (r^4 + 0.3333)) / (21.3 x 44100), r = rate / 100 */


/* the envelopes, the ratios and the LFO for a patch (Voice::Setup and Lfo::Set) */


/* Lfo::Step(scale) */


/* (pitch_mod, Q27; amp_mod, Q28) from a voice's LFO */


/* RenderOperators<1, src, add>, one operator: src -2 the input buffer, -1 none, 0 its own feedback. Written for
 * constant src and add, so each call site below gets its own loop. */


/* RenderOperators<n, modulation_source, additive> */


/* KeyboardScaling(note, ks), note Q16; level units Q16 */


/* Voice::Render over size samples into the three buffers b (out, then two scratch) */








/* the gains Plaits' voice gives each engine's OUT and AUX (voice.cc, RegisterInstance), Q15. An engine
 * Plaits registers with a negative gain goes through its limiter (limit()) and then 0.8. */
static const int16_t gain_out[MACRO_ENGINES] = {22938, 19661, 26214, 26214, 26214, 26214, 26214, 22938, 26214, 26214, 19661, 26214, 26214, 26214, 32767};  /* WSH .7, FM .6, NOISE, PART lim; drums .8; GRAIN .7 */
static const int16_t gain_aux[MACRO_ENGINES] = {19661, 19661, 26214, 32767, 26214, 26214, 26214, 19661, 26214, 32767, 19661, 26214, 26214, 26214, 32767};  /* WSH .6, FM .6, NOISE lim, PART 1; drums .8; GRAIN .6 */

const char *const macro_engine_name[MACRO_ENGINES] = {"WSHAPE", "2OP FM", "NOISE", "PARTCL", "BDRUM", "SNARE", "HIHAT", "GRAIN", "CHORDS", "SWARM", "WAVES", "VA", "MODAL", "STRING", "6-OP"};

/* Model-TG: which engines are built in, and their order on the ENGN knob. MACRO_SEL is a list of MACRO_*
 * ids (build.py --plaits-engines); an engine not in it is never called, so its code and tables are not
 * linked. Knob zone k (8 values each) plays the k-th engine of the list; past the last, the last. */
#define MACRO_SEL MACRO_WAVETABLE
#define MACRO_MASK (1u << MACRO_WAVETABLE)
static const uint8_t macro_sel[] = { MACRO_SEL };
#define MACRO_NSEL ((int)sizeof macro_sel)
#define HAS(e) ((MACRO_MASK >> (e)) & 1)

int macro_engine_of(int b)
{
    int z = b >> MACRO_ZONE_SHIFT;
    return macro_sel[z < MACRO_NSEL ? z : MACRO_NSEL - 1];
}

static void engine_init(struct macro_voice *m)
{
    m->lim_out = m->lim_aux = defined_shift(1, 23);                     /* the limiters start at a peak of 0.5 (Q24) */
    switch (m->engine) {
    case MACRO_WAVETABLE: if (HAS(MACRO_WAVETABLE)) wt_init(&m->e.wt); break;
    default: break;
    }
}

void macro_init(struct macro_voice *m)
{
    m->engine = macro_sel[0];
    m->latch = 1;
    m->pad[0] = m->pad[1] = 0;
    m->rng = 0x2545f491u;
    engine_init(m);
}

void macro_trig(struct macro_voice *m)
{
    m->latch = 1;
}

template <unsigned Rate, int Model, unsigned Frames>
static void macro_render_e(struct macro_voice *m, const uint8_t *p, uint32_t inc, int16_t *out, int n);

void macro_render(struct macro_voice *m, const uint8_t *p, uint32_t inc, int16_t *out, int n)
{
    struct emac_save e;
    emac_enter(&e);
    macro_render_e<48000, -1, 0>(m, p, inc, out, n);
    emac_leave(&e);
}














#define CL2(v) ((v) > 65535 ? 65535 : (v) < -65535 ? -65535 : (v))     /* +-2 in Q15: clipped below anyway */

template <unsigned Rate, int Model, unsigned Frames>
static void macro_render_e(struct macro_voice *m, const uint8_t *p, uint32_t inc, int16_t *out, int frames)
{
    const int n=Frames ? static_cast<int>(Frames) : frames;
    int32_t o[Frames>32?Frames:32], a[Frames>32?Frames:32], go, ga, mix;
    int i;
    if (m->latch) {                                 /* a note start: the engine, and its per-note choices */
        int e = macro_engine_of(p[MACRO_P_ENGINE]);
        m->latch = 0;
        if (e != m->engine) {
            m->engine = (uint8_t)e;
            engine_init(m);
        }
    }
    const uint32_t maximum_inc=Rate==44100 && Model==MACRO_CHORD ? 1168698583u : INC_MAX;
    if (inc > maximum_inc) inc=maximum_inc;
    if (inc < INC_MIN)
        inc = INC_MIN;
    /* F: OUT up to 55, AUX from 72, a crossfade between (only there are both outputs computed) */
    mix = p[MACRO_P_AUX] <= 55 ? 0 : p[MACRO_P_AUX] >= 72 ? 32767 : (p[MACRO_P_AUX] - 55) * 1927;
    switch (Model >= 0 ? Model : m->engine) {
    case MACRO_WAVETABLE:
        if (!HAS(MACRO_WAVETABLE)) goto none;
        wt_render(&m->e.wt, p, inc, o, a, n);
        break;
    default:
    none:
        for (i = 0; i < n; i++)
            out[i] = 0;
        return;
    }
    if (Rate == 44100 && Model == MACRO_WSH) {
        // Prepared WSH curves and their sine blend stay inside signedQ15.
        // Fuse the output gains/crossfade with two signed16 products. The
        // generic path's +/-2 rails and post-gain clips cannot activate here.
        const int16_t main_gain=static_cast<int16_t>(gain_out[MACRO_WSH]);
        const int16_t aux_gain=static_cast<int16_t>(gain_aux[MACRO_WSH]);
        if (mix == 0 || mix >= 32767) {
            const int32_t* signal=mix ? a : o;
            const int16_t gain=mix ? aux_gain : main_gain;
            for (i=0;i<n;++i) out[i]=static_cast<int16_t>((static_cast<int16_t>(signal[i])*gain)>>15);
        } else {
            const int16_t mg=static_cast<int16_t>((main_gain*(32768-mix))>>15);
            const int16_t ag=static_cast<int16_t>((aux_gain*mix)>>15);
            for (i=0;i<n;++i) {
                const int16_t x=static_cast<int16_t>(o[i]);
                const int16_t y=static_cast<int16_t>(a[i]);
                out[i]=static_cast<int16_t>((x*mg+y*ag)>>15);
            }
        }
        return;
    }
    if(Rate==44100 && Model==MACRO_CHORD && mix>0 && mix<32767) {
        const uint16_t weight=static_cast<uint16_t>(mix);
        for(i=0;i<n;++i)out[i]=static_cast<int16_t>(chord_output_blend(o[i],a[i],weight));
        return;
    }
    go = gain_out[Model >= 0 ? Model : m->engine];
    ga = gain_aux[Model >= 0 ? Model : m->engine];
    if (mix == 0 || mix >= 32767) {                 /* OUT only (the default) or AUX only */
        /* x g / 2^15 on the EMAC: no product overflows, so no clip before it (the gains are over 0.5:
         * whatever CL2 would clip lands past +-1 either way) */
        const int32_t *x = mix ? a : o;
        int32_t g = defined_shift((mix ? ga : go), 16);
        for (i = 0; i < n; i++) {
            int32_t y = fmac1t(x[i], g);
            out[i] = (int16_t)(y > 32767 ? 32767 : y < -32768 ? -32768 : y);
        }
        return;
    }
    for (i = 0; i < n; i++) {
        int32_t x = (CL2(o[i]) * go) >> 15, y = (CL2(a[i]) * ga) >> 15;
        x = x > 32767 ? 32767 : x < -32768 ? -32768 : x;
        y = y > 32767 ? 32767 : y < -32768 ? -32768 : y;
        x += ((y - x) * mix) >> 15;
        out[i] = (int16_t)(x > 32767 ? 32767 : x < -32768 ? -32768 : x);
    }
}




}
