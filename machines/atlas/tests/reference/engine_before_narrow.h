// Original adapter and output path, bound to the frozen Q31 waveform reader.
#pragma once
#include "../../resample.h"
#include "wt_before_settled.h"
namespace atlas_dsp {
template <unsigned Rate, int Model, unsigned Frames>
static void macro_reference_render_e(struct macro_voice *m, const uint8_t *p, uint32_t inc, int16_t *out, int frames)
{
    const int n=Frames ? static_cast<int>(Frames) : frames;
    int32_t o[Frames>32?Frames:32], a[Frames>32?Frames:32], go, ga, mix;
    int i;
    if (m->latch) {                                 /* a note start: the engine, and its per-note choices */
        int e = macro_engine_of(p[MACRO_P_ENGINE]);
        m->latch = 0;
        if (e != m->engine) {
            m->engine = (uint8_t)e;
            engine_init(m);
        }
    }
    const uint32_t maximum_inc=Rate==44100 && Model==MACRO_CHORD ? 1168698583u : INC_MAX;
    if (inc > maximum_inc) inc=maximum_inc;
    if (inc < INC_MIN)
        inc = INC_MIN;
    /* F: OUT up to 55, AUX from 72, a crossfade between (only there are both outputs computed) */
    mix = p[MACRO_P_AUX] <= 55 ? 0 : p[MACRO_P_AUX] >= 72 ? 32767 : (p[MACRO_P_AUX] - 55) * 1927;
    switch (Model >= 0 ? Model : m->engine) {
    case MACRO_WAVETABLE:
        if (!HAS(MACRO_WAVETABLE)) goto none;
        wt_render_reference<wt_hermite_q31_reference>(&m->e.wt, p, inc, o, a, n);
        break;
    default:
    none:
        for (i = 0; i < n; i++)
            out[i] = 0;
        return;
    }
    if (Rate == 44100 && Model == MACRO_WSH) {
        // Prepared WSH curves and their sine blend stay inside signedQ15.
        // Fuse the output gains/crossfade with two signed16 products. The
        // generic path's +/-2 rails and post-gain clips cannot activate here.
        const int16_t main_gain=static_cast<int16_t>(gain_out[MACRO_WSH]);
        const int16_t aux_gain=static_cast<int16_t>(gain_aux[MACRO_WSH]);
        if (mix == 0 || mix >= 32767) {
            const int32_t* signal=mix ? a : o;
            const int16_t gain=mix ? aux_gain : main_gain;
            for (i=0;i<n;++i) out[i]=static_cast<int16_t>((static_cast<int16_t>(signal[i])*gain)>>15);
        } else {
            const int16_t mg=static_cast<int16_t>((main_gain*(32768-mix))>>15);
            const int16_t ag=static_cast<int16_t>((aux_gain*mix)>>15);
            for (i=0;i<n;++i) {
                const int16_t x=static_cast<int16_t>(o[i]);
                const int16_t y=static_cast<int16_t>(a[i]);
                out[i]=static_cast<int16_t>((x*mg+y*ag)>>15);
            }
        }
        return;
    }
    if(Rate==44100 && Model==MACRO_CHORD && mix>0 && mix<32767) {
        const uint16_t weight=static_cast<uint16_t>(mix);
        for(i=0;i<n;++i)out[i]=static_cast<int16_t>(chord_output_blend(o[i],a[i],weight));
        return;
    }
    go = gain_out[Model >= 0 ? Model : m->engine];
    ga = gain_aux[Model >= 0 ? Model : m->engine];
    if (mix == 0 || mix >= 32767) {                 /* OUT only (the default) or AUX only */
        /* x g / 2^15 on the EMAC: no product overflows, so no clip before it (the gains are over 0.5:
         * whatever CL2 would clip lands past +-1 either way) */
        const int32_t *x = mix ? a : o;
        int32_t g = defined_shift((mix ? ga : go), 16);
        for (i = 0; i < n; i++) {
            int32_t y = fmac1t(x[i], g);
            out[i] = (int16_t)(y > 32767 ? 32767 : y < -32768 ? -32768 : y);
        }
        return;
    }
    for (i = 0; i < n; i++) {
        int32_t x = (CL2(o[i]) * go) >> 15, y = (CL2(a[i]) * ga) >> 15;
        x = x > 32767 ? 32767 : x < -32768 ? -32768 : x;
        y = y > 32767 ? 32767 : y < -32768 ? -32768 : y;
        x += ((y - x) * mix) >> 15;
        out[i] = (int16_t)(x > 32767 ? 32767 : x < -32768 ? -32768 : x);
    }
}




static void macro_reference_render(struct macro_voice* m,const uint8_t* p,uint32_t inc,int16_t* out,int n){macro_reference_render_e<48000,-1,0>(m,p,inc,out,n);}
}

