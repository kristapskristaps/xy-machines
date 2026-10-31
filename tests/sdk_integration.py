"""Run X0X through the pinned SDK's real pad editor, paging and storage codec.

Reuses the SDK fixture's stock boundary doubles; no stock addresses or native
hooks belong to the machine. This is host evidence, not a hardware observation.
"""
import argparse
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
    edit(0, 32); // 808 -> 909, independent of type
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
    edit(0, -32); // Default 808 clap -> 707 clap.
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
    std::puts("PASS: X0X + SDK editor: per-pad type/style, both pages, eight tracks, choke, copy and lossless preset words");
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
        driver.write_text(fixture + DRIVER)
        binary = scratch / "interaction"
        subprocess.run([args.cxx, "-std=c++11", "-O2", "-Wall", "-Werror",
                        "-I" + str(scratch), "-I" + str(sdk / "sdk/include"),
                        "-I" + str(platform), "-I" + str(machine), str(driver), *sources,
                        "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    main()
