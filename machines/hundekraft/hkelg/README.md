# HK Elg

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `synth`. ID: `hkelg`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Interval | Octave |
| Detune | 25.0% |
| Timbre | 55.0% |
| Motion | 25.0% |

## Test

From this machine folder, using Python 3.11+ and a Clang with WebAssembly support:

```sh
python ../../../../xy-machines-sdk/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

The DSP has a 64-sample adapter latency. Modulation is independent per voice. The stock XY amp envelope follows the callback.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Arithmetic verification

The sample loop uses the SDK's fixed-point multiply for the filter, gain and
bounded three-saw average. Coefficient products that fit in 32 bits use that
width; the LFO increment splits its wider product at the final shift. These
changes retain the original rounding, control ramps and voice layout.

`tests/arithmetic.cpp` covers every three-saw sum and Q15 Motion value, plus
1,024,288 signed Q15 product cases. Compare complete audio/voice state and
display frames against the earlier source from the collection root:

```sh
python machines/hundekraft/hkelg/tools/benchmark.py --baseline eaee321 --sdk /path/to/pinned-sdk --full-grid --verify-only
```

Add `--sanitize` for address/undefined-behavior checks, excluding the imported
DSP's wrapping signed arithmetic and shifts. Omit `--verify-only` and
`--full-grid` for paired host timings; those use the SDK's portable multiply
and do not measure the Blackfin instruction or device cycles.

## Display reconstruction

See [VISUALS.md](VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
