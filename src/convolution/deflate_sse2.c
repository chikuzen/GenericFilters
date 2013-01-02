/*
  deflate_sse2.c: Copyright (C) 2012  Oka Motofumi

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

#include <stdio.h>
#include "xxflate.h"
#include "sse2.h"


static void TWK_FUNC_ALIGN VS_CC
proc_8bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *dstp, const uint8_t *srcp, int th)
{
    uint8_t *p0 = buff + 16;
    uint8_t *p1 = p0 + bstride;
    uint8_t *p2 = p1 + bstride;
    uint8_t *orig = p0, *end = p2;

    uint8_t threshold = (uint8_t)th;

    line_copy8(p0, srcp, width, 1);
    line_copy8(p1, srcp, width, 1);
    srcp += stride;

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 16) {
            __m128i sumlo = _mm_setzero_si128();
            __m128i sumhi = _mm_setzero_si128();

            uint8_t *coordinates[] = {
                p0 + x - 1, p0 + x, p0 + x + 1,
                p1 + x - 1, p1 + x + 1,
                p2 + x - 1, p2 + x, p2 + x + 1
            };
            __m128i zero = _mm_setzero_si128();

            for (int i = 0; i < 8; i++) {
                __m128i target = _mm_loadu_si128((__m128i *)coordinates[i]);
                sumlo  = _mm_add_epi16(sumlo, _mm_unpacklo_epi8(target, zero));
                sumhi  = _mm_add_epi16(sumhi, _mm_unpackhi_epi8(target, zero));
            }

            sumlo = _mm_srai_epi16(sumlo, 3);
            sumhi = _mm_srai_epi16(sumhi, 3);
            sumlo = _mm_packus_epi16(sumlo, sumhi);

            __m128i src = _mm_load_si128((__m128i *)(p1 + x));

            sumlo = _mm_min_epu8(sumlo, src);

            __m128i thrs = _mm_subs_epu8(src, _mm_set1_epi8(threshold));

            sumlo = _mm_max_epu8(sumlo, thrs);

            _mm_store_si128((__m128i *)(dstp + x), sumlo);
        }

        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


static void TWK_FUNC_ALIGN VS_CC
proc_16bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *d, const uint8_t *s, int th)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride >>= 1;
    bstride >>= 1;
    int16_t threshold = (int16_t)th;

    uint16_t *p0 = (uint16_t *)buff + 8;
    uint16_t *p1 = p0 + bstride;
    uint16_t *p2 = p1 + bstride;
    uint16_t *orig = p0, *end = p2;

    line_copy16(p0, srcp, width, 1);
    line_copy16(p1, srcp, width, 1);
    srcp += stride;

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 8) {
            __m128i sumlo = _mm_setzero_si128();
            __m128i sumhi = _mm_setzero_si128();

            uint16_t *coordinates[] = {
                p0 + x - 1, p0 + x, p0 + x + 1,
                p1 + x - 1, p1 + x + 1,
                p2 + x - 1, p2 + x, p2 + x + 1
            };

            __m128i temp = _mm_setzero_si128();

            for (int i = 0; i < 8; i++) {
                __m128i target = _mm_loadu_si128((__m128i *)coordinates[i]);
                sumlo = _mm_add_epi32(sumlo, _mm_unpacklo_epi16(target, temp));
                sumhi = _mm_add_epi32(sumhi, _mm_unpackhi_epi16(target, temp));
            }

            sumlo = _mm_srai_epi32(sumlo, 3);
            sumhi = _mm_srai_epi32(sumhi, 3);

            sumlo = _mm_shufflelo_epi16(sumlo, _MM_SHUFFLE(3, 1, 2, 0));
            sumlo = _mm_shufflehi_epi16(sumlo, _MM_SHUFFLE(3, 1, 2, 0));
            sumhi = _mm_shufflelo_epi16(sumhi, _MM_SHUFFLE(2, 0, 3, 1));
            sumhi = _mm_shufflehi_epi16(sumhi, _MM_SHUFFLE(2, 0, 3, 1));
            sumlo = _mm_or_si128(sumlo, sumhi);
            sumlo = _mm_shuffle_epi32(sumlo, _MM_SHUFFLE(3, 1, 2, 0));

            __m128i src = _mm_load_si128((__m128i *)(p1 + x));
            temp = _mm_subs_epu16(src, sumlo);
            sumlo = _mm_subs_epu16(src, temp);

            __m128i thrs = _mm_subs_epu16(src, _mm_set1_epi16(threshold));
            temp = _mm_subs_epu16(sumlo, thrs);
            sumlo = _mm_adds_epu16(sumlo, temp);

            _mm_store_si128((__m128i *)(dstp + x), sumlo);
        }

        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


const proc_xxflate deflate[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
