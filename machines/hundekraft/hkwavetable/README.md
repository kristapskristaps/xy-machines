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

From this machine folder, using Python 3.11+ and a Clang with WebAssembly support:

```sh
python ../../open-xy/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

The DSP has a 64-sample adapter latency. Modulation is independent per voice. The stock XY amp envelope follows the callback.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Display reconstruction

See [VISUALS.md](VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
