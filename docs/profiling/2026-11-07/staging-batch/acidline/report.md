# acidline: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 66 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.975%** of the stock 580,352-cycle block budget. Midpoint run: 3.671%. Every control at its heaviest position: 2.748%. Heaviest single setting: Shape Square.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 3.671% | 3.807% |
| Flow 0.0% | 3.665% | 4.007% |
| Flow 12.5% | 3.676% | 3.838% |
| Flow 25.0% | 3.626% | 3.801% |
| Flow 37.5% | 3.618% | 3.758% |
| Flow 50.0% | 3.615% | 3.828% |
| Flow 62.5% | 3.622% | 3.784% |
| Flow 75.0% | 3.627% | 3.809% |
| Flow 87.5% | 3.627% | 3.825% |
| Flow 100.0% | 3.604% | 3.818% |
| Bite 0.0% | 3.633% | 3.789% |
| Bite 12.5% | 3.626% | 3.998% |
| Bite 25.0% | 3.642% | 3.819% |
| Bite 37.5% | 3.626% | 4.022% |
| Bite 50.0% | 3.642% | 3.787% |
| Bite 62.5% | 3.624% | 4.105% |
| Bite 75.0% | 3.662% | 3.827% |
| Bite 87.5% | 3.647% | 4.050% |
| Bite 100.0% | 3.653% | 3.806% |
| Burn 0.0% | 2.741% | 4.109% |
| Burn 12.5% | 3.635% | 3.796% |
| Burn 25.0% | 3.626% | 4.041% |
| Burn 37.5% | 3.633% | 3.787% |
| Burn 50.0% | 3.622% | 3.990% |
| Burn 62.5% | 3.641% | 3.781% |
| Burn 75.0% | 3.636% | 3.986% |
| Burn 87.5% | 3.658% | 3.818% |
| Burn 100.0% | 3.664% | 4.092% |
| Sweep 0.0% | 3.636% | 3.783% |
| Sweep 12.5% | 3.624% | 3.818% |
| Sweep 25.0% | 3.630% | 3.792% |
| Sweep 37.5% | 3.626% | 3.832% |
| Sweep 50.0% | 3.629% | 3.815% |
| Sweep 62.5% | 3.626% | 3.871% |
| Sweep 75.0% | 3.629% | 3.793% |
| Sweep 87.5% | 3.626% | 3.816% |
| Sweep 100.0% | 3.629% | 3.798% |
| Length 0.0% | 3.633% | 3.790% |
| Length 12.5% | 3.627% | 3.771% |
| Length 25.0% | 3.622% | 3.786% |
| Length 37.5% | 3.630% | 3.801% |
| Length 50.0% | 3.626% | 3.796% |
| Length 62.5% | 3.629% | 3.795% |
| Length 75.0% | 3.622% | 3.786% |
| Length 87.5% | 3.622% | 3.769% |
| Length 100.0% | 3.615% | 3.772% |
| Shape Saw | 3.630% | 3.810% |
| Shape Triangle | 3.635% | 4.585% |
| Shape Square | 3.975% | 4.195% |
| Body 0.0% | 3.626% | 4.778% |
| Body 12.5% | 3.554% | 3.804% |
| Body 25.0% | 3.534% | 4.321% |
| Body 37.5% | 3.554% | 3.737% |
| Body 50.0% | 3.534% | 4.393% |
| Body 62.5% | 3.554% | 3.778% |
| Body 75.0% | 3.534% | 4.468% |
| Body 87.5% | 3.552% | 3.818% |
| Body 100.0% | 3.534% | 4.387% |
| Shade 0.0% | 3.636% | 3.795% |
| Shade 12.5% | 3.626% | 4.028% |
| Shade 25.0% | 3.636% | 3.787% |
| Shade 37.5% | 3.626% | 4.025% |
| Shade 50.0% | 3.653% | 3.822% |
| Shade 62.5% | 3.622% | 3.987% |
| Shade 75.0% | 3.633% | 3.799% |
| Shade 87.5% | 3.621% | 3.998% |
| Shade 100.0% | 3.630% | 3.795% |
| combined (as read from the reports): Flow 12.5%, Bite 75.0%, Burn 100.0%, Sweep 0.0%, Length 0.0%, Shape Square, Body 0.0%, Shade 50.0% | 2.748% | 4.114% |

Worst observed block: **4.778%** (Body 0.0%), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Mean 3.975% of the block budget at Shape Square is above the recommended 3.5%; make it lighter where you can (docs/profiling.md); Worst observed block peak 4.778% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
