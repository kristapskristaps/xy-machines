# rebound: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-effect-callback-v1`. Scope: machine render callback only.

One capture: **2.473%** of the stock 580,352-cycle block budget.

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or a settings sweep. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
