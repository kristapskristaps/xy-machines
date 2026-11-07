# Lustre

Lustre turns short phrases into a soft stereo cloud: overlapping grains, octave
shimmer, backwards fragments, and a diffuse reverb tail. It is an original
Microcosm-inspired send effect for the Open-XY SDK, rather than a recreation of
its algorithms or phrase looper.

## Play

Choose **lustre** on send 15 or 16 with **Shift + M1**, then raise an instrument's
send. Start around a quarter send on a pluck, sparse chords, or an arpeggio.
The send moves from the clean source toward the effect. At full send the source
is fully wet; Lustre returns only the effect and the stock mixer handles the dry
level. Space moves from grain definition to a full diffuse reverb at maximum.

Press any encoder to switch all four controls between the two pages.

| Page | Control | What rises clockwise |
| --- | --- | --- |
| 1 | Size | Grain length, 60–360 ms; longer grains reach further into the recent phrase |
| 1 | Swarm | Overlap, from two to six simultaneous grains; denser and louder clouds |
| 1 | Decay | Grain recirculation and reverb sustain; the upper range opens into a long hall |
| 1 | Space | Crossfade from grain definition to a full diffuse stereo reverb |
| 2 | Shimmer | Proportion of newly launched grains played one octave up |
| 2 | Reverse | Proportion of newly launched grains played backwards |
| 2 | Drift | Random detuning per grain, up to approximately half a semitone either way |
| 2 | Tone | Brightness of the grains and the reverb's damping |

Size, Shimmer, Reverse and Drift take effect as new grains launch. Existing grains
finish their envelopes; quick edits do not cut them off. Defaults leave Shimmer,
Reverse and Drift at zero.

The stock send page shows **00–99 for every continuous control**, including Size.
Size 00 means 60 ms and 99 means 360 ms; it does not show milliseconds. The
following recipes use approximate on-device readings (a saved value can show one
step lower because the stock header truncates fractions).
Effects have no native preset browser; the normalized recipes also live in
`definition.json` for offline audition and reference.

| Recipe | Size | Swarm | Decay | Space | Shimmer | Reverse | Drift | Tone |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| init | 45 | 65 | 60 | 65 | 0 | 0 | 0 | 65 |
| Glass garden | 50 | 85 | 72 | 80 | 35 | 10 | 20 | 72 |
| Velvet tide | 85 | 95 | 85 | 95 | 8 | 30 | 30 | 42 |
| Reverse stars | 62 | 70 | 70 | 65 | 55 | 85 | 12 | 80 |
| Fireflies | 8 | 30 | 45 | 30 | 65 | 15 | 15 | 80 |
| Cathedral | 90 | 99 | 99 | 99 | 18 | 20 | 25 | 65 |

Use one send for comparison, with the other send at zero. Raising every control
is not the Cathedral recipe: Shimmer and Reverse at 99 replace essentially all
grains with octave-up, backwards fragments.

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
source and master playback gain. The optional fourth
argument sets the isolated send weight (default `0.75`, or `1` for fully wet).
The renderer scales the input by that send weight, attenuates dry by `1-send`,
and mixes the actual callback return at unity. It applies no extra wet boost or
per-file normalization. These are host renders, not recordings from the OP-XY.

The effect uses six zero-ended, smoothly windowed grains, stereo history, four
all-pass diffusers, and a damped four-line feedback reverb. Its network runs at
22.05 kHz with filtered input and interpolated output: expect a warm, softened
top end. It uses 301,664 bytes of per-send scratch, no audio-thread allocation,
and less than the SDK's 472-byte state limit. Each line tracks valid history,
so changing effects or resetting never exposes old scratch contents.
Audio uses Q28 internally with guard bits for mixing, while coefficient products
use the SDK's Q31 operations. Rendering runs in short passes: input filtering,
grains, tone/feedback capture, diffusion, reverb damping/mixing and output.
Stereo history is interleaved. Each diffuser and reverb delay uses a ring sized
to its exact delay, with a short mirrored prefix for contiguous block reads and
writes. Grain state stays local during a span. Integer-speed grains skip
interpolation, and a contiguous forward grain walks a history pointer.
Initialized spans use unchecked reads as soon as their own history is ready;
wholly uninitialized spans advance silently. The room starts when the first wet
signal arrives. Blocks within the return knee use direct linear scaling.
Near the live history writer, spans shorten to one frame to preserve feedback
and wrap behavior. Gain and filter coefficients still ramp sample by sample.
The six-grain capacity, four-line reverb, rate, mix and tail length are unchanged.
The streaming tests compare exact audio signatures against accepted revision
`6de273f`, including maximum controls, automation, startup and ring wraps.

The return has fourfold gain relative to the initial prototype, restored tank
level, and a soft knee above half-scale. This output gain is outside the feedback
loops, so it does not lengthen or destabilize the tail.

No tempo sync, freeze, phrase recording, or Microcosm mode emulation is provided.
The first fragments arrive after roughly one grain length; this is a blooming
send rather than an immediate reverb. Both sends can run independent instances.

## Hardware validation

Host tests cover dirty scratch, resets, output and stereo energy, tail decay,
all controls, rapid automation, full-scale input and missing buffers. The first
hardware build (commit `b5ed40a`) measured **9.859% average** and **13.823% worst
observed block** at defaults, above the 3.5% recommendation for each. Its wet
return was reported too muted, and its stock readout uses 00–99 rather than the
previous guide's units. That revision did not pass sound QA. Its rendered demos
also mixed the return with an extra 2.3× wet gain; current demos use unity wet.

Revision `6de273f` corrected return level and source mixing; initial device
listening was reported much better. It measured **10.074% average** and
**13.535% worst observed block** at defaults, still above the recommendation.
Revision `163e278` preserved that revision's audio and measured **5.960% average**
and **8.208% worst observed block** in one complete hardware capture at defaults.
Both remain above the 3.5% recommendation. These callback measurements do not
describe a full project or sweep other settings. The compact delay layout,
contiguous grain paths and linear return need a new measurement. Full QA remains pending. Host
estimates cannot establish native cost or acceptance of a new revision.

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
