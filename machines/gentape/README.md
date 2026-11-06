# Gen Tape

A stereo recorder-color prototype for the OP-XY sends, inspired by the sound
families described in Generation Loss MKII. It is an original implementation;
it does not reproduce Chase Bliss's DSP, measured profiles or hysteresis model.

Slow irregular wow, fast flutter, warm saturation and occasional tape failures
can be used lightly on keys and pads or pushed into damaged-recorder textures.
The source stereo image stays intact. Spread introduces differential movement
rather than summing the input to mono.

## Controls

Press any encoder to switch pages.

| Page 1 | Behavior |
| --- | --- |
| Wow | Depth of smooth random transport drift |
| Flutter | Depth of faster random pitch and amplitude variation |
| Saturate | Frequency-shaped soft compression and coloration |
| Failure | Depth and frequency of shaped drops and pitch snags |

| Page 2 | Behavior |
| --- | --- |
| Model | Clean, VHS, cassette or dictation recorder voicing |
| Spread | Independent right-channel transport movement and failure variation |
| Noise | Added hiss, independent of input; zero is silent |
| Dry | Balanced fade from processed sound (0) to original source (100) |

Defaults are clean, with coloration, modulation, failures and hiss off. Dry
starts at zero: at full send, only the processed source remains. Raise Dry to
blend the original source for chorus. At full send, Dry 0 is fully processed,
50 is half processed and half original, and 100 is original only. The stock
mixer supplies the original source; the callback fades its processed return
by the complementary amount. Hiss fades too. This replaces the first
prototype's additive Dry, which kept the wet return full and became louder
as original audio was added. No clean track is duplicated inside the callback.

The clean signal is delayed by 512 samples, approximately 11.61 ms. This delay
provides room for transport modulation and is also present with all controls
off. Blending Dry therefore introduces the expected chorus/comb coloration.

## Sound design and cost

The signal path is record-band saturation, a short stereo fractional delay,
recorder EQ, shaped dropout gain and optional hiss. Wow and Flutter use
more expressive middle/upper ranges; Failure has deeper drops and larger snags. Four-point cubic Hermite
interpolation keeps pitch movement smooth. Read positions and event envelopes
ramp within each block; continuous controls and recorder changes are smoothed.

The saturator uses two 1.8 kHz record poles, a densely interpolated tanh curve,
gentle make-up gain and the complementary upper band. It warms and
compresses the body of the sound while retaining detail. Shaping the nonlinear
input limits foldback; this is not full-band fuzz or physical tape hysteresis.
The recorder profiles are authored filter settings, not measured hardware EQs.
They use a bandwidth pole and a low-band shelf/cut. The hiss generator has its
own random state, so Noise does not change transport movement or failure timing.

Audio is integer-only. The delay uses 32,800 bytes of per-send scratch, guarded
against stale reads after initialization/reset. State fits the native effect
slot. Disabled saturation and clean recorder stages use cheaper paths.

**The CPU goal is at most 3.5% average and peak per stereo effect instance.
This optimization still needs hardware measurement.** The installed revision
`02e6de3` measured 3.79–3.80% average with all controls enabled on the recorder
models, and a worst observed peak of 6.68% in one complete capture. Clean with
all controls enabled measured 3.41% average and 6.12% peak. Those results fail
the target and do not qualify this changed code.

This revision packs each saturation-curve segment with its slope, omits unused
curve tails beyond the full-scale driven signal range, skips
unchanged per-sample control increments, folds the advancing delay writer into
one read-phase accumulator, keeps the hiss RNG in a local register, and caches
unchanged make-up gain and event reciprocals. Saturating output scaling removes
data-dependent clipping branches. The saturation curve, cubic interpolation,
filter settings and modulation/event trajectories retain their precision and
ranges. The only intended audio difference is one Q31 bit at negative clipping.

`tools/compare_revision.py` compares complete callbacks with a committed
reference over startup, resets, full-scale transients, low/high tones, DC,
silence, all four models, failures and moving controls. It requires matching
recorded delay audio, exact transport/noise sequences and at most one Q31 bit
of output difference. Host timing remains advisory. The standard hardware
effect benchmark covers defaults; full-effect diagnostic copies keep every
musical control at maximum and Dry at 0 (fully wet), testing each model.
Two send instances incur two processing costs.

## Listen and check

From the collection root, after `./xy setup` and `./xy doctor`:

```sh
./xy test gentape --native --cpu
.venv/bin/python machines/gentape/tools/render_examples.py --out local/gentape-audio-001
.venv/bin/python machines/gentape/tools/quality.py --out local/gentape-quality.json
.venv/bin/python machines/gentape/tools/benchmark.py --out local/gentape-timing.json
.venv/bin/python machines/gentape/tools/compare_revision.py --before 02e6de3 --out local/gentape-comparison.json
```

Choose a fresh audio output directory. The renderer compiles and runs the real
`fx.cpp`, at 44.1 kHz, on original synthesized keys, pads, drums and bass. Each
example has separate 24-bit stereo dry/wet WAVs and a dry-then-wet comparison:
12 seconds dry, half a second of silence, then 12 seconds processed. Dry is
aligned to the clean-path delay. Files use the same +6.02 dB playback gain, without independent
peak normalization; the DSP input level is unchanged. `examples.json` records settings, levels and source hashes.
These are full-wet host examples; they do not emulate the stock source mixer.

`quality.py` measures lookup error and out-of-harmonic spectral energy at three
coherent high-drive tones. Its -75 dBc guard applies to those fixtures, with
modulation/noise/failures off, not all possible signals or settings. No host
check attests native timing, project stability or hardware sound.

`tools/generate_tables.py` regenerates `tables.h` using only the Python standard
library. The collection gallery can also build this effect with its existing
local-audio/demo-loop player; no SDK browser effect support is required.

For an old/new comparison and a Dry sweep using the documented source-mixer
routing at full send:

```sh
.venv/bin/python machines/gentape/tools/render_revision.py --before 4fe5772 --out local/gentape-revision-001
```

`tools/audit_controls.cpp` renders steady tones and silence through each control,
reporting pitch movement, level, stereo difference and failure activity. The
sound tests also cover Dry endpoints/level and independent Noise randomness.

Before device work, commit the chosen source and follow [QA.md](QA.md). Device
installation and diagnostics require separate authorization. Catalog publication
needs a revision-bound hardware CPU report and a complete QA pass.

Reference: [Generation Loss MKII manual](https://www.chasebliss.com/s/Generation-Loss-MKII_Manual_Pedal_Chase-Bliss.pdf).
