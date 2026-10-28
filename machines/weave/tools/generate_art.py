"""Generate Weave's retained, native-compatible miniature machine room."""
import math
from pathlib import Path
import json

root = Path(__file__).resolve().parents[1]
names = ['belt','bed','flywheel','pulley','piston','gauge','valve','lamp','lamp_glow','vent','conveyor',
         'cargo_0','cargo_1','cargo_2','steam_0','steam_1','steam_2','indicator','caretaker','visor','wrench']
models = ['shaper','fm','noise','dust','kick','snare','hihat','grain','chord','swarm','wavetable','analog','modal','string','sixop']
names += ['model_' + name for name in models]
names += ['operator_' + str(i) for i in range(6)]
names += ['bar_' + str(i) for i in range(8)]
names += ['label_' + name for name in models]
ids = {name:index for index,name in enumerate(names)}
lines = ['<g id="weave_panel">']
def path(d, fill='none', stroke='#63666b', width=.7, name=None, xy=None):
    attrs = f' id="weave_node_{ids[name]}"' if name else ''
    if xy: attrs += f' transform="matrix(1 0 0 1 {xy[0]} {xy[1]})"'
    lines.append(f'<path{attrs} d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}" stroke-linejoin="round" stroke-linecap="round"/>')
def poly(points, fill, stroke='#63666b', width=.7):
    path(' '.join(('M' if i==0 else 'L')+f'{x} {y}' for i,(x,y) in enumerate(points))+' Z',fill,stroke,width)
def circle(x,y,r,fill='none',stroke='#63666b',width=.7,name=None):
    attrs = f' id="weave_node_{ids[name]}" transform="matrix(1 0 0 1 {x} {y})"' if name else ''
    lines.append(f'<circle{attrs} cx="{0 if name else x}" cy="{0 if name else y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"/>')
def ring(x,y,rx,ry=None,n=20):
    if ry is None: ry=rx
    return ' '.join(('M' if i==0 else 'L')+f'{x+rx*math.cos(2*math.pi*i/n):.4f} {y+ry*math.sin(2*math.pi*i/n):.4f}' for i in range(n))+' Z'
def text(x,y,label,size=8,fill='#b6b9bd'):
    lines.append(f'<text transform="matrix(1 0 0 1 {x} {y})" fill="{fill}" font-family="UniversNextPro-Light" font-size="{size}px">{label}</text>')
# An open-front room, with real wall/floor edges rather than a viewport frame.
poly([(34,150),(242,84),(446,140),(238,211)],'#101113','#52555a')
poly([(34,94),(242,30),(242,84),(34,150)],'#17191c','#63666b')
poly([(242,30),(446,84),(446,140),(242,84)],'#202226','#777a7f')
path('M72 162 L274 98 M120 177 L325 112 M175 193 L382 126 M85 134 L290 192 M139 117 L346 174 M190 100 L401 155',stroke='#24272b',width=.5)
# Wall conduits, junctions and a small service door.
path('M54 114 L54 92 L124 70 L124 92 L174 76 M68 122 L68 105 L109 93 M263 50 L263 65 L333 84 L333 109 L389 125 L389 145',stroke='#888b8e',width=2)
path('M58 114 L58 96 L120 77 M273 51 L273 61 L339 79',stroke='#34363a',width=.8)
poly([(62,112),(92,103),(92,134),(62,144)],'#131517','#73767a')
path('M66 117 L88 110 M66 122 L88 115 M66 127 L88 120 M66 132 L88 125',stroke='#34363a')
circle(86,126,1,fill='#b6b9bd',stroke='#b6b9bd',width=.3)
poly([(400,80),(425,87),(425,113),(400,106)],'#0d0e10','#888b8e')
circle(412,98,9,stroke='#34363a',width=1)
circle(346,83,12,fill='#0e1012',stroke='#b6b9bd',width=1)
path('M337 86 L339 85 M337 78 L339 79 M345 73 L345 76 M353 77 L351 79 M355 86 L352 85',stroke='#888b8e',width=.6)
text(330,106,'PRESS',6,'#888b8e')
circle(389,125,9,fill='#111315',stroke='#888b8e',width=1)
# Hanging light and its pool are behind the machinery.
path('M167 53 L167 43 L170 42 M167 53 L173 61',stroke='#b6b9bd')
path('M-6 0 L6 0 L31 91 L-31 91 Z','#17191c','#17191c',.2,'lamp_glow',(173,66))
# Floor mounting plates, drive motor and output belt.
poly([(96,155),(134,143),(162,153),(124,168)],'#1b1d21','#52555a')
poly([(112,158),(129,153),(145,158),(128,164)],'#34363a','#888b8e')
poly([(308,162),(392,138),(411,148),(326,174)],'#34363a','#b6b9bd')
poly([(326,174),(411,148),(411,157),(326,183)],'#17191c','#888b8e')
poly([(308,162),(326,174),(326,183),(308,171)],'#25272b','#888b8e')
path('M331 173 L404 151 M332 179 L404 157',stroke='#63666b')
path('M335 181 L335 190 M400 161 L400 170',stroke='#b6b9bd',width=1.2)
# Mutable drive belt: 32 distinct vertices around the two pulley wheels.
points=[]
for i in range(16):
    a=math.radians(62+i*206/15);points.append((-24+24*math.cos(a),6+24*math.sin(a)))
