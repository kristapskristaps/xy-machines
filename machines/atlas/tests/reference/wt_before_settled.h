// Frozen wavetable renderer before settled-coordinate skipping.
#pragma once
namespace atlas_dsp {
static inline int32_t wt_hermite_q31_reference(const int32_t *t, int32_t i, int32_t f)
{
    int32_t xm1=t[i],x0=t[i+1],x1=t[i+2],x2=t[i+3];
    int32_t c=(x1-xm1)>>1,v=x0-x1,w=c+v,a=w+v+((x2-x0)>>1),bn=w+a;
    return opxy::q31::multiply(opxy::q31::multiply(opxy::q31::multiply(a,f)-bn,f)+c,f)+x0;
}

template <int32_t(*Reader)(const int32_t*,int32_t,int32_t)=wt_hermite>
static void wt_render_reference(struct macro_wavetable *__restrict w, const uint8_t *p, uint32_t inc, int32_t *__restrict out,
                      int32_t *__restrict aux, int n)
{
    static const int32_t lg[3] = { -21098, -21098, -4850 };          /* log2 (1 - 0.2), (1 - 0.2), (1 - 0.05), Q16 */
    int32_t knob[3], tg[3], d[3], cur[3], q, lpc, i, k, sh, z;
    uint32_t f = w->f, ph = w->phase, in, r16;
    int32_t fd, prev = w->prev, dlp = w->dlp;
    knob[0] = defined_shift(k16(p[MACRO_P_TIMB]) * 7, 8);                       /* x 6.9999, Q24 */
    knob[1] = defined_shift(k16(p[MACRO_P_MORPH]) * 7, 8);
    knob[2] = defined_shift(k16(p[MACRO_P_HARM]) * 7, 8);
    for (k = 0; k < 3; k++)
        knob[k] -= knob[k] >> 16;                                   /* (7 - 0.0001) */
    if (w->pre[0] == WT_NEW)                                        /* the engine just started: no sweep */
        for (k = 0; k < 3; k++)
            w->pre[k] = w->tgt[k] = w->lp[k] = knob[k];
    for (k = 0; k < 3; k++) {
        int32_t cp = 65536 - (int32_t)exp2_q16(lg[k] * n / WT_BLOCK);
        // OP-XY: preserve the full smoother product before its six-bit shift.
        const int32_t delta=(knob[k]-w->pre[k])>>8, coefficient=cp>>2;
        w->pre[k] += (delta>>6)*coefficient+((delta&63)*coefficient>>6);
    }
    q = w->pre[2] - (defined_shift(3, 24));                                      /* quantization: Z - 3 in 0..1, Q24 */
    q = q < 0 ? 0 : q > (defined_shift(1, 24)) ? defined_shift(1, 24) : q;
    {   /* 2 f0 (4 - 3 quantization) in 0.01 .. 0.1, Q31; f0 = inc / 2^32 */
        uint32_t m = (4u << 24) - 3u * (uint32_t)q;                 /* Q24, 1..4 */
        uint32_t c = mulhi(inc, m << 5) << 3;                       /* f0 m x 2, Q31 (m << 6 would wrap at 4) */
        lpc = c < 21474836u ? 21474836 : c > 214748365u ? 214748365 : (int32_t)c;
    }
    for (k = 0; k < 3; k++) {                                       /* the target, its fraction snapped */
        int32_t ip = w->pre[k] & ~0xffffff, fr = w->pre[k] & 0xffffff;
        fr += fmac1(wt_snap(fr) - fr, q >= (defined_shift(1, 24)) ? 0x7fffffff : defined_shift(q, 7));   /* (1.0 << 7 would wrap) */
        tg[k] = ip + fr;
        d[k] = divide_block(tg[k] - w->tgt[k], n);
        cur[k] = w->tgt[k];
        w->tgt[k] = tg[k];
    }
    wt_cache(w,static_cast<unsigned>(n));
    fd = divide_block((int32_t)inc - (int32_t)f, n);
    /* y = d / (128 f), table units a table step, Q8: d x 2^25 / inc (as wtv_render) */
    for (z = 0, in = inc; !(in & 0x80000000u); z++)
        in <<= 1;
    r16 = 0xffffffffu / (in >> 16);
    sh = z - 12;
    {
        int32_t g31 = 2040109466 - (int32_t)(inc >> 1);             /* 0.95 - f0, Q31 */
        for (i = 0; i < n; i++) {
            int32_t pi,pf,mix,dd,y,c;
            f += (uint32_t)fd;
            for (k = 0; k < 3; k++) {
                cur[k] += d[k];
                w->lp[k] += fmac1(cur[k] - w->lp[k], lpc);
            }
            ph += f;
            pi=static_cast<int32_t>(ph>>25);
            pf=static_cast<int32_t>((ph<<7)>>1);
            mix=Reader(w->blend[w->active],pi,pf);
            if(w->fade) {
                const int32_t next=Reader(w->blend[w->active^1],pi,pf);
                const int32_t alpha=static_cast<int32_t>((64-w->fade)*33554432u);
                mix+=fmac1(next-mix,alpha);
                if(!--w->fade)w->active^=1;
            }
            dd = mix - prev;
            prev = mix;
            y = fmac1(defined_shift(dd, 6), (int32_t)(r16 << 14));
            y = sh >= 0 ? defined_shift(y, sh) : y >> -sh;
            c = f >= (1u << 25) ? 0x7fffffff : (int32_t)(f << 6);   /* min(128 f, 1) */
            dlp += fmac1(y - dlp, c);
            out[i] = fmac1(defined_shift(dlp, 2), g31) >> 5;                     /* x (0.95 - f) / 1024, Q15 */
            aux[i] = (out[i] / 1024) * 1024;                        /* int(x 32) / 32 */
        }
    }
    w->f = inc;
    w->phase = ph;
    w->prev = prev;
    w->dlp = dlp;
}
}
