# Mando CPU qualification

Measured source: `49ccadf53bc4ddff5ed009d86139269349aef1ba`.
Two complete `opxy-synth-c8-sweep-v1` captures of the same installed bank each
measured 3.450065% at the heaviest setting (Waves: S P), below 3.5%.
The combined report gives midpoint 2.855726% and combined controls 2.816815%.
The first candidate in this session measured 4.487678%; independent filter
passes reduced it to 4.376287%, the adaptive core to 3.779660%, and compact
bass transfers to the final result.

This is the SDK's standard C4 control sweep in a Mando-only profiling bank,
with Capture retained. It is not an all-note or total-project CPU guarantee.
Upper notes retain full-rate synthesis. A complete production collection still
needs its own timing/listening validation before catalog submission.

Bass synthesis runs at 22.05 kHz with cubic reconstruction; filter design is
capped at 8.5 kHz in that path. This deliberately changes the upper harmonics.
Both oscillators, the resonant four-pole filter, controls and 16 factory
presets remain. Host checks include preset spectra/levels, pitch phases,
coefficient stability, reconstruction images and a smooth rate handoff.
Native compilation and ASan/UBSan sound tests pass. Listening acceptance
remains separate from these automated checks and CPU captures.

Raw MIDI captures, packages and device readbacks remain outside the repository.