for i in range(16):
    a=math.radians(268+i*154/15);points.append((36+10*math.cos(a),-10+10*math.sin(a)))
d=' '.join(('M' if i==0 else 'L')+f'{x:.4f} {y:.4f}' for i,(x,y) in enumerate(points))+' Z'
path(d,stroke='#f0443a',width=1,name='belt',xy=(146,131))
circle(122,137,22,fill='#111315',stroke='#b6b9bd',width=1)
circle(122,137,19,stroke='#63666b',width=.7)
path('M-18 0 L18 0 M0 -18 L0 18 M-13 -13 L13 13 M-13 13 L13 -13',stroke='#b6b9bd',name='flywheel',xy=(122,137))
circle(122,137,4,fill='#34363a',stroke='#f0443a',width=1)
circle(182,121,8,fill='#111315',stroke='#b6b9bd',width=1)
path('M-6 0 L6 0 M0 -6 L0 6',stroke='#f0443a',name='pulley',xy=(182,121))
# Main machine: readable front face with a top and right side for depth.
path('M302 144 L321 151 L321 161',stroke='#888b8e',width=4)
poly([(190,86),(208,76),(304,76),(286,86)],'#34363a','#b6b9bd')
poly([(286,86),(304,76),(304,161),(286,171)],'#17191c','#b6b9bd')
poly([(190,86),(286,86),(286,171),(190,171)],'#25272b','#b6b9bd',1)
poly([(202,103),(274,103),(274,149),(202,149)],'#070809','#888b8e')
poly([(204,153),(272,153),(272,166),(204,166)],'#17191c','#52555a',.5)
path('M292 99 L298 96 M292 105 L298 102 M292 111 L298 108 M292 117 L298 114 M292 123 L298 120 M292 129 L298 126',stroke='#63666b')
for x,y in [(195,91),(281,91),(195,166),(281,166)]:circle(x,y,.9,fill='#888b8e',stroke='#888b8e',width=.3)
text(203,98,'WEAVE',7.5,'#e7e8e5')
# A chimney, ceiling press and some workshop details.
poly([(264,76),(264,58),(272,55),(272,73)],'#34363a','#b6b9bd')
path('M263 58 L270 61 L274 58 L267 55 Z',fill='#111315',stroke='#b6b9bd')
path('M208 77 L208 58 L245 58 L245 64',stroke='#b6b9bd',width=1.5)
path('M-3 -11 L3 -11 L3 0 L9 0 L9 5 L-9 5 L-9 0 L-3 0 Z',fill='#f0443a',stroke='#f0443a',width=.6,name='piston',xy=(245,70))
path('M-8 0 L-4 -6 L4 -6 L8 0 Z',fill='#b6b9bd',stroke='#e7e8e5',name='lamp',xy=(173,66))
path('M-1 1 L8 -6',stroke='#f0443a',width=1.1,name='gauge',xy=(346,83))
path('M-7 0 L7 0 M0 -7 L0 7 M-5 -5 L5 5 M-5 5 L5 -5',stroke='#f0443a',width=1,name='valve',xy=(389,125))
path('M0 0 L-3 -7 L2 -7 L4 -3 Z M0 0 L7 0 L5 5 L1 5 Z M0 0 L-5 5 L-7 1 L-3 -2 Z',fill='#888b8e',stroke='#b6b9bd',width=.5,name='vent',xy=(412,98))
# The cartridge changes with the synth model. Every glyph is one retained path.
glyphs={
 'shaper':'M-21 13 L21 13 M-15 9 L15 9 M-10 -14 L10 -14 L10 -8 L-10 -8 Z M-3 -8 L3 -8 L3 5 L-3 5 Z M-12 5 L12 5 L12 9 L-12 9 Z',
 'fm':ring(-12,0,10)+ring(12,0,10)+' M-2 0 L2 0',
 'noise':'M-22 -13 L22 -13 L12 5 L5 5 L5 14 L-5 14 L-5 5 L-12 5 Z M-10 -8 L-8 -8 M0 -5 L2 -5 M10 -8 L12 -8 M-4 2 L-2 2',
 'dust':'M-22 -13 L22 -13 L13 2 L5 2 L5 14 L-5 14 L-5 2 L-13 2 Z M-12 -8 L-10 -8 M-3 -10 L-1 -10 M7 -7 L9 -7 M-3 6 L-1 6 M5 10 L7 10 M-9 12 L-7 12',
 'kick':ring(0,6,20,6)+' M-20 6 L-20 13 L20 13 L20 6 M-11 -12 L-7 -14 L3 -1 L-1 2 Z',
 'snare':ring(0,4,20,6)+' M-20 4 L-20 13 L20 13 L20 4 M-16 7 L-10 13 L-4 7 L2 13 L8 7 L14 13 M-13 -13 L-10 -14 L1 -2 L-2 0 Z',
 'hihat':ring(0,-3,21,5)+ring(0,5,21,5)+' M0 -14 L0 13 M-6 14 L6 14',
 'grain':'M-13 -15 L13 -15 L13 14 L-13 14 Z M-8 -10 L8 -6 L-8 -2 L8 2 L-8 6 L8 10 M0 -15 L0 14',
 'chord':' '.join(f'M{x} 14 L{x} {top} L{x+5} {top} L{x+5} 14 Z M{x+1} {top+3} L{x+4} {top+3}' for x,top in [(-23,-6),(-14,-13),(-5,-16),(4,-11),(13,-4)]),
 'swarm':ring(0,0,17)+' M0 0 L-5 -14 L5 -14 Z M0 0 L14 -5 L14 5 Z M0 0 L5 14 L-5 14 Z M0 0 L-14 5 L-14 -5 Z',
 'wavetable':'M-23 -7 L12 -14 L23 -8 L-12 0 Z M-23 -1 L-12 6 L23 -2 M-23 5 L-12 12 L23 4 M-23 10 L-12 17 L23 9 M-12 0 L-12 17',
 'analog':' '.join(f'M{x} 11 L{x} -7 L{x+2} -11 L{x+7} -11 L{x+9} -7 L{x+9} 11 Z M{x+2} 4 L{x+7} 4 M{x+4} 4 L{x+4} -4' for x in [-21,-5,11])+' M-24 13 L24 13',
 'modal':'M-24 -14 L24 -14 '+ ' '.join(f'M{x} -14 L{x} -8 M{x-3} -8 L{x+3} -8 L{x+3} {bottom} L{x-3} {bottom} Z' for x,bottom in [(-18,9),(-9,13),(0,15),(9,11),(18,6)]),
 'string':'M-24 13 L-17 -13 L21 -13 L25 13 Z M-17 -10 L-12 11 M-9 -10 L-5 11 M-1 -10 L2 11 M7 -10 L9 11 M15 -10 L16 11',
 'sixop':' '.join(ring(x,y,6) for y in [-8,8] for x in [-18,0,18])+' M-12 -8 L-6 -8 M6 -8 L12 -8 M-12 8 L-6 8 M6 8 L12 8 M0 -2 L0 2'
}
for model in models:path(glyphs[model],stroke='#b6b9bd',width=.75,name='model_'+model,xy=(640,320))
for i in range(6):path('M-4 0 L4 0 M0 -4 L0 4 M-3 -3 L3 3 M-3 3 L3 -3',stroke='#f0443a',width=.7,name='operator_'+str(i),xy=(640,320))
# The suspension/strings remain thin closed ribbons, never a zero-area return line.
points=[(-25+i*50/15,-.3) for i in range(16)]+[(25-i*50/15,.3) for i in range(16)]
d=' '.join(('M' if i==0 else 'L')+f'{x:.4f} {y:.4f}' for i,(x,y) in enumerate(points))+' Z'
path(d,stroke='#f0443a',width=.5,name='bed',xy=(238,141))
path('M207 107 L215 105 M207 111 L220 107',stroke='#34363a',width=.6)
circle(279,97,1.5,fill='#f0443a',stroke='#f0443a',width=.5,name='indicator')
# Conveyor teeth, little parcels and steam are individually reusable actors.
path('M-24 7 L-20 1 M-12 3 L-8 -3 M0 -1 L4 -7 M12 -5 L16 -11 M24 -9 L28 -15',stroke='#888b8e',width=.7,name='conveyor',xy=(364,163))
for i in range(3):path('M0 -7 L5 -4 L5 2 L0 5 L-5 2 L-5 -4 Z M-5 -4 L0 -1 L5 -4 M0 -1 L0 5',fill='#34363a',stroke='#b6b9bd',width=.6,name='cargo_'+str(i),xy=(640,320))
for i in range(3):path('M-2 6 L1 3 L2 1 L-1 -1 L-2 -3 L1 -6',stroke='#888b8e',width=.9,name='steam_'+str(i),xy=(640,320))
# A tiny maintenance robot provides some of the OP-1-like playfulness.
path(ring(84,164,10,3),fill='#08090a',stroke='#08090a',width=.3)
path('M-6 -10 L6 -10 L6 0 L-6 0 Z M-4 0 L4 0 L5 7 L-5 7 Z M-4 7 L-4 12 L-8 12 M4 7 L4 12 L8 12 M-5 1 L-10 4 M5 1 L12 -2 M-3 -13 L3 -13 M0 -13 L0 -10',fill='#34363a',stroke='#b6b9bd',width=.8,name='caretaker',xy=(84,151))
path('M-3 -7 L3 -7 L3 -5 L-3 -5 Z',fill='#f0443a',stroke='#f0443a',width=.4,name='visor',xy=(84,151))
path('M-1 5 L2 -4 L0 -7 L2 -10 L3 -6 L5 -9 L6 -5 L4 -2 L2 6 Z',fill='#888b8e',stroke='#b6b9bd',width=.5,name='wrench',xy=(98,147))
path('M169 178 L177 175 L180 177 M173 179 L174 184 M424 167 L429 166 L430 170',stroke='#63666b',width=.9)
# The engine name is in the shared header; keep only static engraving on the cabinet.
# Original monoline lettering, retained as paths so it needs no extra text slots.
letters={
 'A':'M0 5 L0 1 L1 0 L2 1 L2 5 M0 3 L2 3',
 'C':'M2 0 L0 0 L0 5 L2 5',
 'D':'M0 0 L1 0 L2 1 L2 4 L1 5 L0 5 Z',
 'E':'M2 0 L0 0 L0 5 L2 5 M0 2.5 L1.5 2.5',
 'F':'M2 0 L0 0 L0 5 M0 2.5 L1.5 2.5',
 'G':'M2 0 L0 0 L0 5 L2 5 L2 3 L1 3',
 'H':'M0 0 L0 5 M2 0 L2 5 M0 2.5 L2 2.5',
 'I':'M0 0 L2 0 M1 0 L1 5 M0 5 L2 5',
 'K':'M0 0 L0 5 M2 0 L0 2.5 L2 5',
 'L':'M0 0 L0 5 L2 5',
 'M':'M0 5 L0 0 L1 2 L2 0 L2 5',
 'N':'M0 5 L0 0 L2 5 L2 0',
 'O':'M0 0 L2 0 L2 5 L0 5 Z',
 'P':'M0 5 L0 0 L2 0 L2 2.5 L0 2.5',
 'R':'M0 5 L0 0 L2 0 L2 2.5 L0 2.5 M1 2.5 L2 5',
 'S':'M2 0 L0 0 L0 2.5 L2 2.5 L2 5 L0 5',
 'T':'M0 0 L2 0 M1 0 L1 5',
 'U':'M0 0 L0 5 L2 5 L2 0',
 'V':'M0 0 L1 5 L2 0',
 'W':'M0 0 L0 5 L1 3 L2 5 L2 0',
 'Y':'M0 0 L1 2.5 L2 0 M1 2.5 L1 5',
 '2':'M0 0 L2 0 L2 2.5 L0 2.5 L0 5 L2 5',
 '6':'M2 0 L0 0 L0 5 L2 5 L2 2.5 L0 2.5',
 '-':'M0 2.5 L2 2.5',
}
import re
labels=['SHAPER','2-OP FM','NOISE','DUST','KICK','SNARE','HIHAT','GRAIN','CHORDS','SWARM','WAVES','ANALOG','MODAL','STRING','6-OP FM']
for model,label in zip(models,labels):
    commands=[]
    for i,char in enumerate(label):
        if char==' ':continue
        tokens=re.findall(r'[MLZ]|[0-9.]+',letters[char]);at=0
        while at<len(tokens):
            command=tokens[at];at+=1
            if command=='Z':commands.append('Z');continue
            x=float(tokens[at]);y=float(tokens[at+1]);at+=2
            commands.append(f'{command}{(i*3.4+x-len(label)*1.7)*1.2:.4f} {y*1.2:.4f}')
    path(' '.join(commands),stroke='#e7e8e5',width=.55,name='label_'+model,xy=(640,320))
