// Copyright 2014 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//

#pragma once
#include <opxy/types.hpp>
#include <opxy/q31.hpp>
namespace matter_dsp {
using int32_t=opxy::i32;
using uint32_t=opxy::u32;
inline int32_t product_hi(int32_t a, int32_t b) {
    if (a == (-2147483647 - 1) && b == (-2147483647 - 1)) return (-2147483647 - 1);
    return opxy::q31::multiply(a, b);
}
inline int32_t fmac2(int32_t a, int32_t x, int32_t b, int32_t y) {
    const uint32_t tail = (((uint32_t)a * (uint32_t)x >> 23) & 255u)
                        + (((uint32_t)b * (uint32_t)y >> 23) & 255u) + 128u;
    return (int32_t)((uint32_t)product_hi(a,x) + (uint32_t)product_hi(b,y) + (tail >> 8));
}
inline int32_t fmac1(int32_t a, int32_t x) {
    return (int32_t)((uint32_t)product_hi(a,x) + (((uint32_t)a * (uint32_t)x >> 30) & 1u));
}
inline int32_t fmac1t(int32_t a, int32_t x) { return product_hi(a,x); }
struct emac_save { int32_t unused; };
inline void emac_enter(emac_save*) {}
inline void emac_leave(const emac_save*) {}


inline int clz32(uint32_t v)
{
    int z = 0;
    if (!(v & 0xffff0000u)) { z += 16; v <<= 16; }
    if (!(v & 0xff000000u)) { z += 8; v <<= 8; }
    if (!(v & 0xf0000000u)) { z += 4; v <<= 4; }
    if (!(v & 0xc0000000u)) { z += 2; v <<= 2; }
    if (!(v & 0x80000000u)) z += 1;
    return z;
}


inline uint32_t mulhi(uint32_t a, uint32_t b)
{
    uint32_t ah = a >> 16, al = a & 0xffff, bh = b >> 16, bl = b & 0xffff;
    uint32_t m1 = ah * bl, m2 = al * bh, lo = al * bl;
    return ah * bh + (m1 >> 16) + (m2 >> 16) + (((m1 & 0xffff) + (m2 & 0xffff) + (lo >> 16)) >> 16);
}


static const uint32_t pitch_semi[12] = {
    2147483648u, 2275179671u, 2410468894u, 2553802834u, 2705659852u, 2866546760u,
    3037000500u, 3217589947u, 3408917802u, 3611622603u, 3826380858u, 4053909305u,
};
static const uint32_t pitch_fine[256] = {
    2147483648u, 2147968248u, 2148452957u, 2148937775u, 2149422703u, 2149907740u,
    2150392887u, 2150878143u, 2151363509u, 2151848984u, 2152334569u, 2152820263u,
    2153306067u, 2153791980u, 2154278004u, 2154764136u, 2155250379u, 2155736731u,
    2156223193u, 2156709765u, 2157196447u, 2157683238u, 2158170140u, 2158657151u,
    2159144272u, 2159631503u, 2160118844u, 2160606295u, 2161093856u, 2161581527u,
    2162069308u, 2162557199u, 2163045200u, 2163533311u, 2164021532u, 2164509864u,
    2164998306u, 2165486858u, 2165975520u, 2166464293u, 2166953175u, 2167442169u,
    2167931272u, 2168420486u, 2168909810u, 2169399245u, 2169888790u, 2170378446u,
    2170868212u, 2171358088u, 2171848076u, 2172338173u, 2172828382u, 2173318700u,
    2173809130u, 2174299670u, 2174790321u, 2175281083u, 2175771955u, 2176262939u,
    2176754033u, 2177245237u, 2177736553u, 2178227980u, 2178719517u, 2179211165u,
    2179702925u, 2180194795u, 2180686776u, 2181178868u, 2181671072u, 2182163386u,
    2182655811u, 2183148348u, 2183640996u, 2184133755u, 2184626625u, 2185119606u,
    2185612699u, 2186105903u, 2186599218u, 2187092644u, 2187586182u, 2188079831u,
    2188573592u, 2189067464u, 2189561447u, 2190055542u, 2190549748u, 2191044066u,
    2191538496u, 2192033037u, 2192527690u, 2193022454u, 2193517330u, 2194012317u,
    2194507417u, 2195002628u, 2195497950u, 2195993385u, 2196488931u, 2196984590u,
    2197480360u, 2197976241u, 2198472235u, 2198968341u, 2199464559u, 2199960888u,
    2200457330u, 2200953884u, 2201450549u, 2201947327u, 2202444217u, 2202941219u,
    2203438333u, 2203935560u, 2204432898u, 2204930349u, 2205427912u, 2205925587u,
    2206423375u, 2206921275u, 2207419287u, 2207917412u, 2208415649u, 2208913999u,
    2209412461u, 2209911035u, 2210409722u, 2210908522u, 2211407434u, 2211906458u,
    2212405596u, 2212904845u, 2213404208u, 2213903683u, 2214403271u, 2214902972u,
    2215402785u, 2215902712u, 2216402751u, 2216902903u, 2217403167u, 2217903545u,
    2218404036u, 2218904639u, 2219405356u, 2219906185u, 2220407128u, 2220908183u,
    2221409352u, 2221910633u, 2222412028u, 2222913536u, 2223415157u, 2223916892u,
    2224418739u, 2224920700u, 2225422774u, 2225924961u, 2226427262u, 2226929676u,
    2227432204u, 2227934844u, 2228437599u, 2228940466u, 2229443447u, 2229946542u,
    2230449750u, 2230953072u, 2231456507u, 2231960056u, 2232463719u, 2232967495u,
    2233471385u, 2233975388u, 2234479506u, 2234983737u, 2235488082u, 2235992540u,
    2236497113u, 2237001799u, 2237506600u, 2238011514u, 2238516542u, 2239021684u,
    2239526940u, 2240032310u, 2240537794u, 2241043393u, 2241549105u, 2242054931u,
    2242560872u, 2243066927u, 2243573095u, 2244079379u, 2244585776u, 2245092288u,
    2245598914u, 2246105654u, 2246612509u, 2247119478u, 2247626561u, 2248133759u,
    2248641071u, 2249148498u, 2249656039u, 2250163695u, 2250671465u, 2251179350u,
    2251687350u, 2252195464u, 2252703693u, 2253212037u, 2253720495u, 2254229068u,
    2254737756u, 2255246558u, 2255755475u, 2256264508u, 2256773655u, 2257282917u,
    2257792294u, 2258301786u, 2258811392u, 2259321114u, 2259830951u, 2260340903u,
    2260850970u, 2261361152u, 2261871449u, 2262381861u, 2262892389u, 2263403032u,
    2263913790u, 2264424663u, 2264935651u, 2265446755u, 2265957974u, 2266469309u,
    2266980759u, 2267492324u, 2268004005u, 2268515801u, 2269027713u, 2269539740u,
    2270051883u, 2270564141u, 2271076515u, 2271589004u, 2272101610u, 2272614330u,
    2273127167u, 2273640119u, 2274153187u, 2274666371u,
};

struct uf { int32_t m, e; };
struct svf_c { int32_t a1,a2,a3,k2; };

inline struct uf uf_q(uint32_t v, int32_t q)               /* v / 2^q */
{
    struct uf r;
    int z;
    if (!v) {
        r.m = 0;
        r.e = -128;
        return r;
    }
    z = clz32(v);
    if (z) {
        r.m = (int32_t)(v << (z - 1));
        r.e = 31 - z - q;
    } else {
        r.m = (int32_t)(v >> 1);
        r.e = 31 - q;
    }
    return r;
}

inline struct uf uf_mul(struct uf a, struct uf b)
{
    struct uf r;
    int32_t p;
    if (!a.m || !b.m)
        return uf_q(0, 0);
    p = fmac1(a.m, b.m);                                            /* [2^29, 2^31) */
    if (p & 0x40000000) {
        r.m = p;
        r.e = a.e + b.e + 1;
    } else {
        r.m = p << 1;
        r.e = a.e + b.e;
    }
    return r;
}

inline struct uf uf_add(struct uf a, struct uf b)            /* a, b >= 0 */
{
    int32_t d;
    uint32_t s;
    if (!b.m)
        return a;
    if (!a.m)
        return b;
    if (a.e < b.e) {
        struct uf t = a;
        a = b;
        b = t;
    }
    d = a.e - b.e;
    s = (uint32_t)a.m + (d > 30 ? 0 : (uint32_t)b.m >> d);
    if (s & 0x80000000u) {
        a.m = (int32_t)(s >> 1);
        a.e++;
    } else
        a.m = (int32_t)s;
    return a;
}

/* 1 / a: a 16-bit quotient, then two Newton steps */
inline struct uf uf_recip(struct uf a)
{
    struct uf o;
    int32_t r, d, k;
    o.e = -a.e - 1;
    if (a.m == 0x40000000) {
        o.m = a.m;
        o.e = -a.e;
        return o;
    }
    r = (int32_t)((0x40000000u / (uint32_t)(a.m >> 15)) << 16);     /* 2^61 / m, in (2^30, 2^31) */
    if (r <= 0)
        r = 0x7fffffff;
    for (k = 0; k < 2; k++) {
        d = 0x40000000 - fmac1(a.m, r);                             /* (1 - m r / 2^61) 2^30 */
        r += fmac1(r, d * 2);
        if (r <= 0)
            r = 0x7fffffff;
    }
    o.m = r;
    return o;
}

inline uint32_t uf_to(struct uf a, int32_t q)               /* a 2^q, saturated */
{
    int32_t sh = a.e - 30 + q;
    if (!a.m)
        return 0;
    if (sh > 1)
        return 0xffffffffu;
    if (sh == 1)
        return (uint32_t)a.m << 1;
    return sh < -31 ? 0 : (uint32_t)a.m >> -sh;
}

/* 2^(semitones / 12), semitones Q16, floored to 1/256 semitone as SemitonesToRatio reads its tables */
inline struct uf uf_semis(int32_t semis)
{
    int32_t q = semis >> 8, oct = (q + 3072 * 16) / 3072 - 16, r = q - oct * 3072;
    struct uf x = uf_q(mulhi(pitch_semi[r >> 8], pitch_fine[r & 255]), 30);
    x.e += oct;
    return x;
}

/* OnePole::tan<FREQUENCY_FAST>: f (pi + f^2 (a + b f^2)), f a phase increment up to 0.499 (below 2^31) */
inline struct uf tan_fast(uint32_t finc)
{
    int32_t f2 = fmac1((int32_t)finc, (int32_t)finc) >> 1;          /* Q32, up to 0.25 */
    int32_t p = 678339520 + fmac1(f2, 1871914112);            /* a + b f^2, Q26 */
    p = 421657428 + fmac1(f2, p);                       /* Q27 */
    return uf_mul(uf_q(finc, 32), uf_q((uint32_t)p, 27));
}

/* a trapezoidal SVF's a1 = 1 / D, a2 = g / D, a3 = g^2 / D, D = 1 + g (g + 1/q), as q / (q (1 + g^2) + g) */
inline void svf_hq(struct svf_c *c, struct uf g, struct uf q)
{
    const struct uf one = {0x40000000, 0};
    struct uf a1, a2, a3;
    uint32_t v;
    a1 = uf_mul(q, uf_recip(uf_add(uf_mul(q, uf_add(one, uf_mul(g, g))), g)));
    a2 = uf_mul(g, a1);
    a3 = uf_mul(g, a2);
    v = uf_to(a1, 31);
    c->a1 = v > 0x7fffffffu ? 0x7fffffff : (int32_t)v;
    v = uf_to(a2, 31);
    c->a2 = v > 0x7fffffffu ? 0x7fffffff : (int32_t)v;
    v = uf_to(a3, 31);
    c->a3 = v > 0x7fffffffu ? 0x7fffffff : (int32_t)v;
    c->k2 = 0;
}


}
