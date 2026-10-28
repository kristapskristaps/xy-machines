"""Write Westfold's control metadata and vector panel."""
from pathlib import Path
import json
import math
import re
ROOT=Path(__file__).resolve().parents[1]
keys=['overtone','fold','balance','decay','fm_ratio','fm_depth','slope_rate','slope_amount']
params=[]
for key,label,default,mapping,display in [
 ('overtone','Overtone',.24,{'kind':'linear','minimum':0,'maximum':1},'percent'),
 ('fold','Multiply',.44,{'kind':'linear','minimum':0,'maximum':1},'percent'),
 ('balance','Balance',.68,{'kind':'linear','minimum':0,'maximum':1},'percent'),
 ('decay','Decay',.62,{'kind':'exponential','minimum':45,'maximum':8000},'duration'),
 ('fm_ratio','FM Ratio',1/7,{'kind':'anchors','values':[.5,1,1.5,2,3,4,6,8]},'ratio'),
 ('fm_depth','FM Depth',.05,{'kind':'linear','minimum':0,'maximum':1},'percent'),
 ('slope_rate','Slope Hz',.32,{'kind':'exponential','minimum':.05,'maximum':400},'ratio'),
 ('slope_amount','Motion',.12,{'kind':'linear','minimum':0,'maximum':1},'percent')]:
 parameter=dict(id=key,label=label,default=default,mapping=mapping,display=display)
 if len(params)>=4:parameter['control']=len(params)-4
 params.append(parameter)
presets={
 'Initial Coast':[.24,.44,.68,.62,1/7,.05,.32,.12],
 'Buchla Wood':[.12,.58,.74,.40,3/7,.12,.43,.08],
 'Rubber Bass':[.18,.36,.52,.56,0,.16,.25,.10],
 'Copper Bells':[.55,.42,.68,.78,4/7,.56,.34,.04],
 'Broken Marimba':[.42,.75,.76,.36,2/7,.27,.54,.20],
 'Velvet Fold':[.08,.23,.40,.86,1/7,.05,.14,.18],
 'Coastline Keys':[.38,.47,.65,.70,2/7,.19,.29,.12],
 'Glass Current':[.68,.66,.71,.81,5/7,.43,.38,.17],
 'Wire Choir':[.30,.81,.66,.90,4/7,.33,.21,.34],
 'Slow Voltage':[.28,.60,.75,1,1/7,.12,.13,.72],
 'Circuit Birds':[.65,.50,.80,.67,6/7,.64,.90,.64],
 'Open Drone':[.16,.69,.65,1,3/7,.24,.32,.46],
}
definition=dict(schema=1,id='westfold',name='Westfold',parameters=params,
 presets={name:dict(zip(keys,values)) for name,values in presets.items()},
 resources=dict(voiceBytes=72,voiceAlignment=4,voiceCapacity=8,sharedBytes=0,outputPeakQ31=268435456,
 display=dict(stateBytes=44,stateAlignment=4,paths=4,points=64,nodes=61,texts=0,palette=16)))
(ROOT/'definition.json').write_text(json.dumps(definition,indent=2)+'\n')
machine=json.loads((ROOT/'machine.json').read_text())
machine['description']='A West Coast voice: animated harmonic folding, odd/even overtones, FM, cycling slope and a plucked low-pass gate.'
machine['icon']='M3 24h7l5-15 9 30 8-30 7 15h5'
machine['presetDefaults']={'amp':{'attack':0,'decay':0,'sustain':32767,'release':25000}}
(ROOT/'machine.json').write_text(json.dumps(machine,indent=2)+'\n')
native_presets=ROOT/'presets'
native_presets.mkdir(exist_ok=True)
for name,values in presets.items():
 preset={
  'engine':{'bendrange':8191,'highpass':0,
   'modulation':{source:{'amount':16384,'target':0} for source in ['aftertouch','modwheel','pitchbend','velocity']},
   'params':[round(value*32767) for value in values],'playmode':'poly',
   'portamento.amount':0,'portamento.type':0,'transpose':0,'tuning.root':0,'tuning.scale':0,
   'velocity.sensitivity':0,'volume':16384,'width':0},
  'envelope':{'amp':machine['presetDefaults']['amp'],
              'filter':{'attack':0,'decay':0,'release':0,'sustain':32767}},
  'fx':{'active':False,'params':[0,0,0,0,0,32767,0,0],'type':'svf'},
  'lfo':{'active':False,'params':[0]*9,'type':'value'},
  'octave':0,'platform':'OP-XY','regions':[],'type':'westfold','version':4}
 filename='default' if name=='Initial Coast' else name.lower().replace(' ','-')
 (native_presets/(filename+'.json')).write_text(json.dumps(preset,indent=2)+'\n')
