"""Run X0X through the pinned SDK's real pad editor, paging and storage codec.

Reuses the SDK fixture's stock boundary doubles; no stock addresses or native
hooks belong to the machine. This is host evidence, not a hardware observation.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess
import tempfile


DRIVER = r'''
int main() {
    using namespace opxy;
    using namespace opxy::platform;
    const DrumCodec codec = drum_codec(x0x::model_count, x0x::parameter_count);
    require(codec.fits() && codec.fulls == 12);
    for (u32 t = 0; t < 8; ++t) for (u32 p = 0; p < 24; ++p) {
        regions[t][p][0] = 123456;
        regions[t][p][5] = 987654;
        reinterpret_cast<u8*>(regions[t][p])[26] = u8(53 + p);
    }
    auto edit = [](u32 control, i32 delta) {
        const u32 event[] = {1000, u32(delta), 0};
        sdk_drum_edit(key, control, event, 0);
    };
    u8 model, raw[8];
    track = 0; key = 53;
    require(sdk_drum_values(track, &model, raw) && model == x0x::kick && raw[1] == 64);
    edit(4, 1); // Shift + black: kick -> snare
    require(sdk_drum_values(track, &model, raw) && model == x0x::snare);
    edit(0, 1); // One detent: 808 -> 909, independent of type
    edit(1, 7);
    require(sdk_drum_values(track, &model, raw) && x0x::family_index(model, raw[0]) == 3 && raw[1] == 71);
    key = 54;
    require(sdk_drum_values(track, &model, raw) && model == x0x::snare && raw[1] == 64);
    for (u32 encoder = 0; encoder < 4; ++encoder) {
        sdk_synth_press(encoder); shown(4,5,6,7);
        require(sdk_drum_values(track, &model, raw));
        u8 before[8]; std::memcpy(before, raw, 8);
        edit(encoder, 1);
        require(sdk_drum_values(track, &model, raw));
        for (u32 s = 0; s < 8; ++s) require(raw[s] == before[s] + (s == 4 + encoder ? 1 : 0));
        sdk_synth_press(encoder); shown(0,1,2,3);
    }
    key = 53;
    require(sdk_drum_values(track, &model, raw) && model == x0x::snare && raw[1] == 71);
    key = 55;
    edit(0, -1); // One detent: default 808 clap -> 707 clap.
    require(sdk_drum_values(track, &model, raw) && x0x::family_index(model, raw[0]) == 1);
    edit(4, -2); // Clap -> kick, preserving the same requested family.
    require(sdk_drum_values(track, &model, raw) && model == x0x::kick && x0x::family_index(model, raw[0]) == 1);
    key = 53;
    // The Shift model control remains the same while the second page is up.
    sdk_synth_press(2); shown(4,5,6,7);
    edit(4, 1);
    require(sdk_drum_values(track, &model, raw) && model == x0x::clap && raw[1] == 71);
    edit(5, 2); // stock choke group 2
    require(sdk_drum_choke_choice(track) == 2);
    edit(4, -1);
    require(sdk_drum_choke_choice(track) == 2);
    sdk_synth_press(0); shown(0,1,2,3);
    // Other tracks have their own pad records and page selection.
    track = 1; key = 53; shown(0,1,2,3);
    require(sdk_drum_values(track, &model, raw) && model == x0x::kick && raw[1] == 64);
    edit(4, 7);
    require(sdk_drum_values(track, &model, raw) && model == x0x::closed_hat);
    track = 0;
    require(sdk_drum_values(track, &model, raw) && model == x0x::snare && raw[1] == 71);
    // Stock's copy keeps packed controls and choke; the destination owns its key.
    std::memcpy(regions[0][23], regions[0][0], sizeof(regions[0][0]));
    reinterpret_cast<u8*>(regions[0][23])[26] = 76;
    key = 76;
    require(sdk_drum_values(track, &model, raw) && model == x0x::snare && raw[1] == 71);
    require(sdk_drum_choke_choice(track) == 2);
    // Each model/control corner through stock's four Q15 preset words. The
    // conversion is precisely the stock writer's >>16 / reader's <<16 path.
    for (u32 m = 0; m < x0x::model_count; ++m) for (u32 v = 0; v < 128; ++v) {
        u8 expected[8], loaded[8] = {}; i32 packed[4], restored[4];
        for (u32 s = 0; s < 8; ++s) expected[s] = u8((v + 17 * s) % 128);
        require(drum_pack(m, expected, packed, codec));
        for (u32 w = 0; w < 4; ++w) {
            const u32 stored = u32(packed[w]) >> 16;
            require(stored <= 32767);
            restored[w] = i32(stored << 16);
        }
        u8 decoded = 255;
        require(drum_unpack(restored, &decoded, loaded, codec) && decoded == m);
        for (u32 s = 0; s < x0x::parameter_count(m); ++s) require(loaded[s] == expected[s]);
    }
    // Save/reload all real edited pad records; clearing GUI page state cannot
    // change any saved model, value or choke group.
    i32 saved[8][24][32]; std::memcpy(saved, regions, sizeof(saved));
    std::memset(regions, 0, sizeof(regions));
    std::memcpy(regions, saved, sizeof(regions));
    track = 0; key = 53;
    require(sdk_drum_values(track, &model, raw) && model == x0x::snare && raw[1] == 71);
    require(sdk_drum_choke_choice(track) == 2);
    for (u32 t = 0; t < 8; ++t) for (u32 p = 0; p < 24; ++p) {
        require(regions[t][p][0] == 123456 && regions[t][p][5] == 987654);
        require(reinterpret_cast<u8*>(regions[t][p])[26] == 53 + p);
    }
    track = 7; key = 76; shown(0,1,2,3);
    for (u32 m = 0; m < x0x::model_count; ++m) for (u32 value = 0; value < 128; ++value) {
        u32 options[4], size = 0, position = 0;
        for (u32 f = 0; f < 4; ++f) if (x0x::family_masks[m] & (1u << f)) {
            if (x0x::family_index(m,value) == f) position = size;
            options[size++] = f;
        }
        for (i32 direction = -1; direction <= 1; direction += 2) {
            u8 initial[8] = {u8(value),64,48,64,0,64,64,0};
            require(drum_pack(m,initial,regions[7][23]+1,codec));
            reinterpret_cast<u8*>(regions[7][23])[24] = drum_mark(0);
            edit(0,direction);
            require(sdk_drum_values(track,&model,raw) && model == m);
            const i32 next = i32(position)+direction;
            const u32 expected = options[next < 0 ? 0 : next >= i32(size) ? size-1 : u32(next)];
            require(x0x::family_index(m,raw[0]) == expected);
            for (u32 slot=1;slot<x0x::parameter_count(m);++slot) require(raw[slot] == initial[slot]);
        }
    }
    require(regions[7][23][0] == 123456 && regions[7][23][5] == 987654);
    check_presets();
    std::puts("PASS: X0X + SDK editor: one-detent styles, per-pad type, both pages, eight tracks, choke, copy, lossless preset words and seven editable kits");
}
'''


def preset_checks(machine):
    # An independent JS projection supplies the browser's expected per-pad
    # values. The real SDK decoder/editor below receives the device JSON words.
    script = """import {kitPads} from './audition/models.mjs';