text(12,218,'SHIFT + 1: MODEL',6,'#63666b')
text(266,98,'XY',7,'#888b8e')
# Shared two-page header, painted last so the room cannot cover it.
lines.append('<path d="M0 0 L480 0 L480 38 L0 38 Z" fill="#000000"/>')
for i,color in enumerate(['#000000','#323232','#4B4B4B','#646464','#7D7D7D','#969696','#AFAFAF','#CDCDCD']):
    path('M0 0 L60 0 L60 20 L0 20 Z',fill=color,stroke=color,width=.5,name='bar_'+str(i),xy=(60*i,0))
for i in range(16):
    col=i%4;grey=i>=8;value=i%8>=4
    fill=('#707070' if value else '#5A5A5A') if grey else ('#CBCBCB' if col<2 else '#000000')
    size=(13 if grey else 20) if value else 10
    lines.append(f'<text id="weave_text_{i}" fill="{fill}" font-family="UniversNextPro-Light" font-size="{size}px">-</text>')
colors=['#000000','#181a1d','#34363a','#63666b','#b6b9bd','#e7e8e5','#7c2929','#f0443a','#ff9387']
for i,color in enumerate(colors):lines.append(f'<circle id="weave_palette_{i}" cx="640" cy="320" r="1" fill="{color}" stroke="{color}" stroke-width="0.7"/>')
lines.append('</g>')
(root/'panel.svg.fragment').write_text('\n'.join(lines)+'\n')
(root/'art.h').write_text('// Generated by tools/generate_art.py.\n#pragma once\nnamespace weave { namespace art {\nenum Node : unsigned {\n'+''.join(f'    {name} = {i},\n' for i,name in enumerate(names))+f'    node_count = {len(names)}\n'+'};\n}}\n')
p=root/'definition.json';d=json.loads(p.read_text());d['resources']['display'].update(stateBytes=156,paths=2,points=32,nodes=len(names),texts=16,palette=len(colors));p.write_text(json.dumps(d,indent=2)+'\n')
