# crease: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **3.516%** of the stock 580,352-cycle block budget. Midpoint run: 3.444%. Every control at its heaviest position: 2.625%. Heaviest single setting: Alt mix 50.0%.

| Setting | Median |
| --- | ---: |
| midpoint (stock) | 3.444% |
| Harmonic 0.0% | 2.715% |
| Harmonic 12.5% | 2.684% |
| Harmonic 25.0% | 2.704% |
| Harmonic 37.5% | 2.661% |
| Harmonic 50.0% | 2.704% |
| Harmonic 62.5% | 2.673% |
| Harmonic 75.0% | 2.684% |
| Harmonic 87.5% | 2.666% |
| Harmonic 100.0% | 2.686% |
| Timbre 0.0% | 2.612% |
| Timbre 12.5% | 2.643% |
| Timbre 25.0% | 2.718% |
| Timbre 37.5% | 2.686% |
| Timbre 50.0% | 2.660% |
| Timbre 62.5% | 2.702% |
| Timbre 75.0% | 2.661% |
| Timbre 87.5% | 2.644% |
| Timbre 100.0% | 2.695% |
| Morph 0.0% | 2.699% |
| Morph 12.5% | 2.667% |
| Morph 25.0% | 2.689% |
| Morph 37.5% | 2.663% |
| Morph 50.0% | 2.702% |
| Morph 62.5% | 2.655% |
| Morph 75.0% | 2.669% |
| Morph 87.5% | 2.643% |
| Morph 100.0% | 2.655% |
| Alt mix 0.0% | 2.689% |
| Alt mix 12.5% | 2.707% |
| Alt mix 25.0% | 2.689% |
| Alt mix 37.5% | 2.707% |
| Alt mix 50.0% | 3.516% |
| Alt mix 62.5% | 3.194% |
| Alt mix 75.0% | 3.195% |
| Alt mix 87.5% | 3.194% |
| Alt mix 100.0% | 3.195% |
| combined (as read from the reports): Harmonic 0.0%, Timbre 25.0%, Morph 50.0%, Alt mix 50.0% | 2.625% |

Recommended: at most 3.5%. **Warning: 3.516% of the block budget at Alt mix 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
