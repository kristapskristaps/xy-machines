# HK Snowglobe display

Reconstructed from Hundekraft 3.0.3 `plugins/snowglobe/snowglobe.svg` and its accompanying screen code. The original vector contours and painter order are retained, recoloured in greys with selective red, orange and blue. Original art SHA-256: `dcdd1dfcc8314b827346f94e44f534b0d2b714c6ba17da777430ba74355b17f8`. Existing LICENSE, NOTICE and UPSTREAM notices apply to the imported artwork too.

The C++ display uses only public `opxy::draw` nodes and runs separately from audio. This is an audition display study. The SDK has no effect/sample custom-display registration, so it is not wired into native firmware. Both preview and native-ready code use the same C++ frame computation. Nonuniform source transforms use pre-baked geometry keyframes; uniform scale, rotations and translation remain continuous.

The current public display input supplies parameters and note-gate events, but no oscillator, grain or effect audio telemetry. Moving parts follow note activity and a visual decay model; phase portraits and grain positions illustrate the controls rather than reading the DSP. Browser effects use measured return activity to gate motion. Reduced motion freezes incidental motion; track activation establishes a baseline without inventing a note. Source audio is unchanged.
