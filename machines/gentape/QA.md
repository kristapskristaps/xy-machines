# Gen Tape device checklist

Hardware acceptance and CPU qualification are pending. Record the machine commit,
SDK commit, package identity and hardware readout with the completed pass.
Use an isolated project and a build preserving the last installed layout.
Install/flash/reboot and diagnostics require separate user authorization.

## Startup and controls

- [ ] Confirm normal boot, stock instruments, sequencer and project save/load.
- [ ] Select Gen Tape on FX I and FX II using Shift + M1; confirm eight controls
      and encoder-press page switching, with readable choices and values.
- [ ] Confirm clean defaults; zero send has no effect contribution.
- [ ] At full send and Dry 0, confirm no source dry remains. Raise Dry and listen
      for chorus; confirm returning the send to zero restores the source.
- [ ] Sweep every control slowly, rapidly and with parameter locks, including
      adjacent-step locks and transitions between every Model choice.

## Sound

- [ ] Sustained keys/pad: Wow is smooth and irregular; Flutter is faster and
      lighter. Neither has unintended clicks, repeating zipper steps or silence.
- [ ] Saturate on bass, drums and bright chords: body compresses smoothly;
      detail stays clear. Compare matched listening levels, including extremes.
- [ ] Clean/VHS/cassette/dictation voices have distinct bandwidth and level;
      changes settle smoothly, with no persistent level loss or DC artifacts.
- [ ] Failure produces intentional drops and snags; zero disables new failures
      and existing movement settles. Distinguish intentional pitch changes from
      unexpected buffer glitches.
- [ ] Spread 0 preserves identical channels for identical input with Noise 0;
      Spread adds stereo movement. Check stereo input, mono input and mono sum.
- [ ] Noise 0 has no self-generated sound after the input/tail ends. Raising it
      adds controlled hiss; turn it off again without a stuck tail.
- [ ] Hot input and loud transients stay bounded without wraparound, runaway
      gain or crashes. Check quiet material and silence for residual tones.

## Projects and stress

- [ ] Both send slots at once keep independent state and parameter values.
- [ ] Switch away from Gen Tape and back after another buffered effect; reset,
      load a project and stop/restart without stale audio from previous effects.
- [ ] Patterns, copies, scenes, songs and saved/reloaded projects retain controls.
- [ ] Mute/unmute and effect changes behave correctly during playback.
- [ ] Play a demanding multi-track project while moving controls and switching
      patterns. Confirm stable playback, responsiveness and normal shutdown/boot.

## CPU

- [ ] Commit clean source and collect the SDK's default effect callback report.
- [ ] Separately exercise all controls enabled for every Model, the audio-example
      recipes, fast automation and frequent Failure events. Check startup/reset
      and both slots; the standard effect benchmark covers defaults only.
- [ ] Collect at least three fresh-boot runs for this random effect. Preserve
      private captures and publish only sanitized revision-bound reports.
- [ ] Verify at most 3.5% for one stereo instance in the required hardware scope.
      If a demanding path exceeds it, optimize and repeat qualification before
      advertising the target as achieved. Do not substitute host estimates.
