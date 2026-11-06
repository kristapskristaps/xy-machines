# Lustre

Lustre turns short phrases into a soft stereo cloud: overlapping grains, octave
shimmer, backwards fragments, and a diffuse reverb tail. It is an original
Microcosm-inspired send effect for the Open-XY SDK, rather than a recreation of
its algorithms or phrase looper.

## Play

Choose **lustre** on send 15 or 16 with **Shift + M1**, then raise an instrument's
send. Start around a quarter send on a pluck, sparse chords, or an arpeggio.
The source stays dry in the stock mixer; Lustre returns only the effect.

Press any encoder to switch all four controls between the two pages.

| Page | Control | What rises clockwise |
| --- | --- | --- |
| 1 | Size | Grain length, 60–360 ms; longer grains reach further into the recent phrase |
| 1 | Swarm | Overlap, from two to six simultaneous grains; denser and louder clouds |
| 1 | Decay | Grain recirculation and reverb sustain; the upper range opens into a long hall |
| 1 | Space | Diffuse stereo reverb, retaining a little of the grain definition |
| 2 | Shimmer | Proportion of newly launched grains played one octave up |
| 2 | Reverse | Proportion of newly launched grains played backwards |
| 2 | Drift | Random detuning per grain, up to approximately half a semitone either way |
| 2 | Tone | Brightness of the grains and the reverb's damping |

Size, Shimmer, Reverse and Drift take effect as new grains launch. Existing grains
finish their envelopes; quick edits do not cut them off. Defaults leave Shimmer,
Reverse and Drift at zero.

These recipes use the values shown by the stock header (Size in milliseconds,
all others 0–100). Effects do not have a native preset browser; the same recipes
are included in `definition.json` for offline audition and reference.

| Recipe | Size | Swarm | Decay | Space | Shimmer | Reverse | Drift | Tone |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| init | 195 | 65 | 60 | 65 | 0 | 0 | 0 | 65 |
| Glass garden | 210 | 85 | 72 | 80 | 35 | 10 | 20 | 72 |
| Velvet tide | 315 | 95 | 85 | 95 | 8 | 30 | 30 | 42 |
| Reverse stars | 246 | 70 | 70 | 65 | 55 | 85 | 12 | 80 |
| Fireflies | 84 | 30 | 45 | 30 | 65 | 15 | 15 | 80 |
| Cathedral | 330 | 100 | 100 | 100 | 18 | 20 | 25 | 65 |

For a large, lingering reverb, start with Cathedral. Its maximum Decay setting
has a host-rendered T30 extrapolation of about 42 seconds on the broadband test
fixture; brighter frequencies fade sooner. The feedback stays below unity, so
it fades naturally. Lower Decay for quicker changes between chords. Defaults
retain the original, shorter decay.

## Develop and audition

Keep the [Open-XY SDK](https://github.com/etcetc000/open-xy) on `staging` beside
this repository. Use Python 3.11 or later. From this folder:

```sh
python ../open-xy/sdk.py test --machine .
c++ -std=c++11 -O2 -I../open-xy/sdk/include tools/render.cpp -o ../open-xy/local/lustre-render
../open-xy/local/lustre-render ../open-xy/local/lustre-glass-garden.wav 1
```

The renderer accepts recipe numbers 0–5 in the table's order. Examples have four
seconds dry, ten seconds with the effect, then eight seconds of tail. Cathedral
(5) instead lets the tail ring for fifty seconds. Every recipe uses the same
source and fixed playback gain. These are host
renders of the actual callback, not a recording from the OP-XY.

The effect uses six zero-ended, smoothly windowed grains, stereo history, four
all-pass diffusers, and a damped four-line feedback reverb. Its network runs at
22.05 kHz with filtered input and interpolated output: expect a warm, softened
top end. It uses 335,872 bytes of per-send scratch, no audio-thread allocation,
and less than the SDK's 472-byte state limit. Each line tracks valid history,
so changing effects or resetting never exposes old scratch contents.
Audio uses Q28 internally with guard bits for mixing, while coefficient products
use the SDK's Q31 operations. Shared stereo tap checks and coefficients prepared
once per block reduce audio-loop work. Gain and filter coefficients ramp within
each block to keep automation smooth. The six-grain capacity and four reverb
lines are preserved; the CPU saving does not shorten the tail or lower the sample rate.

No tempo sync, freeze, phrase recording, or Microcosm mode emulation is provided.
The first fragments arrive after roughly one grain length; this is a blooming
send rather than an immediate reverb. Both sends can run independent instances.

## Hardware validation

Host tests cover dirty scratch, resets, output and stereo energy, tail decay,
all controls, rapid automation, full-scale input and missing buffers. Hardware
sound, controls, and CPU qualification remain pending. Host estimates vary with
machine load; only a device measurement can establish its actual cost. The SDK
prefers 30,000 instructions per effect block and warns at 48,000.
Five paired local host runs gave a median estimate of about 22,000 at the slowest
SDK control-sweep setting, down from about 35,000 for the original. The median of
an additional sweep covering all 256 minimum/maximum control combinations was
about 23,000. Individual timings can spike above the target under host scheduling;
these estimates are guidance, not OP-XY CPU percentages or a hardware guarantee.

Build a profiling image from committed source. This SDK revision's effect-only
collection fails an SDK startup compilation check; including the core FM synth
supplies a useful audition instrument and builds the supported collection:

```sh
python ../open-xy/sdk.py build --machine . \
  --machine ../open-xy-core-machines/machines/fm2op --profiling on \
  --stock-os "$PRIVATE/stock-1.2.1/os.ldr" --out "$PRIVATE/lustre-dev-001"
```

`PRIVATE` must point outside every checkout. Device operations require a separate
instruction. Run [QA.md](QA.md) as a complete pass before publication, and collect
the [revision-bound CPU readout](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
The SDK checkout and catalog are unchanged. See [AGENTS.md](AGENTS.md) for the
machine development rules. Original implementation; [MIT licensed](LICENSE).
