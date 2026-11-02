# HK SID

Private Open-XY development port. See [the collection testing guide](../README.md).

Machine kind: `synth`. ID: `hksid`. Source and component terms: [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE), [NOTICE](NOTICE).

## Controls

| Control | Default |
| --- | --- |
| Wave | Saw |
| Color | 50.0% |
| Cutoff | 98.1% |
| Reso | 25.0% |

## Test

From the collection root with its configured pinned SDK:

```sh
./xy test hksid --native --cpu
./xy preview hksid --watch
```

Native module compilation and host checks pass; hardware CPU and acceptance remain pending.

The DSP has a 64-sample adapter latency. Modulation is independent per voice. The stock XY amp envelope follows the callback.

The retained firmware exception names the OP-1. Clarification for OP-XY distribution is pending; nothing was published.

## Coefficient optimization

Oscillator step and reciprocal coefficients are updated only when their inputs
change: pitch, whether Wave is Sync or Ring, or Color in either of those two
modes. Filter-only changes still update Cutoff and Resonance immediately, with
the original smoothing and state. Color in other modes still updates pulse
width and secondary levels without recalculating unchanged oscillator coefficients.
The saw-slope division by 288 uses an exact Q31 reciprocal over the capped step
range, removing one of the three native division sites in coefficient setup.
The six sample loops, phase/noise updates and original tables are unchanged.

The native module footprint increases from 74,132 to 74,412 bytes; reported
audio stack changes from 312 to 316 bytes. Voice memory remains 984 bytes with
the same 16-voice pool. No cache fields or other state were added.

Compare complete output and voice state with the original revision:

```sh
python machines/hundekraft/hksid/tools/benchmark.py --baseline feaabc5 --sdk /path/to/open-xy
python machines/hundekraft/hksid/tools/benchmark.py --baseline feaabc5 --sdk /path/to/open-xy \
  --verify-only --full-grid
```

The default comparison covers 708 scenarios; the full grid covers 5,208, each
for 256 blocks, plus 4,096 exact display frames. All six models are exercised
with static settings, filter-only, Color-only and wave changes, combined
automation, retrigger/reset and pitch motion. The full comparison passes
AddressSanitizer and UndefinedBehaviorSanitizer. An independent coefficient
test checks all 6,291,457 valid slope inputs and 30,000 state transitions against
the original formulas, including evolving oscillator and filter history.

Overall paired host timing against `feaabc5` was roughly flat. Native output
avoids software division work during control changes; hardware CPU
improvement remains unmeasured. The SDK's separate host advisory reported
7,500 instructions per voice block at its heaviest setting, below its 15,000
aim. That advisory is not a catalog CPU record or device acceptance.

## Display reconstruction

See [VISUALS.md](VISUALS.md) for the original artwork, animation mapping and fidelity limits. Custom synth displays run on XY; effect and sample custom displays currently run in the collection browser only.
