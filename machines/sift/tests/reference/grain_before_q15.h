// Frozen Q24 grain renderer and adapter before the Q15 signal redesign.
#pragma once
namespace sift_dsp {
static void grain_reference_render(struct macro_grain *__restrict z, const uint8_t *p, uint32_t inc, int32_t *__restrict out, int32_t *__restrict aux, int n,
                         int want_out, int want_aux)
{
    int32_t harm = k16(p[MACRO_P_HARM]), timb = k16(p[MACRO_P_TIMB]), morph = k16(p[MACRO_P_MORPH]), i, j;
    int32_t lf0 = log2_q16(inc), G;
    uint32_t c_inc = inc > 0x20000000u ? 0x20000000u : inc;      /* the carrier: at most 0.125 */
    {
        uint32_t f = inc >> 1;                                     /* 0.3 f0: the DC blockers */
        G = 0x7fffffff - recip27((1 << 27) + tan_dirty27(fmac1((int32_t)f, Q31(0.3))));
    }
    if (want_out) {
        struct carrier_c cc;
        int32_t bleed, inv, shape, lim;
        uint32_t f_inc[2];
        /* f1 = NoteToFrequency(24 + 84 TIMBRE); the second x 2^((48 HARMONICS - 24) / 12) */
        f_inc[0] = inc_of_log2(1653513 + ((84 * timb - 45 * 65536) / 12));
        f_inc[1] = inc_of_log2(1653513 + ((84 * timb - 45 * 65536) / 12) + 4 * harm - 2 * 65536);
        for (j = 0; j < 2; j++)
            if (f_inc[j] > 0x40000000u)
                f_inc[j] = 0x40000000u;
        /* bleed: cb (2 - cb), cb = 1 - 2 HARMONICS below the middle; the grainlet's 1 / (1 + bleed) */
        bleed = harm < 32768 ? (32768 - harm) * 2 - (harm == 0) : 0;          /* cb, Q16 (65535 at most) */
        bleed = defined_shift((int32_t)(((uint32_t)bleed * (uint32_t)(131072 - bleed)) >> 16), 8);   /* Q24 */
        inv = recip_q((defined_shift(1, 24)) + bleed, 24);
        /* shape 0.33 + (MORPH - 0.33) max(1 - 24 f0, 0) */
        lim = inc >= 0x0aaaaaaau ? 0 : 65536 - (int32_t)((inc >> 12) * 24 >> 4);
        shape = 21627 + (((morph - 21627) * (lim >> 1)) >> 15);
        carrier_coefs(&cc, shape < 0 ? 0 : shape > 65535 ? 65535 : shape);
        for (i = 0; i < n; i++)
            out[i] = 0;
        for (j = 0; j < 2; j++) {
            uint32_t fi = f_inc[j], gc = z->gc[j], gf = z->gf[j];   /* (in locals: registers) */
            int32_t gn = z->gnext[j];
            for (i = 0; i < n; i++) {
                int32_t this_s = gn, next = 0, g;
                uint32_t old = gc;
                gc += c_inc;
                if (gc < old) {                                  /* the carrier restarts: the formant too */
                    int32_t rt = sub_sample(gc, c_inc), before, after, disc;
                    before = fmac1(defined_shift(carrier24(&cc, 0x80000000u), 7),
                                   sine24(gf + mul_inc(fi, (uint32_t)(65536 - rt))) + bleed);
                    after = fmac1(defined_shift(carrier24(&cc, 0), 7), bleed);
                    disc = fmac1(after - before, inv);
                    this_s += fmac1(disc, defined_shift(blep_q15(rt), 16));
                    next -= fmac1(disc, defined_shift(blep_q15(65536 - rt), 16));
                    gf = mul_inc(fi, (uint32_t)rt);
                } else
                    gf += fi;
                g = fmac1(defined_shift(carrier24(&cc, gc >> 1), 7), sine24(gf) + bleed);
                next += fmac1(g, inv);
                gn = next;
                out[i] += this_s;
            }
            z->gc[j] = gc;
            z->gf[j] = gf;
            z->gnext[j] = gn;
        }
        for (i = 0; i < n; i++) {                               /* the DC blocker (a one-pole high-pass) */
            int32_t lp;
            ONEPOLE_LP(z->dc[0], G, out[i], lp);
            out[i] = (out[i] - lp) >> 9;
        }
    }
    if (want_aux) {
        /* the formant: NoteToFrequency(note + 96 TIMBRE), at most 0.25; the shape MORPH; the mode HARMONICS */
        uint32_t fi = inc_of_log2(lf0 + 8 * timb), ps;
        int32_t offset, s2 = 0, lowshape = morph < 32768, i2;
        if (fi > 0x40000000u)
            fi = 0x40000000u;
        if (harm < 21823) {                                     /* 0.333 */
            offset = defined_shift(1, 24);
            ps = 0x40000000u + (uint32_t)harm * 98304u;          /* 0.25 + 1.5 mode */
        } else {
            ps = 0xbfdf3b64u - (uint32_t)(harm - 21627) * 49152u;   /* 0.7495 - 0.75 (mode - 0.33) */
            offset = harm < 43647 ? -sine24(ps) : Q24(0.001);
        }
        if (lowshape)
            s2 = defined_shift(morph, 16);                                     /* 2 shape, Q31 */
#define ZFN(C, D, F, R) do {                                                                       \
            int32_t rd_ = (sine24(((uint32_t)(D)) + 0x40000000u) + (1 << 24)) >> 1;               \
            int32_t ct_;                                                                           \
            if (lowshape) {                                                                        \
                if ((C) >= 0x40000000u)                                                            \
                    rd_ = fmac1(rd_, s2);                                                          \
                ct_ = (1 << 24) + fmac1(sine24(((uint32_t)(C) << 1) + 0x40000000u) - (1 << 24), s2); \
            } else                                                                                 \
                ct_ = sine24(((uint32_t)(C) << 1) + ((uint32_t)morph << 15));                     \
            R = defined_shift(fmac1((fmac1(defined_shift(rd_,7), offset + sine24((F) + ps)) - offset), defined_shift(ct_,6)),1);       \
        } while (0)
        for (i = 0; i < n; i++) {
            int32_t this_s = z->znext, next = 0, v;
            uint32_t zi = c_inc >> 1;                             /* f0, Q31 */
            z->zd += c_inc;                                       /* 2 f0 */
            z->zc += zi;
            if (z->zd >= 0x80000000u) {
                int32_t rt, before, after, disc;
                uint32_t cb, ca;
                z->zd -= 0x80000000u;
                rt = sub_sample(z->zd, c_inc);
                cb = z->zc >= 0x80000000u ? 0x80000000u : 0x40000000u;
                ca = z->zc >= 0x80000000u ? 0u : 0x40000000u;
                ZFN(cb, 0x80000000u, z->zf + mul_inc(fi, (uint32_t)(65536 - rt)), before);
                ZFN(ca, 0u, 0u, after);
                disc = after - before;
                this_s += fmac1(disc, defined_shift(blep_q15(rt), 16));
                next -= fmac1(disc, defined_shift(blep_q15(65536 - rt), 16));
                z->zf = mul_inc(fi, (uint32_t)rt);
                if (z->zc > 0x80000000u)
                    z->zc = z->zd >> 1;
            } else
                z->zf += fi;
            if (z->zc >= 0x80000000u)
                z->zc -= 0x80000000u;
            ZFN(z->zc, z->zd, z->zf, v);
            next += v;
            z->znext = next;
            aux[i] = this_s;
        }
#undef ZFN
        for (i2 = 0; i2 < n; i2++) {
            int32_t lp;
            ONEPOLE_LP(z->dc[1], G, aux[i2], lp);
            aux[i2] = (aux[i2] - lp) >> 9;
        }
    }
}

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
    case MACRO_GRAIN:
        if (!HAS(MACRO_GRAIN)) goto none;
        grain_reference_render(&m->e.grain, p, inc, o, a, n, mix < 32767, mix > 0);
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

