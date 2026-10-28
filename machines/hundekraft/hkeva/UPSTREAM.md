# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/eva`. Original notices follow.

eva.c

/* EVA: a space choir. A warm, sung 1980s film-score pad: two slightly detuned oscillators (a saw and a soft pulse),
 * a lowpass that opens with the breath, a gentle vowel colour on top of it, slow vibrato with a little drift per voice
 * and a wide ensemble chorus; the astronaut's helmet radio is the character on top (kennel synth plugin, eva.knl).
 * blue   (knob 0) PRESSURE: brightness and attack: soft, slow and distant (low) .. bright, quick and close (high)
 * green  (knob 1) FREQ: the vowel colour, oo - ah - ee: two wide formant bands blended with the lowpass
 * white  (knob 2) OXYGEN: air and width: breath noise in the attack, the ensemble chorus's depth, a slow swell
 * orange (knob 3) COM: the radio as a colour. Up to 40 %: a warm band-limit and soft saturation. Above 40 %: the
 *                  channel gets thinner (300 Hz .. 3 kHz), hiss rises, the carrier drops out now and then, and the
 *                  Key-pitched radio tones come in, louder with COM, for 250 ms when a note starts with no other note
 *                  held, the same pitch for 250 ms and a squelch tail when the last note is released (in the voice's
 *                  release, so a short release cuts it). Above 75 % the signal is crushed (sample rate and bits).
 * The engine never sees the voice sum, so the "shared" radio is shared state (the swell, dropouts, the beep), one owner
 * voice for each beep, advanced once per block by the first voice rendered (voices render in ascending order).
 * Everything runs at 22.05 kHz (half the output rate, which halves the cost; the oscillators are polyBLEP, the pad's
 * lowpass ends below 6 kHz), the output is interpolated 2x. Every knob-driven value moves from the previous block's
 * value to the new one in steps (the first at the old value): 8 for the level and the carrier, 4 for the filters
 * and the noises, so a knob step never jumps at the first sample; the
 * crush's hold and step change at block boundaries behind the stepped mix. Phases and LFOs are uint32 that wrap by
 * design (2^32 is one cycle), nothing else counts. Follows note velocity.
 * Cost: the block is built in passes over a buffer (source, pad, ensemble, radio), each with only its own state; the
 * divisions run when FREQ, COM or the pitch change; the polyBLEP runs only near an edge; the delay line is kept twice
 * so the taps read on through plain pointers.
 * Copyright (C) 2026 Hundekraft. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
