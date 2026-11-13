# acidline: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 66 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.975%** of the stock 580,352-cycle block budget. Midpoint run: 3.671%. Every control at its heaviest position: 2.748%. Heaviest single setting: Shape Square.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.671% |
| Flow 0.0% | 3.665% |
| Flow 12.5% | 3.676% |
| Flow 25.0% | 3.626% |
| Flow 37.5% | 3.618% |
| Flow 50.0% | 3.615% |
| Flow 62.5% | 3.622% |
| Flow 75.0% | 3.627% |
| Flow 87.5% | 3.627% |
| Flow 100.0% | 3.604% |
| Bite 0.0% | 3.633% |
| Bite 12.5% | 3.626% |
| Bite 25.0% | 3.642% |
| Bite 37.5% | 3.626% |
| Bite 50.0% | 3.642% |
| Bite 62.5% | 3.624% |
| Bite 75.0% | 3.662% |
| Bite 87.5% | 3.647% |
| Bite 100.0% | 3.653% |
| Burn 0.0% | 2.741% |
| Burn 12.5% | 3.635% |
| Burn 25.0% | 3.626% |
| Burn 37.5% | 3.633% |
| Burn 50.0% | 3.622% |
| Burn 62.5% | 3.641% |
| Burn 75.0% | 3.636% |
| Burn 87.5% | 3.658% |
| Burn 100.0% | 3.664% |
| Sweep 0.0% | 3.636% |
| Sweep 12.5% | 3.624% |
| Sweep 25.0% | 3.630% |
| Sweep 37.5% | 3.626% |
| Sweep 50.0% | 3.629% |
| Sweep 62.5% | 3.626% |
| Sweep 75.0% | 3.629% |
| Sweep 87.5% | 3.626% |
| Sweep 100.0% | 3.629% |
| Length 0.0% | 3.633% |
| Length 12.5% | 3.627% |
| Length 25.0% | 3.622% |
| Length 37.5% | 3.630% |
| Length 50.0% | 3.626% |
| Length 62.5% | 3.629% |
| Length 75.0% | 3.622% |
| Length 87.5% | 3.622% |
| Length 100.0% | 3.615% |
| Shape Saw | 3.630% |
| Shape Triangle | 3.635% |
| Shape Square | 3.975% |
| Body 0.0% | 3.626% |
| Body 12.5% | 3.554% |
| Body 25.0% | 3.534% |
| Body 37.5% | 3.554% |
| Body 50.0% | 3.534% |
| Body 62.5% | 3.554% |
| Body 75.0% | 3.534% |
| Body 87.5% | 3.552% |
| Body 100.0% | 3.534% |
| Shade 0.0% | 3.636% |
| Shade 12.5% | 3.626% |
| Shade 25.0% | 3.636% |
| Shade 37.5% | 3.626% |
| Shade 50.0% | 3.653% |
| Shade 62.5% | 3.622% |
| Shade 75.0% | 3.633% |
| Shade 87.5% | 3.621% |
| Shade 100.0% | 3.630% |
| combined (as read from the reports): Flow 12.5%, Bite 75.0%, Burn 100.0%, Sweep 0.0%, Length 0.0%, Shape Square, Body 0.0%, Shade 50.0% | 2.748% |

Recommended: at most 3.5%. **Warning: 3.975% of the block budget at Shape Square is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
