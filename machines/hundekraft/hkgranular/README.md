# HK Granular

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `sample`. ID: `hkgranular`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Position | 12.2% |
| Size | 24.4% |
| Spray | 9.2% |
| Density | 36.6% |
| Gain | 25.0% |

## Test

From this machine folder, using Python 3.11+ and a Clang with WebAssembly support:

```sh
python ../../open-xy/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

Assign a sample through the stock XY sampler page. Trim and reverse use the saved sample region; stereo samples remain stereo. Gain defaults to 1x at 25% of its 0–4x range.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Display reconstruction

See [visual/VISUALS.md](visual/VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
