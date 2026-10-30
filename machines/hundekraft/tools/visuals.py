#!/usr/bin/env python3
"""Reconstruct Hundekraft screens as portable OP-XY retained artwork and C++ motion."""
from pathlib import Path
import argparse, hashlib, itertools, json, math, re, subprocess, sys, xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parents[1]/'tools'))
from workspace import sdk_path
SDK=sdk_path()
UPSTREAM=Path('/tmp/openxy-hundekraft-3.0.3-port-source/ext/kennel/synth_sdk/plugins')
sys.path.insert(0,str(SDK))
from tooling.custom_preview import page_header_artwork

COLORS=['#eeeeee']*5+['#666666']*2+['#6495df']*3+['#df584c']*3+['#e89a54']*3
WIDTHS=[1,2,4,8,16,1,4,1,4,16,1,4,16,1,4,16]
# Axis-only transforms are sampled offline; native retained nodes support uniform scaling.
AXES={
 'wavetable':{'coil':('y',.55,1.,0,53.36,9)},
 'taffy':{'spring':('y',.35,1.3,0,31.01,11)},
 'pew':{**{f'pipes{i}':('y',.45,1.,0,82.87,7) for i in range(3)},'horn':('x',-1.,1.,280.75,0,9)},
 'elg':{'aurora':('y',.4,1.5,0,14.96,11),'eyes':('y',.15,1.,0,79.55,2)},
 'oxbow':{'hornl':('x',.88,1.12,141149/1024,36508/1024,5),'hornr':('x',.88,1.12,210761/1024,39200/1024,5)},
 'eva':{'fog':('fog',.02,1.,113254/1024,103834/1024,11)},
}
BUDGETS={
 'wavetable':(32,0),'taffy':(135,0),'pew':(30,0),'elg':(0,0),'supersaw':(150,8),
 'fmbell':(64,0),'sid':(0,0),'mbira':(112,0),'oxbow':(0,0),'eva':(0,0),
 'arc':(150,0),'mixtur':(145,8),'washer':(85,0),'crush':(160,0),'tape':(120,20),
 'warble':(100,12),'plate':(140,0),'snowglobe':(0,0),'father':(85,0),'reverb':(0,0),
 'clouds':(65,4),'hail':(170,24),'stutter':(160,8),'palme':(110,20),'in_a_room':(180,8),'granular':(140,8),
}
ACCENTS={
 'wavetable':{'coil':'red'},'taffy':{'spark':'orange'},'elg':{'aurora':'blue'},
 'oxbow':{'bell':'orange'},'eva':{'strip':'red','cross':'blue'},'snowglobe':{'key':'red'},
 'reverb':{'kfire':'orange',**{f'kmix{i}':'red' for i in range(10)}},
}
ALIASES={'hkinaroom':'in_a_room'}
def num(v):
    s=f'{v:.2f}'.rstrip('0').rstrip('.')
    return '0' if s in ('-0','') else s
def color(c,accent=None):
    if c in ('none',None):return 'none'
    if not re.fullmatch('#[0-9a-fA-F]{6}',c):raise ValueError('Unexpected source paint '+c)
    rgb=[int(c[i:i+2],16) for i in (1,3,5)];light=max(rgb)
    if light<9:return '#000000'
    if accent:
        target={'red':(223,88,76),'orange':(232,154,84),'blue':(100,149,223)}[accent]
        return '#'+''.join(f'{round(v*light/255):02x}' for v in target)
    grey=max(40,min(238,round((max(rgb)*.75+sum(rgb)/12)/16)*16))
    return '#'+f'{grey:02x}'*3
def matrix(text):
    if not text:return (1,0,0,1,0,0)
    match=re.fullmatch(r'matrix\(([^)]+)\)',text)
    if not match:raise ValueError(text)
    return tuple(map(float,match[1].replace(',',' ').split()))
def compose(p,q):
    a,b,c,d,x,y=p;A,B,C,D,X,Y=q
    return a*A+c*B,b*A+d*B,a*C+c*D,b*C+d*D,a*X+c*Y+x,b*X+d*Y+y
def point(m,x,y):
    a,b,c,d,xx,yy=m;return a*x+c*y+xx,b*x+d*y+yy
