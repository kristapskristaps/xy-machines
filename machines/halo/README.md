# Halo

A stereo bloom, echo and shimmer send for OP-XY OS 1.2.1. Halo combines one
modulated plate network with softened tempo echoes, an octave-up feedback layer
and Hold. Three presets start at different points in that connected space:
**Soft Space**, **Cloud Echo** and **Silver Air**.

The sound is inspired by the spacious character of OP-1 field Mother. Halo is
its own algorithm, adapted from PunchKit DSP; it is not an emulation of Mother.
Halo is distributed under the MIT License, Copyright (c) 2026 Kristaps Dreija.
See [PROVENANCE.md](PROVENANCE.md), [LICENSE](LICENSE) and
[third-party notices](THIRD_PARTY_NOTICES.md).

## Controls

| Control | Behavior |
| --- | --- |
| Distance | Room delay lengths, pre-delay and gentle stereo modulation. Changes glide smoothly and can bend the tail's pitch. |
| Bloom | Longer decay and stronger transient ducking, letting the late cloud swell behind attacks. |
| Echo | Filtered stereo repeats feeding the plate, and their direct presence in the wet return. |
| Shimmer | Two Hann-windowed octave-up heads feeding the tail. This is a granular pitch shift with modulation, not transparent polyphonic pitch correction. |
| Color | Input/echo tone and high-frequency damping in the tail. Bass is filtered out of the feedback. |
| Time | Stock tempo divisions, from 1/32 through 1/2; dotted divisions included. |
| Hold | Cuts new input and pitched recirculation, while maintaining the unpitched plate. Release returns to the selected decay. |
| Dry | Original-signal level in the stock mixer; never added inside the wet callback. |

On the native send page, press an encoder to alternate its control: Distance /
Color, Bloom / Time, Echo / Hold, and Shimmer / Dry.

The track's send controls how much enters Halo. Time follows transport at
40–300 BPM, using 120 BPM when no tempo is available. At 40 BPM the longest
echo is three seconds. The browser tempo control spans 40–240 BPM.

The wet network runs at 11.025 kHz behind two cascaded seven-tap halfband
filters; the callback runs at 44.1 kHz. The filters use exact integer coefficients
`[-1, 0, 9, 16, 9, 0, -1]/32`, embedded at the preceding filters' delay centers.
Their sums, group delay and network rate are unchanged. Tempo echoes, modulation,
ducking and octave-shift timing retain their original settings.

This version trades conversion-filter rejection for lower CPU. A 16 kHz tone
is rejected by about 22 dB in one stage, and an 8 kHz tone by about 24 dB through
the downsampling cascade; the preceding 31-tap filters exceeded 55 dB. Bright
sources can therefore add more aliasing to the wet tail, and the upper wet band
is slightly softer. Across the 15 checked impulse, tone and filtered-noise
comparisons, total level changed by at most 0.473 dB and late energy by 0.645 dB.
These checks describe those stimuli, not every possible source.

The plate, diffusion, predelay, shimmer, packed echo and all controls are unchanged.
Each send still borrows 231,424 bytes (226 KiB) of scratch and has its own
248-byte state. Reset cannot expose previous scratch audio. Hold maintains an
audible tail; it is not a lossless recording. Control-dependent coefficients
update once per block. Downsampling, wet-network processing and upsampling use
separate block passes, reusing the caller's output and bounded scratch arrays.

Every normal delay read is older than the complete 16-frame wet block: early
allpasses start at 34 frames, pre-delay at 22, tank reads above 126, pitch heads at 551
and tempo echo at 275. The network can therefore prefetch its old ring reads
before any writes, then process input/echo filters, individual diffusers, tank
stages and ducking in short counted streams. Cross-feedback uses the incoming
plate state for frame 0 and each opposite pre-read final delay thereafter.
All recursive states retain their original sample order, integer operations
and rounding. Availability still advances per wet frame and ring positions
advance by 16 mono/32 stereo words. Short or out-of-domain delay states retain
the original per-frame fallback without partially changing its inputs.

