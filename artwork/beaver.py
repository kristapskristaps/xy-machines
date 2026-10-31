"""Build the layered, native-compatible beaver stage and its pose table.

The character is original vector artwork developed from the generated concept
described in artwork/README.md. Every moving layer has a shared local pivot;
the C++ renderer moves these same paths in the browser and on the instrument.
"""
from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parents[1]
panel = ROOT / 'panel.svg.fragment'
old = panel.read_text()
header = old.split('<!-- Drum bodies')[0].split('<!-- Beaver stage')[0]
header = re.sub(r'<text x="[^"]+".*?</text>\n', '', header).rstrip()
logos = '<!-- Drum bodies' + old.split('<!-- Drum bodies', 1)[1].split('<circle id="x0x_palette_0"')[0]
logos = logos.replace('stroke-width="1.5"','stroke-width="3"')
LIGHT, MID, DARK, BLUE, RED, BLACK = '#ECECEC', '#606060', '#292929', '#3B86FF', '#E64C4C', '#000000'
layers = []
def path(name, group, x, y, d, fill=MID, stroke=LIGHT, width=1.2):
    layers.append((name, group, x, y, f'd="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"'))

# Tail behind the stool. Crosshatching makes the paddle unmistakable at 1x.
path('paddle', 'tail', 164, 169, 'M20,20 C-4,22 -35,2 -36,-15 C-38,-35 -15,-37 0,-23 C13,-13 20,4 20,20 Z', DARK)
path('paddle grain', 'tail', 164, 169, 'M-29,-25 L6,12 L5,14 L-31,-23 Z M-16,-30 L15,7 L14,9 L-18,-28 Z M-34,-10 L-3,21 L-5,22 L-35,-8 Z M-25,-30 L-35,-4 L-33,-3 L-23,-29 Z M-11,-26 L-25,11 L-23,12 L-9,-25 Z M3,-14 L-9,19 L-7,20 L5,-12 Z', DARK, MID, .8)
path('chest', 'body', 240, 132, 'M-19,-27 C-35,-30 -44,-15 -49,3 L-40,0 L-42,11 L-32,7 L-28,31 L30,31 L35,9 L44,14 L41,2 L49,4 C44,-17 32,-27 20,-27 Z')
path('chest fur', 'body', 240, 132, 'M-17,-19 L-11,-10 L-9,-23 L-3,-11 L2,-25 L7,-10 L13,-23 L17,-10 L24,-19 L16,6 L9,1 L5,14 L-3,5 L-9,14 L-13,0 L-21,6 Z', DARK, DARK, .7)
path('left shoe', 'left_foot', 209, 198, 'M-8,-7 C-18,-8 -25,-3 -27,4 L8,4 L12,-3 Z', DARK)
path('right shoe', 'right_foot', 283, 198, 'M-6,-6 C3,-8 19,-2 24,5 L-10,5 L-13,-2 Z', DARK)

