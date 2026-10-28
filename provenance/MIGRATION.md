# Source migration record

Created 2026-10-29. All source imports are copies; original checkouts, working
branches, indexes and uncommitted files were not changed. Later work in those
folders is independent of this frozen import and can be integrated separately.

## Integrity

- All 35 machines are listed once in the root collection: 18 synths, 15 effects,
  and 2 sample engines. The duplicate Vader optimization checkout and the SDK's
  upstream core examples are not imported as additional machines.
- 900 imported files verified against the source snapshot hashes.
- All 110 licence, notice and upstream/vendor-provenance files retained byte for byte.
- All 433 C++/header, artwork and test-source files retained byte for byte.
- 85 documentation/tooling changes adapt relative SDK paths and folder descriptions;
  their before/after hashes are in `layout-adaptations.json`.
- Seven Git histories imported without squashing or rewriting original commits.
  Original branch/tag names and SHAs are in `imports.json`, with namespaced archive
  refs retained in this repository. Sources without commits have initial snapshots.
- All imported source histories passed the SDK's private-data scan before import.
  Firmware, device data, generated binaries, audition recordings, caches and
  source-folder `local/` directories were excluded from source snapshots.

## Local validation

- All 35 machine SDK checks passed, including 109 host test executables and
  previews for all 18 synths.
- All 35 native checks passed using the pinned Blackfin compiler. Each effect
  is linked with Subvista to satisfy the SDK startup requirement for an engine.
  Compilation outputs were temporary and outside all checkouts.
- SDK 1.5 public checks passed; 11 private-fixture tests were explicitly skipped.
- No DSP, saved parameter definitions, artwork or sound presets were changed.
- No device operations or hardware CPU measurements were performed for this
  migration. Earlier measurements remain tied to their original commits.

Weave's committed import requires SDK 1.5's Shift selector. CI pins
`kristapskristaps/open-xy` at `fbc62e3bfa30fb2be865b5106572c280723d712f`,
which is available on `codex/shift-model-selector`. Validation used a separate
SDK copy. Existing SDK checkouts used by other threads were not modified.

## Source revisions

| Original folder | Imported source revision | Destination |
| --- | --- | --- |
| vader | `ee31a8525ba7d381743d682eda232397f8929c83` | `machines/vader` |
| westfold | `c887eae4f181ae3e3ccb163213ffd90c7b61ed36` | `machines/westfold` |
| open-xy-plaits | `7d6634b7c0015f476eee7c42e8bc6dea1afa36ad` | `machines/weave` |
| open-xy-clouds | `a6fec04bd674a948c92a913c5514376133755499` | `machines/vapour` |
| open-xy-elements | `a6abe947d59efe4b7396b68d5ca44be920203d2d` | `machines/matter` |
| open-xy-halo | `d9a69127d4d6edd2d3bc2168c796db24edd66a92` | `machines/halo` |
| granular | `ca1c235fefe040e03dd3907bc857431242b8a282` | `machines/granular` |
| open-xy-hundekraft | Initial source snapshot | `machines/hundekraft` |
| acidline | Initial source snapshot | `machines/acidline` |
| subvista | Initial source snapshot | `machines/subvista` |
