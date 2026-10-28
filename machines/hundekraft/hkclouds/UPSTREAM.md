# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/clouds`. Original notices follow.

clouds.c

/* "clouds" - granular texture FX, proof of concept: fixed-point port of Mutable Instruments Clouds,
 * GRANULAR mode only (verified against the original, harness/clouds_compare.py).
 *
 *   knob 0 (blue)  : position   where in the recorded buffer (~2.7 s) grains are taken from
 *   knob 1 (green) : size       grain size
 *   knob 2 (white) : density    < 50 %: regular grains, > 50 %: random grains; more grains away from 50 %
 *   knob 3 (red)   : texture    grain window shape; above 75 % adds diffusion
 * encoder press (page 2, params[5..7], saved with the patch):
 *   blue press  -> pitch      -24 .. +24 semitones (centre = 0)
 *   green press -> dry/wet
 *   white press -> reverb
 *   red press   -> FREEZE on/off (latch, not saved)
 * fixed: no feedback
 *
 * Memory: recording buffer + fx delay lines (CLOUDS_MEM_BYTES = 164 KB) live in this FX position's
 * scratch buffer (172 KB, shared with the built-in delay). OP-1 speed options: 24 grains (original 40),
 * linear grain interpolation, idle diffuser/reverb/feedback skipped. Grain envelopes and reverb retain the
 * full-rate mapping; all 24 grains remain available. Fused grain reads/envelopes, cached grain setup and
 * bounded fixed-point arithmetic avoid redundant work without changing samples or the random sequence.
 * Original code (c) 2014 Emilie Gillet, MIT license (third_party/eurorack/LICENSE.clouds). */

clouds_dsp.c

/* Fixed-point port of Mutable Instruments Clouds, see clouds_dsp.h.
 * Structure and names follow the original files (granular_processor.cc, granular_sample_player.h, grain.h,
 * audio_buffer.h, looping_sample_player.h, fx/diffuser.h, fx/reverb.h, fx/pitch_shifter.h, stmlib Svf).
 * Original code (c) 2014 Emilie Gillet, MIT license. */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
