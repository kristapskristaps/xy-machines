# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/stutter`. Original notices follow.

stutter.c

/* STUTTER: a beat-locked stutter. The OP-1's beat clock (op1_beat_clock, the metronome's) cuts time into slices;
 * at each slice a dice roll decides whether the slice plays through or stutters: its first part is played and
 * recorded, then repeated until the slice ends, so the repeats always land on the grid.
 *   blue   RATE     slice length: 1/4, 1/8, 1/8 triplet, 1/16, 1/16 triplet, 1/32 of a beat ... (6 steps)
 *   green  MODE     REPEAT (plain), BRAKE (each repeat slower and lower, a tape stop), REVERSE (repeats backwards),
 *                   GATE (no repeats: the slice is chopped into short bursts of the live sound)
 *   white  SHAPE    edge softness of each repeat (2 .. 12 ms fades); in GATE the open time (10 .. 90 %)
 *   red    CHANCE   how many slices stutter: none .. all
 * Page 2 / press:
 *   blue   word 5   REPEATS: how many parts a stuttering slice is cut into: 2, 3, 4, 6, 8, 16
 *   red    latch    FREEZE: keep repeating the part playing now, across slices, until pressed again
 * With no clock (tape stopped while it follows the tape, or no tempo yet) the grid runs on at 120 BPM.
 * Ring buffer: 65536 samples (1.49 s) of the position's FX scratch; a slice longer than that (1/4 under 40 BPM)
 * is cut short. While frozen the ring keeps the frozen part (nothing new is recorded). State per FX position 0..2. On the master the dispatcher feeds a mono sum and adds the
 * difference to both sides, so a stutter there repeats the mid signal over the live sides.
 * Copyright (C) 2026 Hundekraft. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
