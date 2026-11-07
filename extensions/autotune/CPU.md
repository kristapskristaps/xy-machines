# Auto Tune hardware measurements

Measured on OP-XY OS 1.2.1. Each workload timed 1,500 stereo blocks of 64 samples
on a private instance of the production tuner. Percentages use the stock
580,352-cycle block budget. The timer covered tuner preparation and processing.
Stimulus generation, Aux routing and mixing, and graphics were outside the timer.

| Workload | Average | Worst observed block |
| --- | ---: | ---: |
| Off, voiced phrase | 0.009% | 0.029% |
| Smooth, voiced phrase | 2.173% | 4.613% |
| Hard, voiced phrase | 2.344% | 4.543% |
| Hard, quiet voiced phrase | 2.351% | 5.423% |
| Hard, low vowel | 2.368% | 4.572% |
| Hard, unvoiced noise | 1.263% | 3.664% |
| Hard, silence | 0.760% | 2.129% |

The heaviest average was 2.368% on a low vowel in Hard mode. Quiet voiced input
in Hard mode produced the highest observed peak, 5.423%.

The diagnostic used the combined development image with optimized Checkpoint and
a 4 MiB reservation. Its DSP sources match this release by SHA-256. The
diagnostic verified the native squared-distance result against the scalar
calculation at all 81 lags before timing. Full live-input QA on a submission
build remains pending.

| Input | SHA-256 |
| --- | --- |
| Diagnostic firmware | `bfcb6fea67b2341ad0f6e61f30c2ccb2cdb795322812a1fd90a091ae2c0e3fe1` |
| `extension.cpp` | `ee416b37154d95ca405a9c388930f545934a62c2dbdfac1b251d24673771b46f` |
| `tuner.hpp` | `ec75ed40d2b382c62162c04f0f3684fce1aed5750283f6e74b5e3f71be803399` |
