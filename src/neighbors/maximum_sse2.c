/*
  maximum_sse2.c: Copyright (C) 2012  Oka Motofumi

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


static unsigned VS_CC get_max(unsigned x, unsigned y, unsigned z)
{
    if (x > y) {
        return x > z ? x : z;
    }
    return y > z ? y : z;
}


static void NBS_FUNC_ALIGN VS_CC
proc_tb_u8_sse2(const uint8_t *rt, const uint8_t *rb, int w, uint8_t *dstp)
{
    uint8_t maxt, maxb;
    maxt = get_max(rt[0], rt[1], rb[0]);
    dstp[0] = maxt > rb[1] ? maxt : rb[1];
    for (int x = 1; x < w - 16; x += 16) {
        __m128i srcl = _mm_load_si128((__m128i *)(rt + x - 1));
        __m128i srcc = _mm_loadu_si128((__m128i *)(rt + x ));
        __m128i srcr = _mm_loadu_si128((__m128i *)(rt + x + 1));
        __m128i srct = _mm_max_epu8(srcl, srcc);
        srct = _mm_max_epu8(srcr, srct);

        srcl = _mm_load_si128((__m128i *)rb);
        srcc = _mm_loadu_si128((__m128i *)(rb + 1));
        srcr = _mm_loadu_si128((__m128i *)(rb + 2));
        __m128i srcb = _mm_max_epu8(srcl, srcc);
        srcb = _mm_max_epu8(srcr, srcb);

        srct = _mm_max_epu8(srct, srcb);
        _mm_storeu_si128((__m128i *)(dstp + x), srct);
    }
    for (int x = w - 16; x < w; x++) {
        maxt = get_max(rt[x - 1], rt[x], rt[x + 1]);
        maxb = get_max(rb[x - 1], rb[x], rb[x + 1]);
        dstp[x] = maxt > maxb ? maxt : maxb;
    }
    maxb = get_max(rt[w - 1], rt[w], rb[w - 1]);
    dstp[w] = maxb > rb[w] ? maxb : rb[w];
}


static void NBS_FUNC_ALIGN VS_CC
proc_tb_u16_sse2(const uint16_t *rt, const uint16_t *rb, int w, uint16_t *dstp)
{
    uint16_t maxt, maxb;
    maxt = get_max(rt[0], rt[1], rb[0]);
    dstp[0] = maxt > rb[1] ? maxt : rb[1];
    for (int x = 1; x < w - 8; x += 8) {
        __m128i src = _mm_load_si128((__m128i *)(rt + x - 1));
        __m128i max = _mm_loadu_si128((__m128i *)(rt + x));
        max = _mm_subs_epu16(max, src);
        max = _mm_adds_epu16(src, max);

        src = _mm_loadu_si128((__m128i *)(rt + x + 1));
        max = _mm_subs_epu16(max, src);
        max = _mm_adds_epu16(src, max);

        src = _mm_load_si128((__m128i *)(rb + x - 1));
        max = _mm_subs_epu16(max, src);
        max = _mm_adds_epu16(src, max);

        src = _mm_loadu_si128((__m128i *)(rb + x));
        max = _mm_subs_epu16(max, src);
        max = _mm_adds_epu16(src, max);

        src = _mm_loadu_si128((__m128i *)(rb + x + 1));
        max = _mm_subs_epu16(max, src);
        max = _mm_adds_epu16(src, max);

        _mm_storeu_si128((__m128i *)(dstp + x), max);
    }
    for (int x = w - 8; x < w; x++) {
        maxt = get_max(rt[x - 1], rt[x], rt[x + 1]);
        maxb = get_max(rb[x - 1], rb[x], rb[x + 1]);
        dstp[x] = maxt > maxb ? maxt : maxb;
    }
    maxb = get_max(rt[w - 1], rt[w], rb[w - 1]);
    dstp[w] = maxb > rb[w] ? maxb : rb[w];
}


static void NBS_FUNC_ALIGN VS_CC
proc_8bit_sse2(int w, int h, int stride, uint8_t *dstp, const uint8_t *r1)
{
    const uint8_t *r0 = r1;
    const uint8_t *r2 = r1 + stride;

    proc_tb_u8_sse2(r1, r2, w, dstp);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        uint8_t max0 = get_max(r0[0], r1[0], r2[0]);
        uint8_t max1 = get_max(r0[1], r1[1], r2[1]);
        dstp[0] = max0 > max1 ? max0 : max1;

        for (int x = 1; x < w - 16; x += 16) {
            __m128i src = _mm_load_si128((__m128i *)(r0 + x - 1));
            __m128i max = _mm_load_si128((__m128i *)(r1 + x - 1));
            max = _mm_max_epu8(src, max);

            src = _mm_load_si128((__m128i *)(r2 + x - 1));
            max = _mm_max_epu8(src, max);

            src = _mm_loadu_si128((__m128i *)(r0 + x));
            max = _mm_max_epu8(src, max);

            src = _mm_loadu_si128((__m128i *)(r1 + x));
            max = _mm_max_epu8(src, max);

            src = _mm_loadu_si128((__m128i *)(r2 + x));
            max = _mm_max_epu8(src, max);

            src = _mm_loadu_si128((__m128i *)(r0 + x + 1));
            max = _mm_max_epu8(src, max);

            src = _mm_loadu_si128((__m128i *)(r1 + x + 1));
            max = _mm_max_epu8(src, max);

            src = _mm_loadu_si128((__m128i *)(r2 + x + 1));
            max = _mm_max_epu8(src, max);

            _mm_storeu_si128((__m128i *)(dstp + x), max);
        }
        for (int x = w - 16; x < w; x++) {
            max0 = get_max(r0[x - 1], r0[x], r0[x + 1]);
            max1 = get_max(r1[x - 1], r1[x], r1[x + 1]);
            uint8_t max2 = get_max(r2[x - 1], r2[x], r2[x + 1]);
            dstp[x] = get_max(max0, max1, max2);
        }

        max0 = get_max(r0[w - 1], r1[w - 1], r2[w - 1]);
        max1 = get_max(r0[w], r1[w], r2[w]);
        dstp[w] = max0 > max1 ? max0 : max1;

        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }

    proc_tb_u8_sse2(r0, r1, w, dstp);
}


static void VS_CC
proc_16bit_sse2(int w, int h, int stride, uint8_t *d, const uint8_t *srcp)
{
    stride >>= 1;

    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *r0 = (uint16_t *)srcp;
    const uint16_t *r1 = r0;
    const uint16_t *r2 = r1 + stride;

    proc_tb_u16_sse2(r1, r2, w, dstp);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        uint16_t max0 = get_max(r0[0], r1[0], r2[0]);
        uint16_t max1 = get_max(r0[1], r1[1], r2[1]);
        dstp[0] = max0 > max1 ? max0 : max1;

        for (int x = 1; x < w - 8; x += 8) {
            __m128i src = _mm_load_si128((__m128i *)(r0 + x - 1));
            __m128i max = _mm_load_si128((__m128i *)(r1 + x - 1));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_load_si128((__m128i *)(r2 + x - 1));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_loadu_si128((__m128i *)(r0 + x));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_loadu_si128((__m128i *)(r1 + x));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_loadu_si128((__m128i *)(r2 + x));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_loadu_si128((__m128i *)(r0 + x + 1));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_loadu_si128((__m128i *)(r1 + x + 1));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            src = _mm_loadu_si128((__m128i *)(r2 + x + 1));
            max = _mm_subs_epu16(max, src);
            max = _mm_adds_epu16(src, max);

            _mm_storeu_si128((__m128i *)(dstp + x), max);
        }
        for (int x = w - 8; x < w; x++) {
            max0 = get_max(r0[x - 1], r0[x], r0[x + 1]);
            max1 = get_max(r1[x - 1], r1[x], r1[x + 1]);
            uint16_t max2 = get_max(r2[x - 1], r2[x], r2[x + 1]);
            dstp[x] = get_max(max0, max1, max2);
        }

        max0 = get_max(r0[w - 1], r1[w - 1], r2[w - 1]);
        max1 = get_max(r0[w], r1[w], r2[w]);
        dstp[w] = max0 > max1 ? max0 : max1;

        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }

    proc_tb_u16_sse2(r0, r1, w, dstp);
}


const proc_neighbors maximum[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
