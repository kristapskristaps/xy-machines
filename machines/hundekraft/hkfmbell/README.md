# HK FM Bell

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `synth`. ID: `hkfmbell`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Ratio | 2.5 |
| Index | 48.8% |
| Decay | 24.4% |
| Feedback | 0.0% |

## Test

From this machine folder, using Python 3.11+ and a Clang with WebAssembly support:

```sh
python ../../../../open-xy/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

The DSP has a 64-sample adapter latency. Modulation is independent per voice. The stock XY amp envelope follows the callback.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Display reconstruction

See [VISUALS.md](VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
