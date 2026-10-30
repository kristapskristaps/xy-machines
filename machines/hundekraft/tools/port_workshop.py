#!/usr/bin/env python3
"""Reproduce the local source ports from Hundekraft's checksum-pinned source bundle.

The imported algorithms are GPL-3.0-or-later with their retained OP-1 firmware
permission. These are private development ports; no publication is performed.
"""
from pathlib import Path
import hashlib
import json
import math
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parents[1]/'tools'))
from workspace import sdk_path
SDK=sdk_path()
SOURCE = Path('/tmp/openxy-hundekraft-3.0.3-port-source/ext/kennel')
PLUGINS = SOURCE / 'synth_sdk/plugins'
PYTHON = '/Users/tapsy/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3'
CLANG = '/opt/homebrew/opt/llvm/bin/clang'

# source folder, kind, readable name, controls; secondary controls use words 5..7.
SPECS = [
 ('wavetable','synth','Wavetable',['Wave','Detune','Cutoff','Reso']),
 ('taffy','synth','Taffy',['Wave','Bend','Envelope','Reso']),
 ('pew','synth','PEW',['Low','Middle','High','Rotor']),
 ('elg','synth','Elg',['Interval','Detune','Timbre','Motion']),
 ('supersaw','synth','Supersaw',['Detune','Morph','Sub','Drive']),
 ('fmbell','synth','FM Bell',['Ratio','Index','Decay','Feedback']),
 ('sid','synth','SID',['Wave','Color','Cutoff','Reso']),
 ('mbira','synth','MBIRA',['Strike','Tine','Box','Buzz','Ring','Double','Wah']),
 ('oxbow','synth','Oxbow',['Cutoff','Reso','Envelope','Detune','Wave','Filter','Drive']),
 ('eva','synth','EVA',['Pressure','Vowel','Oxygen','Com','Octave']),
 ('arc','synth','Arc',['Current','Carbon','Hiss','Strike','Ratio','Coupling','Bright']),
 ('mixtur','synth','Mixtur',['Divider1','Divider2','Divider3','Divider4','Formant1','Formant2','Reso']),
 ('washer','effect','Washer',['Speed','Depth','Foam','Dry']),
 ('crush','effect','Crush',['Bits','Rate','Drive','Dry']),
 ('tape','effect','Tape',['Age','Feedback','Time','Flutter','Dry']),
 ('warble','effect','Warble',['Rate','Depth','Tone','Dry']),
 ('plate','effect','Plate',['Decay','Tone','Predelay','Dry']),
 ('snowglobe','effect','Snowglobe',['Decay','Shimmer','Size','Dry','Freeze']),
 ('father','effect','Father',['Size','Gate','Tone','Dry']),
 ('reverb','effect','Reverb',['Time','Tone','Diffuse','Dry']),
 ('clouds','effect','Clouds',['Position','Size','Density','Texture','Pitch','Dry','Space','Freeze']),
 ('hail','effect','HAIL',['Time','Size','Shape','Density','Pitch','Dry','Space','Freeze']),
 ('stutter','effect','Stutter',['Rate','Mode','Shape','Chance','Repeats','Freeze','Dry']),
 ('palme','effect','Palme',['Tuning','Sustain','Excite','Dry','Metal','Damping','Spread','Latch']),
 ('in_a_room','effect','In a room',['Size','Shape','Erosion','Dry','Length','Absorb','Fade','Capture']),
]
CALLBACKS = {
 'wavetable':('engine_note_on','engine_render'), 'taffy':('engine_note_on','engine_render'),
 'pew':('engine_note_on','engine_render'), 'elg':('engine_note_on','engine_render'),
 'supersaw':('ss_note_on','ss_render'), 'fmbell':('fm_note_on','fm_render'),
 'sid':('note_on','render'), 'mbira':('mb_note_on','mb_render'),
 'oxbow':('obv_note_on','obv_render'), 'eva':('eva_note_on','eva_render'),
 'arc':('arc_note_on','arc_render'), 'mixtur':('mx_note_on','mx_render'),
 'washer':('fx_reset','fx_process'), 'snowglobe':('fx_reset','fx_process'),
 'crush':('init','stereo'), 'father':('init','stereo'), 'warble':('init','render_stereo'),
 'plate':('init','stereo'), 'reverb':('reverb_init','reverb_render'),
 'tape':('tape_init','tape_render'), 'clouds':('clouds_fx_init','clouds_fx_render'),
 'hail':('bd_init','bd_render'), 'stutter':('sx_init','sx_render'),
 'palme':('palme_init','palme_render'), 'in_a_room':('sit_init','sit_render'),
}
STEREO = {'crush','father','warble','plate'}
DIRECT = {'supersaw':'voice_t','fmbell':'fm_voice_t','arc':'arc_voice_t','mixtur':'mx_voice_t'}
SELECTORS={
 'elg':{0:['Unison','Fifth','Octave','Oct 5th','Two oct']},
 'fmbell':{0:[str(i/2) for i in range(1,17)]},
 'sid':{0:['Triangle','Saw','Pulse','Noise','Sync','Ring']},
 'oxbow':{4:['Saw','Pulse','Both','Sync','XMod','Fifth'],5:['LP12','HP12','BP12','Notch','LP24','SEM']},
 'eva':{4:['Low','Middle','High']},
 'mixtur':{i:['Off']+[str(j) for j in range(1,25)] for i in range(4)},
 'in_a_room':{1:['Cube','Golden','Bolt','Lucier']},
 'stutter':{4:['2','3','4','6','8','16']},
}

