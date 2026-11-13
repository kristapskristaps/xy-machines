# mando: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 61 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.644%** of the stock 580,352-cycle block budget. Midpoint run: 3.015%. Every control at its heaviest position: 3.046%. Heaviest single setting: Octave +12.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 3.015% | 3.925% |
| Cutoff 0.0% | 3.299% | 4.706% |
| Cutoff 12.5% | 3.320% | 3.595% |
| Cutoff 25.0% | 3.330% | 3.615% |
| Cutoff 37.5% | 3.352% | 3.603% |
| Cutoff 50.0% | 3.336% | 3.716% |
| Cutoff 62.5% | 3.340% | 3.615% |
| Cutoff 75.0% | 3.317% | 3.664% |
| Cutoff 87.5% | 3.349% | 3.600% |
| Cutoff 100.0% | 3.330% | 3.636% |
| Reso 0.0% | 3.340% | 3.543% |
| Reso 12.5% | 3.325% | 3.662% |
| Reso 25.0% | 3.343% | 3.537% |
| Reso 37.5% | 3.326% | 3.554% |
| Reso 50.0% | 3.342% | 3.557% |
| Reso 62.5% | 3.326% | 3.575% |
| Reso 75.0% | 3.352% | 3.581% |
| Reso 87.5% | 3.337% | 3.574% |
| Reso 100.0% | 3.375% | 3.664% |
| Drive 0.0% | 3.325% | 3.609% |
| Drive 12.5% | 3.339% | 3.575% |
| Drive 25.0% | 3.337% | 3.537% |
| Drive 37.5% | 3.392% | 3.572% |
| Drive 50.0% | 3.389% | 3.590% |
| Drive 62.5% | 3.410% | 3.604% |
| Drive 75.0% | 3.409% | 3.589% |
| Drive 87.5% | 3.429% | 3.616% |
| Drive 100.0% | 3.413% | 3.604% |
| Waves S S | 3.340% | 3.534% |
| Waves S P | 3.598% | 4.280% |
| Waves P S | 3.642% | 3.978% |
| Waves P P | 3.302% | 4.337% |
| Detune 0.0% | 3.340% | 3.969% |
| Detune 12.5% | 3.433% | 3.694% |
| Detune 25.0% | 3.447% | 3.699% |
| Detune 37.5% | 3.435% | 3.688% |
| Detune 50.0% | 3.447% | 3.714% |
| Detune 62.5% | 3.435% | 3.687% |
| Detune 75.0% | 3.449% | 3.723% |
| Detune 87.5% | 3.436% | 3.706% |
| Detune 100.0% | 3.459% | 3.723% |
| Balance 0.0% | 3.093% | 3.536% |
| Balance 12.5% | 3.337% | 3.571% |
| Balance 25.0% | 3.323% | 3.545% |
| Balance 37.5% | 3.337% | 3.558% |
| Balance 50.0% | 3.325% | 3.552% |
| Balance 62.5% | 3.340% | 3.578% |
| Balance 75.0% | 3.325% | 3.505% |
| Balance 87.5% | 3.337% | 3.548% |
| Balance 100.0% | 3.085% | 3.413% |
| Octave -12 | 3.328% | 3.563% |
| Octave 0 | 3.323% | 3.621% |
| Octave +12 | 3.644% | 3.981% |
| Keytrack 0.0% | 3.323% | 3.693% |
| Keytrack 12.5% | 3.386% | 3.630% |
| Keytrack 25.0% | 3.360% | 3.722% |
| Keytrack 37.5% | 3.386% | 3.589% |
| Keytrack 50.0% | 3.362% | 3.726% |
| Keytrack 62.5% | 3.389% | 3.648% |
| Keytrack 75.0% | 3.363% | 3.748% |
| Keytrack 87.5% | 3.394% | 3.653% |
| Keytrack 100.0% | 3.371% | 3.685% |
| combined (as read from the reports): Cutoff 37.5%, Reso 100.0%, Drive 87.5%, Waves P S, Detune 100.0%, Balance 62.5%, Octave +12, Keytrack 87.5% | 3.046% | 3.423% |

Worst observed block: **4.706%** (Cutoff 0.0%), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Mean 3.644% of the block budget at Octave +12 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md); Worst observed block peak 4.706% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
