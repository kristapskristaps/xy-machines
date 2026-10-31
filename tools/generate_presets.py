"""Author the X0X browser audition kit bank."""
import json
from pathlib import Path

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


def generate():
    bank = []
    for identity, name, offsets in KITS:
        if len(offsets) != 8 or any(type(v) is not int or not -64 <= v <= 63 for v in offsets):
            raise ValueError('Invalid kit offsets')
        bank.append({'id':identity,'name':name,'offsets':offsets})
    (ROOT / 'audition/presets.json').write_text(json.dumps(bank,indent=2)+'\n')


if __name__ == '__main__':
    generate()
