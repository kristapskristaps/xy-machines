# Halo CPU qualification

Measured source: `5a2dbae391eac77bc7bb2af568766c151bf6514e`.
One complete `opxy-effect-callback-v1` capture measured **3.364614%** on
OP-XY OS 1.2.1. All 48 expected collection reports arrived without errors.
This is the SDK default effect workload in a Halo/Karplus profiling bank,
with Capture retained. It does not measure total project CPU or every preset
and parameter combination. Karplus is the bank's required synth reference.

The first four-line FDN candidate measured 5.888457%. Separating its scalar
network into a function regressed to 6.144808% and was dropped. The final DSP
runs its stereo echo, four modulated feedback lines and shared octave-up heads
at 5.5125 kHz, with three-stage CIC conversion from/to 44.1 kHz. Delay lengths,
modulation and envelope rates were rescaled; feedback compensation preserves
long shimmer tails. Separate channel conversion passes reduce live state.
Scratch is 97,536 bytes, versus 231,424 bytes for the earlier plate. Send state
remains 232 bytes; controls, presets and saved IDs are preserved.

The deliberate tonal tradeoff is a warmer, darker wet return. This is a
reverb redesign, not an exact rendering of the earlier plate. Factory-preset
impulse tests require total and late energy within 3 dB of that reference,
decay estimates within 70–140% of the reference, and a decorrelated stereo tail. Extrapolated
preset T60 estimates are about 5.5, 10.1 and 14.2 seconds, compared with 6.8, 11.6
and 17.0 seconds for the reference. These are host IR estimates, not hardware
acoustic measurements or listening acceptance.

Seven host tests, native Blackfin compilation, host/WebAssembly sample parity,
the browser worklet, exact zero-send dry playback and ASan/UBSan checks pass.
A 120-second host render at maximum continuous controls, including driven input,
release and Hold, remained bounded without sanitizer errors. The near-unity
pole conversion is clamped and exhaustively checked over all 32,768 Q15 values.
The SDK public check completed 382 tests successfully, with 16 private/optional
tests skipped under its public-check contract. No SDK source was modified.

Raw MIDI captures, packages, reboot evidence and full device readbacks remain
outside the repository. The pre-test audition collection was transferred back
and readback-verified as generation 166; the software USB restart was observed.
The final active-collection check is pending: normal MIDI endpoints disappeared
after startup, and the device could not be opened over MTP. A normal-screen/USB
reconnect confirmation has been requested. No presets were transferred or
overwritten. This Halo work remains local and has not been pushed or submitted
to the catalog.
