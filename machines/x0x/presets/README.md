Seven editable 24-pad kits: **808 Classic** (`default`), **606 Dry**,
**707 Studio**, **909 Club**, **Night Wire**, **Warehouse** and **Soft Machines**.

Each preset stores its drum types and all used controls in the 24 pad records.
The eight track parameters stay neutral, so Style and both control pages remain
freely editable after loading. Open/closed hats share choke group 1.
An instrument unavailable in the requested family keeps an available source.

Regenerate with `python tools/generate_presets.py --sdk ../xy-sdk-gui-size`.
The host authoring helper uses the SDK's own packed-pad codec. The bank references
no sample-library files. `tests/sdk_integration.py` compares every saved pad with
the browser kit, through the real SDK decoder and one-detent editor.
