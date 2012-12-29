/*
  minimum_sse2.c: Copyright (C) 2012  Oka Motofumi

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

#define PROC_NEIGHBORS
#include "neighbors.h"


static void NBS_FUNC_ALIGN VS_CC
proc_8bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *dstp, const uint8_t *srcp)
{
    uint8_t *p0 = buff + 16;
    uint8_t *p1 = p0 + bstride;
    uint8_t *p2 = p1 + bstride;
    uint8_t *orig = p0, *end = p2;

    line_copy8(p0, srcp, width);
    line_copy8(p1, srcp, width);
    srcp += stride;

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width);

        for (int x = 0; x < width; x += 16) {
            __m128i src = _mm_loadu_si128((__m128i *)(p0 + x - 1));
            __m128i min = _mm_loadu_si128((__m128i *)(p1 + x - 1));
            min = _mm_min_epu8(src, min);

            src = _mm_loadu_si128((__m128i *)(p2 + x - 1));
            min = _mm_min_epu8(src, min);

            src = _mm_load_si128((__m128i *)(p0 + x));
            min = _mm_min_epu8(src, min);

            src = _mm_load_si128((__m128i *)(p1 + x));
            min = _mm_min_epu8(src, min);

            src = _mm_load_si128((__m128i *)(p2 + x));
            min = _mm_min_epu8(src, min);

            src = _mm_loadu_si128((__m128i *)(p0 + x + 1));
            min = _mm_min_epu8(src, min);

            src = _mm_loadu_si128((__m128i *)(p1 + x + 1));
            min = _mm_min_epu8(src, min);

            src = _mm_loadu_si128((__m128i *)(p2 + x + 1));
            min = _mm_min_epu8(src, min);

            _mm_store_si128((__m128i *)(dstp + x), min);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


static void NBS_FUNC_ALIGN VS_CC
proc_16bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *d, const uint8_t *s)
{
    stride >>= 1;
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *srcp = (uint16_t *)s;

    bstride >>= 1;
    uint16_t *p0 = (uint16_t *)buff + 8;
    uint16_t *p1 = p0 + bstride;
    uint16_t *p2 = p1 + bstride;
    uint16_t *orig = p0, *end = p2;

    line_copy16(p0, srcp, width);
    line_copy16(p1, srcp, width);

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width);
    
        for (int x = 0; x < width; x += 8) {
            __m128i src = _mm_loadu_si128((__m128i *)(p0 + x - 1));
            __m128i min = _mm_loadu_si128((__m128i *)(p1 + x - 1));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_loadu_si128((__m128i *)(p2 + x - 1));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_load_si128((__m128i *)(p0 + x));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_load_si128((__m128i *)(p1 + x));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_load_si128((__m128i *)(p2 + x));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_loadu_si128((__m128i *)(p0 + x + 1));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_loadu_si128((__m128i *)(p1 + x + 1));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            src = _mm_loadu_si128((__m128i *)(p2 + x + 1));
            min = _mm_subs_epu16(src, min);
            min = _mm_subs_epu16(src, min);

            _mm_store_si128((__m128i *)(dstp + x), min);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


const proc_neighbors minimum[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
