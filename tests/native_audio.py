"""Exercise real X0X audio through the SDK's generated native drum adapter.

The pinned SDK fixture supplies stock-boundary doubles. This checks callback,
payload and amplitude routing; it does not measure hardware timing.
"""
import argparse
import copy
import importlib.util
import json
from pathlib import Path
import sys


DRIVER = r'''
int main() {
    using namespace opxy;
    using namespace opxy::platform;
    for (unsigned i = 0; i < block_frames; ++i) {
        fixture_pitch[i] = 123;
        fixture_amplitude[i] = 0x7fffffff;
    }
    for (unsigned s = 0; s < 8; ++s) offsets[s] = 0x40000000;
    CHECK(objects[0] = probe_factory(0, 0));
    const DrumCodec codec = drum_codec(x0x::model_count, x0x::parameter_count);
    unsigned cases = 0;
    for (unsigned model = 0; model < x0x::model_count; ++model) {
        for (unsigned family = 0; family < 4; ++family) {
            if (!(x0x::family_masks[model] & (1u << family))) continue;
            u8 raw[8] = {x0x::family_raw(model, family),64,48,64,0,64,64,0};
            i32 payload[5] = {kMarked};
            CHECK(drum_pack(model, raw, payload + 1, codec));
            fixture_region = payload;
            note(0, 53);
            x0x::Voice direct; x0x::Shared state;
            x0x::initialize(direct, state, 1); x0x::start(direct, state, 1);
            x0x::drum_select(direct, state, 1, model);
            i32 parameters[8];
            for (unsigned s = 0; s < 8; ++s)
                parameters[s] = raw[s] == 127 ? 0x7fffffff : i32(u32(raw[s]) << 24);
            bool heard = false;
            for (unsigned block = 0; block < 100; ++block) {
                i32 actual[block_frames], expected[block_frames];
                for (auto& sample : actual) sample = 123456;
                reinterpret_cast<Render>(const_cast<void*>(table(0)[3]))(objects[0], actual, payload, offsets);
                x0x::render(direct, state, 1, expected, parameters, fixture_pitch);
                for (unsigned i = 0; i < block_frames; ++i) {
                    CHECK(actual[i] == opxy::q31::multiply(expected[i], fixture_amplitude[i]));
                    heard = heard || actual[i] != 0;
                }
            }
            CHECK(heard);
            end(0);
            ++cases;
        }
    }
    CHECK(cases == 47);
    std::puts("PASS: all 47 X0X voices are audible through the real native drum adapter, with exact payload and gain routing");
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--cxx', default='c++')
    args = parser.parse_args()
    sdk = args.sdk.resolve()
    root = Path(__file__).resolve().parents[1]
    sys.path[:0] = [str(sdk), str(sdk / 'tests')]
    spec = importlib.util.spec_from_file_location('x0x_sdk_voice_fixture', sdk / 'tests/test_drum_voices.py')
    fixture = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(fixture)
    from tooling import adapters
    definition = json.loads((root / 'definition.json').read_text())
    metadata = json.loads((root / 'machine.json').read_text())
    generate = adapters.generate_voice

    def actual_voice(folder, machine):
        machine.definition = copy.deepcopy(definition)
        return generate(folder, machine)

    fixture.generate_voice = actual_voice
    fixture.HOST_CXX = args.cxx
    # Use the fixture's established pitch/amplitude substitutions, with the
    # machine's actual state and callbacks in place of its four-byte probe.
    fixture.CORE = fixture.CORE.split('namespace probe {')[0] + \
        '#include ' + json.dumps(str(root / 'core.h')) + '\nnamespace probe = x0x;\n'
    engine = '#include ' + json.dumps(str(root / 'engine.cpp')) + '\n'
    engine += ''.join('#include ' + json.dumps(str(root / source)) + '\n' for source in metadata['sources'])
    driver = fixture.DRIVER.split('int main() {')[0] + DRIVER
    fixture.DrumVoices().run_adapter(engine, driver, definition['resources']['voiceCapacity'],
                                    chokeSameKey=definition['resources'].get('chokeSameKey', 0))
    print('PASS: all 47 X0X voices are audible through the generated native adapter; payload and gain match the DSP')


if __name__ == '__main__':
    main()
