"""Author original retained vector scenes for the five standalone models."""
from pathlib import Path
import json

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
void ribbon(Frame& f,unsigned p,i32 amplitude,i32 cycles,i32 phase,i32 skew=0) {
    auto& path=f.paths[p];
    for(unsigned i=0;i<16;++i) {
        const i32 x=-54*unit+static_cast<i32>(i)*108*unit/15;
        const i32 y=floor_q10(sine(static_cast<i32>(i)*3600*cycles/15+phase)*amplitude)+x*skew/32768;
        path.points[i]={x,y-768};path.points[31-i]={x,y+768};
    }
    u32 hash=2166136261u;
    for(const auto& point:path.points)hash=((hash^static_cast<u32>(point.x))*16777619u^static_cast<u32>(point.y))*16777619u;
    path.key=hash?hash:1;
}
void oval(Frame& f,unsigned p,i32 radius,i32 phase,i32 depth) {
    auto& path=f.paths[p];u32 hash=2166136261u;
    for(unsigned i=0;i<32;++i) {
        const i32 angle=static_cast<i32>(i)*3600/32;
        const i32 r=radius+floor_q10(sine(angle*3+phase)*depth);
        path.points[i]={floor_q10(sine(angle+900)*r),floor_q10(sine(angle)*r)/2};
        hash=(hash^static_cast<u32>(path.points[i].x))*16777619u;
        hash=(hash^static_cast<u32>(path.points[i].y))*16777619u;
    }
    path.key=hash?hash:1;
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
    ribbon(f,0,(8+b/2048)*unit,1+a/10923,phase);
    ribbon(f,1,(8+b/2048)*unit,1+a/10923,phase+c/12);
    ribbon(f,2,(4+energy/160)*unit,2,phase);
    place(f,0,132*unit,126*unit,unit,4,stroke);
    place(f,1,348*unit,126*unit,unit,4,stroke);
    place(f,2,240*unit,183*unit,unit,5,stroke);
    for(unsigned i=0;i<2;++i) {
        place(f,3+i,(i?348:132)*unit,(73*unit+pulse*4),unit,4,stroke);
        turn(f,3+i,rotation(i?-phase-c/20:phase));
    }
    place(f,5,(206+c/500)*unit,75*unit,unit,5,fill_and_stroke);
    place(f,6,240*unit,(153+d/2048)*unit,unit,held?5:2,fill_and_stroke);
''',
'waves': r'''
    for(unsigned i=0;i<3;++i) {
        oval(f,i,(50-static_cast<i32>(i)*10)*unit,phase+static_cast<i32>(i)*420,(2+c/4096)*unit);
        place(f,i,(218+static_cast<i32>(i)*12)*unit,(119-static_cast<i32>(i)*10)*unit,unit,i==0?5:3,stroke);
        turn(f,i,rotation(-180+a/160));
    }
    for(unsigned i=0;i<4;++i) {
        place(f,3+i,(162+static_cast<i32>(i)*35)*unit,(145-static_cast<i32>(i)*16)*unit+pulse*3,unit,i==static_cast<unsigned>(a)/8192u?4:2,stroke);
        turn(f,3+i,rotation(-180));
    }
    place(f,7,(170+b/260)*unit,(143-c/450)*unit,unit,5,fill_and_stroke);
    place(f,8,(184+d/350)*unit,185*unit,unit,4,stroke);
''',
'chords': r'''
    for(unsigned i=0;i<5;++i) {
        const i32 at=static_cast<i32>(i);
        const i32 register_step=(at+ b/6554)%5;
        const i32 height=700+register_step*95;
        const i32 age=s.pulse-at*12;
        const i32 delay=clamp(age*settle/(settle-at*12),0,settle);
        const i32 press=in.reduced_motion || age<0 || s.pulse>=settle?0:(unit-ease(delay))*5;
        place(f,3+i,(148+at*46)*unit,129*unit+press,height,i==2 && d>16384?5:4,fill_and_stroke);
        place(f,8+i,(148+at*46)*unit,179*unit+press/2,unit,i==2?5:3,fill_and_stroke);
    }
    for(unsigned i=0;i<3;++i) {
        ribbon(f,i,(4+c/5000+energy/256)*unit,1+(a/8192+static_cast<i32>(i))%4,phase+static_cast<i32>(i)*500);
        place(f,i,240*unit,(72+static_cast<i32>(i)*12)*unit,768,i==0?5:3,stroke);
    }
    place(f,13,(132+a/150)*unit,196*unit,unit,5,fill_and_stroke);
''',
'grain': r'''
    for(unsigned i=0;i<3;++i) {
        oval(f,i,(22+static_cast<i32>(i)*9)*unit,phase,(1+c/8192)*unit);
        place(f,i,240*unit,(176+static_cast<i32>(i)*6)*unit,unit,i==0?5:2,stroke);
    }
    for(unsigned i=0;i<12;++i) {
        const i32 at=(phase+static_cast<i32>(i)*300)%3600;
        const i32 y=74+at*92/3600;
        const i32 width=y<120?120-y:y-120;
        const i32 offset=(static_cast<i32>((i*37)%101)-50)*width/55;
        if(!held && energy==0)continue;
        place(f,3+i,(240+offset)*unit,y*unit,500+b/64,i%3?4:5,fill_and_stroke);
        turn(f,3+i,rotation(static_cast<i32>(i)*310+c/80));
    }
    place(f,15,(220+a/800)*unit,122*unit,unit,5,stroke);
    place(f,16,(185+d/300)*unit,202*unit,unit,4,stroke);
''',
'shaper': r'''
    ribbon(f,0,11*unit,1,phase);
    ribbon(f,1,(6+b/2400)*unit,1+a/8192,phase,c/4-4096);
    ribbon(f,2,(7+c/2800)*unit,1+a/8192,phase);
    place(f,0,105*unit,132*unit,unit,3,stroke);
    place(f,1,240*unit,132*unit,unit,5,stroke);
    place(f,2,375*unit,132*unit,unit,4,stroke);
    place(f,3,240*unit,(91+b/4096)*unit+pulse*9,unit,4,fill_and_stroke);
    place(f,4,240*unit,(173-b/4096)*unit-pulse*9,unit,4,fill_and_stroke);
    place(f,5,240*unit,62*unit,unit,4,stroke);turn(f,5,rotation(phase+a/30));
    place(f,6,(339+d/450)*unit,183*unit,unit,5,fill_and_stroke);
'''
}


def generate(short, name):
    mid='weave_'+short
    root=ROOT/mid
    paths=[]
    for p in range(3):
        points=[(-54+i*108/15, -1) for i in range(16)]+[(54-i*108/15,1) for i in range(16)]
        d=' '.join(('M' if i==0 else 'L')+f'{x:g} {y:g}' for i,(x,y) in enumerate(points))+' Z'
        paths.append(f'<path id="{mid}_node_{p}" d="{d}" fill="none" stroke="#c6c6c6" stroke-width=".8"/>')
    def node(n,d,fill='none'):
        paths.append(f'<path id="{mid}_node_{n}" d="{d}" fill="{fill}" stroke="#c6c6c6" stroke-width=".8" stroke-linejoin="round" stroke-linecap="round"/>')
    def static(d,colour='#505050',width='.7'):
        paths.append(f'<path d="{d}" fill="none" stroke="{colour}" stroke-width="{width}" stroke-linejoin="round" stroke-linecap="round"/>')
    if short=='analog':
        static('M66 99 L66 152 L198 152 L198 99 Z M282 99 L282 152 L414 152 L414 99 Z')
        static('M198 125 L223 125 L223 163 M282 125 L257 125 L257 163 M190 183 L66 183 M290 183 L414 183')
        static('M132 93 L132 80 M348 93 L348 80 M181 75 L299 75','#808080')
        for n in (3,4):node(n,'M-14 0 A14 14 0 1 0 14 0 A14 14 0 1 0 -14 0 M-10 0 L10 0 M0 -10 L0 10')
        node(5,'M-3 -5 L3 -5 L3 5 L-3 5 Z','#ec4237');node(6,'M-5 -5 L5 -5 L5 5 L-5 5 Z')
    elif short=='waves':
        for n in range(3,7):node(n,'M-50 -25 L50 -25 L50 25 L-50 25 Z M-42 17 L-25 17')
        node(7,'M-4 0 L0 -4 L4 0 L0 4 Z','#ec4237');node(8,'M-4 -3 L4 -3 L4 3 L-4 3 Z')
        static('M112 166 L112 175 L365 175 L365 166 M181 185 L306 185')
        static('M98 151 L111 151 M98 132 L111 132 M98 113 L111 113')
    elif short=='chords':
        static('M128 171 L352 171 M128 186 L352 186 M130 196 L350 196','#505050')
        for n in range(3,8):node(n,'M-7 -34 L7 -34 L7 27 L3 34 L-3 34 L-7 27 Z M-3 -24 L3 -24 M-3 -19 L3 -19','#242424')
        for n in range(8,13):node(n,'M-13 -4 L13 -4 L13 4 L-13 4 Z','#242424')
        node(13,'M-3 -3 L3 -3 L3 3 L-3 3 Z','#ec4237')
    elif short=='grain':
        static('M172 64 L308 64 M181 69 L216 107 L216 126 L181 166 M299 69 L264 107 L264 126 L299 166 M172 170 L308 170','#c6c6c6','1.1')
        static('M190 66 L219 95 M290 66 L261 95 M205 202 L300 202')
        for n in range(3,15):node(n,'M-2 -2 L2 -2 L2 2 L-2 2 Z','#c6c6c6')
        node(15,'M-8 0 L8 0 M0 -5 L0 5');node(16,'M-3 -3 L3 -3 L3 3 L-3 3 Z')
    else:
        static('M178 66 L178 192 L302 192 L302 66 M164 111 L177 111 M303 111 L316 111 M61 183 L153 183 M329 183 L420 183')
        node(3,'M-45 -5 L45 -5 L45 5 L12 5 L0 15 L-12 5 L-45 5 Z','#242424')
        node(4,'M-45 -5 L-12 -5 L0 -15 L12 -5 L45 -5 L45 5 L-45 5 Z','#242424')
        node(5,'M-12 0 A12 12 0 1 0 12 0 A12 12 0 1 0 -12 0 M-17 0 L17 0 M0 -17 L0 17')
        node(6,'M-3 -3 L3 -3 L3 3 L-3 3 Z','#ec4237')
    bound={int(part.split('_node_')[1].split('"')[0]) for part in paths if '_node_' in part}
    for n in range(3,24):
        if n not in bound:node(n,'M-1 -1 L1 -1 L1 1 L-1 1 Z')
    for i,colour in enumerate(PALETTE):
        paths.append(f'<circle id="{mid}_palette_{i}" cx="640" cy="0" r="1" fill="{colour}" stroke="{colour}" stroke-width="1"/>')
    paths.append(f'<text x="26" y="206" fill="#808080" font-family="UniversNextPro-Light" font-size="8px">{name.upper()}</text>')
    (root/'panel.svg.fragment').write_text('<g id="'+mid+'_panel">\n'+'\n'.join(paths)+'\n</g>\n')
    (root/'display.h').write_text('''#pragma once
#include <opxy/draw.hpp>
namespace ID {
using Frame=opxy::draw::Frame<3,32,24,0,8>;
struct DisplayState {
    opxy::draw::NoteTracker notes;
    opxy::u32 initialized,track,serial,phase;
    opxy::i32 pulse,knob_age,energy,values[4],from[4],target[4];
};
void display_reset(DisplayState&);
void display(DisplayState&,const opxy::draw::Input&,Frame&);
}
'''.replace('ID',mid))
    helpers=COMMON
    if short in ('analog','chords','shaper'):
        start=helpers.index('void oval(');end=helpers.index('\n}\nvoid display_reset',start)
        helpers=helpers[:start]+helpers[end:]
    if short in ('waves','grain'):
        start=helpers.index('void ribbon(');end=helpers.index('void oval(',start)
        helpers=helpers[:start]+helpers[end:]
    scene=SCENES[short]
    # Keep only each scene's used control/event temporaries.
    if short=='waves':helpers=helpers.replace('const i32 pulse=unit-ease(s.pulse),energy=s.energy;','const i32 pulse=unit-ease(s.pulse);')
    if short=='grain':helpers=helpers.replace('const i32 pulse=unit-ease(s.pulse),energy=s.energy;','const i32 energy=s.energy;')
    if short=='shaper':helpers=helpers.replace('const i32 pulse=unit-ease(s.pulse),energy=s.energy;','const i32 pulse=unit-ease(s.pulse);')
    if short=='chords':helpers=helpers.replace('const i32 pulse=unit-ease(s.pulse),energy=s.energy;','const i32 energy=s.energy;')
    (root/'display.cpp').write_text(helpers.replace('ID',mid).replace('SCENE',scene))
    definition=json.loads((root/'definition.json').read_text());definition['name']=name
    (root/'definition.json').write_text(json.dumps(definition,indent=2)+'\n')


if __name__=='__main__':
    for short,name in NAMES.items():generate(short,name)
