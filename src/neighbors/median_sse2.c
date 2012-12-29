/*
  median_sse2.c: Copyright (C) 2012  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of Tweak.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with the author; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include <stdint.h>
#include "neighbors.h"


/* from 'Implementing Median Filters in XC4000E FPGAs' by John L. Smith */
#define LOWHIGH(low, high) {\
    if (low > high) {\
        unsigned t = low;\
        low = high;\
        high = t;\
    }\
}\

static unsigned VS_CC get_median_9(unsigned m0, unsigned m1, unsigned m2,
                                   unsigned m3, unsigned m4, unsigned m5,
                                   unsigned m6, unsigned m7, unsigned m8)
{
    unsigned  x0, x1, x2, x3, x4, x5, x6, x7;

    x0 = m1;
    x1 = m2;
    LOWHIGH(x0, x1);
    x2 = m0;
    LOWHIGH(x0, x2);
    LOWHIGH(x1, x2);
    x3 = m4;
    x4 = m5;
    LOWHIGH(x3, x4);
    x5 = m3;
    LOWHIGH(x3, x5);
    LOWHIGH(x4, x5);
    if (x0 < x3) x0 = x3;
    x3 = m7;
    x6 = m8;
    LOWHIGH(x3, x6);
    x7 = m6;
    LOWHIGH(x3, x7);
    if (x0 < x3) x0 = x3;
    LOWHIGH(x6, x7);
    LOWHIGH(x4, x6);
    if (x1 < x4) x1 = x4;
    if (x1 > x6) x1 = x6;
    if (x5 > x7) x5 = x7;
    if (x2 > x5) x2 = x5;
    LOWHIGH(x1, x2);
    if (x0 < x1) x0 = x1;
    return x0 > x2 ? x2 : x0;
}
#undef LOWHIGH

#define LOWHIGHu8(X, Y) {\
    __m128i min = _mm_min_epu8(X, Y); \
    __m128i max = _mm_max_epu8(X, Y); \
    X = min; \
    Y = max; \
}

static void NBS_FUNC_ALIGN VS_CC
proc_8bit_sse2(int w, int h, int stride, uint8_t *dstp, const uint8_t *r1)
{
    const uint8_t *r0 = r1;
    for (int y = 0; y <= h; y++) {
        const uint8_t *r2 = r1 + stride * !!(h - y);

        dstp[0] = get_median_9(r0[0], r0[0], r0[1], r1[0], r1[0],
                               r1[1], r2[0], r2[0], r2[1]);
        int x;
        for (x = 1; x <= w - 16; x += 16) {
            __m128i x0 = _mm_loadu_si128((__m128i *)(r0 + x));
            __m128i x1 = _mm_loadu_si128((__m128i *)(r0 + x + 1));
            LOWHIGHu8(x0, x1);
            __m128i x2 = _mm_load_si128((__m128i *)(r0 + x - 1));
            LOWHIGHu8(x0, x2);
            LOWHIGHu8(x1, x2);
            __m128i x3 = _mm_loadu_si128((__m128i *)(r1 + x));
            __m128i x4 = _mm_loadu_si128((__m128i *)(r1 + x + 1));
            LOWHIGHu8(x3, x4);
            __m128i x5 = _mm_load_si128((__m128i *)(r1 + x - 1));
            LOWHIGHu8(x3, x5);
            LOWHIGHu8(x4, x5);
            x0 = _mm_max_epu8(x0, x3);
            x3 = _mm_loadu_si128((__m128i *)(r2 + x));
            __m128i x6 = _mm_loadu_si128((__m128i *)(r2 + x + 1));
            LOWHIGHu8(x3, x6);
            __m128i x7 = _mm_load_si128((__m128i *)(r2 + x - 1));
            LOWHIGHu8(x3, x7);
            LOWHIGHu8(x6, x7);
            x0 = _mm_max_epu8(x0, x3);
            LOWHIGHu8(x4, x6);
            x1 = _mm_max_epu8(x1, x4);
            x5 = _mm_min_epu8(x5, x7);
            x2 = _mm_min_epu8(x2, x5);
            x1 = _mm_min_epu8(x1, x6);
            LOWHIGHu8(x1, x2);
            x0 = _mm_max_epu8(x0, x1);
            x0 = _mm_min_epu8(x0, x2);
            _mm_storeu_si128((__m128i *)(dstp + x), x0);
        }
        while (x <= w) {
            int xl = x - 1;
            int xr = x + !!(w - x);
            dstp[x] = get_median_9(r0[xl], r0[x], r0[xr],
                                   r1[xl], r1[x], r1[xr],
                                   r2[xl], r2[x], r2[xr]);
            x++;
        }
        r0 = r1;
        r1 += stride;
        dstp += stride;
    }
}
#undef LOWHIGHu8

