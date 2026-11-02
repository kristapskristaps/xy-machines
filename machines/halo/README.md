# Halo

A stereo bloom, echo and shimmer send for OP-XY OS 1.2.1. Halo combines one
modulated plate network with softened tempo echoes, an octave-up feedback layer
and Hold. Three presets start at different points in that connected space:
**Soft Space**, **Cloud Echo** and **Silver Air**.

The sound is inspired by the spacious character of OP-1 field Mother. Halo is
its own algorithm, adapted from PunchKit DSP; it is not an emulation of Mother.
See [PROVENANCE.md](PROVENANCE.md) and [LICENSE](LICENSE).

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

The wet network runs at 11.025 kHz behind two cascaded stages of 31-tap halfband filters; the callback
runs at 44.1 kHz. This deliberately suits a warm, diffuse tail, with a 4 kHz maximum Color
cutoff and less high-frequency shimmer than the earlier 22.05 kHz network.
Delay lengths, modulation, ducking and pitch-shift timing are retimed for the
lower rate; tempo echoes still reach three seconds. The symmetric conversion filters share mirrored histories and paired taps.
Control-dependent tone, room and feedback coefficients update once per block;
the modulated audio network still runs per wet sample. The long stereo
echo uses packed 16-bit samples with error feedback. Plate, diffusion, pre-delay
and shimmer storage use Q24 words. Each send borrows 231,424 bytes (226 KiB)
of scratch memory and has its own 248-byte state. Reset cannot expose previous
scratch audio. Hold maintains an audible tail; it is not a lossless recording.

The conversion filters expand their seven nonzero mirrored tap pairs directly,
preserving each product's rounding and accumulation order. Unpacked delay rings
use masks for their power-of-two lengths; the packed three-second echo retains
its original wrap. Network gains are scaled to Q31 once per callback. These
changes preserve the original output, 248-byte send state and scratch layout.

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

`tests/equivalence.cpp` freezes complete stereo output plus final state and
scratch from the preceding implementation across six long scenarios. The
numerical tests also check every ring's wrap boundaries against modulo indexing.
For paired host comparisons against that implementation, from the collection
root run:

```sh
python machines/halo/tools/benchmark.py --cxx /path/to/clang++
```

The benchmark alternates timing order for five repeats across 60 preset,
control, tail and automation scenarios, checking matching output hashes, state
and scratch. Host timings do not establish the Blackfin benefit of removing
tap loops and coefficient scaling; measure the exact revision on the device.

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
