# Auto Tune hardware measurements

## Vocal FX

**Choir is CPU-high**, especially with Hard tuning, Compress and De-ess enabled.
The table reports DSP time as a share of the stock 580,352-cycle audio block
budget. Each workload ran for 1,500 blocks of 64 frames on an OP-XY with OS
1.2.1, optimized Checkpoint and a 4 MiB layout. The timer covered control
preparation and Vocal FX processing on private state. Stock mixing, routing
and project playback were outside the measurement.

| Workload | Average | Worst observed block |
| --- | ---: | ---: |
| Choir at 70% Blend, Tune Off, stereo phrase | 5.711% | 8.836% |
| Hard + Choir, stereo phrase | 7.056% | 11.496% |
| Hard + Choir + dynamics, stereo low vowel | 7.933% | 12.091% |
| Hard + Choir + dynamics, mono low vowel through the stereo renderer | 7.800% | 13.178% |
| Hard + Choir + dynamics, mono low vowel through the mono fast path | 6.748% | 9.793% |
| Hard + Choir + dynamics, mono loud phrase through the mono fast path | 6.719% | 11.727% |

The fixed-key workloads use C major, 100% Tune Amount and 70% Harmony Blend.
Dynamics uses Compress and De-ess at Soft. The two mono low-vowel runs use
identical input. The fast path requires identical left and right samples in
the delay history. Its use with the built-in mic on the device remains
unconfirmed.

Measured DSP revision `4893e10c08bc755deec91c21676012de06a5323a`.
Diagnostic firmware SHA-256
`cea8bf9287d34a54a79f3082defaf77e39db0251caff35462f84f92b6b678a01`.

## v0.1.0 pitch correction

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
calculation at all 81 lags before timing. These measurements apply to the
tagged pitch-correction release.

| Input | SHA-256 |
| --- | --- |
| Diagnostic firmware | `bfcb6fea67b2341ad0f6e61f30c2ccb2cdb795322812a1fd90a091ae2c0e3fe1` |
| `extension.cpp` | `ee416b37154d95ca405a9c388930f545934a62c2dbdfac1b251d24673771b46f` |
| `tuner.hpp` | `ec75ed40d2b382c62162c04f0f3684fce1aed5750283f6e74b5e3f71be803399` |
