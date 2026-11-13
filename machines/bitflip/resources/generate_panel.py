"""Original platform world and runner cels; the SDK header is retained."""
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
'2':['01110','10001','00001','00010','00100','01000','11111'],
'3':['11110','00001','00001','01110','00001','00001','11110'],
'4':['00010','00110','01010','10010','11111','00010','00010'],
'5':['11111','10000','10000','11110','00001','00001','11110'],
'6':['01110','10000','10000','11110','10001','10001','01110'],
'7':['11111','00001','00010','00100','01000','01000','01000'],
'9':['01110','10001','10001','01111','00001','00001','01110'],
}
parts = [(ROOT/'resources/header.svg.fragment').read_text().rstrip()]
ids = {};next_node=8

def allocate(name,count):
    global next_node
    at=next_node;ids[name]=at;next_node+=count;return at


def path(d, fill='none', stroke='none', width=1, node=None):
    identity=f' id="bitflip_node_{node}"' if node is not None else ''
    parts.append(f'<path{identity} d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"/>')


def rect(x,y,w,h,fill,stroke='none',node=None):
    path(f'M{x} {y}h{w}v{h}h{-w}Z',fill,stroke,node=node)


def pixels(rows,x,y,size,color,node=None):
    # Merge adjacent pixels into horizontal runs to keep the native SVG small.
    d=[]
    for row,line in enumerate(rows):
        col=0
        while col<len(line):
            if line[col]!='1':col+=1;continue
            end=col+1
            while end<len(line) and line[end]=='1':end+=1
            width=(end-col)*size
            d.append(f'M{x+col*size} {y+row*size}h{width}v{size}h{-width}Z');col=end
    path(' '.join(d),fill=color,node=node)


def text(value,x,y,size=1,color=COLORS[2]):
    at=x
    for char in value:
        if char==' ':at+=4*size;continue
        glyph=FONT[char];pixels(glyph,at,y,size,color);at+=(len(glyph[0])+1)*size


rect(0,38,480,182,COLORS[0])
text('BITFLIP',14,45,1.5,COLORS[3])
text('WORLD',227,47,1,COLORS[2])
# HUD coin and a black sky; the readable SDK header remains above y=38.
path('M411 46h6v2h2v8h-2v2h-6v-2h-2v-8h2Z',stroke=COLORS[5])
for x,y in [(18,103),(92,77),(188,110),(329,77),(461,113)]:rect(x,y,2,2,COLORS[1])
hills=allocate('hills',5)
for i in range(5):
    path('M-64 0v-8h8v-8h8v-8h8v-8h8v-8h8v-8h16v-8h16v8h16v8h8v8h8v8h8v8h8v8h8v8Z',fill=COLORS[1],node=hills+i)
clouds=allocate('clouds',4)
for i in range(4):
    path('M-26 0v-8h8v-6h8v-4h14v4h8v6h10v8Z',fill=COLORS[2],node=clouds+i)
# One stationary ground fill, with individually moving mortar tiles.
rect(0,184,480,36,COLORS[1]);rect(0,181,480,4,COLORS[3])
ground=allocate('ground',16)
for i in range(16):
    path('M-16 4H16 M-16 20H16 M0 4V20 M-16 20V36 M16 20V36',stroke=COLORS[2],node=ground+i)
obstacles=allocate('obstacles',2)
path('M-16-28H16V-20H-16Z M-12-20V0H12V-20 M-7-16V-4 M-3-16V-4',stroke=COLORS[3],width=2,node=obstacles)
path('M-14-28H14V0H-14Z M-10-24H10V-4H-10Z M-8-22L8-6 M8-22L-8-6',stroke=COLORS[7],width=2,node=obstacles+1)
platforms=allocate('platforms',2)
for i in range(2):
    path('M-32 0H32V16H-32Z M-16 0V16 M0 0V16 M16 0V16 M-28 4H-20 M4 4H12 M20 4H28 M-11 4H-5V8H-8V10 M-8 12V14',stroke=COLORS[5],node=platforms+i)
