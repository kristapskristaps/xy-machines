"""Build the layered, native-compatible Grogu drummer and its pose table.

Every moving layer has a shared local pivot;
the C++ renderer moves these same paths in the browser and on the instrument.
"""
from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parents[1]
panel = ROOT / 'panel.svg.fragment'
old = panel.read_text()
header = old.split('<!-- Drum bodies')[0].split('<!-- Drummer stage')[0].split('<!-- Beaver stage')[0]
header = re.sub(r'<text x="[^"]+".*?</text>\n', '', header).rstrip()
for i in range(8):
    width = 50 if i % 2 == 0 else 70
    header = re.sub(rf'(<path id="x0x_node_{i}"[^>]* d=")[^"]+',
                    rf'\g<1>M0,0 L{width},0 L{width},20 L0,20 Z', header)
header = header.replace('font-size="20px"', 'font-size="17px"').replace('font-size="13px"', 'font-size="12px"')
logos = '<!-- Drum bodies' + old.split('<!-- Drum bodies', 1)[1].split('<!-- Selected pad')[0].split('<circle id="x0x_palette_0"')[0]
logos = logos.replace('stroke-width="1.5"','stroke-width="3"')
LIGHT, MID, DARK, BLUE, RED, BLACK = '#ECECEC', '#606060', '#292929', '#3B86FF', '#E64C4C', '#000000'
layers = []
def path(name, group, x, y, d, fill=MID, stroke=LIGHT, width=1.2):
    layers.append((name, group, x, y, f'd="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"'))

# A broad robe collar and little three-toed feet replace the fur and paddle.
path('robe hem', 'tail', 164, 169, 'M34,-51 C41,-61 87,-62 108,-48 L127,25 C100,40 37,39 19,21 Z', DARK)
path('robe folds', 'tail', 164, 169, 'M39,-34 L33,20 L35,24 L42,-34 Z M94,-34 L105,26 L108,24 L97,-34 Z', DARK, MID, .7)
path('robe', 'body', 240, 132, 'M-25,-27 C-37,-28 -47,-15 -46,5 L-37,30 L-15,34 L0,30 L18,34 L38,29 L46,4 C47,-17 36,-29 25,-27 Z', MID, LIGHT, 1.1)
path('wrapped collar', 'body', 240, 132, 'M-30,-28 L-40,-18 C-28,-7 -13,-1 1,3 L35,-20 L27,-29 L0,-13 Z M-3,3 L5,3 L5,30 L-3,30 Z', LIGHT, BLACK, 1.2)
path('left foot', 'left_foot', 209, 198, 'M-13,-5 C-21,-8 -28,-1 -25,4 L-18,4 L-17,0 L-15,4 L-7,4 L-6,0 L-3,4 L8,4 L7,-4 Z', MID)
path('right foot', 'right_foot', 283, 198, 'M-7,-5 C5,-9 14,-3 23,0 L24,5 L14,5 L12,1 L10,5 L2,5 L0,1 L-2,5 L-12,5 Z', MID)

