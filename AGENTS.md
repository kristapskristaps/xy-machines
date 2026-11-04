# XY Machines

Use [docs/development.md](docs/development.md) and the root `./xy` commands.
The SDK is a separate pinned checkout configured by `./xy setup`; never vendor
or modify it to make a machine work. `SDK_REPOSITORY` and `SDK_REVISION` define
the dependency; local overrides live in ignored `local/dev.json`.
Read the SDK's authoring, UX, API, build and profiling guides before machine work.

## Standard development loop

1. Work on a `codex/<topic>` branch. Use separate Git worktrees per chat when
   needed, coordinate shared inventory/tooling edits, and merge completed work.
2. Run `./xy setup` once and `./xy doctor` to check the pinned SDK and preview
   compiler. Setup never resets an existing SDK or another chat's changes.
3. Use `./xy new <id> --name "Name" --controls 8` for a new synth, or add
   `--kind effect` for an effect. It creates tests/docs and updates the inventory
   without nested Git repositories. Sample engines use the HK Granular/Timestretch API examples;
   the SDK does not scaffold them. Preserve saved IDs and control contracts.
4. Edit the machine's C++, metadata, artwork, presets and machine-local tests.
   Run `./xy test <id>` for quick checks. `./xy test --changed` includes staged,
   unstaged and untracked machine files relative to HEAD; `--base origin/main`
   includes branch changes. Inventory/SDK-pin changes select all machines.
5. For synths and drums use `./xy preview <id> --watch`. The localhost allowlist serves
   fresh authenticated snapshots and reloads only successful builds. A failed
   edit keeps the previous audition available. Playing a key starts audio.
   The default chooses a free port; never stop another chat's server.
6. Run `./xy test <id> --native` before hardware work. Add `--cpu` for DSP
   performance changes; this is an advisory host estimate, not Blackfin timing.
   Effect native checks include Tandem because the SDK startup check needs an engine.

The SDK browser workbench supports synths and drum machines. Effects/sample engines use the same
test/build commands plus their specialist auditions listed in the development
guide. Do not claim that browser tests reproduce native mono/glide, stock
envelopes, sends, scheduling, device CPU or hardware display behaviour.

## Browser workflow

Use `./xy browser` for the cached official builder with catalog and private
machines. `./xy browser --refresh --stock-os <external original OS>` exports a
fresh pool and runs all SDK kit checks; a failed export never replaces the
cached builder. Catalog extensions are preserved and local machine IDs take
precedence. Exclude the rejected HK effects and HK Granular. Read the browser
section of docs/development.md before changing this workflow. Keep all native
objects, stock content, kits and site output outside every checkout. Use the
upstream browser's transfer implementation; do not add another device writer.

## Hardware workflow

- DSP/display C++ is statically linked: updating it needs a firmware rebuild,
  boot-partition write and reboot. MTP cannot hot-load a plugin. Iterate locally
  first and batch device tests. Native preset-only JSON changes can be copied
  and reloaded without flashing when the engine contract remains unchanged.
- Commit clean source, then run `./xy build --kit <collection.json> --layout
  <last-installed/layout.json>`. Never default hardware builds to the full
  inventory or omit the saved layout. Keep the other machines used by projects.
- `./xy stage <package>` verifies hashes and prepares an external install bundle.
  `--against <previous-verified/install-manifest.json>` lists bundle differences;
  it does not attest the live device. Presets are opt-in with `--presets`.
  Back up replaced device files, verify complete readbacks, and never delete
  unrelated content or silently overwrite edited user presets.
- Use upstream `sdk.py device push` on Windows and the documented OpenMTP or
  browser installation path on macOS. Do not create another transfer script.
  Confirm any required physical flash and verify reboot completion before
  claiming an installed build. Prefer an authorized software reboot over asking
  the user to power-cycle. Stock OS 1.2.1 accepts TE SysEx command 0x03 with
  payload 0x05: `F0 00 20 76 21 40 60 01 03 00 05 F7`. Send it once, after
  captures finish and with the user's project saved/stopped. Verify USB
  disconnect/reconnect and normal startup; for resident collections, also
  verify the confirmed generation and full active payload readback. Never
  infer reboot success from sending the packet or retry it blindly.
- Once device work is authorized, continue necessary reversible steps without
  repeatedly requesting approval. Ask for physical MTP/flash/reboot readiness
  when needed; build/test/stage commands themselves never contact a device.
- After a confirmed reboot, saved/stopped project and USB audio off, run
  `./xy profile --package <package> --machine <id> --installed` for the upstream-default capture
  (use `--runs 3` for repeated measurements) and the SDK report. Restart without saving diagnostic
  settings afterwards. Do not infer total project CPU or polyphony from it.
- For full-collection tests follow docs/hardware-testing.md. Keep lab-bank
  layouts/projects isolated from personal projects. Use `profile --all` to
  reuse one capture batch for all machines in an installed image;
  never repeat captures per report or imply that one benchmark covers every
  internal synthesis model, preset or demanding parameter setting.

## Repository boundaries and releases

- Work in this repository. The original sibling repositories and the original
  `open-xy-hundekraft` folder are independent working copies used by other chats;
  do not move, reset, commit, clean or overwrite their work as part of this repo.
- Standalone machines live in `machines/<id>`. The 26 Hundekraft ports live
  in `machines/hundekraft/<id>` with their original collection tools and notices.
  Each folder containing `machine.json` is an independently releasable machine.
  New machines belong in `machines/<id>` without a nested Git repository.
- Use root `collection.json` as the complete inventory. Files in `collections/`
  select groups; an inventory is not a promise that all machines fit one image.
  Keep IDs, parameter contracts, licences, notices and upstream attribution intact.
- Run `./xy test` for collection-wide previews and host tests, and
  `python -m unittest discover -s tests` for shared workflow tools.
  Use `--native` for per-machine Blackfin compilation into external temporary
  directories. Run the SDK public check and the staged/history private-data scan.
  Preserve imported DSP during layout/tooling changes; initial provenance hashes
  describe the historical snapshot, not future intentional source revisions.
- Contributor C++ uses public opxy headers, with no OS headers, platform
  conditionals, assembly, allocation, blocking, I/O or graphics in audio callbacks.
- Firmware, stock content, device captures, firmware packages and private backups
  stay outside every checkout, including ignored folders. Never commit them.
- Building/checking locally does not authorize device operations. Hardware tests
  require user authorization, confirmed install/reboot and the upstream-required capture(s).
  Measurements bind the exact source revision; old measurements are historical
  evidence, not CPU qualification for a newly reorganized release commit.
- Preserve original commits and namespaced `archive/<id>/...` branch/tag refs.
  `provenance/imports.json` describes initial imports, not mutable current hashes.
  Each release can have its own tag, commit, catalog path and CPU report. SDK
  catalog submissions follow the pinned SDK policy and change only the catalog entry.
  Current upstream targets main, or staging when an unreleased SDK API is needed.
