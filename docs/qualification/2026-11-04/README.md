# Standalone release qualification — 4 November 2026

Measured source: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`.
These candidates are **not ready for catalog submission**.

Version correction: this attempt measured GitHub main's older 32-voice X0X and
older Westfold core. The local eight-voice X0X optimization branch and the
compact-core Westfold redesign were not integrated into this snapshot. Their
current CPU cost is unmeasured; the numbers below must not be attributed to
those newer versions. The five working machines were subsequently remeasured
with the standard staging SDK and submitted with the author's acceptance of
CPU recommendations: see [standard release reports](../../profiling/2026-11-04/standard-release/README.md).

The seven-machine bank completed all 342 expected hardware reports with no
protocol errors. Source-bound reports are stored beside this file.

| Machine | Hardware block budget | Project target |
| --- | ---: | --- |
| Halo | 3.210498% | Pass |
| Rebound | 2.418555% | Pass |
| Mando | 3.714046% | Fail: Waves P S |
| Vader | 3.878843% | Fail: Drive 37.5% |
| Acidline | 3.982605% | Fail: Shape Square |
| Westfold | 9.040970% | Fail: Decay 100% |
| X0X | No valid publishable record | Kit and 32-voice ceiling saturate the budget |

The project target is 3.5%. Synth results are the SDK's complete control sweep;
effects use its default callback workload. These are not total-project CPU or
all-note guarantees. Mando's older isolated-bank 3.450065% result does not prove
that this build meets the target. Further optimization needs headroom for
layout-dependent timing, not a result barely under the threshold in one bank.

X0X's heaviest measured single-voice diagnostic was 15.564202% (Cymbal/model 9,
level 64). Both its 32-voice kit and ceiling runs saturated the metric encoding.
The SDK refused a CPU record; no clipped 100% value is published as an actual
measurement. Reducing advertised polyphony alone would not solve its heavy
single-voice path.

## Startup failures

The complete twelve-machine profiling bank fit the host link limit but did not
confirm startup (generation 168). After the required physical restart, the
loader used its embedded fallback and retained a pending attempt. The five
spin-off bank (170) and Tandem/Sift/Atlas subgroup (171) behaved the same way.
This rules out image size alone; it does not establish which machine or shared
integration path caused the failure. Choir and Crease were not tested alone.

The seven-machine bank booted and passed full active-payload readback at
169 and again at 172. Their non-profiling listening version is installed and boot-confirmed as
generation 173, with full active-payload readback and normal MIDI return. The full twelve-machine listening bank is built locally
but has not been accepted on hardware and must not be described as installed.
All 37 prior IDs were preserved, and five additional IDs remain reserved.
No boot partition, user project or existing preset was overwritten.

## Remaining device QA

Each candidate has a tailored QA.md. A full pass on the exact catalog-pinned
commit is recommended before publication. Sound, display/control behavior,
parameter locks, LFOs, patterns/scenes/songs, presets and save/load remain
pending human observation. CPU captures and a normal boot do not satisfy those
checks. Builder checks follow catalog publication and are separately pending.

Raw captures, firmware-containing packages, backups and device status records
remain outside every checkout. No catalog entry, release tag or public branch
has been updated by this qualification attempt.