# A soft round face, oversized eyes and long curved ears follow the reference.
path('wide ears', 'head', 240, 86, 'M-32,-25 C-52,-31 -75,-43 -94,-35 C-83,-16 -63,5 -35,4 Z M32,-25 C52,-31 75,-43 94,-35 C83,-16 63,5 35,4 Z', LIGHT, MID, 1.3)
path('ear folds', 'head', 240, 86, 'M-37,-21 C-53,-27 -72,-36 -85,-34 C-74,-18 -57,-2 -38,-1 L-38,-3 C-55,-6 -70,-18 -80,-31 C-66,-30 -51,-23 -37,-19 Z M37,-21 C53,-27 72,-36 85,-34 C74,-18 57,-2 38,-1 L38,-3 C55,-6 70,-18 80,-31 C66,-30 51,-23 37,-19 Z', BLACK, BLACK, .4)
path('head', 'head', 240, 86, 'M-37,-17 C-38,-36 -21,-43 0,-43 C22,-43 39,-34 38,-16 L37,5 C37,25 20,32 0,33 C-21,32 -38,24 -38,5 Z', LIGHT, BLACK, 1.3)
path('forehead and hair', 'head', 240, 86, 'M-25,-29 C-14,-36 13,-36 25,-29 L24,-28 C11,-33 -13,-33 -24,-28 Z M-18,-23 C-8,-28 9,-28 19,-23 L18,-22 C8,-26 -7,-26 -17,-22 Z M-7,-42 L-11,-46 L-4,-44 L0,-48 L2,-43 L8,-46 L5,-41 Z', BLACK, BLACK, .4)
path('brows', 'head', 240, 86, 'M-31,-12 C-27,-22 -13,-24 -7,-15 L-6,-13 C-16,-21 -24,-19 -30,-10 Z M7,-15 C14,-24 28,-22 32,-12 L31,-10 C25,-19 16,-21 6,-13 Z', BLACK, BLACK, .5)
path('eye creases', 'head', 240, 86, 'M-32,5 C-24,13 -13,14 -6,9 L-7,11 C-15,16 -27,14 -33,7 Z M7,9 C14,14 26,13 33,5 L34,7 C27,15 15,16 7,11 Z', MID, MID, .4)
path('smile', 'head', 240, 86, 'M-12,20 C-7,17 -4,18 0,20 C5,17 10,18 14,20 L13,22 C8,20 5,20 1,22 L-1,22 C-5,20 -8,20 -12,22 Z', BLACK, BLACK, .4)
path('large eyes', 'head', 240, 86, 'M-31,-4 C-32,-18 -13,-21 -8,-10 C-5,-3 -7,7 -17,9 C-26,10 -31,5 -31,-4 Z M9,-10 C14,-21 33,-18 32,-4 C32,5 27,10 18,9 C8,7 6,-3 9,-10 Z', BLACK, BLACK, .8)
path('eye highlights', 'head', 240, 86, 'M-26,-11 C-26,-16 -18,-17 -17,-12 C-16,-8 -24,-6 -26,-11 Z M-12,1 C-12,-1 -9,-1 -9,1 C-9,3 -12,3 -12,1 Z M13,-11 C13,-16 21,-17 22,-12 C23,-8 15,-6 13,-11 Z M26,1 C26,-1 29,-1 29,1 C29,3 26,3 26,1 Z', LIGHT, LIGHT, .4)
path('nose', 'head', 240, 86, 'M-4,9 C-2,6 3,6 6,9 L5,11 C2,9 -1,9 -3,11 Z', BLACK, BLACK, .5)
path('cheeks', 'head', 240, 86, 'M-24,18 L-20,17 L-19,18 L-23,19 Z M21,17 L25,18 L24,19 L20,18 Z', MID, MID, .4)

# Shoulder pivots make the paws and sticks swing as one articulated assembly.
path('left arm', 'left_arm', 202, 120, 'M2,1 C-2,-8 -8,-14 -11,-20 L-18,-18 L-15,-10 L-23,-13 C-28,-21 -23,-29 -18,-30 L-16,-35 C-11,-38 -8,-31 -7,-29 L-3,-29 C2,-25 0,-20 -3,-17 L11,1 Z')
path('left fingers', 'left_arm', 202, 120, 'M-20,-26 L-11,-22 L-11,-20 L-21,-24 Z M-17,-30 L-8,-26 L-8,-24 L-18,-28 Z', DARK, DARK, .7)
path('left stick', 'left_stick', 187, 94, 'M-1,0 L-20,-34 L-18,-35 L2,-2 Z M-20,-34 C-25,-37 -19,-42 -17,-37 C-16,-36 -17,-35 -18,-35 Z', LIGHT, LIGHT, .7)
path('right arm', 'right_arm', 279, 124, 'M-3,1 C2,-4 8,-9 11,-14 L17,-11 L14,-5 L21,-11 C27,-15 26,-22 21,-25 L20,-30 C16,-34 12,-27 11,-24 L7,-24 C3,-20 5,-16 7,-14 L-12,-1 Z')
path('right fingers', 'right_arm', 279, 124, 'M13,-22 L23,-20 L23,-18 L13,-20 Z M14,-26 L23,-24 L23,-22 L14,-24 Z', DARK, DARK, .7)
path('right stick', 'right_stick', 299, 104, 'M-2,0 L27,-34 L29,-32 L1,2 Z M27,-34 C27,-39 33,-39 32,-34 C31,-32 29,-31 29,-32 Z', LIGHT, LIGHT, .7)

