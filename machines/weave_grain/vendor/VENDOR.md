# DSP provenance

Plaits by Émilie Gillet (MIT), via the attributed DMDaudio integer port.
See LICENSE and the copyright notices retained in macro.cpp.

This fixed-model extraction uses XY Machines revision 957afdbac6f2ef9916991366e24517f1483dc0e4.
It fixes the model dispatch, retains only that model in the voice union,
isolates public symbols in its own namespace, and removes unused entry points.
Controls map directly to Weave Harmonic, Timbre, Morph and Output. DSP formulas,
gains, physical clocks and the optimized Chords path are retained. Other models
are not selectable. Host regression fixtures bind audio to the frozen Weave
revision; hardware CPU for this new machine remains unmeasured.
