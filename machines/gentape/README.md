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
| Dry | Source dry level in the stock mixer |

Defaults are clean, with coloration, modulation, failures and hiss off. Dry
starts at zero: at full send, only the processed source remains. Raise Dry to
blend the original source for chorus. The callback supplies the wet return;
Dry does not duplicate the clean track inside the callback.

The clean signal is delayed by 512 samples, approximately 11.61 ms. This delay
provides room for transport modulation and is also present with all controls
off. Blending Dry therefore introduces the expected chorus/comb coloration.

## Sound design and cost

The signal path is record-band saturation, a short stereo fractional delay,
recorder EQ, shaped dropout gain and optional hiss. Four-point cubic Hermite
interpolation keeps pitch movement smooth. Read positions and event envelopes
ramp within each block; continuous controls and recorder changes are smoothed.

The saturator uses two 1.8 kHz record poles, a densely interpolated tanh curve,
inverse-drive compensation and the complementary upper band. It warms and
compresses the body of the sound while retaining detail. Shaping the nonlinear
input limits foldback; this is not full-band fuzz or physical tape hysteresis.
The recorder profiles are authored filter settings, not measured hardware EQs.

Audio is integer-only. The delay uses 32,800 bytes of per-send scratch, guarded
against stale reads after initialization/reset. State fits the native effect
slot. Disabled saturation and clean recorder stages use cheaper paths.

**The CPU goal is at most 3.5% per stereo effect instance. Hardware qualification
is pending.** Host instruction estimates are advisory, not cycles or a device
percentage. The SDK sweep varies one control at a time from clean defaults;
use the combined benchmark below as well. Neither establishes a hardware bound.
Two send instances incur two processing costs.

## Listen and check

From the collection root, after `./xy setup` and `./xy doctor`:

```sh
./xy test gentape --native --cpu
.venv/bin/python machines/gentape/tools/render_examples.py --out local/gentape-audio-001
.venv/bin/python machines/gentape/tools/quality.py --out local/gentape-quality.json
.venv/bin/python machines/gentape/tools/benchmark.py --out local/gentape-timing.json
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

Before device work, commit the chosen source and follow [QA.md](QA.md). Device
installation and diagnostics require separate authorization. Catalog publication
needs a revision-bound hardware CPU report and a complete QA pass.

Reference: [Generation Loss MKII manual](https://www.chasebliss.com/s/Generation-Loss-MKII_Manual_Pedal_Chase-Bliss.pdf).