def path_data(node,m):
    tag=node.tag.split('}')[-1]
    if tag=='path':
        tokens=re.findall(r'[A-Za-z]|[-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?',node.get('d',''))
        out=[];i=0;x=y=0;start=(0,0);cmd=None
        while i<len(tokens):
            if tokens[i].isalpha():cmd=tokens[i];i+=1
            if cmd in ('Z','z'):out.append('Z');x,y=start;cmd=None;continue
            count={'M':2,'L':2,'C':6,'Q':4}.get(cmd.upper() if cmd else '')
            if count is None:raise ValueError('Unsupported source path command '+str(cmd))
            values=list(map(float,tokens[i:i+count]));i+=count;pairs=[]
            for j in range(0,count,2):
                xx,yy=values[j:j+2]
                if cmd.islower():xx+=x;yy+=y
                pairs.extend(point(m,xx,yy))
            x,y=(values[-2]+x,values[-1]+y) if cmd.islower() else values[-2:]
            out.append(cmd.upper()+' '.join(num(v) for v in pairs))
            if cmd.upper()=='M':start=(x,y);cmd='l' if cmd.islower() else 'L'
        return ''.join(out)
    if tag in ('line','polyline','polygon','rect'):
        if tag=='line':p=[(float(node.get('x1','0')),float(node.get('y1','0'))),(float(node.get('x2','0')),float(node.get('y2','0')))];closed=False
        elif tag=='rect':
            x,y,w,h=[float(node.get(a,'0')) for a in ('x','y','width','height')];p=[(x,y),(x+w,y),(x+w,y+h),(x,y+h)];closed=True
        else:
            ns=list(map(float,re.findall(r'[-+]?(?:\d*\.\d+|\d+\.?\d*)',node.get('points',''))));p=list(zip(ns[::2],ns[1::2]));closed=tag=='polygon'
        return ''.join(('M' if i==0 else 'L')+' '.join(map(num,point(m,*p0))) for i,p0 in enumerate(p))+('Z' if closed else '')
    if tag=='circle':
        x,y,r=[float(node.get(a,'0')) for a in ('cx','cy','r')]
        return ''.join(('M' if i==0 else 'L')+' '.join(map(num,point(m,x+r*math.cos(i*math.tau/24),y+r*math.sin(i*math.tau/24)))) for i in range(24))+'Z'
    raise ValueError('Unexpected source element '+tag)
def source_art(name):
    record=json.loads((UPSTREAM/name/'plugin.json').read_text());path=UPSTREAM/name/record['screen']['svg']
    root=ET.parse(path).getroot()
    if name=='plate':
        # Upstream deliberately renders its vector rectangle artwork procedurally.
        text=(UPSTREAM/name/'plate_art.h').read_text()
        for x,y,xx,yy,r,g,b in re.findall(r'\{(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}',text):
            ET.SubElement(root,'rect',{'x':x,'y':str(int(y)-12),'width':str(int(xx)-int(x)),'height':str(int(yy)-int(y)),
                'fill':f'#{int(r):02x}{int(g):02x}{int(b):02x}','stroke':'none'})
    return path,root
