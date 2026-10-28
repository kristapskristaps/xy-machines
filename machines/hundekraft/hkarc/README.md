# HK Arc

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `synth`. ID: `hkarc`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Current | 36.6% |
| Carbon | 18.3% |
| Hiss | 15.3% |
| Strike | 27.5% |
| Ratio | 0.0% |
| Coupling | 0.0% |
| Bright | 0.0% |

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
