"""Original pixel cartridge artwork; the SDK header is retained verbatim."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
COLORS = ['#000000','#263c40','#586d75','#8bffc8','#7cbfff','#ffe68a','#e5f3e8','#ff9fbc']
FONT = {
'A':['01110','10001','10001','11111','10001','10001','10001'],
'B':['11110','10001','10001','11110','10001','10001','11110'],
'C':['01111','10000','10000','10000','10000','10000','01111'],
'D':['11110','10001','10001','10001','10001','10001','11110'],
'E':['11111','10000','10000','11110','10000','10000','11111'],
'F':['11111','10000','10000','11110','10000','10000','10000'],
'G':['01111','10000','10000','10111','10001','10001','01110'],
'H':['10001','10001','10001','11111','10001','10001','10001'],
'I':['111','010','010','010','010','010','111'],
'J':['00111','00010','00010','00010','00010','10010','01100'],
'K':['10001','10010','10100','11000','10100','10010','10001'],
'L':['10000','10000','10000','10000','10000','10000','11111'],
'M':['10001','11011','10101','10101','10001','10001','10001'],
'N':['10001','11001','10101','10011','10001','10001','10001'],
'O':['01110','10001','10001','10001','10001','10001','01110'],
'P':['11110','10001','10001','11110','10000','10000','10000'],
'Q':['01110','10001','10001','10001','10101','10010','01101'],
'R':['11110','10001','10001','11110','10100','10010','10001'],
'S':['01111','10000','10000','01110','00001','00001','11110'],
'T':['11111','00100','00100','00100','00100','00100','00100'],
'U':['10001','10001','10001','10001','10001','10001','01110'],
'V':['10001','10001','10001','10001','10001','01010','00100'],
'W':['10001','10001','10001','10101','10101','11011','10001'],
'X':['10001','10001','01010','00100','01010','10001','10001'],
'Y':['10001','10001','01010','00100','00100','00100','00100'],
'Z':['11111','00001','00010','00100','01000','10000','11111'],
'0':['01110','10011','10101','10101','11001','10001','01110'],
'1':['010','110','010','010','010','010','111'],
'8':['01110','10001','10001','01110','10001','10001','01110'],
}
parts = [(ROOT/'resources/header.svg.fragment').read_text().rstrip()]

def path(d, fill='none', stroke='none', width=1, node=None):
    identity = f' id="bitflip_node_{node}"' if node is not None else ''
    parts.append(f'<path{identity} d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"/>')

def rect(x,y,w,h,fill,stroke='none',node=None):
    path(f'M{x} {y}h{w}v{h}h{-w}Z',fill,stroke,node=node)

def pixels(rows,x,y,size,color,node=None):
    d=[]
    for row,line in enumerate(rows):
        for col,value in enumerate(line):
            if value == '1': d.append(f'M{x+col*size} {y+row*size}h{size}v{size}h{-size}Z')
    path(' '.join(d),fill=color,node=node)

def text(value,x,y,size=1,color=COLORS[2]):
    at=x
    for char in value:
        if char==' ':at+=4*size;continue
        glyph=FONT[char]
        pixels(glyph,at,y,size,color)
        at+=(len(glyph[0])+1)*size

path('M24 49H456V57H464V195H456V203H24V195H16V57H24Z',stroke=COLORS[1])
path('M24 71H456 M150 80V183 M345 80V183',stroke=COLORS[1])
text('BITFLIP',32,52,2,COLORS[3])
text('8 MODELS',302,57,1,COLORS[2])
text('CHIP VOICE',32,188,1,COLORS[2])
text('SHAPE',171,82,1,COLORS[2])
text('ARP',171,157,1,COLORS[2])
for x in range(178,329,25):path(f'M{x} 98V143',stroke=COLORS[1],width=.5)
path('M175 120H327 M175 98V143H327V98',stroke=COLORS[1],width=.5)
# Mutable waveform: 64 points, centered at (248,120).
path(' '.join(('M' if i==0 else 'L')+f'{-62+(i if i<32 else 63-i)*4} {-.5 if i<32 else .5}' for i in range(64))+' Z',stroke=COLORS[3],node=0)
hero=['0010000100','0001111000','0011111100','0111111110','1111111111',
      '1111111111','0111111110','0011111100','0110110110','1100000011']
pixels(hero,-20,-20,4,COLORS[3],9)
rect(-10,-5,4,8,COLORS[0],node=10)
# One eye path includes the other eye; authored separately to preserve clear cutouts.
parts[-1]=parts[-1].replace('h-4Z','h-4Z M6 -5h4v8h-4Z')
pixels(['10001','00000','00100','00000','10001'],-10,-10,4,COLORS[5],11)
path('M63 149H111',stroke=COLORS[1],width=2)
text('1UP',74,88,1,COLORS[2])
for i,label in enumerate(['PLS','TRI','TWN','WAV','FM','MET','NSE','XOR']):
    x=357+(i%2)*46;y=81+(i//2)*26
    path('M0 0H38V19H0Z',stroke=COLORS[1],node=12+i)
    text(label,x+5,y+6,1,COLORS[6])
for i in range(8):
    rect(-5,-12,10,12,COLORS[1],node=20+i)
for i in range(8):rect(177+i*21,198,10,8,COLORS[2])
for i,color in enumerate(COLORS):
    parts.append(f'<circle id="bitflip_palette_{i}" cx="640" cy="0" r="1" fill="{color}" stroke="{color}" stroke-width="1"/>')
parts.append('</g>')
(ROOT/'panel.svg.fragment').write_text('\n'.join(parts)+'\n')