def artwork(name,ident):
    source,root=source_art(name);group_names=[x.get('id') for x in root.iter() if x.get('id') and x is not root]
    groups={v:i for i,v in enumerate(group_names)};axes=AXES.get(name,{})
    records=[]
    def walk(node,parents=(),style=None,m=None):
        tag=node.tag.split('}')[-1];style=dict(style or {'fill':'none','stroke':'none','stroke-width':'1'})
        style.update({k:v for k,v in node.attrib.items() if k in style})
        m=compose(m or matrix(None),matrix(node.get('transform')))
        ident0=node.get('id')
        if ident0 in groups:parents=parents+(ident0,)
        if tag in ('svg','g'):
            for child in node:walk(child,parents,style,m)
            return
        if ident0 and re.fullmatch('bar[0-3]',ident0) and float(node.get('y1','999'))<3:return
        # The OP-XY header replaces the original alternate-page label swaps.
        if name=='eva' and any(p in ('beep0','beep1','beep2') for p in parents):return
        if len(parents)>4:raise ValueError('Source group nesting exceeds retained mapping')
        accent=next((ACCENTS.get(name,{}).get(p) for p in reversed(parents) if p in ACCENTS.get(name,{})),None)
        fill,stroke=color(style['fill'],accent),color(style['stroke'],accent)
        width=round(float(style['stroke-width'])*4)/4
        axis=next((p for p in parents if p in axes),None)
        variations=axes[axis][-1] if axis else 1
        for v in range(variations):
            mm=m
            if axis:
                dim,low,high,x,y,count=axes[axis];factor=low+(high-low)*v/(count-1)
                sx,sy=(factor,1) if dim=='x' else (1,factor) if dim=='y' else (.15+.85*factor,factor)
                mm=compose((sx,0,0,sy,x*(1-sx),y*(1-sy)),m)
            records.append(dict(parents=parents,variantGroup=groups[axis] if axis else -1,variant=v,
                fill=fill,stroke=stroke,width=width,d=path_data(node,mm)))
    walk(root)
    # Alternate geometry copies are mutually exclusive. Keep each variant's
    # source painter order together, then coalesce stroked leaves within a layer.
    ordered=[]
    for _,run in itertools.groupby(records,key=lambda r:(r['parents'],r['variantGroup'])):
        run=list(run)
        for variant in sorted(set(r['variant'] for r in run)):
            selected=[r for r in run if r['variant']==variant]
            if selected[0]['variantGroup']>=0 or name in ('plate','warble','crush','sid'):
                by_style={}
                for r in selected:
                    key=(r['fill'],r['stroke'],r['width'])
                    if key not in by_style:by_style[key]=dict(r)
                    else:by_style[key]['d']+=r['d']
                ordered.extend(by_style.values());continue
            for filled,layer in itertools.groupby(selected,key=lambda r:r['fill']!='none'):
                layer=list(layer)
                if filled:ordered.extend(layer);continue
                by_style={}
                for r in layer:
                    key=(r['stroke'],r['width'])
                    if key not in by_style:by_style[key]=dict(r)
                    else:by_style[key]['d']+=r['d']
                ordered.extend(by_style.values())
    merged=[]
    for r in ordered:
        key=tuple((k,str(v)) for k,v in r.items() if k!='d')
        if merged and merged[-1][0]==key:merged[-1][1]['d']+=r['d']
        else:merged.append((key,r))
    return source,groups,[r for _,r in merged]

