"""Generate abstract geometric native scenes; audio and controls are unchanged."""
from pathlib import Path
import math

ROOT = Path(__file__).resolve().parents[2]
NAMES = {'analog': 'Tandem', 'waves': 'Atlas', 'chords': 'Choir', 'grain': 'Sift', 'shaper': 'Crease'}
PALETTE = ['#000000', '#000000', '#eeeeee', '#eeeeee', '#eeeeee', '#eeeeee', '#eeeeee', '#eeeeee']

COMMON = r'''#include "display.h"
#include <opxy/page_header.hpp>
namespace ID {
namespace {
using namespace opxy::draw;
using opxy::i32;
using opxy::u32;
const i32 settle=180;
i32 ease(i32 age) {
    const i32 left=unit-clamp(age,0,settle)*unit/settle;
    return unit-left*left/unit*left/unit;
}
inline void key(Frame& f,unsigned p) {
    u32 hash=2166136261u;
    for(const auto& point:f.paths[p].points) {
        hash=(hash^static_cast<u32>(point.x))*16777619u;
        hash=(hash^static_cast<u32>(point.y))*16777619u;
    }
    f.paths[p].key=hash?hash:1;
}
inline void ribbon(Frame& f,unsigned p,i32 base,i32 phase,i32 amplitude,bool folded) {
    for(unsigned i=0;i<32;++i) {
        const i32 column=static_cast<i32>(i<16?i:31-i);
        const i32 wave=folded?((column%6<3?static_cast<i32>(column%6):6-static_cast<i32>(column%6))*2*unit/3-unit):sine(column*220+phase);
        const i32 bend=folded ? floor_q10(sine(phase+column*120)*6*unit) : 0;
        const i32 y=base+floor_q10(wave*amplitude)+bend+(i<16?-5:5)*unit;
        f.paths[p].points[i]={(-62*unit+column*124*unit/15)/2,(y-123*unit)/2};
    }
    key(f,p);
}
inline void capsule(Frame& f,unsigned p,i32 width,i32 height,i32 phase,i32 depth) {
    for(unsigned i=0;i<32;++i) {
        const i32 angle=static_cast<i32>(i)*3600/32;
        const i32 x=floor_q10(sine(angle+900)*width);
        const i32 y=floor_q10(sine(angle)*height);
        f.paths[p].points[i]={(x+floor_q10(sine(angle*2+phase)*depth))/2,y/2};
    }
    key(f,p);
}
inline void arch(Frame& f,unsigned p,i32 height,i32 lean) {
    for(unsigned i=0;i<32;++i) {
        i32 x,y;
        if(i<16) {const i32 angle=1800-static_cast<i32>(i)*1800/15;
            x=floor_q10(sine(angle+900)*27*unit);y=-height+27*unit-floor_q10(sine(angle)*27*unit);}
        else if(i<21) {x=27*unit;y=-height+27*unit+static_cast<i32>(i-15)*(2*height-27*unit)/5;}
        else if(i<27) {x=27*unit-static_cast<i32>(i-20)*54*unit/6;y=height;}
        else {x=-27*unit;y=height-static_cast<i32>(i-26)*(2*height-27*unit)/5;}
        f.paths[p].points[i]={(x+floor_q10(sine(y/20)*lean))/2,y/2};
    }
    key(f,p);
}
}
void display_reset(DisplayState& state) {state=DisplayState();}
void display(DisplayState& s,const opxy::draw::Input& in,Frame& f) {
    using namespace opxy::draw;
    const bool snapshot=!s.initialized || s.track!=in.track;
    const bool held=in.held_notes>0 && in.held_notes<=128;
    const bool hit=!snapshot && in.track<8 && s.serial!=in.note_serial;
    s.notes.update(in);
    const i32 dt=clamp(in.elapsed,0,102);
    i32 target[4];for(unsigned i=0;i<4;++i)target[i]=in.slots[i]<0?0:in.slots[i]>>16;
    if(snapshot || in.reduced_motion) {
        s.phase=0;s.pulse=settle;s.knob_age=settle;s.energy=held?unit:0;
        for(unsigned i=0;i<4;++i)s.values[i]=s.from[i]=s.target[i]=target[i];
    } else {
        bool changed=false;for(unsigned i=0;i<4;++i)changed=changed || target[i]!=s.target[i];
        if(changed){for(unsigned i=0;i<4;++i){s.from[i]=s.values[i];s.target[i]=target[i];}s.knob_age=0;}
        s.knob_age=clamp(s.knob_age+dt,0,settle);
        const i32 mix=ease(s.knob_age);
        for(unsigned i=0;i<4;++i)s.values[i]=s.from[i]+(s.target[i]-s.from[i])*mix/unit;
        s.pulse=hit?0:clamp(s.pulse+dt,0,settle);
        s.energy=held?unit:clamp(s.energy-dt*unit/settle,0,unit);
        if(held)s.phase=(s.phase+static_cast<u32>(dt)*2u)%3600u;
    }
    for(unsigned i=0;i<Frame::node_count;++i)hide(f,i);
    for(unsigned i=0;i<4;++i)header_amount(f.header[i],header_capacity,in.slots[i]);
    const i32 a=s.values[0],b=s.values[1],c=s.values[2],d=s.values[3];
    const i32 phase=static_cast<i32>(s.phase);
    const i32 pulse=unit-ease(s.pulse),energy=s.energy;
SCENE
    s.initialized=1;s.track=in.track;s.serial=in.note_serial;
}
}
'''

