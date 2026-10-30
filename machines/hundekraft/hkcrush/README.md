# HK Crush

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `effect`. ID: `hkcrush`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Bits | 33.3% |
| Rate | 64.8% |
| Drive | 25.0% |
| Dry | 100.0% |

## Test

From this machine folder, using Python 3.11+ and a Clang with WebAssembly support:

```sh
python ../../../../xy-machines-sdk/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
```

Native module compilation and host checks pass. The installed word-type fix
measured 0.954% of the audio-block budget at default settings, but the user still
reported bad FX audio. That measurement predates the working-level correction.

The adapter now normalizes quiet sends before the original 16-bit DSP and
restores their return level afterwards. Its gain only falls when louder input
arrives; held audio is rescaled at the same time. Tests cover transparent
processing from 512-word peaks through loud signals and held-sample continuity.
Hardware audio acceptance and CPU measurement for this change remain pending.

This is a stereo send effect. Dry is applied by the XY mixer. The buffer is cleared before first use; a short silent warmup is expected.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Display reconstruction

See [visual/VISUALS.md](visual/VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
