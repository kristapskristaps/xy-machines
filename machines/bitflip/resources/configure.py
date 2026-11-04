"""Write Bitflip's control contract and native factory presets."""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]
MODELS = ['Pulse', 'Tri', 'Twin', 'Wave', 'FM', 'Metal', 'Noise', 'Logic']
ARPS = ['Off', 'Oct', 'Major', 'Minor', 'Sus', 'Maj7', 'Min7', 'Power']
KEYS = ['model', 'color', 'edge', 'sweep', 'chord', 'speed', 'bits', 'clock']
PARAMETERS = [
    ('Model', 0, {'kind':'enum','labels':MODELS}, 'enum'),
    ('Color', 1, {'kind':'linear','minimum':0,'maximum':1}, 'percent'),
    ('Edge', 0, {'kind':'linear','minimum':0,'maximum':1}, 'percent'),
    ('Sweep', .5, {'kind':'linear','minimum':-24,'maximum':24}, 'semitones'),
    ('Chord', 0, {'kind':'enum','labels':ARPS}, 'enum'),
    ('Speed', .4, {'kind':'enum','labels':['8Hz','12Hz','16Hz','24Hz','32Hz','40Hz','64Hz','80Hz']}, 'enum'),
    ('Bits', 1, {'kind':'enum','labels':[str(i) for i in range(2,17)]}, 'enum'),
    ('Clock', 1, {'kind':'enum','labels':['1.4k','2.8k','5.5k','8.8k','11.0k','14.7k','22.1k','44.1k']}, 'enum'),
]

def model(i): return (i + .5) / 8

def chord(i): return (i + .5) / 8 if i else 0

PRESETS = {
    'Init Pulse': [0,1,0,.5,0,.4,1,1],
    'Pocket Hero': [0,1/3,.15,.5,0,.4,1,1],
    'Triangle Bass': [model(1),.1,.12,.5,0,.3,1,1],
    'Twin Blades': [model(2),.7,.44,.5,0,.35,1,1],
    'Cartridge Keys': [model(3),.38,.18,.5,0,.4,.88,1],
    'Crystal Bell': [model(4),.44,.32,.5,0,.4,1,1],
    'Metal Coin': [model(5),.18,.85,.64,0,.4,1,1],
    'Pixel Dust': [model(6),.75,0,.65,0,.5,.88,1],
    'Short Circuit': [model(6),.5,1,.5,0,.5,.75,.8],
    'Glitch Dungeon': [model(7),.75,.4,.5,0,.4,.60,.86],
    'Major Quest': [0,.45,.1,.5,chord(2),.60,1,1],
    'Minor Boss': [model(3),.8,.25,.5,chord(3),.72,.88,1],
    'Power Up': [0,.5,.3,0,chord(1),.70,1,1],
    'Laser Down': [model(7),.25,.20,1,0,.4,1,1],
    'Save Point': [model(4),.2,.2,.5,chord(5),.40,1,1],
    'Game Over': [model(2),.7,.3,.5,chord(6),.25,.8,.82],
}
params = []
for index, (label, default, mapping, display) in enumerate(PARAMETERS):
    item = dict(id=KEYS[index],label=label,default=default,mapping=mapping,display=display)
    if index >= 4: item['control'] = index - 4
    params.append(item)
definition = dict(schema=1,id='bitflip',name='Bitflip',press='page',parameters=params,
    presets={name:dict(zip(KEYS,values)) for name,values in PRESETS.items()},
    resources=dict(voiceBytes=92,voiceAlignment=4,voiceCapacity=8,sharedBytes=0,outputPeakQ31=268435456,
        display=dict(stateBytes=36,stateAlignment=4,paths=1,points=64,nodes=28,texts=16,palette=8)))
(ROOT/'definition.json').write_text(json.dumps(definition,indent=2)+'\n')
amp = dict(attack=128,decay=0,sustain=32767,release=13000)
machine = dict(schema=1,sdk='1.8',kind='synth',
    description='Eight chip voices for pulse leads, triangle bass, game arpeggios, FM bells, metallic hits and digital noise.',
    icon='M8 4h32v36H8z M14 10h20v12H14z M14 30h4v6h-4z M24 30h4v6h-4z',
    presetDefaults={'amp':amp})
(ROOT/'machine.json').write_text(json.dumps(machine,indent=2)+'\n')
folder = ROOT/'presets'; folder.mkdir(exist_ok=True)
for name,values in PRESETS.items():
    envelope = amp.copy()
    if name in ['Metal Coin','Pixel Dust','Laser Down','Power Up']:
        envelope = dict(attack=0,decay=15000,sustain=0,release=12000)
    elif name in ['Crystal Bell','Save Point']:
        envelope = dict(attack=128,decay=23000,sustain=10000,release=21000)
    preset = dict(engine=dict(bendrange=8191,highpass=0,
        modulation={source:dict(amount=16384,target=0) for source in ['aftertouch','modwheel','pitchbend','velocity']},
        params=[round(value*32767) for value in values],playmode='poly',
        **{'portamento.amount':0,'portamento.type':0,'transpose':0,'tuning.root':0,'tuning.scale':0,
           'velocity.sensitivity':0,'volume':24576,'width':0}),
        envelope={'amp':envelope,'filter':dict(attack=0,decay=0,release=0,sustain=32767)},
        fx=dict(active=False,params=[0,0,0,0,0,32767,0,0],type='svf'),
        lfo=dict(active=False,params=[0]*9,type='value'),octave=0,platform='OP-XY',regions=[],type='bitflip',version=4)
    filename = 'default' if name == 'Init Pulse' else name.lower().replace(' ','-')
    (folder/(filename+'.json')).write_text(json.dumps(preset,indent=2)+'\n')
