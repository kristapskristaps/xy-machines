"""Generate abstract geometric native scenes; audio and controls are unchanged."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
NAMES = {'analog': 'Tandem', 'waves': 'Atlas', 'chords': 'Choir', 'grain': 'Sift', 'shaper': 'Crease'}
PALETTE = ['#080808', '#242424', '#505050', '#808080', '#c6c6c6', '#ec4237', '#edb6b0', '#eeeeee']

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
inline void orbit(Frame& f,unsigned p,i32 lobes,i32 phase,i32 depth) {
    for(unsigned i=0;i<32;++i) {
        const i32 angle=static_cast<i32>(i)*3600/32;
        const i32 radius=51*unit+floor_q10(sine(angle*lobes+phase)*depth);
        f.paths[p].points[i]={floor_q10(sine(angle+900)*radius),floor_q10(sine(angle)*radius)};
    }
    key(f,p);
}
inline void plane(Frame& f,unsigned p,i32 height,i32 shear,i32 tilt) {
    const opxy::draw::Point corners[4]={{-56*unit+shear,-height},{56*unit+shear,-height+tilt},
                            {56*unit-shear,height},{-56*unit-shear,height-tilt}};
    for(unsigned edge=0;edge<4;++edge)for(unsigned i=0;i<8;++i) {
        const auto& a=corners[edge];const auto& b=corners[(edge+1)%4];
        f.paths[p].points[edge*8+i]={a.x+(b.x-a.x)*static_cast<i32>(i)/8,
                                    a.y+(b.y-a.y)*static_cast<i32>(i)/8};
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
    const i32 left=132*unit+pulse*9,right=348*unit-pulse*9;
    for(unsigned p=0;p<3;++p) {
        orbit(f,p,2+a/8192,phase*(p==1?-1:1)+c/12+(p?900:0),(1+b/5000)*unit);
        place(f,p,p==0?left:p==1?right:240*unit,123*unit,p==2?1050:1450,p==2?5:4,stroke);
    }
    place(f,3,left,123*unit,1450,1,fill_and_stroke);
    place(f,4,right,123*unit,1450,2,fill_and_stroke);
    for(unsigned i=0;i<8;++i) {
        const bool second=i>=4;
        place(f,5+i,second?right:left,123*unit,1350,i%2?3:4,fill_and_stroke);
        turn(f,5+i,rotation(static_cast<i32>(i%4)*450+(second?-phase-c/14:phase)));
    }
    place(f,13,(218+d/750)*unit,123*unit,1100+energy/5,5,fill_and_stroke);
    turn(f,13,rotation(phase+a/30));
''',
'waves': r'''
    for(unsigned i=0;i<3;++i) {
        const i32 step=static_cast<i32>(i);
        plane(f,i,47*unit,(b/6500-2)*unit,(c/5000-3)*unit);
        place(f,i,(142+step*98)*unit+(b-16384)/5,123*unit+pulse*3,1350,i==1?5:4,i==0?fill_and_stroke:stroke);
        turn(f,i,rotation(-80+step*80+a/600+sine(phase)/40));
    }
    for(unsigned i=0;i<6;++i) {
        const i32 step=static_cast<i32>(i);
        place(f,3+i,(91+step*59)*unit,(123-step*2)*unit,1350,i%2?2:1,fill_and_stroke);
        turn(f,3+i,rotation(-130+step*80+sine(phase)/30));
    }
    place(f,12,(71+d/100)*unit,(66+c/700)*unit,1050+energy/8,5,fill_and_stroke);
    turn(f,12,rotation(450));
''',
'chords': r'''
    for(unsigned i=0;i<5;++i) {
        const i32 at=static_cast<i32>(i),voice=(at+b/6554)%5;
        const i32 age=s.pulse-at*12;
        const i32 kick=in.reduced_motion || age<0 || s.pulse>=settle?0:unit-ease(age*settle/(settle-at*12));
        place(f,3+i,(72+at*84)*unit,(123+(voice-2)*3)*unit,1190+kick/6,i==2 && d>16384?5:(i%2?2:4),fill_and_stroke);
        turn(f,3+i,rotation((at-2)*90+(a-16384)/180+sine(phase)*(i%2?-1:1)/12));
    }
    for(unsigned i=0;i<3;++i) {
        orbit(f,i,5,phase+static_cast<i32>(i)*420,(3+c/9000)*unit);
        place(f,i,240*unit,122*unit,1430-static_cast<i32>(i)*210,i==0?5:3,stroke);
        turn(f,i,rotation(sine(phase)/12));
    }
    place(f,14,240*unit,122*unit,900+d/100+energy/7+pulse/16,5,fill_and_stroke);
    turn(f,14,rotation(phase));
''',
'grain': r'''
    for(unsigned p=0;p<3;++p) {
        plane(f,p,12*unit,(a/6500-2)*unit,(c/6500-2)*unit);
        place(f,p,(132+static_cast<i32>(p)*108)*unit,(76+static_cast<i32>(p)*46)*unit,1300,p==1?5:3,stroke);
        turn(f,p,rotation(-180+sine(phase)/22));
    }
    for(unsigned i=0;i<18;++i) {
        const i32 column=static_cast<i32>(i%6),row=static_cast<i32>(i/6);
        const i32 drift=floor_q10(sine(phase+column*450+row*900)*(energy*5));
        const i32 colour=i==(static_cast<unsigned>(d)*17u/32767u)?5:(i%3?4:2);
        place(f,3+i,(68+column*69)*unit+drift,(66+row*58)*unit,840+b/150+pulse/12,colour,colour==5 || i%3==0?fill_and_stroke:stroke);
        turn(f,3+i,rotation((column+row)%2?c/90:-c/90));
    }
''',
'shaper': r'''
    const i32 height=(42+b/5000)*unit;
    for(unsigned p=0;p<3;++p) {
        const i32 step=static_cast<i32>(p);
        plane(f,p,height,(c/7000-2)*unit,(a/3200-5)*unit);
        place(f,p,(112+step*128)*unit,123*unit,1380+pulse/18,p==1?5:(p==0?2:4),p==1?stroke:fill_and_stroke);
        turn(f,p,rotation((step-1)*(40+c/400)+sine(phase)*energy/(unit*30)));
    }
    for(unsigned i=0;i<8;++i) {
        const i32 at=static_cast<i32>(i);
        place(f,12+i,(43+at*56)*unit,123*unit,1300,(i==static_cast<unsigned>(d)/4096u)?5:3,stroke);
        turn(f,12+i,rotation(i%2?120+a/180:-120-a/180));
    }
''',
}

SHAPES = {'analog': {3: ('M-55 0 A55 55 0 1 0 55 0 A55 55 0 1 0 -55 0 Z', '#242424'),
            4: ('M-55 0 A55 55 0 1 0 55 0 A55 55 0 1 0 -55 0 Z', '#505050'),
            5: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            6: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            7: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            8: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            9: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            10: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            11: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            12: ('M-1 -50 L1 -50 L1 50 L-1 50 Z', '#c6c6c6'),
            13: ('M-18 -23 L23 0 L-18 23 Z', '#ec4237')},
 'waves': {3: ('M-35 -38 L35 -38 L35 38 L-35 38 Z', '#242424'),
           4: ('M-35 -38 L35 -38 L35 38 L-35 38 Z', '#242424'),
           5: ('M-35 -38 L35 -38 L35 38 L-35 38 Z', '#242424'),
           6: ('M-35 -38 L35 -38 L35 38 L-35 38 Z', '#242424'),
           7: ('M-35 -38 L35 -38 L35 38 L-35 38 Z', '#242424'),
           8: ('M-35 -38 L35 -38 L35 38 L-35 38 Z', '#242424'),
           12: ('M-10 -10 L10 -10 L10 10 L-10 10 Z', '#ec4237')},
 'chords': {3: ('M0 -54 L30 0 L0 54 L-30 0 Z', '#080808'),
            4: ('M0 -54 L30 0 L0 54 L-30 0 Z', '#080808'),
            5: ('M0 -54 L30 0 L0 54 L-30 0 Z', '#080808'),
            6: ('M0 -54 L30 0 L0 54 L-30 0 Z', '#080808'),
            7: ('M0 -54 L30 0 L0 54 L-30 0 Z', '#080808'),
            14: ('M0 -14 L14 -4 L9 12 L-9 12 L-14 -4 Z', '#ec4237')},
 'grain': {3: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           4: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           5: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           6: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           7: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           8: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           9: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           10: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           11: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           12: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           13: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           14: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           15: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           16: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           17: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           18: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           19: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808'),
           20: ('M-24 -20 L24 -20 L24 20 L-24 20 Z M-24 0 L24 0 M0 -20 L0 20', '#080808')},
 'shaper': {12: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            13: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            14: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            15: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            16: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            17: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            18: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808'),
            19: ('M0 -49 L0 49 M-8 -42 L0 -49 L8 -42 M-8 42 L0 49 L8 42', '#080808')}}

def generate(short, name):
    mid='weave_'+short
    root=ROOT/mid
    nodes={}
    for n in range(3,24):
        d,fill=SHAPES[short].get(n,('M-1 -1 L1 -1 L1 1 L-1 1 Z','#080808'))
        nodes[n]=f'<path id="{mid}_node_{n}" d="{d}" fill="{fill}" stroke="#c6c6c6" stroke-width="1.6" stroke-linejoin="round" stroke-linecap="round"/>'
    paths=[]
    corners=[(-56,-47),(56,-47),(56,47),(-56,47)]
    points=[]
    for i,a in enumerate(corners):
        b=corners[(i+1)%4]
        points.extend((a[0]+(b[0]-a[0])*j/8,a[1]+(b[1]-a[1])*j/8) for j in range(8))
    d=' '.join(('M' if i==0 else 'L')+f'{x:g} {y:g}' for i,(x,y) in enumerate(points))+' Z'
    paths += [nodes[n] for n in range(3,12)]
    paths += [f'<path id="{mid}_node_{p}" d="{d}" fill="none" stroke="#c6c6c6" stroke-width="1.6"/>' for p in range(3)]
    paths += [nodes[n] for n in range(12,24)]
    for i,colour in enumerate(PALETTE):
        paths.append(f'<circle id="{mid}_palette_{i}" cx="640" cy="0" r="1" fill="{colour}" stroke="{colour}" stroke-width="1"/>')
    (root/'panel.svg.fragment').write_text('<g id="'+mid+'_panel">\n'+'\n'.join(paths)+'\n</g>\n')
    (root/'display.cpp').write_text(COMMON.replace('ID',mid).replace('SCENE',SCENES[short]))

if __name__=='__main__':
    for short,name in NAMES.items():generate(short,name)
