/*
  proc_3x3.c: Copyright (C) 2012  Oka Motofumi

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


#include "convolution.h"
#include "sse2.h"


static void TWK_FUNC_ALIGN VS_CC
proc_8bit_sse2(convolution_t *ch, uint8_t *buff, int bstride, int width, int height,
               int stride, uint8_t *dstp, const uint8_t *srcp)
{
    uint8_t *p0 = buff + 16;
    uint8_t *p1 = p0 + bstride;
    uint8_t *p2 = p1 + bstride;
    uint8_t *orig = p0, *end = p2;

    line_copy8(p0, srcp, width, 1);
    line_copy8(p1, srcp, width, 1);
    srcp += stride;

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 16) {
            __m128i sum[4] = { _mm_setzero_si128(), _mm_setzero_si128(),
                               _mm_setzero_si128(), _mm_setzero_si128() };

            uint8_t *array[] = { p0 + x - 1, p0 + x, p0 + x + 1,
                                 p1 + x - 1, p1 + x, p1 + x + 1,
                                 p2 + x - 1, p2 + x, p2 + x + 1 };

            for (int i = 0; i < 9; i++) {
                __m128i x0, x1, y0, y1, s0, temp;

                x0     = _mm_loadu_si128((__m128i *)array[i]);

                temp   = _mm_setzero_si128();
                y0     = _mm_unpackhi_epi8(x0, temp);
                x0     = _mm_unpacklo_epi8(x0, temp);

                temp   = _mm_set1_epi16(ch->m[i]);
                x1     = _mm_mulhi_epi16(x0, temp);
                x0     = _mm_mullo_epi16(x0, temp);

                s0     = _mm_unpacklo_epi16(x0, x1);
                sum[0] = _mm_add_epi32(sum[0], s0);
                s0     = _mm_unpackhi_epi16(x0, x1);
                sum[1] = _mm_add_epi32(sum[1], s0);

                y1     = _mm_mulhi_epi16(y0, temp);
                y0     = _mm_mullo_epi16(y0, temp);

                s0     = _mm_unpacklo_epi16(y0, y1);
                sum[2] = _mm_add_epi32(sum[2], s0);
                s0     = _mm_unpackhi_epi16(y0, y1);
                sum[3] = _mm_add_epi32(sum[3], s0);
            }

            __m128 rdiv = _mm_set1_ps((float)ch->rdiv);
            __m128 bias = _mm_set1_ps((float)ch->bias);

            for (int i = 0; i < 4; i++) {
                __m128 sumfp = _mm_cvtepi32_ps(sum[i]);
                sumfp = _mm_mul_ps(sumfp, rdiv);
                sumfp = _mm_add_ps(sumfp, bias);
                sum[i] = _mm_cvttps_epi32(sumfp);
            }

            sum[0] = _mm_packs_epi32(sum[0], sum[1]);
            sum[2] = _mm_packs_epi32(sum[2], sum[3]);
            sum[0] = _mm_packus_epi16(sum[0], sum[2]);

            _mm_store_si128((__m128i *)(dstp + x), sum[0]);
        }

        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


static void TWK_FUNC_ALIGN VS_CC
proc_16bit_sse2(convolution_t *ch, uint8_t *buff, int bstride, int width,
                int height, int stride, uint8_t *d, const uint8_t *s)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride >>= 1;
    bstride >>= 1;

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
            __m128i sum[2] = { _mm_setzero_si128(), _mm_setzero_si128() };

            uint16_t *array[] = { p0 + x - 1, p0 + x, p0 + x + 1,
                                  p1 + x - 1, p1 + x, p1 + x + 1,
                                  p2 + x - 1, p2 + x, p2 + x + 1 };

            for (int i = 0; i < 9; i++) {
                __m128i x0, x1, s0, temp;

                x0     = _mm_loadu_si128((__m128i *)array[i]);

                temp   = _mm_set1_epi16(ch->m[i]);
                x1     = _mm_mulhi_epi16(x0, temp);
                x0     = _mm_mullo_epi16(x0, temp);

                s0     = _mm_unpacklo_epi16(x0, x1);
                sum[0] = _mm_add_epi32(sum[0], s0);
                s0     = _mm_unpackhi_epi16(x0, x1);
                sum[1] = _mm_add_epi32(sum[1], s0);
            }

            __m128 rdiv = _mm_set1_ps((float)ch->rdiv);
            __m128 bias = _mm_set1_ps((float)ch->bias);

            for (int i = 0; i < 2; i++) {
                __m128 sumfp = _mm_cvtepi32_ps(sum[i]);
                sumfp = _mm_mul_ps(sumfp, rdiv);
                sumfp = _mm_add_ps(sumfp, bias);
                sum[i] = _mm_cvttps_epi32(sumfp);
            }

            sum[0] = _mm_packs_epi32(sum[0], sum[1]);

            _mm_store_si128((__m128i *)(dstp + x), sum[0]);
        }

        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


const proc_convolution convo_3x3[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