coins=allocate('coins',24)
for coin in range(6):
    for spin,width in enumerate([6,4,2,4]):
        path(f'M{-width} -5h2v-2h{2*width-4}v2h2v10h-2v2h{-2*width+4}v-2h-2Z',stroke=COLORS[5],node=coins+coin*4+spin)

head=['.....CCCCC......','....CCCCCCCC....','....CCCCCCCCCC..','.....SSSSSS.....',
      '.....SSSSKSS....','......SSSSSSSS..','.....SSSSSSS....','...RRRRRRR......',
      '.....BBBBBB.....','.....BBBBBB.....','.....BBBBBB.....','....BBBBBBB.....','.....BBBBBB.....']
body=allocate('body',5)
for i,(letter,color) in enumerate([('C',3),('S',5),('K',0),('R',7),('B',4)]):
    pixels([''.join('1' if v==letter else '0' for v in row) for row in head],-16,-40,2,COLORS[color],body+i)

def line(grid,a,b,letter,width=2):
    x0,y0=a;x1,y1=b;steps=max(abs(x1-x0),abs(y1-y0),1)
    for at in range(steps+1):
        x=round(x0+(x1-x0)*at/steps);y=round(y0+(y1-y0)*at/steps)
        for row in range(y,y+width):
            for col in range(x,x+width):
                if 0<=row<20 and 0<=col<16:grid[row][col]=letter

# Knees and feet, then opposing elbows and hands: idle, four run cels, jump.
poses=[
 ((6,15),(6,18),(9,15),(9,18),(4,10),(4,12),(11,10),(11,12)),
 ((4,15),(2,18),(10,14),(12,15),(3,10),(2,12),(12,9),(13,8)),
 ((6,15),(5,18),(10,16),(11,18),(4,10),(3,11),(11,10),(12,11)),
 ((5,14),(3,15),(11,15),(13,18),(4,9),(3,8),(12,10),(13,12)),
 ((6,16),(5,18),(9,15),(10,18),(4,10),(3,11),(11,10),(12,11)),
 ((4,14),(3,16),(11,14),(12,16),(4,8),(4,6),(12,9),(13,8)),
]
limbs=allocate('limbs',18)
for pose,points in enumerate(poses):
    grid=[list('.'*16) for _ in range(20)]
    lk,lf,rk,rf,le,lh,re,rh=points
    for a,b in [((6,12),lk),(lk,lf),((9,12),rk),(rk,rf),((5,8),le),(le,lh),((10,8),re),(re,rh)]:line(grid,a,b,'B')
    for hand in [lh,rh]:line(grid,hand,hand,'S')
    for foot in [lf,rf]:line(grid,foot,(min(13,foot[0]+1),foot[1]),'W')
    for layer,(letter,color) in enumerate([('B',4),('S',5),('W',6)]):
        pixels([''.join('1' if v==letter else '0' for v in row) for row in grid],-16,-40,2,COLORS[color],limbs+pose*3+layer)
sparks=allocate('sparks',4)
for i in range(4):path('M-2 0H2 M0-2V2',stroke=COLORS[5],node=sparks+i)
digits=allocate('digits',20)
for place in range(2):
    for digit in range(10):pixels(FONT[str(digit)],0,0,1.5,COLORS[5],digits+place*10+digit)
models=allocate('models',8)
for i in range(8):rect(-2,-2,4,4,COLORS[1],node=models+i)
for i,color in enumerate(COLORS):parts.append(f'<circle id="bitflip_palette_{i}" cx="640" cy="0" r="1" fill="{color}" stroke="{color}" stroke-width="1"/>')
parts.append('</g>')
asset='\n'.join(parts)+'\n'
if len(asset.encode())>65536:raise RuntimeError('Panel exceeds the SDK asset budget')
(ROOT/'panel.svg.fragment').write_text(asset)
constants='// Generated by resources/generate_panel.py.\n#pragma once\nnamespace bitflip { namespace scene {\n'
for name,at in ids.items():constants+=f'const unsigned {name}={at};\n'
constants+=f'const unsigned node_count={next_node};\n'+'} }\n'
(ROOT/'scene.h').write_text(constants)
