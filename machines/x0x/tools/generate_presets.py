"""Author matching X0X browser kits and editable, sample-free OP-XY presets."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
# Kit shaping relative to each default pad. A uniform requested family
# leaves instruments unavailable in that family on an available source.
KITS = [
    ('default', '808 Classic', [0,0,0,0,0,0,0,0]),
    ('606-dry', '606 Dry', [-64,0,-14,8,0,0,-10,0]),
    ('707-studio', '707 Studio', [-32,0,0,0,0,0,0,0]),
    ('909-club', '909 Club', [32,0,8,6,0,8,0,12]),
    ('night-wire', 'Night Wire', [-64,-8,-20,-12,0,24,-10,24]),
    ('warehouse', 'Warehouse', [32,-10,14,-10,0,16,16,24]),
    ('soft-machines', 'Soft Machines', [0,-5,22,-16,10,-12,14,0]),
]


def preset(regions):
    return {
        'engine': {
            'bendrange':8191, 'highpass':0,
            'modulation': {source:{'amount':16384,'target':0}
                           for source in ('aftertouch','modwheel','pitchbend','velocity')},
            'params':[16384]*8, 'playmode':'poly',
            'portamento.amount':0, 'portamento.type':0, 'transpose':0,
            'tuning.root':0, 'tuning.scale':0, 'velocity.sensitivity':0,
            'volume':16384, 'width':0,
        },
        'envelope': {
            'amp':{'attack':0,'decay':0,'release':32767,'sustain':32767},
            'filter':{'attack':0,'decay':0,'release':0,'sustain':32767},
        },
        'fx':{'active':False,'params':[0,0,0,0,0,32767,0,0],'type':'svf'},
        'lfo':{'active':False,'params':[0,0,0,0,0,0,0,0],'type':'value'},
        'octave':0, 'platform':'OP-XY', 'regions':regions, 'type':'x0x', 'version':4,
    }


def generate(sdk, cxx):
    bank = []
    folder = ROOT / 'presets'
    folder.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='x0x-presets-') as temporary:
        binary = Path(temporary) / 'preset-export'
        subprocess.run([cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),
                        '-I'+str(sdk/'sdk/platform/opxy-1.2.1'),
                        str(ROOT/'tools/preset_export.cpp'),'-o',str(binary)],check=True)
        for identity, name, offsets in KITS:
            if len(offsets) != 8 or any(type(v) is not int or not -64 <= v <= 63 for v in offsets):
                raise ValueError('Invalid kit offsets')
            result = subprocess.run([str(binary),*map(str,offsets)],check=True,capture_output=True,text=True)
            regions = json.loads(result.stdout)
            data = json.dumps(preset(regions),indent=2)+'\n'
            if len(data.encode()) > 16384:
                raise ValueError('Preset exceeds the SDK file limit')
            (folder/(identity+'.json')).write_text(data)
            bank.append({'id':identity,'name':name,'offsets':offsets})
    (ROOT / 'audition/presets.json').write_text(json.dumps(bank,indent=2)+'\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk',type=Path,default=ROOT.parent/'xy-sdk-gui-size')
    parser.add_argument('--cxx',default='c++')
    args = parser.parse_args()
    generate(args.sdk.resolve(),args.cxx)