# Head, ears, muzzle and lenses move together; the glasses remain crisp black.
path('ears', 'head', 240, 86, 'M-28,-23 C-46,-39 -39,-50 -26,-45 C-17,-43 -15,-33 -20,-26 Z M24,-27 C31,-47 44,-39 41,-27 L34,-19 Z')
path('ear centres', 'head', 240, 86, 'M-28,-28 C-39,-38 -32,-44 -26,-39 C-20,-35 -21,-31 -23,-28 Z M29,-28 C34,-39 39,-35 35,-27 Z', DARK, DARK, .7)
path('head', 'head', 240, 86, 'M-29,-25 L-18,-30 L-24,-32 L-10,-35 L-15,-38 L-1,-37 L7,-34 L16,-34 L24,-27 L31,-24 L34,-16 L41,-11 L36,-7 L41,1 L36,2 L38,12 L32,10 C31,30 12,39 -6,34 C-21,32 -32,23 -33,9 L-40,13 L-36,2 L-42,4 L-36,-7 L-41,-6 L-35,-15 Z')
path('hair light', 'head', 240, 86, 'M-18,-30 L-7,-34 L-11,-35 L-1,-34 L13,-28 L4,-29 L-4,-31 Z', LIGHT, LIGHT, .5)
path('smile', 'head', 240, 86, 'M-16,12 C-4,4 24,7 28,17 C25,33 0,34 -10,25 Z', BLACK, BLACK, .7)
path('muzzle', 'head', 240, 86, 'M8,2 C-3,-5 -18,-1 -19,10 C-20,20 -7,22 3,16 C11,26 27,23 29,13 C31,3 18,-1 8,2 Z', LIGHT)
path('teeth', 'head', 240, 86, 'M-3,19 L7,20 L5,33 L-6,31 Z M8,20 L17,19 L15,31 L6,33 Z', LIGHT, BLACK, 1)
path('sunglasses', 'head', 240, 86, 'M-30,-18 L-6,-18 L1,-13 L11,-14 L15,-17 L36,-12 L33,-1 C29,6 18,6 12,0 L7,-8 L1,-8 L-3,1 C-10,7 -23,1 -26,-7 Z', BLACK, LIGHT, 1.3)
path('lens glints', 'head', 240, 86, 'M-23,-13 L-10,-13 L-23,-5 L-24,-9 Z M-16,-1 L-5,-9 L-6,-6 L-12,0 Z M18,-12 L29,-9 L17,-2 L15,-5 Z', BLUE, BLUE, .5)
path('nose', 'head', 240, 86, 'M4,3 C5,-3 19,-3 22,3 C24,7 14,12 11,11 C8,9 3,7 4,3 Z', BLACK, DARK, .9)
path('whiskers', 'head', 240, 86, 'M-18,8 L-34,3 L-34,4 L-18,9 Z M-18,12 L-36,12 L-36,13 L-18,13 Z M25,9 L40,6 L40,7 L25,10 Z M26,13 L42,17 L42,18 L26,14 Z', LIGHT, LIGHT, .5)

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
svg = [header, '<!-- Beaver stage: generated by artwork/beaver.py. -->', stands]
for i, (name, group, x, y, attrs) in enumerate(layers, 56):
    svg.append(f'<!-- {name} --><path id="x0x_node_{i}" {attrs}/>')
svg.append(logos.rstrip())
palette = ['#000000','#CBCBCB','#444444','#777777','#999999',BLUE,RED,MID,DARK,LIGHT]
for i, color in enumerate(palette):
    svg.append(f'<circle id="x0x_palette_{i}" cx="640" cy="0" r="1" fill="{color}" stroke="{color}" stroke-width="1"/>')
svg.append('</g>')
panel.write_text('\n'.join(svg)+'\n')

poses = ['// Generated by artwork/beaver.py.', '#pragma once', 'namespace x0x {',
         'enum BeaverPart { '+','.join('beaver_'+g for g in groups)+' };',
         'struct BeaverPose { opxy::i32 x,y; BeaverPart part; };',
         'const opxy::u32 beaver_node = 56;',
         'static const BeaverPose beaver_poses[] = {']
for name, group, x, y, _ in layers:
    poses.append(f'    {{{x},{y},beaver_{group}}}, // {name}')
poses.extend(['};', 'const opxy::u32 beaver_layers = sizeof(beaver_poses)/sizeof(beaver_poses[0]);','}'])
(ROOT/'beaver_pose.h').write_text('\n'.join(poses)+'\n')

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
hits = ['// Generated by artwork/beaver.py; exact two-link contact poses.', '#pragma once',
        'namespace x0x {', 'struct BeaverHit { opxy::i32 x,y,arm[2],stick[2]; opxy::u32 hands; };',
        'static const BeaverHit beaver_hits[] = {']
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
(ROOT/'beaver_hits.h').write_text('\n'.join(hits)+'\n')
print(f'{len(layers)} beaver layers; {56+len(layers)} total nodes')
