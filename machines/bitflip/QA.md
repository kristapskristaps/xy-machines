# Bitflip hardware acceptance

Status: pending. Local checks and demonstrations do not complete this checklist.
Record source commit, SDK commit, exact package/firmware hash, date and observations.
Use the repository's hardware-testing guide, a saved/stopped stock-only LAB BOOT
project, the last verified layout and an authorized installation/reboot. Keep
other machines needed by projects in their matching kit. Do not overwrite edited
user presets or save diagnostic settings.

- [ ] Confirm normal boot, Bitflip selection and both fixed header rows. Press
      each encoder to switch the same page; test Shift fine turns and detent enums.
- [ ] Audition all eight models and all sixteen presets at low, middle and high
      notes; compare dry levels, tuning, bass weight, transients and high-note grit.
- [ ] Sweep Color and Edge slowly and with parameter locks/LFOs in every model.
      Check pulse width, triangle harmonics, detuning, wave ROM morphs, PM, ring,
      both noise periods and Logic; listen for clicks or unintended silence.
- [ ] Set Sweep to −24, 0 and +24; check note-start capture, tuned tails, fast
      retriggers, legato/mono/glide and octave changes.
- [ ] Test every Chord at both Speed extremes; compare independent held voices,
      root restart, changes during a note and Chord off. Speed is not tempo sync.
- [ ] Test 2/8/16 Bits and low/full Clock on every model. Confirm intended grit,
      full-rate restoration, note release and absence of stuck DC or tails.
- [ ] Check the character, model selection, waveform and chord bars; confirm
      idle/held poses settle, reduced motion highlights held notes without bounce,
      and switching tracks establishes a fresh note baseline.
- [ ] Test stock AMP/filter/LFO pages, velocity, modulation, sends, mute, preset
      reload and save/reload of projects, patterns, scenes and a song.
- [ ] Hold notes across several tracks with demanding Twin, PM, Noise, rapid
      arpeggios and DAC settings; record audible scheduling/polyphony limits.
- [ ] Collect the revision-bound full control-sweep CPU capture with USB audio
      off and playback stopped. Inspect the heaviest model/setting and combined
      setting; aim for at most 3.5%, optimize heavier paths, and use extra captures
      for random/noise settings or to confirm a reported silent setting.
- [ ] Restart without saving diagnostic settings, verify normal listening again
      and restore the user's verified everyday collection/layout before normal work.
