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

From the collection root with its configured pinned SDK:

```sh
./xy test hkarc --native --cpu
./xy preview hkarc --watch
python machines/hundekraft/hkarc/tools/generate_gp.py --check
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

The DSP has a 64-sample adapter latency. Modulation is independent per voice. The stock XY amp envelope follows the callback.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Tuning-table optimization

Arc now shares one immutable 33-by-97 calibration table across all voices.
`tools/gp_source.json` retains the original compressed data and the expanded
SHA-256 from the original C++ decoder. `tools/generate_gp.py` losslessly expands
it into `gp_table.hpp`; `--check` verifies every generated value. The calibration
test also freezes the complete decoded table's fingerprint. Oscillator math,
parameter mappings, random seeding, traces, displays and adapter latency are unchanged.

Each native voice shrinks from 9,320 to 2,912 bytes, saving 102,528 bytes across
the 16-voice pool. The profiling module also has a benchmark voice: the complete
native module shrinks from 224,820 to 118,884 bytes, a 105,936-byte net saving
after including the immutable table. The callback no longer decodes calibration
rows on first use. Reported native audio stack remains 148 bytes.

Compare the original revision with exact audio and retained voice state:

```sh
python machines/hundekraft/hkarc/tools/benchmark.py --baseline 6456867 --sdk /path/to/open-xy
python machines/hundekraft/hkarc/tools/benchmark.py --baseline 6456867 --sdk /path/to/open-xy \
  --verify-only --full-grid
```

The default comparison covers 432 scenarios; the full grid covers 1,968, each
for 256 blocks, plus 4,096 exact display frames. It covers cold table use,
retrigger/reset, both lamps, parameter automation and pitch motion. The paired
fixture uses identical random seeds because the production seed includes the
voice address. Only the removed calibration cache is excluded from state
comparison. Steady host timing against `6456867` was roughly flat; this is a
memory and first-use-work optimization, not a measured hardware CPU reduction.
The SDK's separate host advisory reported 17,600 instructions at its heaviest
setting, above its 15,000 aim; device timing remains necessary.

The full equivalence grid also passes AddressSanitizer with bounds, alignment,
null and division-by-zero checks. As with the imported DSP's existing checks,
this excludes its signed-wrap/shift conventions and is not a full undefined-
behavior clearance.

## Display reconstruction

See [VISUALS.md](VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
