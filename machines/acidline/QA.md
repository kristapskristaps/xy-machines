# Testing Acidline on an OP-XY

Checks to run on a device with a firmware that includes Acidline, from top to bottom, in one project:
each section leaves the project ready for the next. To run only some sections, set up what their first
line says. Host tests (`python ../open-xy/sdk.py test --machine .`) don't replace these: they can't hear
the sound or see the screen.

Report a failure with the section, the line and what you saw. A photo of the screen helps.

Run a complete pass on the exact commit that the catalog will pin. Record source commit,
SDK commit, package digest, firmware/OS, device generation and the tester/date with the results.
All boxes start unchecked; host tests and CPU captures do not imply listening or panel acceptance.
Any deferred section needs a reason and remains pending.

## Setup

- Build a firmware with the machine ([building firmware](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md#3-add-your-machine))
  and install it.
- Create a new project, **plain**, without Acidline: give track 1 a stock sound of its own and save it.
  Later steps load it as a project that never had the machine.
- Create another new project, **qa**, for the pass: don't use a project you care about.
- Track 1: Acidline (Shift + M1 on track 1, then choose it), with one note on step 1 lasting a whole
  bar, so it sounds all the time while the sequencer plays.
- Press play, and leave it playing unless a step says otherwise.

## Firmware basics

- [ ] The OP-XY boots, and stock engines, effects, the sequencer and project save/load work
- [ ] Acidline is in the engine list of tracks 1-8 (Shift + M1 there), under its name, and in no other
      track's list

## Its page and controls

Controls to exercise: **Flow**, **Bite**, **Burn**, **Sweep**, **Length**, **Shape**, **Body**, **Shade**.

- [ ] Each control through its whole range, both directions: the sound follows, with no clicks, jumps or
      silence at the ends; the value shown reads in its unit (Hz, ms, %, a name)
- [ ] Choice controls move one choice per detent, both directions, and wrap or stop as the page says
- [ ] Fast turns: the value keeps up, no flicker or slowdown
- [ ] With playback stopped, each control immediately moves its part of the flow cell.
      During a held note, compare the on-screen response with the change in sound.
- [ ] Controls with an alternate (an encoder press, a second page): pressing switches, the page shows
      which is active, and each keeps its own value
- [ ] The page while playing and while stopped, and after leaving it and coming back: nothing stale,
      missing or drawn twice
- [ ] Every key across the keyboard's octaves starts a note at its pitch, and releasing it ends the note;
      no note gets stuck, also after long holds
- [ ] Chords and fast repeated notes: every note sounds and ends
- [ ] M2 and M3 (envelope and filter) change the sound as on a stock engine

## Parameter locks

- [ ] Lock each control on a step (hold the step, turn the knob): it plays on that step, and the page
      shows the lock while the step is held
- [ ] Several locks on one step, and locks on neighbouring steps: each plays as written
- [ ] Clear the locks: the control plays its own value again

## LFOs

- [ ] The track's LFO (M4) lists the machine's controls by their names, and moving one of them is heard,
      stopped and playing
- [ ] The LFO on a choice control switches choices as it moves, and nothing freezes
- [ ] Rate, depth and shape behave as on a stock engine; turning the LFO off stops the movement

Then turn the LFO off.

## Patterns, scenes and songs

- [ ] Give the track a second pattern with different values: each pattern keeps its own
- [ ] Copy a pattern: the copy holds the same values, and changing one leaves the other alone
- [ ] Switch between the patterns while playing and while stopped: the sound and the page follow at once
- [ ] Make scenes 1 and 2 with the two patterns, and a song of scene 1 then scene 2, looping: each scene
      plays its own values from its first bar, on every pass

Then play scene 1.

## Presets

- [ ] Shift + track key lists presets: Acidline's are listed where they belong, and choosing one sets its
      values
- [ ] Save the track's sound as a preset, change it, load the preset: the saved values come back
- [ ] A stock preset loaded on a track holding Acidline (and the reverse) gives what it says, with no stray
      values

## Mute

- [ ] Mute the track (hold aux and press the track's key): it's silent, with no click or tail left behind;
      unmute: it plays as before
- [ ] The same from the mixer
- [ ] Muted in scene 2 only: switching between scenes 1 and 2 mutes and unmutes it

Unmute it everywhere.

## Machine-specific checks

- [ ] Compare saw/square Shape, resonance and drive at low and high pitches; verify legato/glide, note retrigger and low Body without runaway resonance.

## Saving and loading

- [ ] Save **qa**, power off and on, load it: the machine is there with every value as you left it, on
      each pattern
- [ ] Change a value, wait a few seconds and power off without saving: the OP-XY starts normally, and
      **qa** loads as last saved
- [ ] Load **plain**: track 1 has its own sound, nothing of **qa** shows. Load **qa** again: all back
- [ ] While playing, load **plain** and then **qa** again: nothing freezes, no stray values
- [ ] Create a new project: put Acidline on a track; it starts at its defaults
- [ ] On a firmware without Acidline, open **qa**: it loads, and the rest of the project plays

## CPU

- [ ] Capture the committed source with the SDK profiler and keep its revision-bound report.
      Use the required instrument control sweep and record its heaviest setting.
- [ ] Check the measured maximum against the project target of 3.5%; a higher result is recorded
      as a failed budget target and sent back for optimization.

## In the builder

Once the machine's catalog entry is merged and released (the beta site from `staging`, open-xy.com from
`main`; see [submitting a machine](https://github.com/etcetc000/open-xy/blob/staging/docs/catalog.md#submit-a-machine)):

- [ ] The machine list shows Acidline as Community, with its name, maintainer and description
- [ ] Filtering by its kind (synths) lists it, and filtering by the other kinds doesn't
- [ ] Searching its name, and each of its keywords, finds it; its details panel lists its keywords, and
      clicking one lists it among that keyword's machines
- [ ] Choosing it builds a firmware that includes it, and that firmware passes
      [Firmware basics](#firmware-basics)

## Known limits

Confirm the limitations documented in README.md. Browser audition does not reproduce stock envelopes,
sends, scheduling, saved project behavior or hardware CPU. Builder checks remain pending until catalog
publication; the missing-machine project check needs an alternate installed bank.
