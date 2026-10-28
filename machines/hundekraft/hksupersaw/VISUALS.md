# HK Supersaw display

Reconstructed from Hundekraft 3.0.3 `plugins/supersaw/supersaw.svg` and its accompanying screen code. The original vector contours and painter order are retained, recoloured in greys with selective red, orange and blue. Original art SHA-256: `f2e02a1a325e3ef9751c5ac15b7da890aab775facd38c808ca34f345392da7ce`. Existing LICENSE, NOTICE and UPSTREAM notices apply to the imported artwork too.

The C++ display uses only public `opxy::draw` nodes and runs separately from audio. It is wired into the native synth and SDK browser workbench. Both preview and native-ready code use the same C++ frame computation. Nonuniform source transforms use pre-baked geometry keyframes; uniform scale, rotations and translation remain continuous.

The current public display input supplies parameters and note-gate events, but no oscillator, grain or effect audio telemetry. Moving parts follow note activity and a visual decay model; phase portraits and grain positions illustrate the controls rather than reading the DSP. Browser effects use measured return activity to gate motion. Reduced motion freezes incidental motion; track activation establishes a baseline without inventing a note. Source audio is unchanged.
