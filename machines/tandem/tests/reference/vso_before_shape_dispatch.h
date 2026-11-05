// Frozen variable-shape kernel before block-level waveform specialization.
#pragma once
namespace tandem_dsp {
static void vso_reference(struct macro_vso *__restrict o, int sync, uint32_t mf, uint32_t sf, int32_t pw31, int32_t ws,
                       int32_t *__restrict out, int n)
{
    uint32_t up, down, mph = o->mph, sph = o->sph, m_f = o->mf, s_f = o->sf;
    int32_t mfd, sfd, pwd, wsd, pw = o->pw31, ppw = o->ppw31, w = o->ws, next = o->next, high = o->high, i;
    if (mf > 0x40000000u)
        mf = 0x40000000u;
    if (sf > 0x40000000u)
        sf = 0x40000000u;
    if (sf >= 0x40000000u)
        pw31 = defined_shift(1, 30);
    else if (pw31 < (int32_t)sf)                              /* within 2f .. 1 - 2f */
        pw31 = (int32_t)sf;
    else if (pw31 > 0x7fffffff - (int32_t)sf)
        pw31 = 0x7fffffff - (int32_t)sf;
    mfd = divide_block((int32_t)mf - (int32_t)m_f, n);
    sfd = divide_block((int32_t)sf - (int32_t)s_f, n);
    pwd = divide_block(pw31 - pw, n);
    wsd = divide_block(ws - w, n);
    slopes(pw31, &up, &down);
    for (i = 0; i < n; i++) {
        int32_t this_s = next, sq, tri, reset = 0, trans = 0, step;
        uint32_t rt = 0, pw32, old;
        int wrapped;
        next = 0;
        m_f += (uint32_t)mfd;
        s_f += (uint32_t)sfd;
        pw += pwd;
        w += wsd;
        sq = w > 32768 ? (w - 32768) * 2 : 0;
        tri = w < 32768 ? 65536 - 2 * w : 0;
        pw32 = (uint32_t)pw << 1;
        step = amt((int32_t)((((up + down) >> 1) * (s_f >> 16)) >> 2), tri);   /* (up + down) f tri, Q28 */
        if (sync) {
            old = mph;
            mph += m_f;
            if (mph < old) {                                     /* the master restarts: reset */
                uint32_t at, a0 = sph;
                int32_t value;
                rt = (uint32_t)sub_sample(mph, m_f);
                at = sph + mulq16(s_f, 65536 - rt);
                reset = 1;
                if (at < a0)
                    trans = 1;
                if (!high && at >= pw32)
                    trans = 1;
                value = vso_naive(at, pw32, up, down, tri, sq);
                this_s -= q28mul(value, bthis(rt));
                next -= q28mul(value, bnext(rt));
            }
        }
        old = sph;
        sph += s_f;
        wrapped = sph < old;
        if (trans || !reset) {
            for (;;) {
                if (!high) {
                    uint32_t t;
                    if (!wrapped && sph < pw32)
                        break;
                    t = (uint32_t)sub_sample(sph - pw32, (uint32_t)((int32_t)(((uint32_t)ppw << 1) - pw32) + (int32_t)s_f));
                    this_s += amt(bthis(t), sq);
                    next += amt(bnext(t), sq);
                    this_s -= q28mul(step, ithis(t));
                    next -= q28mul(step, inext(t));
                    high = 1;
                }
                if (high) {
                    uint32_t t;
                    if (!wrapped)
                        break;
                    wrapped = 0;
                    t = (uint32_t)sub_sample(sph, s_f);
                    this_s -= amt(bthis(t), 65536 - tri);
                    next -= amt(bnext(t), 65536 - tri);
                    this_s += q28mul(step, ithis(t));
                    next += q28mul(step, inext(t));
                    high = 0;
                }
            }
        }
        if (sync && reset) {
            sph = mulq16(s_f, rt);
            high = 0;
        }
        next += vso_naive(sph, pw32, up, down, tri, sq);
        ppw = pw;
        out[i] = (defined_shift(this_s, 1)) - (defined_shift(1, 28));
    }
    o->mph = mph;
    o->sph = sph;
    o->mf = mf;
    o->sf = sf;
    o->pw31 = pw31;
    o->ppw31 = ppw;
    o->ws = ws;
    o->next = next;
    o->high = high;
}

}