# Kit surfaces. The black kick head is deliberately left open for the band logo.
path('crash plate', 'cymbal', 144, 111, 'M-37,-6 C-27,-14 -11,-11 -5,-7 L-3,-11 L4,-10 L6,-5 C24,-1 32,3 37,9 C17,17 -18,5 -37,-6 Z', LIGHT, LIGHT, 1.1)
path('crash underside', 'cymbal', 144, 111, 'M-32,-3 C-15,4 10,11 29,10 C15,13 -12,7 -32,-3 Z M-3,-6 C2,-6 6,-3 7,-1 C2,1 -2,-1 -3,-6 Z', RED, RED, .6)
path('hat lower', 'hat_lower', 345, 119, 'M-28,6 L-7,1 L7,1 L28,6 L19,10 L-19,10 Z', LIGHT, LIGHT, 1)
path('hat upper', 'hat_upper', 345, 119, 'M-28,0 L-7,-5 L-3,-8 L3,-8 L7,-5 L28,0 L19,3 L-19,3 Z', LIGHT, LIGHT, 1)
path('hat blue edge', 'hat_upper', 345, 119, 'M-24,1 L22,1 L18,3 L-18,3 Z', BLUE, BLUE, .5)
path('snare shell', 'snare', 314, 161, 'M-27,-8 C-12,-13 12,-13 27,-8 L25,12 C10,17 -10,17 -25,12 Z', DARK, LIGHT, 1.2)
path('snare head and hoops', 'snare', 314, 161, 'M-27,-8 C-27,-17 27,-17 27,-8 C27,1 -27,1 -27,-8 Z M-25,9 C-10,13 10,13 25,9 L25,12 C10,17 -10,17 -25,12 Z M-17,-7 L-17,10 L-15,10 L-15,-7 Z M15,-7 L15,10 L17,10 L17,-7 Z', BLACK, LIGHT, 1)
path('low tom shell', 'tom_left', 208, 134, 'M-24,-10 C-11,-15 11,-15 24,-10 L22,16 C9,24 -9,24 -22,16 Z', MID, LIGHT, 1.2)
path('low tom skin', 'skin_left', 208, 134, 'M-24,-10 C-24,-23 24,-23 24,-10 C24,3 -24,3 -24,-10 Z M-22,14 C-10,20 10,20 22,14 L22,17 C10,24 -10,24 -22,17 Z', BLACK, LIGHT, 1)
path('high tom shell', 'tom_right', 271, 135, 'M-23,-12 C-10,-17 10,-17 23,-12 L21,13 C9,21 -9,21 -21,13 Z', MID, LIGHT, 1.2)
path('high tom skin', 'skin_right', 271, 135, 'M-23,-12 C-23,-25 23,-25 23,-12 C23,1 -23,1 -23,-12 Z M-21,11 C-9,17 9,17 21,11 L21,14 C9,21 -9,21 -21,14 Z', BLACK, LIGHT, 1)
path('kick shell', 'bass', 240, 172, 'M0,-40 C23,-40 42,-22 42,0 C42,23 23,40 0,40 C-23,40 -42,23 -42,0 C-42,-22 -23,-40 0,-40 Z', DARK, LIGHT, 1.2)
path('kick head', 'bass_skin', 240, 172, 'M0,-35 C20,-35 36,-20 36,0 C36,20 20,35 0,35 C-20,35 -36,20 -36,0 C-36,-20 -20,-35 0,-35 Z', BLACK, LIGHT, 1)
path('kick lugs', 'bass', 240, 172, 'M-5,-42 L5,-42 L5,-37 L-5,-37 Z M-5,37 L5,37 L5,42 L-5,42 Z M-43,-4 L-37,-4 L-37,4 L-43,4 Z M37,-4 L43,-4 L43,4 L37,4 Z M-33,-30 L-28,-34 L-24,-29 L-29,-25 Z M28,-34 L33,-30 L29,-25 L24,-29 Z M-33,30 L-29,25 L-24,29 L-28,34 Z M28,34 L24,29 L29,25 L33,30 Z', LIGHT, BLACK, .7)
path('pedal', 'pedal', 240, 172, 'M-1,29 L1,29 L1,43 L-1,43 Z M-4,40 L4,40 L4,45 L-4,45 Z', BLUE, BLUE, .6)

