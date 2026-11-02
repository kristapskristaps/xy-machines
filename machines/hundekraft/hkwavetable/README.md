# HK Wavetable

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `synth`. ID: `hkwavetable`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Wave | 25.0% |
| Detune | 12.0% |
| Cutoff | 65.0% |
| Reso | 20.0% |

## Test

From the collection root with its configured pinned SDK:

```sh
./xy test hkwavetable --native --cpu
./xy preview hkwavetable --watch
python machines/hundekraft/hkwavetable/tools/generate_display_waves.py --check
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

The DSP has a 64-sample adapter latency. Modulation is independent per voice. The stock XY amp envelope follows the callback.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Oscillator and display optimization

With controls settled at Wave zero, the oscillator reads only the first bank;
the former second-bank blend had zero weight. Other steady settings and all
control ramps retain the original blended path. The render chain stays inlined
so the added specialization does not introduce a DSP call in the native callback.
The audio wave, cutoff and level tables, voice state and control mappings are unchanged.

The display uses exact samples at its 25 existing drawing points, generated
from the canonical audio waves. It stores 400 bytes of wave data instead of
another 4,112-byte audio table, and reuses adjoining endpoints to evaluate 25
points instead of 48 per frame. The generator verifies these samples against a
frozen readout from the original C++ interpolator. The display-wave test checks
all 65,536 signed Q15 control values at all 25 points against that interpolator.

The native module footprint shrinks from 77,908 to 74,436 bytes, a 3,472-byte net saving.
Reported audio stack changes from 252 to 244 bytes. Voice memory remains 896
bytes per voice, with the same 16-voice pool.

Compare the original revision with exact audio and complete voice state:

```sh
python machines/hundekraft/hkwavetable/tools/benchmark.py --baseline 06b51cd --sdk /path/to/open-xy
python machines/hundekraft/hkwavetable/tools/benchmark.py --baseline 06b51cd --sdk /path/to/open-xy \
  --verify-only --full-grid
```

The default comparison covers 474 scenarios; the full grid covers 2,010, each
for 256 blocks, plus 4,096 exact display frames. Cases include waveform-bank
boundaries, reset/retrigger, control ramps and pitch motion. The full comparison
also passes AddressSanitizer and UndefinedBehaviorSanitizer. Overall host timing
against `06b51cd` was roughly flat; native CPU improvement remains unmeasured.
The SDK's separate host advisory reported 17,100 instructions at its heaviest
setting, above its 15,000 aim, so device profiling remains necessary.

## Display reconstruction

See [VISUALS.md](VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
