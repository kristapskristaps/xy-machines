# Weave

Fifteen Plaits-derived synthesis models for OP-XY OS 1.2.1, with a red and silver
flux loom. Built with the [Open-XY SDK](https://github.com/etcetc000/open-xy).

The model bank contains waveshaping, two-operator FM, filtered noise, particles,
kick, snare, hi-hat, grain formants, chords, swarm, wavetable, virtual analog,
modal, string and six-operator FM. The six-operator model includes Plaits' 96
factory FM patches across its three banks. This is a selected Plaits port, not
the complete 24-model module.

## Controls

| Encoder | Primary | Press to select alternate |
| --- | --- | --- |
| 1 | Model | Output: main through a crossfade to auxiliary |
| 2 | Harmonic | — |
| 3 | Timbre | — |
| 4 | Morph | — |

Harmonic, Timbre and Morph follow each model's original meanings. The underlying
integer port accepts 128 steps for these controls and interpolates within its
render blocks. Changing model retriggers it. The XY's stock amp envelope,
filter and mixing remain downstream. Percussive models supply their own decay;
use a sufficiently long stock amp envelope to hear their full tails.

The display follows controls and per-track note gates. Four interlocking strands
form a different knot for each model: dust and noise become a dense mesh, strings
flatten into a resonant band, and FM models fold into multiple loops. Harmonic
changes tension and orientation, Timbre spreads the strands and changes travel
speed, Morph opens the projection, and Output turns the red thread through it.
A note starts a short expanding ring and brightens the moving signal beads.
Held notes keep the loom turning; release and control changes settle in about
0.18 seconds. Idle geometry stays still. Track changes establish a fresh baseline,
and reduced motion shows a static pose with an accurate gate indicator.

The retained artwork uses nineteen transformable elements, four 64-point outlines,
two native text slots and nine palette colors. Shared trigonometry and cached poses
limit geometry updates to about 20 Hz while held; note starts respond immediately.
Regenerate the fragment with `python tools/generate_art.py`. Update the firmware
and `content/display/weave.svg` together when installing this display revision.

## Port fidelity

The DSP derives from Émilie Gillet's MIT-licensed Plaits, via the integer port
by gdeo607 and DMDaudio. [vendor/VENDOR.md](vendor/VENDOR.md) records the source
revisions and changes. The engine runs internally at 48 kHz, followed by a
24-tap, 147-phase fixed-point converter to the SDK's 44.1 kHz. Original sample
counts and time constants therefore keep their intended duration.

There is no module-panel CV calibration, user-data/SysEx import, or Plaits LPG.
The stock XY envelope supplies the voice gate. The native build is limited to two simultaneous Weave voices across all tracks,
with oldest-note stealing. The browser audition can schedule eight voices. All
fifteen models remain available. Dust uses four particle resonators, Swarm four
oscillators, Modal twelve modes, and String one internal string per external
note. Chords keeps its five-note voicing and 6-op keeps all six operators and
96 patches. The reduced banks trade density and upper partials for headroom;
String relies on the external voices for overlapping notes. All source DSP
still runs at 48 kHz, preserving pitch and time constants. The limiter and FM
soft clip use an exact reciprocal division with a checked remainder; wavetable
cubic interpolation drops only sub-LSB rounding work. Normal playback and
worst-case model CPU still require device validation.
The 24-tap converter uses mirrored histories to remove ring-wrap work while
retaining bit-identical audio.

## Develop and audition

Keep the SDK beside this repository as `../open-xy`, using Python 3.11+ and a
Clang with WebAssembly support:

```sh
python ../open-xy/sdk.py test --machine . --cxx clang++ --firmware
python ../open-xy/sdk.py preview --machine . --cxx clang++ --out ../previews/weave-001
python ../open-xy/tools/serve_harness.py --preview ../previews/weave-001
```

Tests cover all models and both outputs, deterministic voice ownership, moving
controls, fractional arithmetic, 41 original/corrected reference renders plus 19 bounded-quality fixtures, exhaustive divisor checks, cubic interpolation error bounds, sample-rate
conversion, and regressions for wavetable bank bounds and the FM sine LFO.

[Hardware profiling](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
remains required before catalog submission. Host CPU estimates are not measured
device cost. No firmware is included; keep device builds outside every checkout.

MIT, with the third-party notices in [vendor/LICENSE](vendor/LICENSE).