stands = '''<path d="M142,117 L144,117 L144,195 L142,195 Z M143,187 L125,208 L123,207 L142,184 Z M144,185 L162,207 L160,208 L143,188 Z M344,129 L346,129 L346,195 L344,195 Z M345,187 L329,208 L327,207 L344,184 Z M346,185 L364,207 L362,208 L345,188 Z M312,178 L314,178 L314,197 L312,197 Z M313,190 L300,209 L298,208 L312,187 Z M314,189 L327,208 L325,209 L313,192 Z M206,149 L208,149 L211,171 L209,171 Z M271,149 L273,149 L271,171 L269,171 Z" fill="#CBCBCB" stroke="#CBCBCB" stroke-width="0.6"/>
<path d="M210,202 L200,215 L197,214 L207,201 Z M269,201 L280,214 L277,215 L266,202 Z" fill="#777777" stroke="#777777" stroke-width="0.6"/>'''

groups = list(dict.fromkeys(item[1] for item in layers))
svg = [header, '<!-- Drummer stage: generated by artwork/drummer.py. -->', stands]
for i, (name, group, x, y, attrs) in enumerate(layers, 56):
    svg.append(f'<!-- {name} --><path id="x0x_node_{i}" {attrs}/>')
# Native vector lettering uses no extra text slots or device font assumptions.
glyphs = {
    ' ': [0]*7, '.': [0,0,0,0,0,6,6],
    'A':[14,17,17,31,17,17,17], 'B':[30,17,17,30,17,17,30],
    'C':[14,17,16,16,16,17,14], 'D':[30,17,17,17,17,17,30],
    'E':[31,16,16,30,16,16,31], 'G':[14,17,16,23,17,17,15],
    'H':[17,17,17,31,17,17,17], 'I':[14,4,4,4,4,4,14],
    'K':[17,18,20,24,20,18,17], 'L':[16,16,16,16,16,16,31],
    'M':[17,27,21,21,17,17,17], 'N':[17,25,21,19,17,17,17],
    'O':[14,17,17,17,17,17,14], 'P':[30,17,17,30,16,16,16],
    'R':[30,17,17,30,20,18,17], 'S':[15,16,16,14,1,1,30],
    'T':[31,4,4,4,4,4,4], 'U':[17,17,17,17,17,17,14],
    'V':[17,17,17,17,17,10,4], 'W':[17,17,17,21,21,21,10],
    'Y':[17,17,10,4,4,4,4],
    '0':[14,17,19,21,25,17,14], '6':[14,16,16,30,17,17,14],
    '7':[31,1,2,4,8,8,8], '8':[14,17,17,14,17,17,14],
    '9':[14,17,17,15,1,1,14],
}
def lettering(text, size=1):
    rectangles = []
    for at, char in enumerate(text):
        runs = {}
        for y, row in enumerate(glyphs[char]):
            x = 0
            while x < 5:
                if row & (16 >> x):
                    start = x
                    while x < 5 and row & (16 >> x): x += 1
                    key = (start,x)
                    if key in runs and runs[key][-1][1] == y:
                        runs[key][-1][1] = y+1
                    else: runs.setdefault(key,[]).append([y,y+1])
                else: x += 1
        for (left,right), spans in runs.items():
            for top,bottom in spans:
                l,t = (at*6+left)*size,top*size
                r,b = (at*6+right)*size,bottom*size
                rectangles.append(f'M{l:g},{t:g}L{r:g},{t:g}L{r:g},{b:g}L{l:g},{b:g}Z')
    return ' '.join(rectangles)
for family, name in enumerate(['606','707','808','909']):
    logos = re.sub(rf'<(?:path|circle) id="x0x_node_{44+family}"[^>]*/>',
                   f'<path id="x0x_node_{44+family}" d="{lettering(name,3)}" fill="{LIGHT}" stroke="{LIGHT}" stroke-width="0.1"/>', logos)
svg.append(logos.rstrip())
label_node = 56+len(layers)
svg.append('<!-- Selected pad and style, independent of the played drum. -->')
names = ['KICK','SNARE','CLAP','RIM','TOM LO','TOM MID','TOM HI','CL.HAT','OP.HAT',
         'CYMBAL','RIDE','COWBELL','CLAVES','MARACAS','CONGA L','CONGA M','CONGA H','TAMB']