namespace atlas_reference {
using Voice=atlas::Voice;using Shared=atlas::Shared;
namespace {
unsigned normalized(opxy::i32 word) {
    return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 16;
}

inline void advance_source(Voice& voice,const opxy::u8* controls,const opxy::u32* pitch,opxy::u32& read,opxy::u32& write) {
    if(read==32) {
        const opxy::u32 input=*pitch;
        const opxy::u32 increment=(input/160u)*147u+((input%160u)*147u)/160u;
        atlas_dsp::macro_reference_render(&voice.oscillator,controls,increment,voice.cache,32);
        read=0;
    }
    const opxy::i16 sample=voice.cache[read++];
    voice.history[write]=voice.history[write+24]=sample;
    if(++write==24)write=0;
}
}



void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    atlas_dsp::macro_init(&voice.oscillator);
    voice.oscillator.rng = 0x9e3779b9u * owner + 1u;
    voice.pending = 1;
    voice.cache_read=32;voice.history_write=voice.resample_phase=0;
    for(auto& sample:voice.history)sample=0;
}
void start(Voice& voice, Shared&, opxy::u32) { voice.pending = 1; }
void destroy(Voice& voice, Shared&, opxy::u32) { voice.pending = 0; }

void render(Voice& voice, Shared&, opxy::u32, opxy::Output output,
            opxy::Parameters parameters, opxy::Pitch pitch) {
    const bool trigger = voice.pending != 0;
    voice.pending = 0;
    opxy::u8 controls[7] = {};
    controls[0] = 0;
    for (unsigned i = 1; i < 4; ++i)
        controls[i] = static_cast<opxy::u8>(normalized(parameters[i-1]) >> 8);
    controls[4] = static_cast<opxy::u8>(normalized(parameters[3]) >> 8);
    if (trigger) {
        atlas_dsp::macro_trig(&voice.oscillator);
        voice.cache_read=32;
    }
    // Keep the source DSP at its original 48 kHz so all time constants and
    // key-scaling tables retain their meaning. A rational polyphase filter
    // supplies the SDK's 44.1 kHz stream without software floating point.
    // Source DSP only observes oscillator/cache, so retain SRC indices in
    // callback locals and publish their identical endpoints once per block.
    opxy::u32 read=voice.cache_read,write=voice.history_write,phase=voice.resample_phase;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        // A phase in 0..146 consumes one source sample, or occasionally two.
        // Fold the first subtraction into the increment and test only the second.
        phase+=13;
        advance_source(voice,controls,pitch+i,read,write);
        if(phase>=147) {
            phase-=147;
            advance_source(voice,controls,pitch+i,read,write);
        }
        const opxy::i16* history=voice.history+write+23;
        const opxy::i32 sample=atlas::resample_sum(history,phase)>>15;
        output[i]=(sample<-32768?-32768:sample>32767?32767:sample)*16384;
    }
    voice.cache_read=read;voice.history_write=write;voice.resample_phase=phase;
}
}
