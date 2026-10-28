# Weave

Fifteen Plaits-derived synthesis models for OP-XY OS 1.2.0, with a monochrome
woven-oscillation screen. Built with the [Open-XY SDK](https://github.com/etcetc000/open-xy).

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

The display follows controls and note activity; it does not read audio buffers.
Reduced motion freezes its continuous animation while preserving control edits.

## Port fidelity

The DSP derives from Émilie Gillet's MIT-licensed Plaits, via the integer port
by gdeo607 and DMDaudio. [vendor/VENDOR.md](vendor/VENDOR.md) records the source
revisions and changes. The engine runs internally at 48 kHz, followed by a
24-tap, 147-phase fixed-point converter to the SDK's 44.1 kHz. Original sample
counts and time constants therefore keep their intended duration.

There is no module-panel CV calibration, user-data/SysEx import, or Plaits LPG.
The stock XY envelope supplies the voice gate. Eight browser voices and up to
sixteen native voice-state slots are allocated; those storage counts do not
establish usable hardware polyphony.

## Develop and audition

Keep the SDK beside this repository as `../open-xy`, using Python 3.11+ and a
Clang with WebAssembly support:

```sh
python ../open-xy/sdk.py test --machine . --cxx clang++ --firmware
python ../open-xy/sdk.py preview --machine . --cxx clang++ --out ../previews/weave-001
python ../open-xy/tools/serve_harness.py --preview ../previews/weave-001
```

Tests cover all models and both outputs, deterministic voice ownership, moving
controls, fractional arithmetic, 59 upstream reference renders plus one corrected FM fixture, sample-rate
conversion, and regressions for wavetable bank bounds and the FM sine LFO.

[Hardware profiling](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
remains required before catalog submission. Host CPU estimates are not measured
device cost. No firmware is included; keep device builds outside every checkout.

MIT, with the third-party notices in [vendor/LICENSE](vendor/LICENSE).