The new prefetch array is 1280 bytes and reuses the existing conversion scratch
for temporary pitch-head, delay/modulation/sample work. The SDK native call-graph bound for this implementation is 2036 bytes of
audio-entry stack, including saved registers and wrapper frames. This adds no
heap allocation or persistent state/buffer space. Exact comparisons retain complete output, 248-byte state
and all 57856 scratch words across 60×4096 callbacks, including dirty history,
resets, tails, hold, parameter/tempo automation and ring wrapping. Independent
checks cover read availability, minimum ages, the age 17 block boundary and
fallback guards. A fresh hardware measurement is required; these checks do not
establish CPU improvement or stack headroom on the running device.

The hardware uses the SDK's stock FX page and a halo glyph. The animated rings
belong to the machine-owned browser audition, not a custom native FX screen.

## Browser audition

The SDK currently has no effect preview. This repository supplies its own
allowlisted audition server and WebAssembly bridge, compiling the **same fx.cpp**.
It supports the three presets, keys/plucks/drum pulses, local audio-file loading,
a keyboard, send/dry mixing, tempo, Hold, tail clearing and dry/effect comparison.
Files never leave the browser. The demo sources and monitor belong to the
workbench; only the wet DSP is the machine callback.

Keep the SDK beside this repository as `../../../xy-machines-sdk`. With Python 3.11+, Node,
and Clang/wasm-ld available:

```sh
python tools/audition.py --out ../../../xy-machines-sdk/local/halo-audition-001
python tools/audition.py --serve ../../../xy-machines-sdk/local/halo-audition-001 --port 8977
```

Open http://127.0.0.1:8977, enable audio, choose a preset, then play a phrase or
use `A W S E D F T G Y H U J K`. Stop input to hear the tail. AudioContext must
run at 44.1 kHz. Use a fresh output folder after source edits. The server checks
source/output hashes and exposes only five generated files, never the checkout.

## Validation and native development

```sh
python ../../../xy-machines-sdk/sdk.py test --machine . --host-cxx clang++
```

Tests cover dirty scratch/reset, stereo indexing, audible late tails, rapid
control/tempo changes, wet-only/Dry behavior, Hold's input isolation, tempo math,
conversion-filter rejection and a phase-coherent octave reference. Run address
and undefined-behavior sanitizers for the C++ tests. To compare host/Wasm and
exercise the real browser worklet without a browser:

```sh
clang++ -std=c++11 -O2 -I ../../../xy-machines-sdk/sdk/include tests/parity.cpp -o /tmp/halo-parity
node tools/check_audition.mjs ../../../xy-machines-sdk/local/halo-audition-001 "$(/tmp/halo-parity)"
```

`tests/equivalence.cpp` authenticates the frozen preceding DSP in
`tests/reference/` against its original complete-output, state and scratch hashes.
`tests/quality.cpp` compares the production callback against that reference across
15 long impulse, tone and filtered-noise cases, including control extremes.
Numerical tests check conversion gain/rejection, circular convolution, delay
wrapping and octave heads. Behavior tests retain the Hold and wet-only contracts.
The new sound intentionally differs; old exact-output hashes are not reassigned
to it. Hardware CPU qualification for this revision is pending.

The SDK needs a synth in a native collection. Compile alongside a sibling
instrument or the core 2op FM example, with output outside every checkout:

```sh
python ../../../xy-machines-sdk/sdk.py compile --machine . --machine ../subvista \
  --profiling on --out "$HOME/Library/Application Support/OPXY-SDK/private/halo-module-001"
```

Host comparisons are development guidance; hardware CPU timing for this
revision and normal project playback remain unvalidated. A full package
requires an authenticated stock OS 1.2.1 from the user's device. Follow the
SDK's [build guide](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md)
and [profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
Device operations require separate authorization. No firmware is included.