SHIM = r'''
#pragma once
#include <stdint.h>
#define OP1_BLOCK 128
#define ENGINE_BLOCK 128
#define OP1_VOICES 1
#define ENGINE_VOICES 1
#define OP1_SAMPLE_RATE 44100
#define OP1_VOICE_SIZE 36
#define OP1_PITCH_INC pitch_input
#define ENGINE_PITCH pitch_input
#define OP1_SINE_TABLE sine_table
#define FX_INSTANCES 1
#define FX_MASTER 2
#define OP1_FX_BLOCK 64
#define FX_BUFFER
#define FX_SCRATCH
#define OP1_FX_SCRATCH_BYTES 176460
#define OP1_PRESS_NONE 0
#define OP1_PRESS_PAGE 1
#define OP1_PRESS_LATCH 2
#define op1_voice_index(self) 0
#define ENGINE_VOICE(self) 0
#define op1_voice_vel_gain(self) 32767
#define ENGINE_VELOCITY(vi) 127
#define op1_fx_scratch(pos) scratch
#define op1_fx_param_q15(p,i) ((int32_t)(p)[i])
#define op1_fx_page fx_page
#define op1_fx_meter(pos) (&meter)
#define op1_beat_clock(b) (*(b)=beat)
#define engine_sat16 op1_sat16
#define fx_sat16 op1_sat16
#define FX_SAT16 op1_sat16
#define _Static_assert static_assert
typedef struct {int32_t len,next,step,tape_pos;uint8_t follows_tape;} op1_beat_t;
static inline int16_t op1_sat16(int32_t x){return x>32767?32767:x< -32768?-32768:(int16_t)x;}
static inline int32_t op1_mul16(int32_t a,int32_t b){return (a*b)>>15;}
'''

def setup_shim():
    p=ROOT/'local/import-headers';p.mkdir(parents=True,exist_ok=True)
    (p/'hk').mkdir(exist_ok=True)
    (p/'stdint.h').write_text('''#pragma once
typedef __INT32_TYPE__ int32_t; typedef __UINT32_TYPE__ uint32_t;
typedef __INT16_TYPE__ int16_t; typedef __UINT16_TYPE__ uint16_t;
typedef __INT8_TYPE__ int8_t; typedef __UINT8_TYPE__ uint8_t;
typedef __INT64_TYPE__ int64_t; typedef __UINT64_TYPE__ uint64_t;
typedef __UINTPTR_TYPE__ uintptr_t;
#define INT32_MAX 2147483647
#define INT32_MIN (-2147483647-1)
#define INT64_MAX 9223372036854775807LL
''')
    (p/'stddef.h').write_text('#pragma once\ntypedef __SIZE_TYPE__ size_t;\n#define NULL 0\n')
    for h in ['op1_plugin.h','hk/engine.h','hk/fx.h']:(p/h).write_text(SHIM)
    return p

