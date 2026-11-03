# CPU measurements, 3 November 2026

Retained DSP at commit `6cf46e53d30cb4e3b4aa37b2476bb1c07107a367` preserves the sound and saved controls. Exact PCM/state regression tests cover control motion, extremes, retriggers and effect tails; independent interpolation/convolution references and UBSAN also pass. These checks establish equivalence in the tested scenarios. Normal project playback, listening and UI acceptance remain pending.

## Matched before/after comparisons

These percentages compare matched hardware images with the same selection and workload. Effects time their default stereo callback; synths use the largest control-sweep reading. No sample-rate, model, voice-capacity or precision reductions were made.

| Machine | Before | After | Reduction | Captures per build |
| --- | ---: | ---: | ---: | ---: |
| rebound | 3.619% | 2.484% | 31.4% | 3 |
| vader | 4.732% | 3.848% | 18.7% | 3 |
| vapour | 4.996% | 4.552% | 8.9% | 3 |
| acidline | 10.193% | 9.360% | 8.2% | 1 |
| westfold | 10.170% | 9.250% | 9.0% | 1 |
| halo | 15.973% | 13.858% | 13.2% | 1 |
| weave | 16.735% | 16.597% | 0.8% | 1 |

Every retained machine improved across all fixed settings in its comparison. Weave improves 3.0% at the median, but only 0.8% in its expensive modal model. Subvista (two trials), Mando and Matter changes were rejected because they did not improve the measured CPU. Their production DSP is unchanged.

Unchanged reference machines expose layout sensitivity: Karplus shifts from 3.6% slower in the first pair to 4.5% faster in the Westfold/Halo pair at the fixed-setting median; it is stable in the Vader/Vapour pair. Ping Pong varies by at most 0.1%. Results have not been normalized against references. The full reference summaries are in [summary.json](summary.json).

## Combined-source qualification

These are fresh readings of the retained source at the commit above, in three small selections. They bind that revision for later catalog preparation. Their image layouts differ from the comparison banks, so differences between these readings and the table above are not additional optimization gains.

| Machine | Block budget | Heaviest tested path | Evidence |
| --- | ---: | --- | --- |
| rebound | 2.361% | default stereo callback | [report](rebound/report.md), [CPU record](rebound/catalog-cpu.json) |
| vader | 3.755% | Drive 100.0% | [report](vader/report.md), [CPU record](vader/catalog-cpu.json) |
| vapour | 4.559% | default stereo callback | [report](vapour/report.md), [CPU record](vapour/catalog-cpu.json) |
| acidline | 9.323% | Wave Square | [report](acidline/report.md), [CPU record](acidline/catalog-cpu.json) |
| westfold | 9.133% | FM Ratio 75.0% | [report](westfold/report.md), [CPU record](westfold/catalog-cpu.json) |
| halo | 13.335% | default stereo callback | [report](halo/report.md), [CPU record](halo/catalog-cpu.json) |
| weave | 16.619% | Model modal | [report](weave/report.md), [CPU record](weave/catalog-cpu.json) |

Only Rebound is below the recommended 3.5% in these qualification banks. Make the listed paths lighter before proposing the remaining machines for inclusion. Measurements are percentages of the stock 580,352-cycle block budget, not total device CPU or maximum polyphony. Other control combinations and normal playback are outside these diagnostics.

The three captures contain 507 reports with zero protocol errors. All banks preserved the 36-ID ledger; installation and activation were readback-verified. Software reboots worked for these banks. The usual 23-machine collection plus Capture was restored and verified afterward. Raw captures, device identity, firmware and executable bundles remain private.

## Validation and integration

Collection-wide host/preview tests passed, with focused host and Blackfin checks after the final additions. Shared workflow tests pass (15); the pinned SDK public check passes (382 tests, with 16 explicit private-fixture skips). The private-data history scan passes. Acidline adds 128 bytes of mirrored history per voice, 2 KiB at the existing 16-voice capacity.

The current upstream SDK compiles all ten release candidates, and each fits beside the official machines. All ten plus the official machines do not fit simultaneously; the builder selects a fitting collection.

These are preparation records, not published releases. Existing version tags are preserved. RAM CPU records retain their collection hash and require the schema support proposed in [Open-XY PR 347](https://github.com/etcetc000/open-xy/pull/347).