def build(name):
    ident='hk'+name.replace('_','');folder=ROOT/ident
    record=json.loads((folder/'machine.json').read_text());definition=json.loads((folder/'definition.json').read_text());synth=record['kind']=='synth'
    if name=='eva':definition['parameters'][4]['control']=3
    dest=folder if synth else folder/'visual';dest.mkdir(exist_ok=True)
    source,groups,pieces=artwork(name,ident);lines,circles=BUDGETS[name]
    paged=synth and len(definition['parameters'])>4
    headers=8 if paged else 0;texts=16 if paged else 8 if name in ('sid','crush','warble','plate','father','in_a_room','mixtur') else 0
    count=len(pieces)+lines+circles+headers
    if count>256:raise ValueError(f'{name}: {count} nodes ({len(pieces)} art + {lines} lines + {circles} circles)')
    art=[f'<g id="{ident}_panel">']
    for i,p in enumerate(pieces):
        coords=list(map(float,re.findall(r'[-+]?(?:\d*\.\d+|\d+\.?\d*)',p['d'])))
        cx=round((min(coords[::2])+max(coords[::2]))/2,2);cy=round((min(coords[1::2])+max(coords[1::2]))/2,2)
        p['cx'],p['cy']=round(cx*1024),round(cy*1024)
        d=path_data(ET.Element('path',{'d':p['d']}),(1,0,0,1,-cx,-cy))
        art.append(f'<path id="{ident}_node_{i}" d="{d}" fill="{p["fill"]}" stroke="{p["stroke"]}" stroke-width="{num(p["width"])}" stroke-linecap="round" stroke-linejoin="round" transform="matrix(1 0 0 1 {num(64+cx*1.1)} {num(42+cy*1.1)})"/>')
    for i in range(lines):art.append(f'<path id="{ident}_node_{len(pieces)+i}" d="M0 0L32 0" fill="none" stroke="#eeeeee" stroke-width="1"/>')
    for i in range(circles):art.append(f'<circle id="{ident}_node_{len(pieces)+lines+i}" cx="0" cy="0" r="16" fill="#000000" stroke="#eeeeee" stroke-width="1"/>')
    for i,(col,w) in enumerate(zip(COLORS,WIDTHS)):art.append(f'<circle id="{ident}_palette_{i}" cx="640" cy="320" r="1" fill="{col}" stroke="{col}" stroke-width="{w}"/>')
    if paged:art.append(page_header_artwork(ident,count-8))
    else:
        for i in range(texts):art.append(f'<text id="{ident}_text_{i}" transform="matrix(1 0 0 1 640 320)" fill="#dddddd" stroke="none" font-family="UniversNextPro-Light" font-size="13"> </text>')
    art.append('</g>');svg='\n'.join(art)+'\n'
    if len(svg.encode())>65536:raise ValueError(f'{name}: SVG {len(svg.encode())} bytes exceeds SDK allowance')
    (dest/'panel.svg.fragment').write_text(svg)
    declarations='\n'.join(f'const unsigned G_{k.replace("-","_")}={i};' for k,i in groups.items())
    members=[]
    for p in pieces:
        parents=[groups[v] for v in p['parents']];parents += [-1]*(4-len(parents))
        members.append('{{'+','.join(map(str,parents))+'},'+','.join(map(str,[p['variantGroup'],p['variant'],p['cx'],p['cy']]))+'}')
    variants=[]
    for k in groups:
        a=AXES.get(name,{}).get(k)
        variants.append('{'+','.join(map(str,[round(a[1]*1024),round(a[2]*1024),a[-1]] if a else [0,1024,1]))+'}')
    geometry_path=UPSTREAM/name/(dict(reverb='kali',in_a_room='sitting').get(name,name)+'_geom.h')
    geometry=geometry_path.read_text() if geometry_path.is_file() and name!='father' else ''
    geometry=re.sub(r'OP1_Q10\(([-+\d.]+)\)',lambda m:str(round(float(m[1])*1024)),geometry)
    geometry=re.sub(r'\bint32_t\b','opxy::i32',geometry)
    geometry=re.sub(r'\bint16_t\b','opxy::i16',geometry)
    geometry=re.sub(r'\buint8_t\b','opxy::u8',geometry)
    tables=f'''// Geometry ownership generated from Hundekraft 3.0.3 vector artwork.
#pragma once
namespace {ident} {{ namespace art {{
{declarations}
struct Piece {{int parents[4],variantGroup,variant,cx,cy;}};
struct Variant {{int low,high,count;}};
static const Piece kPieces[{max(1,len(pieces))}]={{{','.join(members)}}};
static const Variant kVariants[{max(1,len(groups))}]={{{','.join(variants) if variants else '{0,1024,1}'}}};
{geometry}
}}}}
'''
    (dest/'visual_geometry.h').write_text(tables)
    replacements={'@ID@':ident,'@GROUP_COUNT@':str(max(1,len(groups))),'@ART_COUNT@':str(len(pieces)),
                  '@LINE_COUNT@':str(lines),'@CIRCLE_COUNT@':str(circles)}
    runtime=(ROOT/'tools/visual_runtime.hpp.in').read_text()
    for k,v in replacements.items():runtime=runtime.replace(k,v)
    (dest/'visual_runtime.h').write_text(runtime)
    header=f'''#pragma once
#include <opxy/draw.hpp>
#include <opxy/page_header.hpp>
namespace {ident} {{
using Frame=opxy::draw::Frame<0,0,{count},{texts},16>;
struct DisplayState {{
    opxy::draw::PageHeader header;
    opxy::u32 initialized,track,serial,released,seed;
    opxy::i32 time,age,level,held,memory[32];
}};
}}
'''
    (dest/'display.h').write_text(header)
    logic=(ROOT/'tools/visual_motion'/f'{name}.inc').read_text()
    params=definition['parameters'];title_values=[p['label'] for p in params]+['']*(8-len(params))
    captions=[]
    for i,p in enumerate(params):
        labels=p['mapping'].get('labels')
        if labels:
            short={'triangle':'tri','unison':'uni','octave':'oct','twelfth':'12th','middle':'mid','reverse':'rev','random':'rand','semitone':'semi','eleventh':'11th','feedback':'fback'}
            labels=[short.get(x.lower(),x.lower())[:5] for x in labels]
            captions.append(f'{{static const char* const labels[]={{{json.dumps(labels)[1:-1]}}};copy(values[{i}],header_capacity,labels[static_cast<unsigned>(amount(input.slots[{i}]))*{len(labels)}/1024]);}}')
        else:captions.append(f'header_amount(values[{i}],header_capacity,input.slots[{i}]);')
    page_map=[[0,1,2,3],[0,1,2,3]]
    for i,p in enumerate(params[4:],4):page_map[1][p.get('control',i%4)]=i
    header_call=(f'''static const char* const titles[8]={{{','.join(json.dumps(v) for v in title_values)}}};\n''' if paged else '')+f'''char values[8][header_capacity]={{}};
    {''.join(captions)}
    '''
    if name=='crush':header_call+='number(values[0],bits);khz(values[1],rate);number(values[2],drive);\n'
    if paged:header_call+=f'''static const opxy::u8 pageMap[2][4]={{{{0,1,2,3}},{{{','.join(map(str,page_map[1]))}}}}};
    page_header(state.header,input,frame,titles,values,{count-8},pageMap);'''
    else:header_call+='for(unsigned i=0;i<4;++i)copy(frame.header[i],header_capacity,values[input.shown[i]<8?input.shown[i]:i]);'
    waveform_includes='#include "sine.hpp"\n#include "dsp.hpp"\n' if name in ('wavetable','taffy') else ''
    if name=='crush':waveform_includes='#include "../sine.hpp"\n#include "../dsp.hpp"\n'
    aliases=''.join(f'const i32 {v}={expression};\n' for v,expression in [('t','reduced?0:state.time'),('level','state.level')] if re.search(r'\b'+v+r'\b',logic))
    code=f'''// Artwork and animation reconstruction from Hundekraft 3.0.3. See VISUALS.md.
#include "display.h"
#include "visual_geometry.h"
#include "visual_runtime.h"
{waveform_includes}
namespace {ident} {{
inline void display_reset(DisplayState& s) {{
    s.header.reset();s.initialized=s.track=s.serial=s.released=0;s.seed=4711;
    s.time=0;s.age=60000;s.level=s.held=0;for(unsigned i=0;i<32;++i)s.memory[i]=0;
}}
inline void display(DisplayState& state,const opxy::draw::Input& input,Frame& frame) {{
    using namespace art;
    const bool fresh=!state.initialized || state.track!=input.track;
    const i32 dt=clamp(input.elapsed,0,102);
    const bool reduced=input.reduced_motion!=0;
    const bool held=input.held_notes>0 && input.held_notes<=128;
    const bool strike=!fresh && state.serial!=input.note_on_serial;
    if(fresh){{display_reset(state);state.track=input.track;}}
    state.initialized=1;state.serial=input.note_on_serial;state.released=input.note_off_serial;
    if(strike)state.age=0;
    state.age=clamp(state.age+dt,0,60000);
    const i32 target=held?1024:!reduced && state.age<1200?1024-state.age*1024/1200:0;
    state.level=reduced?target:state.level+(target-state.level)*clamp(dt*12,0,1024)/1024;
    if(state.level<2)state.level=0;
    const bool moving=!reduced && (held || state.level>0);
    if(moving)state.time=(state.time+dt)%61440;
    {aliases}
    i32 k[8]={{}};for(unsigned i=0;i<8;++i)k[i]=amount(input.slots[i]);
    Scene<Frame> scene(frame);
    {logic}
    scene.finish();
    {header_call}
    state.memory[31]=static_cast<opxy::i32>(scene.overflow);
    state.held=held;
}}
}}
'''
    (dest/'display.cpp').write_text(code)
    display=dict(stateBytes=184,stateAlignment=4,paths=0,points=0,nodes=count,texts=texts,palette=16)
    if synth:
        definition['resources']['display']=display
        if paged:definition['press']='page'
        record['sdk']='1.6'
        record.pop('identity',None)
        (folder/'definition.json').write_text(json.dumps(definition,indent=2)+'\n');(folder/'machine.json').write_text(json.dumps(record,indent=2)+'\n')
    metadata=dict(id=ident,name=definition['name'],kind=record['kind'],display=display,
                  nodes=[f'{ident}_node_{i}' for i in range(count)],texts=[f'{ident}_text_{i}' for i in range(texts)],
                  palette=[f'{ident}_palette_{i}' for i in range(16)],paths=0,points=0,
                  source=str(source.relative_to(UPSTREAM.parent)),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                  native=synth)
    (dest/'visual.json').write_text(json.dumps(metadata,indent=2)+'\n')
    if paged:(folder/'SDK-LICENSE.txt').write_bytes((SDK/'LICENSE').read_bytes())
    test_include='../display.cpp' if synth else '../visual/display.cpp'
    test=f'''#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "{test_include}"
static void require(bool yes,const char* message){{if(!yes){{std::fprintf(stderr,"{ident}: %s\\n",message);std::exit(1);}}}}
int main(){{
    using namespace {ident};using namespace opxy;using namespace opxy::draw;
    static_assert(sizeof(DisplayState)==184,"Display ABI size");
    DisplayState s;display_reset(s);{ident}::Frame f={{}};Input in={{}};
    in.elapsed=17;in.held_notes=0;for(unsigned i=0;i<4;++i)in.shown[i]=i;
    display(s,in,f);require(valid(f),"initial frame valid");
    require(s.age>1000 && s.level==0,"activation must not fabricate a strike");
    const {ident}::Frame idle=f;for(unsigned i=0;i<240;++i)display(s,in,f);
    require(!std::memcmp(&idle,&f,sizeof(f)),"idle must stay still");
    unsigned seed=1234567;
    for(unsigned n=0;n<1800;++n){{
        for(unsigned i=0;i<8;++i){{seed=seed*1664525u+1013904223u;in.slots[i]=n%3==0?0:n%3==1?0x7fffffff:static_cast<i32>(seed&0x7fffffffu);}}
        for(unsigned i=0;i<4;++i){{in.shown[i]=i+(n&1?4:0);in.parameters[i]=in.slots[in.shown[i]];}}
        in.elapsed=n%31==0?-1:n%37==0?0x7fffffff:17;in.note_serial=n/9;in.held_notes=n%17;in.note_off_serial=n/13;
        in.track=n/300;in.reduced_motion=n%7==0;
        display(s,in,f);
        if(!valid(f))for(unsigned j=0;j<f.node_count;++j){{const Node& p=f.nodes[j];
            if(!placement(p.x,p.y,p.scale)||!rotation_valid(p.cos,p.sin)||p.shade>=16)
                std::fprintf(stderr,"frame %u node %u: %d %d scale %d turn %d %d ink %u\\n",n,j,p.x,p.y,p.scale,p.cos,p.sin,p.shade);}}
        require(valid(f),"control corners and note activity must emit valid frames");
        require(s.memory[31]==0,"scene exceeded its retained geometry budget");
    }}
    in.reduced_motion=1;in.elapsed=17;in.held_notes=1;in.note_serial++;
    display(s,in,f);const {ident}::Frame reduced=f;
    for(unsigned i=0;i<240;++i)display(s,in,f);
    require(!std::memcmp(&reduced,&f,sizeof(f)),"reduced motion must hold a stable pose");
    in.track++;display(s,in,f);require(s.age>1000,"track changes must establish a fresh baseline");
    std::puts("PASS: {ident} drawing bounds, budgets, note baseline, idle and reduced motion");
}}
'''
    (folder/'tests/display.cpp').write_text(test)
    subprocess.run(['/opt/homebrew/opt/llvm/bin/clang-format','-i',*(str(dest/n) for n in ('display.h','display.cpp','visual_geometry.h','visual_runtime.h')),str(folder/'tests/display.cpp')],check=True)
    (dest/'VISUALS.md').write_text(f'''# {definition['name']} display

Reconstructed from Hundekraft 3.0.3 `{metadata['source']}` and its accompanying screen code. The original vector contours and painter order are retained, recoloured in greys with selective red, orange and blue. Original art SHA-256: `{metadata['source_sha256']}`. Existing LICENSE, NOTICE and UPSTREAM notices apply to the imported artwork too.

The C++ display uses only public `opxy::draw` nodes and runs separately from audio. {"It is wired into the native synth and SDK browser workbench." if synth else "This is an audition display study. The SDK has no effect/sample custom-display registration, so it is not wired into native firmware."} Both preview and native-ready code use the same C++ frame computation. Nonuniform source transforms use pre-baked geometry keyframes; uniform scale, rotations and translation remain continuous.

The current public display input supplies parameters and note-gate events, but no oscillator, grain or effect audio telemetry. Moving parts follow note activity and a visual decay model; phase portraits and grain positions illustrate the controls rather than reading the DSP. Browser effects use measured return activity to gate motion. Reduced motion freezes incidental motion; track activation establishes a baseline without inventing a note. Source audio is unchanged.
''')
    print(name, len(pieces),'art +',lines,'lines +',circles,'circles =',count,'nodes;',len(svg.encode()),'SVG bytes')
    return metadata

def main():
    p=argparse.ArgumentParser();p.add_argument('names',nargs='*');args=p.parse_args()
    names=args.names or list(BUDGETS)
    for name in names:build(name)
if __name__=='__main__':main()
