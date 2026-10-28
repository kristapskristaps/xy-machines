# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/mbira`. Original notices follow.

mbira.c

/* MBIRA: a thumb piano. Each note plucks a steel tine: four decaying partials (modal synthesis, sines with their own
 * decay) plus a short click, through a wooden box, with optional bottle-cap buzz.
 *   blue   STRIKE   soft thumb (dark, round) .. hard nail (bright click, strong upper partials)
 *   green  TINE     the tine's material: wood bar (1 : 4 : 9.8 : 17) .. steel tine (1 : 2 : 6.2 : 15.6, the kalimba's
 *                   weak octave from the box) .. glass (1 : 2.76 : 5.40 : 8.93, bell-like)
 *   white  BOX      the box resonance (a low band-pass about 220 Hz fed by the tine) and its thump
 *   red    BUZZ     bottle caps on a wire rattle against the tine while it swings wide (sizzle that follows the
 *                   fundamental and dies with it)
 * Page 2 (press the encoder; the words are saved with the sound, 0 = off / natural):
 *   blue   word 5  RING     how long the tine rings: 1.6 s (0) .. 8 s
 *   green  word 6  DOUBLE   a second tine tuned up to 12 cents sharp: slow beating, a chorus of two tines
 *   white  word 7  WAH      the player's hand over the sound hole: a band-pass sweeping 400 Hz .. 1.5 kHz at
 *                           5 Hz, depth = word
 * The partial decays and pitches are set once per block; amplitudes ramp across the block, so knob changes never
 * jump at the first sample. Notes above the tine's range lose the partials that would pass 19 kHz.
 * Cost (the unit adds SDRAM stalls to bcore's count, so the budget is tight): the block is built in passes over a
 * buffer, two partials per pass; a partial 60 dB under the fundamental (or under -80 dBFS) is dropped for the rest of
 * the note; the divisions run only when TINE, RING, DOUBLE or the pitch change; the box runs at 5.5 kHz, the wah at
 * 22 kHz, the buzz contact every 8 samples (the noise skipped while the caps are off the tine). A restrike resets
 * the voice's partials (no second set rings on). Coefficient and increment caches survive restrikes when their
 * control/pitch keys match, so the first render does not repeat unchanged divisions.
 * Copyright (C) 2026 Hundekraft. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