namespace sift_reference {
using Voice=sift::Voice;using Shared=sift::Shared;
namespace {
unsigned normalized(opxy::i32 word) {
    return word < 0 ? 0u : static_cast<opxy::u32>(word) >> 16;
}

inline void advance_source(Voice& voice,const opxy::u8* controls,const opxy::u32* pitch,opxy::u32& read,opxy::u32& write) {
    if(read==32) {
        const opxy::u32 input=*pitch;
        const opxy::u32 increment=(input/160u)*147u+((input%160u)*147u)/160u;
        sift_dsp::macro_reference_render(&voice.oscillator,controls,increment,voice.cache,32);
        read=0;
    }
    const opxy::i16 sample=voice.cache[read++];
    voice.history[write]=voice.history[write+24]=sample;
    if(++write==24)write=0;
}
}



void initialize(Voice& voice, Shared&, opxy::u32 owner) {
    sift_dsp::macro_init(&voice.oscillator);
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
        sift_dsp::macro_trig(&voice.oscillator);
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
        const opxy::i32 sample=sift::resample_sum(history,phase)>>15;
        output[i]=(sample<-32768?-32768:sample>32767?32767:sample)*16384;
    }
    voice.cache_read=read;voice.history_write=write;voice.resample_phase=phase;
}
}
