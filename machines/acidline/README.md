# Acidline

An acid synth for Open-XY: band-limited saw/triangle/square, a resonant four-pole
filter, a pluck envelope, soft clipping and an octave-down sine sub. Stock OP-XY supplies pitch,
portamento, velocity, amplitude envelopes and mixing.

## Mono playback

The default and all five named factory presets select stock **mono** play mode.
This is mono independently on each track: using Acidline on another track does
not steal the first track's voice. Existing projects retain their saved play
mode; load an updated factory preset or switch M2 / Shift play mode to mono
once. Loading a preset replaces its sound settings, so save an edited sound
first. The machine API does not lock the stock play-mode menu: selecting poly
manually enables overlapping voices and multiplies DSP cost again.

Stock portamento follows the supplied sample-by-sample pitch ramp. No second
glide, global voice lock, velocity detector or internal amplitude gate is added.
Use parameter locks on Cutoff, Env Amt, Reso or Distort for filter accents.
The 16-slot DSP pool remains available for independent tracks and stock fades;
it is not a per-track polyphony setting.

## Controls and display

Press any encoder to switch the complete page. The SDK's shared two-page
header shows both rows and highlights the active row. The original six saved
slots keep their order; Sub and Damp use the two previously unused slots.

| Encoder | Page 1 | Page 2 |
| --- | --- | --- |
| 1 | Cutoff | Decay |
| 2 | Reso | Wave |
| 3 | Distort | Sub |
| 4 | Env Amt | Damp |

Wave now selects **Saw / Triangle / Square** with a named readout, rather than
morphing continuously between saw and pulse. Existing low/high settings still
select saw/square, but intermediate saved values now select one of the three
shapes. Waveform changes crossfade over one audio block.

Sub blends in a sine exactly one octave below the supplied pitch, including
stock glide. At full Sub the main and sub each contribute half, preserving
output headroom. Sub bypasses its lookup when off. Damp is a post-distortion
low-pass: zero bypasses it, turning clockwise increasingly darkens the sound.
Both new controls default off; the bass factory presets demonstrate them.

A laboratory bench holds a red triangular Erlenmeyer flask, a blue round-bottom
reaction flask and a slender green test tube. Their liquid outlines and bubble
positions follow their different vessel profiles. All three are connected
by a recirculating glass pipe network. Coloured fluid packets travel through
the pipes while notes sound; cutoff controls flow speed. Turning valves, two
pressure gauges and droplets respond to the controls and
note pulses. The apparatus sits against a tiled lab wall with pump housings
and an instrument cabinet. Cutoff,
resonance and distortion change the liquid levels. Notes trigger ripples,
bubbles and short vapour bursts; held notes keep the reaction active. Idle
motion settles, track changes reset activity, and reduced motion removes the
continuous movement. Display work is separate from the audio callback.

## DSP

The oscillator, nonlinear feedback filter and distortion run at 88.2 kHz.
A symmetric 31-tap half-band FIR returns 44.1 kHz output. The original filter
saturation and anti-aliasing remain. Distort now drives a symmetric tanh soft
clipper across its whole range; it no longer folds at the top. A single small
interpolated curve replaces the 33-row distortion table, with gain computed
per block and smoothly ramped. Its zero setting is an exact bypass.

The FIR's seven nonzero symmetric pairs remain unrolled. Each complementary
filter blend uses one multiply instead of two, changing only fixed-point
rounding. No oversampling or filter stages were removed. A paired host timing
comparison against the preceding version took about 24–25% less time across
all three waveform positions, including Sub and Damp enabled. The native
machine plus SDK occupied about 120 KiB, down from about 175 KiB. These are
host timings and compile sizes, not device CPU measurements. Stock mono keeps
rapid notes from accumulating polyphonic voices; multiple tracks still add cost.

The native compiler previously lowered the filter saturation's division by
three to a software divide inside the oversampled loop: 128 calls per block.
It now uses an exact bounded reciprocal. Exhaustive saturation tests and a
307,200-sample render comparison are bit-identical, and the Blackfin voice
object has no software division call. This fixes a target-specific cost the
desktop timing comparison did not capture; hardware load still needs testing.

## Development

From the XY Machines root:

```sh
./xy test acidline --native --cpu
node --test machines/acidline/tests/presets.test.mjs
./xy preview acidline --watch
```

The C++ tests cover control extremes, peak bounds, filter attenuation,
monotonic symmetric soft clipping, waveform spectra, octave-down sub,
damping, exact stock pitch ramps, independent DSP states, display bounds,
both header pages, idle/reduced-motion behavior, full-convolution equivalence
and spectral alias rejection. Preset tests check stock mono mode and unchanged
sound parameters. The optional dedicated acid sequencer under `audition/`
provides its own simple mono gate and glide; it does not emulate stock's
allocator or envelopes.

Use the root browser workflow to refresh the cached builder after source
changes. This revision still needs device testing with rapid notes and several
independent tracks, plus a revision-bound CPU profile before publication.
Host/native checks do not establish real-time hardware headroom. Keep firmware
and stock content outside every checkout; device operations require a separate
user instruction.

MIT; see LICENSE. DSP, tables and browser audition are original to this work.
