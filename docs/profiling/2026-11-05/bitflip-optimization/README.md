# Bitflip waveform-cache optimization

The selected redesign measures **3.862058%** on hardware, below the user
acceptance threshold of 4%. Midpoint: **3.509575%**. The heaviest swept setting
is Model FM; the recommended 3.5% remains a further optimization target.

| Trial | Heaviest measured run | Decision |
| --- | ---: | --- |
| Model dispatch outside the sample loop | 5.560388% | Retained as a foundation |
| Narrow interpolation products | 5.784695% | Discarded |
| Prepared wavetable addresses | 5.844205% | Discarded |
| Incrementally cached blended waveform | 3.862058% | Retained |

Each trial has one complete hardware control sweep. Images have different
layouts and the unchanged Choir reference also varies, so these are absolute
results, not a controlled estimate of each individual compiler change.
The final capture is complete, has no protocol errors, and binds source
commit `8fca276659ce8b4290f2429c719d479ebdd9a041`.

The cache folds color-frame and band interpolation into a 257-point waveform,
prepared at no more than 32 points per callback after its key stabilizes.
Control or pitch changes that invalidate it immediately use the original
renderer. It adds 536 bytes per voice and keeps eight-voice capacity.

Five host tests and native checks pass. The waveform comparison covers all
four families, five bands, representative color positions and blend weights;
maximum error is two signed16 units. Render comparisons across all eight
models and four pitches, including a color step and a pitch change, have a
worst relative RMS error of 0.000090 with unchanged oscillator phase. The
existing tuning, spectral, arpeggio, DAC, control-corner and automation checks
also pass. These checks do not establish a CPU bound during arbitrary control
motion or replace hardware listening and project QA.

All changes remain local. Firmware and raw captures remain outside the repository.