SCENES = {
'analog': r'''
    // Two phase lenses cross through a tightly spaced interference engraving.
    for(unsigned p=0;p<3;++p) {
        capsule(f,p,(110-static_cast<i32>(p)*10)*unit,(76-static_cast<i32>(p)*10)*unit,phase+static_cast<i32>(p)*600,(2+b/6000)*unit);
        place(f,p,240*unit,123*unit,2*unit,4,stroke);
    }
    for(unsigned i=0;i<21;++i) {
        const i32 at=static_cast<i32>(i)-10;
        const i32 offset=floor_q10(sine(phase+at*100)*(energy*3));
        place(f,3+i,240*unit,(123+at*5)*unit+offset,unit,4,stroke);
        turn(f,3+i,rotation((a-16384)/1200+sine(phase+at*80)/70));
    }
    (void)c;(void)d;(void)pulse;
''',
'waves': r'''
    // Contours behave as one continuous elastic surface.
    for(unsigned p=0;p<3;++p) {
        ribbon(f,p,123*unit,phase+c/10, (55+b/3000)*unit,false);
        place(f,p,(110+static_cast<i32>(p)*130)*unit,123*unit,2*unit,4,stroke);
    }
    for(unsigned i=0;i<15;++i) {
        const i32 at=static_cast<i32>(i);
        place(f,3+i,240*unit,(49+at*10)*unit+floor_q10(sine(phase+at*100)*(energy*2)),unit,4,stroke);
        turn(f,3+i,rotation(sine(phase+at*80)/100+(a-16384)/1800));
    }
    (void)d;(void)pulse;
''',
'chords': r'''
    // Five nested apertures breathe independently, like a harmonic sculpture.
    for(unsigned i=0;i<5;++i) {
        const i32 voice=static_cast<i32>(i),order=(voice+b/6554)%5;
        const i32 breathe=floor_q10(sine(phase+order*720)*(energy*7));
        const i32 root=i==2?0:d/1800;
        const i32 height=(61+a/3277+order*2-root)*unit+breathe+pulse*4;
        const i32 centre=(74+voice*83)*unit+(voice-2)*(b-16384)/5;
        if(i<3) {arch(f,i,height,(c/4000)*unit);place(f,i,centre,123*unit,2*unit,4,stroke);}
        else place(f,3+i-3,centre,123*unit,height/72,4,stroke);
        for(unsigned ring=0;ring<3;++ring) {
            const unsigned node=5+i*3+ring;
            place(f,node,centre,123*unit,clamp(800-static_cast<i32>(ring)*170+breathe/26+a/160-root*12,140,1080),4,stroke);
        }
    }
    (void)a;(void)d;
''',
'grain': r'''
    // Ordered particles bend into a turbulent interference field.
    for(unsigned p=0;p<3;++p) {
        capsule(f,p,(96-static_cast<i32>(p)*5)*unit,(76-static_cast<i32>(p)*12)*unit,phase,(3+c/3000)*unit);
        place(f,p,(130+static_cast<i32>(p)*110)*unit,123*unit,2*unit,4,stroke);
    }
    for(unsigned i=0;i<21;++i) {
        const i32 at=static_cast<i32>(i);
        const i32 drift=floor_q10(sine(phase+at*170)*((3+a/4000)*unit));
        place(f,3+i,(50+at*19)*unit,123*unit+drift,unit+pulse/30+b/200,4,fill_and_stroke);
    }
    (void)d;(void)energy;
''',
'shaper': r'''
    // An accordion surface, drawn as one continuous folded sheet.
    for(unsigned p=0;p<3;++p) {
        ribbon(f,p,123*unit,phase,(48+b/2500+d/9000)*unit+floor_q10(sine(phase)*energy)/130+pulse*3,true);
        place(f,p,(110+static_cast<i32>(p)*130)*unit,123*unit+floor_q10(sine(phase)*energy)/150,2*unit,4,fill_and_stroke);
    }
    for(unsigned i=0;i<16;++i) {
        const i32 at=static_cast<i32>(i);
        place(f,3+i,(48+at*25)*unit,123*unit,unit,4,stroke);
        turn(f,3+i,rotation(sine(phase+at*180)*(120+a/500)/unit+(c-16384)/400));
    }
    (void)d;(void)pulse;
''',
}

