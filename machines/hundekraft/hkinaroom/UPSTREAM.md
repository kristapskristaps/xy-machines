# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/in_a_room`. Original notices follow.

sitting.c

/* "in a room" - Alvin Lucier, "I am sitting in a room" (1969/70). Lucier recorded a paragraph of speech, played
 * it back into a room and re-recorded it, again and again, until the words melted into the room's own
 * resonances. Here the room is a model, the tape is a loop in the FX scratch buffer (22.05 kHz, up to 3.9 s,
 * lo-fi on purpose; 11.025 kHz for long tempo-synced loops) and every pass of the loop is one generation:
 * playback -> room -> peak-normalise -> written back in place. You hear the loop (wet); the dry signal passes
 * underneath with the mix knob. (Source folder and symbols keep the old name "sitting".)
 *
 *   knob 0 (blue)  : room size     longest wall 0.8 .. 8 m (log; 6.1 m at 88 %); the other two scale with it
 *   knob 1 (green) : shape         4 rooms (L : W : H): CUBE 1:1:1, GOLDEN 2.618:1.618:1,
 *                                  BOLT 1.59:1.26:1, LUCIER 6.1 x 4.3 x 2.6 m (20' x 14' x 8.5': plausible
 *                                  for his Middletown living room, not measured)
 *   knob 2 (white) : erosion       how hard each generation is filtered: resonator Q 8 .. 80 and
 *                                  room share 12 .. 100 % (the rest is the direct sound + early reflections)
 *   knob 3 (orange): mix           dry .. wet (wet = the loop)
 * encoder press:
 *   blue press  -> page 2: loop length of the next capture (params[5]): the first 40 % FREE 0.5 .. 3.9 s, then 6
 *                  tempo-synced lengths: 1/4 BEAT, 1/2 BEAT, 1 BEAT, 2 BEATS, 1 BAR, 2 BARS (4/4)
 *   green press -> page 2: absorption, high-frequency loss per generation (none .. one-pole ~1.3 kHz; params[6])
 *   white press -> page 2: crossfade at the loop boundary of the next capture, off .. 93 ms (params[7]): the loop's
 *                  end blends into the input just before its start instead of fading both ends to silence
 *   orange press-> LATCH: every toggle starts a new capture (generation 0): the input is recorded for the loop
 *                  length, then the erosion starts. Before the first capture the wet signal is silence.
 *                  Synced length: the press ARMS the capture (the old loop keeps playing); it starts on the next
 *                  beat (bar lengths: the next bar line, 2 beats: beat 1 or 3), records exactly the musical length
 *                  on the beat clock and loops on it: every generation starts on the grid. Press again while
 *                  armed to cancel.
 *
 * Sync: the OP-1 beat clock (op1_beat_clock: the metronome's, read per block) gives the time to the next beat; the
 * quarter-beat grid points are found per block (at most one per 64 samples) and counted: capture and generations end
 * after exactly `lq` grid points, at the output sample of the grid point - so the loop length follows the clock to
 * the sample and never drifts. While the clock stands still (it follows the tape and the tape is stopped) a copy
 * runs on at the tempo. Bars come from the tape position while the clock follows the tape (as the accent click),
 * else the first synced capture defines beat 1. A loop that does not fit 3.9 s at 22.05 kHz is recorded at
 * 11.025 kHz (7.8 s); one that does not fit that either is halved. Tempo changes while a loop plays: the loop stays
 * on the grid; faster cuts its end (de-clicked), slower leaves a gap of silence before the next start.
 *
 * Room model (per loop sample): 3 early reflections (round trips across L, W, H, from a small delay line) + 6
 * resonant peaks at the room's axial modes f = n c / 2D (c = 343 m/s; the 6 lowest modes of the three dimensions at
 * or above 110 Hz, coincident modes merged into one stronger peak, as in a cube). Resonators: Chamberlin SVFs with
 * 32-bit state and Q15 coefficients (2 sin(pi f/fs) by a Taylor series, per knob change), stable down to the lowest
 * mode. Then a DC blocker, the absorption low-pass and the normalisation gain, updated per generation from the peaks
 * the last pass read and wrote (see next_generation), so the loop neither explodes nor dies (two divisions per
 * generation). Memory (scratch of this FX position): loop 86016 x int16 + early-reflection line 2048 x int16.
 * Copyright (C) 2026 ultralazr. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE). */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