def chunks(text):
    """Top-level C declarations and definitions after preprocessing."""
    start=depth=paren=0;string=None;escape=False;function=False
    for i,c in enumerate(text):
        if string:
            if escape:escape=False
            elif c=='\\':escape=True
            elif c==string:string=None
            continue
        if c in '\"\'':string=c;continue
        if c=='(':paren+=1
        elif c==')':paren-=1
        elif c=='{':
            if depth==0:
                prefix=text[start:i].strip()
                function=prefix.endswith(')') and not re.match(r'(typedef|struct|enum|union)\b',prefix) and '=' not in prefix
            depth+=1
        elif c=='}':
            depth-=1
            if depth==0 and function:
                yield 'function',text[start:i+1].strip();start=i+1
        elif c==';' and depth==0 and paren==0:
            item=text[start:i+1].strip()
            if item:yield 'declaration',item
            start=i+1
    if text[start:].strip():raise ValueError('Unparsed C: '+text[start:start+160])

def source_text(name):
    folder=PLUGINS/name
    main=folder/('sitting.c' if name=='in_a_room' else name+'.c')
    src=main.read_text()
    src=re.split(r'\bconst\s+op1_(?:engine|fx|fx_stereo)_t\s+\w+\s*=',src)[0]
    src=re.sub(r'^#include\s+[^\n]*seq_text[^\n]*\n','',src,flags=re.M)
    if name=='oxbow':
        src=src.replace('VINT_TUNE8[OP1_VOICES]','VINT_TUNE8[]').replace('VINT_CUT8[OP1_VOICES]','VINT_CUT8[]').replace('VINT_ENV[OP1_VOICES]','VINT_ENV[]')
    if name=='sid':src=(folder/'sid_dsp.c').read_text()+'\n'+src
    if name in STEREO:src=(folder/(name+'_dsp.c')).read_text()+'\n'+src
    if name=='clouds':
        src=re.sub(r'/\*.*?\*/','',src,flags=re.S)
        def positional(m):
            assignments=re.findall(r'\.(\w+)\s*=\s*(.*?)(?=,\s*\.|$)',m[1].strip().rstrip(','),re.S)
            return 'clouds_params_t prm = {};\n'+''.join('prm.'+k+' = '+v.strip()+';\n' for k,v in assignments)
        src=re.sub(r'clouds_params_t prm\s*=\s*\{(.*?)\};',positional,src,flags=re.S)
    return src

def preprocess(name,shim):
    folder=PLUGINS/name
    source=ROOT/'local'/('import-'+name+'.c');source.write_text(source_text(name))
    args=[CLANG,'-E','-P','-x','c','-nostdinc','-I'+str(shim),'-I'+str(folder),'-I'+str(SOURCE/'synth_sdk'),str(source)]
    text=subprocess.check_output(args,text=True)
    text=re.sub(r'__attribute__\s*\(\(aligned\([^)]*\)\)\)','',text)
    text=re.sub(r'^\s*#pragma[^\n]*\n','',text,flags=re.M)
    text=text.replace('typedef long long int int64_t;','typedef Wide int64_t;').replace('typedef long long unsigned int uint64_t;','typedef Wide uint64_t;')
    text=text.replace('__builtin_clz','leading_zeroes')
    text=re.sub(r'\b(0[xX][0-9a-fA-F]+|[0-9]+)(?:ULL|LLU|LL|ull|llu|ll)\b',r'Wide(\1)',text)
    if name=='oxbow':text=re.sub(r'\[P_\w+\]\s*=(?!=)\s*','',text)
    return text