import {readFileSync} from 'node:fs';
const kits=JSON.parse(readFileSync('audition/presets.json','utf8'));
console.log(JSON.stringify(kits.map(kit=>({id:kit.id,pads:kitPads(kit)}))));"""
    browser = json.loads(subprocess.check_output(['node','--input-type=module','-e',script],cwd=machine,text=True))
    modes = {'gate':0,'oneshot':1,'mute1':2,'loop':3,'mute2':4,'mute3':5}
    cpp_array = lambda values: '{'+','.join(map(str,values))+'}'
    banks = []
    for kit in browser:
        preset = json.loads((machine/'presets'/(kit['id']+'.json')).read_text())
        if preset['engine']['params'] != [16384]*8 or len(preset['regions']) != 24:
            raise ValueError('A kit needs neutral track offsets and 24 stored pads')
        rows = []
        for i, (region, expected) in enumerate(zip(preset['regions'],kit['pads'])):
            if region['model'] != 'fm slap' or region['hikey'] != 53+i:
                raise ValueError('Invalid pad identity')
            rows.append('{'+','.join([cpp_array(region['params']),str(expected['model']),
                cpp_array(expected['raw']),str(7+16*modes[region['playmode']]),str(expected['choke'])])+'}')
        banks.append('{'+','.join(rows)+'}')
    declarations = '''struct PresetPad { opxy::u32 words[4], model; opxy::u8 raw[8]; opxy::u32 mark, choke; };
static const PresetPad saved_presets[][24] = {'''+','.join(banks)+'};\n'
    return declarations + r'''
void check_presets() {
    using namespace opxy;
    using namespace opxy::platform;
    track = 0;
    for (const auto& kit : saved_presets) {
        for (u32 p = 0; p < 24; ++p) {
            std::memset(regions[0][p],0,sizeof(regions[0][p]));
            for (u32 w = 0; w < 4; ++w) regions[0][p][w+1] = i32(kit[p].words[w] << 16);
            reinterpret_cast<u8*>(regions[0][p])[24] = u8(kit[p].mark);
            reinterpret_cast<u8*>(regions[0][p])[26] = u8(53+p);
        }
        for (u32 p = 0; p < 24; ++p) {
            key = 53+p;
            u8 model, values[8]; require(sdk_drum_values(track,&model,values));
            require(model == kit[p].model && sdk_drum_choke_choice(track) == kit[p].choke);
            for (u32 s = 0; s < x0x::parameter_count(model); ++s) require(values[s] == kit[p].raw[s]);
            // Loaded kit styles remain freely editable at one detent/choice.
            const u32 next = x0x::stepped_value(model,0,values[0],1);
            const u32 event[] = {1000,1,0}; sdk_drum_edit(key,0,event,0);
            require(sdk_drum_values(track,&model,values) && values[0] == next);
            for (u32 s = 1; s < x0x::parameter_count(model); ++s) require(values[s] == kit[p].raw[s]);
        }
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", required=True, type=Path)
    parser.add_argument("--cxx", default="c++")
    args = parser.parse_args()
    sdk = args.sdk.resolve()
    machine = Path(__file__).resolve().parents[1]
    platform = sdk / "sdk/platform/opxy-1.2.1"
    fixture = (sdk / "tests/fixtures/drum_gui.cpp").read_text().split("int main() {")[0]
    fixture = '#include "engine.cpp"\n' + fixture
    fixture = fixture.replace("i32 regions[24][32];", "i32 regions[8][24][32];")
    fixture = fixture.replace("return regions[note - 53];", "return regions[track][note - 53];")
    fixture = re.sub(r"void defaults\(u32 note, u8\* model, u8\* values\) \{.*?\n\}",
                     "void defaults(u32 note, u8* model, u8* values) { x0x::drum_defaults(note, model, values); }",
                     fixture, flags=re.S)
    fixture = re.sub(r"const char\* label\(u32 model, u32 slot\).*?\n", "const char* label(u32 model, u32 slot) { return x0x::parameter_name(model, slot); }\n", fixture)
    fixture = re.sub(r"const char\* name\(u32 model\).*?\n", "const char* name(u32 model) { return x0x::model_name(model); }\n", fixture)
    fixture = re.sub(r"u32 count\(u32 model\).*?\n", "u32 count(u32 model) { return x0x::parameter_count(model); }\n", fixture)
    fixture = fixture.replace("34), ready, 2, defaults", "34), ready, x0x::model_count, defaults")
    entry = '{reinterpret_cast<const char*>(34), ready, x0x::model_count, defaults, label, name, count}'
    if fixture.count(entry) != 1:
        raise ValueError('SDK GUI fixture changed; check the registered editor callback')
    fixture = fixture.replace(entry, entry[:-1] + ', 0, nullptr, nullptr, x0x::edit_parameter}')
    with tempfile.TemporaryDirectory(prefix="x0x-sdk-integration-") as temporary:
        scratch = Path(temporary)
        stock = (platform / "stock.hpp").read_text().replace(
            "typedef __UINTPTR_TYPE__ Address;", "typedef __UINTPTR_TYPE__ Address;\nAddress host_stock_address(Address);")
        for pointer in ("T*", "const volatile u32*", "const volatile unsigned char*"):
            stock = stock.replace(f"reinterpret_cast<{pointer}>(address)",
                                  f"reinterpret_cast<{pointer}>(host_stock_address(address))")
        (scratch / "stock.hpp").write_text(stock)
        (scratch / "drum_values.hpp").write_text((platform / "drum_values.hpp").read_text())
        sources = []
        for name in ("synth_controls.cpp", "drum_synthesis_stock.cpp", "drum_values_stock.cpp", "drum_machines.cpp"):
            target = scratch / name
            target.write_text((platform / name).read_text())
            sources.append(str(target))
        driver = scratch / "interaction.cpp"
        driver.write_text(fixture + preset_checks(machine) + DRIVER)
        binary = scratch / "interaction"
        subprocess.run([args.cxx, "-std=c++11", "-O2", "-Wall", "-Werror",
                        "-I" + str(scratch), "-I" + str(sdk / "sdk/include"),
                        "-I" + str(platform), "-I" + str(machine), str(driver), *sources,
                        "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    main()