def path(points, closed=False):
    value = ' '.join(('M' if i == 0 else 'L') + f'{round(x)} {round(y)}'
                     for i, (x, y) in enumerate(points))
    return value + (' Z' if closed else '')


def scene_shapes():
    # Authored startup geometry uses integer polylines and full-pixel strokes.
    # Runtime mutable geometry and animation remain in the drawing callback.
    shapes = {short: {} for short in NAMES}
    curve = []
    for controls in [((-190,-18),(-110,-70),(-65,70),(0,0)),
                     ((0,0),(65,-70),(110,70),(190,18))]:
        for j in range(17):
            if curve and j == 0:
                continue
            t = j / 16
            weights = ((1-t)**3, 3*(1-t)**2*t, 3*(1-t)*t*t, t**3)
            curve.append(tuple(sum(w*p[axis] for w,p in zip(weights,controls)) for axis in (0,1)))
    for i in range(21):
        shapes['analog'][3+i] = (path(curve), 'none')
    for i in range(15):
        contour = [(-192+j*12, 9*math.sin(j/32*math.tau*1.5+i*.18)+4*math.sin(j/32*math.tau*3))
                   for j in range(33)]
        shapes['waves'][3+i] = (path(contour), 'none')
    capsule = [(-26,65),(-26,-40)]
    for controls in [((-26,-40),(-26,-70),(0,-70)), ((0,-70),(26,-70),(26,-40))]:
        for j in range(1,9):
            t=j/8
            capsule.append(tuple((1-t)**2*controls[0][axis]+2*(1-t)*t*controls[1][axis]+t*t*controls[2][axis]
                                 for axis in (0,1)))
    capsule.append((26,65))
    for i in range(3,20):
        shapes['chords'][i] = (path(capsule,True), '#000000')
    for i in range(21):
        particles = []
        for j in range(11):
            y=-65+j*13; x=round(math.sin(i*.5+j*.9)*4)
            particles.append(path([(x-2,y),(x,y-2),(x+2,y),(x,y+2)],True))
        shapes['grain'][3+i] = (' '.join(particles), '#eeeeee')
    for i in range(16):
        shapes['shaper'][3+i] = ('M-2 -35 L2 35', 'none')
    return shapes

SHAPES = scene_shapes()

def generate(short, name, artwork_only=False):
    mid='weave_'+short
    root=ROOT/mid
    for colour in PALETTE + [fill for _, fill in SHAPES[short].values()]:
        if colour != 'none' and not (colour[1:3] == colour[3:5] == colour[5:7]):
            raise ValueError('Display artwork must remain monochrome')
    nodes={}
    for n in range(3,24):
        d,fill=SHAPES[short].get(n,('M-1 -1 L1 -1 L1 1 L-1 1 Z','#000000'))
        nodes[n]=f'<path id="{mid}_node_{n}" d="{d}" fill="{fill}" stroke="#eeeeee" stroke-width="1" stroke-linejoin="round" stroke-linecap="round"/>'
    paths=[]
    points=[(round(56*math.cos(i*math.tau/32)),round(47*math.sin(i*math.tau/32))) for i in range(32)]
    d=path(points,True)
    paths += [nodes[n] for n in range(3,12)]
    paths += [f'<path id="{mid}_node_{p}" d="{d}" fill="none" stroke="#eeeeee" stroke-width="1"/>' for p in range(3)]
    paths += [nodes[n] for n in range(12,24)]
    for i,colour in enumerate(PALETTE):
        paths.append(f'<circle id="{mid}_palette_{i}" cx="640" cy="0" r="1" fill="{colour}" stroke="{colour}" stroke-width="1"/>')
    (root/'panel.svg.fragment').write_text('<g id="'+mid+'_panel">\n'+'\n'.join(paths)+'\n</g>\n')
    if not artwork_only:
        (root/'display.cpp').write_text(COMMON.replace('ID',mid).replace('SCENE',SCENES[short]))

if __name__=='__main__':
    for short,name in NAMES.items():generate(short,name)
