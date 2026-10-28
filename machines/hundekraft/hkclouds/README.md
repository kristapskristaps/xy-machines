# HK Clouds

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `effect`. ID: `hkclouds`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Position | 50.0% |
| Size | 50.0% |
| Density | 75.0% |
| Texture | 50.0% |
| Pitch | 50.0% |
| Dry | 100.0% |
| Space | 0.0% |
| Freeze | Off |

## Test

From this machine folder, using Python 3.11+ and a Clang with WebAssembly support:

```sh
python ../../open-xy/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

This is a stereo send effect. Dry is applied by the XY mixer. The buffer is cleared before first use; a short silent warmup is expected.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Display reconstruction

See [visual/VISUALS.md](visual/VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