colors=['#000000','#141719','#242525','#48331f','#4a4c48','#6a6c63','#174744','#56877f',
 '#a2b4a7','#a38751','#c4a365','#e2c588','#f6e9c5','#7b6141','#323c39','#ece9df']
parts=['<g id="westfold_panel">']
def path(d,fill='none',stroke='#4a4c48',width=1,identity=None,transform=None):
 attrs={'d':d,'fill':fill,'stroke':stroke,'stroke-width':str(width)}
 if identity:attrs['id']=identity
 if transform:attrs['transform']=transform
 parts.append('<path '+' '.join(f'{k}="{v}"' for k,v in attrs.items())+'/>')
glyphs={
 'A':'M0 7 L2.5 0 L5 7 M1 4.5 L4 4.5',
 'B':'M0 7 L0 0 L3.5 0 L5 1.5 L3.5 3.5 L0 3.5 M3.5 3.5 L5 5 L5 6 L3.5 7 L0 7',
 'C':'M5 1 L4 0 L1 0 L0 1 L0 6 L1 7 L4 7 L5 6',
 'D':'M0 7 L0 0 L3 0 L5 2 L5 5 L3 7 L0 7',
 'E':'M5 0 L0 0 L0 7 L5 7 M0 3.5 L4 3.5',
 'F':'M5 0 L0 0 L0 7 M0 3.5 L4 3.5',
 'G':'M5 1 L4 0 L1 0 L0 1 L0 6 L1 7 L5 7 L5 4 L3 4',
 'H':'M0 0 L0 7 M5 0 L5 7 M0 3.5 L5 3.5',
 'I':'M0 0 L4 0 M2 0 L2 7 M0 7 L4 7',
 'J':'M5 0 L5 6 L4 7 L1 7 L0 6',
 'K':'M0 0 L0 7 M5 0 L0 4 L5 7',
 'L':'M0 0 L0 7 L5 7',
 'M':'M0 7 L0 0 L2.5 3.5 L5 0 L5 7',
 'N':'M0 7 L0 0 L5 7 L5 0',
 'O':'M1 0 L4 0 L5 1 L5 6 L4 7 L1 7 L0 6 L0 1 Z',
 'P':'M0 7 L0 0 L4 0 L5 1 L5 2.5 L4 3.5 L0 3.5',
 'Q':'M1 0 L4 0 L5 1 L5 6 L4 7 L1 7 L0 6 L0 1 Z M3 5 L5.5 7.5',
 'R':'M0 7 L0 0 L4 0 L5 1 L5 2.5 L4 3.5 L0 3.5 M2.5 3.5 L5 7',
 'S':'M5 1 L4 0 L1 0 L0 1 L0 2.5 L1 3.5 L4 3.5 L5 4.5 L5 6 L4 7 L1 7 L0 6',
 'T':'M0 0 L5 0 M2.5 0 L2.5 7',
 'U':'M0 0 L0 6 L1 7 L4 7 L5 6 L5 0',
 'V':'M0 0 L2.5 7 L5 0',
 'W':'M0 0 L1 7 L2.5 3.5 L4 7 L5 0',
 'X':'M0 0 L5 7 M5 0 L0 7',
 'Y':'M0 0 L2.5 3.5 L5 0 M2.5 3.5 L2.5 7',
 'Z':'M0 0 L5 0 L0 7 L5 7',
 '0':'M1 0 L4 0 L5 1 L5 6 L4 7 L1 7 L0 6 L0 1 Z',
 '1':'M1 1 L2.5 0 L2.5 7 M1 7 L4 7',
 '2':'M0 1 L1 0 L4 0 L5 1 L5 2.5 L0 7 L5 7',
 '3':'M0 0 L5 0 L2 3 L4 3 L5 4 L5 6 L4 7 L0 7',
 '6':'M5 0 L1 0 L0 1 L0 6 L1 7 L4 7 L5 6 L5 4 L4 3 L0 3',
 '>':'M0 1 L4 3.5 L0 6',
}
def text(txt,x,y,size=7,color='#6a6c63',identity=None):
 # Single-stroke vector lettering keeps the native and browser artwork identical.
 commands=[]
 for index,char in enumerate(txt):
  if char==' ':continue
  tokens=re.findall(r'[MLZ]|-?\d+(?:\.\d+)?',glyphs[char]);i=0
  while i<len(tokens):
   command=tokens[i];i+=1
   if command=='Z':commands.append('Z');continue
   a=float(tokens[i]);b=float(tokens[i+1]);i+=2
   commands.append(f'{command}{x+(index*6.5+a)*size/7:.4f} {y+(b-7)*size/7:.4f}')
 path(' '.join(commands),stroke=color,width=round(max(.48,size/13),4),identity=identity)
