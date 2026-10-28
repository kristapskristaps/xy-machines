# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/mixtur`. Original notices follow.

mixtur.c

/* "mixtur" - subharmonic organ after Oskar Sala's Mixtur-Trautonium (1952; The Birds, 1963).
 * The played key is the TOP note. Four sawtooth dividers sound its undertones f/n (n = 1..24), all phase-locked
 * to one master phase like the Trautonium's frequency dividers, and the mixture goes through two resonant
 * formant band-passes in parallel (vowel-like "Formantfilter").
 *   knob 0 (blue)  : divider A undertone n   (off, 1..24 - 25 steps)
 *   knob 1 (green) : divider B undertone n
 *   knob 2 (white) : divider C undertone n
 *   knob 3 (red)   : divider D undertone n
 *   blue press     : page 2 = formant 1 frequency  180 .. 4070 Hz   (word 5; 0 = unset = 8192 = 392 Hz)
 *   green press    : page 2 = formant 2 frequency  500 .. 5660 Hz   (word 6; 0 = unset = 8192 = 917 Hz)
 *   white press    : page 2 = resonance Q 0.7 .. 22                 (word 7; 0 = unset = 8192 = Q 3.2)
 * Unset page-2 words read as 8192 because that is where the firmware starts turning an unset word.
 *
 * Dividers, exact and division-free per sample: master phase p (u32, 2^32 = one cycle of the key's pitch).
 * Each divider k counts master wraps in c_k = 0..n_k-1; its own phase is
 *     q_k = c_k * floor(2^32/n) + (p >> 16) * floor(2^16/n)          (tables, gen_tables.py)
 * which never exceeds 2^32 - 65536/n, so it stays locked forever with no drift and only a 2^-16 step where c_k
 * advances. Every saw reset falls on a master wrap, so one 2-sample polyBLEP at the wrap (lookahead for the sample
 * before) band-limits all four dividers at once. Changing n while a note sounds fades that divider out (~12 ms),
 * switches, and fades it back in: no clicks. Level is normalised for 1..4 distinct undertones.
 * Formants: Chamberlin SVFs (Q13 coefficients, int32 state), input scaled by the damping q so the band-pass has
 * unity peak gain, makeup gain over Q, and a soft knee at the output: linear up to 0.7 of full scale, rounded above.
 * Undertones below the audible range fade out (40 Hz full, 25 Hz silent): a 13 Hz "undertone" is not a tone but
 * 13 clicks a second, each ringing the formants - it crackles.
 * Cost: two passes per block (mixture -> stack buffer, then filters), ~102 instructions per sample, all multiplies
 * 16 x 16; per block one division (polyBLEP reciprocal). See README for the CPU estimate.
 * Copyright (C) 2026 ultralazr. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