svg.append(f'<path d="{lettering("PAD")}" transform="matrix(1 0 0 1 22 53)" fill="{MID}" stroke="{MID}" stroke-width="0.1"/>')
for i,name in enumerate(names,label_node):
    svg.append(f'<path id="x0x_node_{i}" d="{lettering(name)}" fill="{LIGHT}" stroke="{LIGHT}" stroke-width="0.1"/>')
palette = ['#000000','#CBCBCB','#444444','#777777','#999999',BLUE,RED,MID,DARK,LIGHT]
for i, color in enumerate(palette):
    svg.append(f'<circle id="x0x_palette_{i}" cx="640" cy="0" r="1" fill="{color}" stroke="{color}" stroke-width="1"/>')
svg.append('</g>')
panel.write_text('\n'.join(svg)+'\n')

poses = ['// Generated by artwork/drummer.py.', '#pragma once', 'namespace x0x {',
         'enum DrummerPart { '+','.join('drummer_'+g for g in groups)+' };',
         'struct DrummerPose { opxy::i32 x,y; DrummerPart part; };',
         'const opxy::u32 drummer_node = 56;',
         'static const DrummerPose drummer_poses[] = {']
for name, group, x, y, _ in layers:
    poses.append(f'    {{{x},{y},drummer_{group}}}, // {name}')
poses.extend(['};', 'const opxy::u32 drummer_layers = sizeof(drummer_poses)/sizeof(drummer_poses[0]);','}'])
(ROOT/'drummer_pose.h').write_text('\n'.join(poses)+'\n')

# Solve the two-link arm/stick contact poses offline. Runtime uses only Q10
# transforms. Choose the elbow solution nearest the relaxed, raised-stick pose.
targets = [(240,172),(314,150),(240,133),(314,150),(218,124),(232,125),
           (267,124),(345,118),(345,118),(144,109),(153,114),(302,143),
           (240,134),(240,134),(218,128),(224,127),(230,126),(240,134)]
def contact(target, shoulder, hand, tip):
    dx,dy = target[0]-shoulder[0],target[1]-shoulder[1]
    a,b,r = math.hypot(*hand),math.hypot(*tip),math.hypot(dx,dy)
    if not abs(a-b)+.1 < r < a+b-.1:
        return None
    base = math.atan2(hand[1],hand[0])
    spread = math.acos((a*a+r*r-b*b)/(2*a*r))
    wrap = lambda angle: (angle+math.pi)%(2*math.pi)-math.pi
    angles = [wrap(math.atan2(dy,dx)+sign*spread-base) for sign in [-1,1]]
    arm = min(angles,key=abs)
    wx = math.cos(arm)*hand[0]-math.sin(arm)*hand[1]
    wy = math.sin(arm)*hand[0]+math.cos(arm)*hand[1]
    stick = wrap(math.atan2(dy-wy,dx-wx)-math.atan2(tip[1],tip[0]))
    return round(arm*1800/math.pi),round(stick*1800/math.pi)
hits = ['// Generated by artwork/drummer.py; exact two-link contact poses.', '#pragma once',
        'namespace x0x {', 'struct DrummerHit { opxy::i32 x,y,arm[2],stick[2]; opxy::u32 hands; };',
        'static const DrummerHit drummer_hits[] = {']
for model,target in enumerate(targets):
    a = contact(target,(202,120),(-15,-26),(-20,-34))
    b = contact(target,(279,124),(20,-20),(27,-34))
    hands = (1 if a else 0)+(2 if b else 0)
    if not hands:
        raise ValueError(f'Unreachable playing surface: model {model}')
    if model == 0: hands = 0 # Kick is played with the pedal.
    a,b = a or (0,0),b or (0,0)
    hits.append(f'    {{{target[0]},{target[1]},{{{a[0]},{b[0]}}},{{{a[1]},{b[1]}}},{hands}}},')
hits.extend(['};','}'])
(ROOT/'drummer_hits.h').write_text('\n'.join(hits)+'\n')
print(f'{len(layers)} drummer layers; {label_node+len(names)} total nodes')