def member_declaration(decl):
    decl=re.sub(r'^static\s+','',decl)
    # Keep explicit defaults in reset(), not in a C++ constructor.
    depth=0;parts=[];start=0
    for i,c in enumerate(decl[:-1]):
        if c in '({[':depth+=1
        elif c in ')}]':depth-=1
        elif c==',' and depth==0:parts.append(decl[start:i]);start=i+1
    parts.append(decl[start:-1])
    initial=[];plain=[]
    for p in parts:
        m=re.search(r'\s*=\s*',p)
        if m:
            lhs=p[:m.start()].strip();value=p[m.end():].strip()
            var=re.search(r'(\w+)(?:\[[^]]*\])?\s*$',lhs).group(1)
            initial.append(var+' = '+value+';');plain.append(p[:m.start()])
        else:plain.append(p)
    return ','.join(plain)+';',initial

def make_dsp(name,ident,shim):
    external=[];members=[];functions=[];initial=[]
    for kind,item in chunks(preprocess(name,shim)):
        if kind=='function':
            at=item.index('{')
            item=re.sub(r'\bstatic\s+','',item[:at])+item[at:]
            def lift(m):
                decl,values=member_declaration(m[0]);members.append(decl);initial.extend(values)
                return ''
            item=re.sub(r'\bstatic\s+(?!const\b)[^;{}]+;',lift,item)
            if name=='wavetable':item=item.replace('knobs, pitch_input,','knobs, reinterpret_cast<const volatile uint32_t*>(pitch_input),')
            if name=='washer':
                item=item.replace('x - ((x * k3) >> 16) + ((wet * k3) >> 15)','wet')
                item=item.replace('const int32_t k2 = knobs[2] < 0 ? 0 : knobs[2], k3 = knobs[3] < 0 ? 0 : knobs[3];','const int32_t k2 = knobs[2] < 0 ? 0 : knobs[2];')
            if name=='snowglobe':
                item=item.replace('x0 - ((x0 * k3) >> 16) + ((wprev + wk) >> 1)','((wprev + wk) >> 1)').replace('x1 - ((x1 * k3) >> 16) + wk','wk')
                item=item.replace('const int32_t x0 = io2[2 * j], x1 = io2[2 * j + 1], wk =','const int32_t wk =')
                item=item.replace('const int freeze = k3 > 31500;','const int freeze = fx_page[0][3] != 0;')
            if name=='tape':item=item.replace('in + ((t->lp * 23000) >> 15)','((t->lp * 23000) >> 15)')
            if name=='father':
                item=item.replace('sat(a+mul(wl-a,wet))','sat(mul(wl,wet))').replace('sat(b+mul(wr-b,wet))','sat(mul(wr,wet))')
                item=item.replace('int32_t a=l[n],b=r?r[n]:a;','int32_t a=l[n],b=r?r[n]:a; (void)b;')
            if name=='reverb':item=item.replace('in + m15(in, -amount) + (((wet >> 3) * amount) >> 12)','(((wet >> 3) * amount) >> 12)')
            if name=='clouds':item=item.replace('mul16(input[i], fout) + mulq15(mulq15(out[i], 39322), fin)','mulq15(mulq15(out[i], 39322), fin)').replace('output[i] = soft_convert(x);','(void)fout; output[i] = soft_convert(x);')
            if name=='hail':item=item.replace('((io[i] * cd) >> 15) + ((xm * (cw >> 1)) >> 14)','((xm * (cw >> 1)) >> 14)').replace('cd += ddr;','cd += ddr; (void)cd;')
            if name=='palme':
                item=item.replace('ADDQ(MULQ15(buf[n], dryg), MULQ15(v, wetg))','MULQ15(v, wetg)').replace('MULQ15(buf[n], dryg)','0')
                item=item.replace('int16_t dryg =','int16_t dryg =')
                if 'dryg' in item:item=item.replace('int32_t peak = 0;','(void)dryg; int32_t peak = 0;').replace('int32_t pk = 0;','(void)dryg; int32_t pk = 0;')
                item=re.sub(r'(int16_t dryg[^;]+;)',r'\1 (void)dryg;',item)
            if name=='in_a_room':
                item=item.replace('op1_sat16(in + (m16(clampi(v - in, 32767), mix) >> 15))','op1_sat16(v)').replace('op1_sat16(in + (m16(clampi(w - in, 32767), mix) >> 15))','op1_sat16(w)')
                if 'int32_t in = buf[n];' in item:item=item.replace('int32_t in = buf[n];','int32_t in = buf[n]; (void)in;')
            functions.append(item)
        elif item.startswith(('typedef ','enum ','static_assert')) or re.match(r'(static\s+)?const\s+',item):
            external.append(item)
        elif re.search(r'\b\w+\s*\([^;]*\)\s*;$',item) and '{' not in item and '=' not in item:
            continue # Prototypes become member functions below.
        else:
            decl,values=member_declaration(item);members.append(decl);initial+=values
    body='\n\n'.join(external)+'\n\nstruct Dsp {\n'
    body+='const int32_t* pitch_input;\nuint8_t* scratch;\nuint8_t fx_page[3][4];\nop1_beat_t beat;\nint16_t meter;\n'
    body+='\n'.join(members)+'\n'
    if name in DIRECT:body+=DIRECT[name]+' note;\n'
    body+='void defaults() { '+' '.join(initial)+' }\n'
    body+='\n\n'.join(functions)+'\n};\n'
    result='#pragma once\n#include "wide.hpp"\n// DSP adapted from Hundekraft 3.0.3. Retained copyright and terms: NOTICE, LICENSE.\nnamespace '+ident+' { namespace port {\n'+body+'}}\n'
    return subprocess.check_output(['/opt/homebrew/opt/llvm/bin/clang-format'],input=result,text=True)