#define LOWHIGHu16(X, Y) {\
    __m128i tmp0 = _mm_subs_epu16(X, Y); \
    __m128i tmp1 = _mm_adds_epu16(tmp0, Y); \
    X = _mm_subs_epu16(X, tmp0); \
    Y = tmp1; \
}

#define HIGHu16(X, Y, Z) {\
    __m128i tmp = _mm_subs_epu16(Y, Z); \
    X = _mm_adds_epu16(tmp, Z); \
}

#define LOWu16(X, Y, Z) {\
    __m128i tmp = _mm_subs_epu16(Y, Z); \
    X = _mm_subs_epu16(Y, tmp); \
}

static void NBS_FUNC_ALIGN VS_CC
proc_16bit_sse2(int w, int h, int stride, uint8_t *d, const uint8_t *srcp)
{
    stride >>= 1;
    const uint16_t *r1 = (uint16_t *)srcp;
    const uint16_t *r0 = r1;
    uint16_t *dstp = (uint16_t *)d;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r2 = r1 + stride * !!(h -y);

        dstp[0] = get_median_9(r0[0], r0[0], r0[1], r1[0], r1[0],
                               r1[1], r2[0], r2[0], r2[1]);
        __m128i x0, x1, x2, x3, x4, x5, x6, x7;
        int x;
        for (x = 1; x <= w - 8; x += 8) {
            x0 = _mm_loadu_si128((__m128i *)(r0 + x));
            x1 = _mm_loadu_si128((__m128i *)(r0 + x + 1));
            LOWHIGHu16(x0, x1);
            x2 = _mm_load_si128((__m128i *)(r0 + x - 1));
            LOWHIGHu16(x0, x2);
            LOWHIGHu16(x1, x2);
            x3 = _mm_loadu_si128((__m128i *)(r1 + x));
            x4 = _mm_loadu_si128((__m128i *)(r1 + x + 1));
            LOWHIGHu16(x3, x4);
            x5 = _mm_load_si128((__m128i *)(r1 + x - 1));
            LOWHIGHu16(x3, x5);
            LOWHIGHu16(x4, x5);
            HIGHu16(x0, x0, x3);
            x3 = _mm_loadu_si128((__m128i *)(r2 + x));
            x6 = _mm_loadu_si128((__m128i *)(r2 + x + 1));
            LOWHIGHu16(x3, x6);
            x7 = _mm_load_si128((__m128i *)(r2 + x - 1));
            LOWHIGHu16(x3, x7);
            LOWHIGHu16(x6, x7);
            HIGHu16(x0, x0, x3);
            LOWHIGHu16(x4, x6);
            HIGHu16(x1, x1, x4);
            LOWu16(x5, x5, x7);
            LOWu16(x2, x2, x5);
            LOWu16(x1, x1, x6);
            LOWHIGHu16(x1, x2);
            HIGHu16(x0, x0, x1);
            LOWu16(x0, x0, x2);
            _mm_storeu_si128((__m128i *)(dstp + x), x0);
        }
        while (x <= w) {
            int xl = x - 1;
            int xr = x + !!(w - x);
            dstp[x] = get_median_9(r0[xl], r0[x], r0[xr],
                                   r1[xl], r1[x], r1[xr],
                                   r2[xl], r2[x], r2[xr]);
            x++;
        }
        r0 = r1;
        r1 += stride;
        dstp += stride;
    }
}
#undef LOWHIGHu16
#undef HIGHu16
#undef LOWu16


const proc_neighbors median[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