text('WESTFOLD',22,45,14,'#ece9df')
text('HARMONIC VOICE',23,57,7,'#a38751')
text('ODD',381,43,7,'#a38751')
text('EVEN',432,43,7,'#56877f')
path('M383 54 L445 54',stroke='#323c39',width=.7)
text('SLOPE > FOLD',32,190,7,'#a2b4a7')
text('PHASE MOD',32,203,7,'#a2b4a7')
text('CONTOUR',383,205,7,'#a38751')
path('M22 67 L458 67',stroke='#242525',width=.7)
path('M22 157 L458 157',stroke='#242525',width=.7)
path('M22 213 L458 213',stroke='#242525',width=.7)
for x in [132,240,348]:
 path(f'M{x} 75 L{x} 79 M{x} 139 L{x} 143',stroke='#4a4c48',width=.7)
path('M123 109 L357 109',stroke='#242525',width=.6)
path('M84 109 L118 109 M362 109 L389 109',stroke='#4a4c48',width=.7)
path('M113 106 L117 109 L113 112 M384 106 L388 109 L384 112',stroke='#a38751',width=.7)
text('CORE',49,144,7,'#a2b4a7')
text('WAVE MULTIPLIER',201,150,6.5,'#a38751')
text('DYNAMICS',393,144,7,'#a2b4a7')
text('01',22,84,6,'#6a6c63')
text('02',131,84,6,'#6a6c63')
text('03',445,84,6,'#6a6c63')
text('PARTIALS',138,205,6,'#6a6c63')
text('1',143,171,5.5,'#6a6c63')
text('16',229,171,5.5,'#6a6c63')
text('32',324,171,5.5,'#6a6c63')
for x in [145,187,229,277,331]:path(f'M{x} 196 L{x} 198',stroke='#4a4c48',width=.6)
for i,width in enumerate([2.4,.65,.9,.9]):
 coords=[]
 for p in range(64):
  angle=2*math.pi*p/64
  coords.append(f'{30*math.cos(angle):.4f} {20*math.sin(angle):.4f}')
 path('M'+coords[0]+' '+' '.join('L'+x for x in coords[1:])+' Z',stroke=colors[10],width=width,identity=f'westfold_node_{i}')
for i in range(4,36):path('M0 0 L0 -14',stroke=colors[10],width=.85,identity=f'westfold_node_{i}')
parts.append('<circle id="westfold_node_36" cx="0" cy="0" r="22" transform="matrix(1 0 0 1 63 109)" fill="none" stroke="#56877f" stroke-width="0.8"/>')
path('M47 117 L55 101 L63 117 L71 101 L79 117',stroke='#a2b4a7',width=.9)
for i in range(37,49):path('M13 -2 L22 -7 L26 -2',stroke=colors[9],width=.9,identity=f'westfold_node_{i}')
for i in range(49,61):
 parts.append(f'<circle id="westfold_node_{i}" cx="0" cy="0" r="{1.1 if i<57 else 5}" fill="{colors[10]}" stroke="{colors[10]}" stroke-width="0.6"/>')
for i,color in enumerate(colors):
 parts.append(f'<circle id="westfold_palette_{i}" cx="640" cy="0" r="1" fill="{color}" stroke="{color}" stroke-width="1"/>')
parts.append('</g>')
(ROOT/'panel.svg.fragment').write_text('\n'.join(parts)+'\n')