def wrappers(port, size):
    name,kind,ident=port['name'],port['kind'],port['id']
    folder=Path(port['path']);on,render=CALLBACKS[name]
    definition=json.loads((folder/'definition.json').read_text())
    mapping=[0,1,2,3]+([7] if name=='eva' else [5,6,7])
    mapping=mapping[:len(port['labels'])]
    copy='\n'.join(f'    q[{word}] = parameters[{i}] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[{i}]) >> 16);' for i,word in enumerate(mapping))
    selector_copy='\n'.join(f'    q[{mapping[i]}]=static_cast<port::int16_t>((static_cast<unsigned>(q[{mapping[i]}])*{len(labels)}u>>15)*512u+256u);' for i,labels in SELECTORS.get(name,{}).items())
    copy+='\n'+selector_copy
    if kind=='synth':
        words=((size+7)//8)*2
        (folder/'core.h').write_text(f'''#pragma once
#include <opxy/audio.hpp>
#include "sine.hpp"
#include "dsp.hpp"
namespace {ident} {{
struct alignas(8) Voice {{ opxy::u32 pending, half; opxy::u32 pitch[128]; opxy::i16 audio[128]; opxy::u32 storage[{words}]; }};
struct Shared {{}};
}}
''')
        selfarg='&d.note' if name in DIRECT else '&voice'
        body=f'''#include "core.h"
namespace {ident} {{
inline port::Dsp& dsp(Voice& voice) {{ return *reinterpret_cast<port::Dsp*>(voice.storage); }}
inline void initialize(Voice& voice, Shared&, opxy::u32) {{
    for (unsigned i=0; i<sizeof(voice.storage)/sizeof(voice.storage[0]); ++i) voice.storage[i]=0;
    dsp(voice).defaults(); voice.pending=1;voice.half=0;
    for(unsigned i=0;i<128;++i) voice.audio[i]=0;
}}
inline void start(Voice& voice, Shared&, opxy::u32) {{ voice.pending=1;voice.half=0;for(unsigned i=0;i<128;++i)voice.audio[i]=0; }}
inline void destroy(Voice&, Shared&, opxy::u32) {{}}
inline void render(Voice& voice, Shared&, opxy::u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {{
    port::Dsp& d=dsp(voice);
    port::int16_t q[8]={{}};
{copy}
    const unsigned offset=voice.half*64;
    for(unsigned i=0;i<64;++i)voice.pitch[offset+i]=pitch[i];
    if(!voice.half) {{
        for(unsigned i=0;i<64;++i)output[i]=static_cast<opxy::i32>(voice.audio[64+i])*32768;
        voice.half=1;return;
    }}
    d.pitch_input=reinterpret_cast<const port::int32_t*>(voice.pitch);
    if (voice.pending) {{ d.{on}({selfarg},q); voice.pending=0; }}
    d.{render}({selfarg},voice.audio,q);
    d.pitch_input=0;
    for(unsigned i=0;i<64;++i) output[i]=static_cast<opxy::i32>(voice.audio[i])*32768;
    voice.half=0;
}}
}}
'''
        (folder/'engine.cpp').write_text(body)
        definition['resources'].update(voiceBytes=words*4+776,voiceAlignment=8,voiceCapacity=16,sharedBytes=0)
    else:
        # Original scratch access appears in methods. Small algorithms need none.
        source=(folder/'dsp.hpp').read_text()
        scratch=176460 if re.search(r'(?<![A-Za-z_])scratch\b', source.split('void defaults()')[1]) else 0
        stride=(size+scratch+7)//8*8
        count=1 if name in STEREO else 2
        total=stride*count
        if total>524288:raise ValueError(f'{name}: scratch {total} exceeds the send buffer')
        dry=port['labels'].index('Dry')
        mixword=6 if name in ['clouds','hail'] else 3
        if name in ['stutter','tape']:mixword=None
        copy='\n'.join(f'    q[{word}] = parameters[{i}] < 0 ? 0 : static_cast<port::int16_t>(static_cast<opxy::u32>(parameters[{i}]) >> 16);' for i,word in enumerate(mapping) if i!=dry)
        if mixword is not None:copy+=f'\n    q[{mixword}]=32767;'
        latch={'snowglobe':4,'clouds':7,'hail':7,'stutter':5,'palme':7,'in_a_room':7}.get(name)
        # Scratch is zeroed incrementally before initialization; no stale send data is read.
        (folder/'core.h').write_text(f'''#pragma once
#include <opxy/effect.hpp>
namespace {ident} {{
const opxy::u32 buffer_bytes={total};
struct State {{ opxy::u32 cleared, ready, clock_phase, clock_tempo, clock_tick; }};
}}
''')
        args='0,audio_left,audio_right,q' if name in STEREO else ('0,audio_left,64,q' if name in ['washer','snowglobe'] else '0,audio_left,q')
        setup=f'd.scratch=reinterpret_cast<port::uint8_t*>(memory)+{size};' if scratch else 'd.scratch=0;'
        init=f'd.{on}(0);'
        latchcode=f'd.fx_page[0][3]=parameters[{latch}]>=0x40000000;' if latch is not None else ''
        copy+='\n'+selector_copy
        processcall=f'd.{render}({args});'
        if name not in STEREO:processcall+='\n        for(unsigned i=0;i<64;++i) (channel?right:left)[i]=static_cast<opxy::i32>(audio_left[i])*65536;'
        else:processcall+='\n        for(unsigned i=0;i<64;++i) {left[i]=static_cast<opxy::i32>(audio_left[i])*65536;right[i]=static_cast<opxy::i32>(audio_right[i])*65536;}'
        extra='const opxy::Transport& transport, ' if name in ['stutter','in_a_room'] else ''
        clock='''
        d.beat.len=static_cast<port::int32_t>(beat_length);
        d.beat.next=state.clock_phase?static_cast<port::int32_t>(beat_length-state.clock_phase):0;
        d.beat.step=1048576;d.beat.tape_pos=0;d.beat.follows_tape=0;
''' if extra else ''
        clock_setup='''const opxy::u32 bpm=transport.bpm_x10>=200&&transport.bpm_x10<=3000?transport.bpm_x10:1200;
    const opxy::u32 beat_length=((26460000u/bpm)<<14)+((26460000u%bpm)<<14)/bpm;
    if(state.clock_tempo!=bpm || transport.ticks96-state.clock_tick>96u) {
        state.clock_phase=(transport.ticks96%96u)*(beat_length/96u);state.clock_tempo=bpm;
    }
    state.clock_tick=transport.ticks96;''' if extra else ''
        clock_advance='state.clock_phase+=1048576u;if(state.clock_phase>=beat_length)state.clock_phase-=beat_length;' if extra else ''
        arrays='port::int16_t audio_left[64],audio_right[64];' if name in STEREO else 'port::int16_t audio_left[64];'
        right_input='audio_right[i]=static_cast<port::int16_t>(right[i]>>16);' if name in STEREO else ''
        body=f'''#include "core.h"
#include "sine.hpp"
#include "dsp.hpp"
namespace {ident} {{
void initialize(State& state) {{ state=State{{}}; }}
void process(State& state, opxy::Buffer buffer, {extra}opxy::Channel left,
             opxy::Channel right, opxy::Parameters parameters) {{
    if (!buffer.words || buffer.count<buffer_bytes/4) {{
        for(unsigned i=0;i<64;++i) left[i]=right[i]=0;
        return;
    }}
    if(state.cleared<buffer_bytes/4) {{
        const opxy::u32 end=state.cleared+1024<buffer_bytes/4?state.cleared+1024:buffer_bytes/4;
        while(state.cleared<end) buffer.words[state.cleared++]=0;
        for(unsigned i=0;i<64;++i) left[i]=right[i]=0;
        return;
    }}
    port::int16_t q[8]={{}};
{copy}
    {clock_setup}
    for(unsigned channel=0;channel<{count};++channel) {{
        opxy::i32* memory=buffer.words+channel*{stride//4};
        port::Dsp& d=*reinterpret_cast<port::Dsp*>(memory);
        {setup}
        if(!state.ready) {{d.defaults();{init}}}
        {latchcode}
        {clock}
        {arrays}
        for(unsigned i=0;i<64;++i) {{
            audio_left[i]=static_cast<port::int16_t>((channel?right:left)[i]>>16);
            {right_input}
        }}
        {processcall}
    }}
    state.ready=1;
    {clock_advance}
}}
}}
'''
        (folder/'fx.cpp').write_text(body)
        definition['resources']={'bufferBytes':total}
        if extra:definition['resources']['transport']=1
    (folder/'definition.json').write_text(json.dumps(definition,indent=2)+'\n')

def control(label,value,index,kind):
    result={'id':label.lower().replace(' ','_'),'label':label,'default':value,
      'mapping':{'kind':'linear','minimum':0,'maximum':1},'display':'percent'}
    if kind=='synth' and index>=4:result['control']=(index-4)%4
    return result

def display(port):
    # The authored display port is regenerated alongside its DSP adapter.
    from visuals import build
    build(port['name'])


def write_machine(spec,index,shim):
    name,kind,title,labels=spec;ident='hk'+name.replace('_','')
    folder=ROOT/ident
    if not folder.exists():
        subprocess.run([PYTHON,str(SDK/'sdk.py'),'new','--repository','--id',ident,'--name','HK '+title,'--kind',kind,'--out',str(folder)],check=True,capture_output=True)
    manifest=json.loads((PLUGINS/name/'plugin.json').read_text())
    values=[v/32767 for v in manifest['defaults']]+[v/32767 for v in manifest.get('page2_defaults',[0,0,0])]
    while len(values)<len(labels):values.append(0)
    for i,label in enumerate(labels):
        if label=='Dry':values[i]=1
    if name=='eva':values[4]=768/32767
    for i,choices in SELECTORS.get(name,{}).items():
        selected=(round(values[i]*32767)>>9)%len(choices)
        values[i]=(selected+.5)/len(choices)
    if name=='wavetable':values[:4]=[.25,.12,.65,.2]
    if name=='elg':values[2]=.55
    if name=='mixtur':values[:4]=[(i+1.5)/25 for i in range(4)]
    params=[control(label,values[i],i,kind) for i,label in enumerate(labels)]
    if name=='eva':params[4]['control']=3
    for i,choices in SELECTORS.get(name,{}).items():params[i].update(mapping={'kind':'enum','labels':choices},display='enum')
    for i,label in enumerate(labels):
        if label in ['Freeze','Latch','Capture']:params[i].update(mapping={'kind':'enum','labels':['Off','On']},display='enum')
    definition=json.loads((folder/'definition.json').read_text())
    definition.update(parameters=params,presets={'Init':{p['id']:p['default'] for p in params}})
    record=json.loads((folder/'machine.json').read_text())
    record['description']='Hundekraft '+title+' DSP port for private OP-XY testing.'
    record['sdk']='1.6'
    record.pop('identity',None)
    if kind=='sample':record['base']='sampler'
    if kind=='effect':record['dryParameter']='dry'
    (folder/'machine.json').write_text(json.dumps(record,indent=2)+'\n')
    (folder/'definition.json').write_text(json.dumps(definition,indent=2)+'\n')
    (folder/'wide.hpp').write_text((ROOT/'tools/wide.hpp.in').read_text().replace('@ID@',ident))
    (folder/'dsp.hpp').write_text(make_dsp(name,ident,shim))
    sine=[round(32767*math.sin(2*math.pi*i/8192)) for i in range(8192)]
    (folder/'sine.hpp').write_text('#pragma once\nnamespace '+ident+' { namespace port {\nstatic const __INT16_TYPE__ sine_table[8192] = {\n'+',\n'.join(','.join(map(str,sine[i:i+32])) for i in range(0,len(sine),32))+'\n};\n}}\n')
    for f in ['LICENSE','NOTICE']:(folder/f).write_bytes((SOURCE/f).read_bytes())
    notices=['# Source provenance','', 'Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz',
      'SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7','',
      'The DSP is ported from `synth_sdk/plugins/'+name+'`. Original notices follow.','']
    for p in sorted((PLUGINS/name).glob('*')):
        if p.is_file() and (p.name.startswith(('LICENSE','NOTICE')) or p.suffix=='.c'):
            s=p.read_text();comments=re.findall(r'/\*.*?\*/',s,re.S)
            notices.extend([p.name,'',comments[0] if p.suffix=='.c' and comments else s,''])
    notices+=['Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.']
    (folder/'UPSTREAM.md').write_text('\n'.join(notices)+'\n')
    ag=re.sub(r'(?:\.\./)+open-xy','../../../../open-xy',(folder/'AGENTS.md').read_text());(folder/'AGENTS.md').write_text(ag)
    return dict(id=ident,name=name,kind=kind,title=title,path=str(folder),labels=labels)

def main():
    shim=setup_shim();selected=set(sys.argv[1:]);result=[]
    for index,spec in enumerate(SPECS):
        if selected and spec[0] not in selected:continue
        result.append(write_machine(spec,index,shim));print('Ported source:',result[-1]['id'])
    existing=json.loads((ROOT/'local/ports.json').read_text()) if selected and (ROOT/'local/ports.json').exists() else []
    failures=[]
    for port in result:
        folder=Path(port['path']);ident=port['id']
        probe=ROOT/'local'/('size-'+ident+'.cpp')
        probe.write_text('#include <cstdio>\n#include "'+str(folder/'sine.hpp')+'"\n#include "'+str(folder/'dsp.hpp')+'"\nint main(){printf("%zu",sizeof('+ident+'::port::Dsp));}\n')
        command=[CLANG+'++','-std=c++11','-O2','-Wall','-Werror',str(probe),'-o',str(probe.with_suffix(''))]
        compiled=subprocess.run(command,text=True,capture_output=True)
        if compiled.returncode:
            failures.append(ident);(ROOT/'local'/('error-'+ident+'.txt')).write_text(compiled.stderr)
            print('FAIL:',ident,compiled.stderr[:1800],flush=True)
            continue
        size=int(subprocess.check_output([str(probe.with_suffix(''))],text=True))
        wrappers(port,size);port['dspBytes']=size;display(port)
        print('Adapter:',ident,'DSP bytes:',size,flush=True)
    merged={p['id']:p for p in existing};merged.update({p['id']:p for p in result})
    (ROOT/'local/ports.json').write_text(json.dumps(list(merged.values()),indent=2)+'\n')
    print('Compiler failures:',failures,flush=True)

if __name__=='__main__':main()
