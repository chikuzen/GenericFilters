/*
  sobel_sse2.c: Copyright (C) 2013  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of GenericFilters.

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


#include "edge.h"
#include "sse2.h"

/*
     pixels         horizontal      vertical
    v0 v1 v2        -1  0  1        -1 -2 -1
    v3 v4 v5        -2  0  2         0  0  0
    v6 v7 v8        -1  0  1         1  2  1

    out = (abs(-v0 + v2 - 2*v3 + 2*v5 - v6 + v8) +
           abs(-v0 - 2*v1 - v2 + v6 + 2*v7 + v8)) / 8
*/


static void GF_FUNC_ALIGN VS_CC
proc_8bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *dstp, const uint8_t *srcp, edge_t *eh)
{
    uint8_t *p0 = buff + 16;
    uint8_t *p1 = p0 + bstride;
    uint8_t *p2 = p1 + bstride;
    uint8_t *orig = p0, *end = p2;

    line_copy8(p0, srcp, width, 1);
    line_copy8(p1, srcp, width, 1);
    srcp += stride;

    const int16_t matrix[2][6] = {
        { -1, 1, -2, 2, -1, 1 }, { -1, -2, -1, 1, 2, 1 }
    };
    uint8_t th_min = eh->min > 0xFF ? 0xFF : (uint8_t)eh->min;
    uint8_t th_max = eh->max > 0xFF ? 0xFF : (uint8_t)eh->max;

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 16) {
            __m128i sum_lo = _mm_setzero_si128();
            __m128i sum_hi = _mm_setzero_si128();

            uint8_t *array[2][6] = {
                { p0 + x - 1, p0 + x + 1, p1 + x - 1, p1 + x + 1, p2 + x - 1, p2 + x + 1 },
                { p0 + x - 1, p0 + x,     p0 + x + 1, p2 + x - 1, p2 + x,     p2 + x + 1 }
            };

            for (int i = 0; i < 2; i++) {
                __m128i sum[4] = {
                    _mm_setzero_si128(), _mm_setzero_si128(),
                    _mm_setzero_si128(), _mm_setzero_si128()
                };

                for (int j = 0; j < 6; j++) {
                    __m128i x0, x1, y0, y1, s0, temp;
                    x0     = _mm_loadu_si128((__m128i *)array[i][j]);
                    temp   = _mm_setzero_si128();
                    y0     = _mm_unpackhi_epi8(x0, temp);
                    x0     = _mm_unpacklo_epi8(x0, temp);
                    temp   = _mm_set1_epi16(matrix[i][j]);
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
                
                sum[0] = _mm_packs_epi32(sum[0], sum[1]);
                sum[1] = _mm_packs_epi32(sum[2], sum[3]);
                for (int j = 0; j < 2; j++) {
                    sum[j] = _mm_srai_epi16(sum[j], 3);
                    __m128i mask = _mm_cmplt_epi16(sum[j], _mm_setzero_si128());
                    __m128i temp = _mm_xor_si128(sum[j], _mm_set1_epi8(0xFF)); // ~sum0
                    temp = _mm_add_epi16(temp, _mm_set1_epi16(0x01));  // -x == ~x + 1
                    temp = _mm_and_si128(temp, mask); // negative -> positive
                    sum[j] = _mm_andnot_si128(mask, sum[j]);
                    sum[j] = _mm_or_si128(sum[0], temp); // abs(sum0)
                }
                sum_lo = _mm_adds_epu16(sum_lo, sum[0]);
                sum_hi = _mm_adds_epu16(sum_hi, sum[1]);
            }
            __m128i out  = _mm_packus_epi16(sum_lo, sum_hi);

            __m128i all1 = _mm_set1_epi8(0xFF);

            __m128i th   = _mm_set1_epi8(th_min);
            __m128i temp = _mm_max_epu8(out, th);
            temp = _mm_cmpeq_epi8(temp, th);
            out = _mm_andnot_si128(temp, out);

            th = _mm_set1_epi8(th_max);
            temp = _mm_min_epu8(out, th);
            temp = _mm_cmpeq_epi8(temp, th);

            th = _mm_and_si128(temp, all1);
            temp = _mm_xor_si128(temp, all1);
            out = _mm_and_si128(out, temp);
            out = _mm_or_si128(out, th);

            _mm_store_si128((__m128i *)(dstp + x), out);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig: p2 + bstride;
    }
}


static void GF_FUNC_ALIGN VS_CC
proc_16bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *d, const uint8_t *s, edge_t *eh)
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

    const int16_t matrix[2][6] = {
        { -1, 1, -2, 2, -1, 1 }, { -1, -2, -1, 1, 2, 1 }
    };
    uint16_t th_min = eh->min;
    uint16_t th_max = eh->max;
    uint16_t plane_max = eh->plane_max;

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 8) {
            __m128i sum = _mm_setzero_si128();

            uint16_t *array[2][6] = {
                { p0 + x - 1, p0 + x + 1, p1 + x - 1, p1 + x + 1, p2 + x - 1, p2 + x + 1 },
                { p0 + x - 1, p0 + x,     p0 + x + 1, p2 + x - 1, p2 + x,     p2 + x + 1 }
            };

            for (int i = 0; i < 2; i++) {
                __m128i sum0 = _mm_setzero_si128();
                __m128i sum1 = _mm_setzero_si128();

                for (int j = 0; j < 6; j++) {
                    __m128i x0, x1, s0, temp;
                    x0   = _mm_loadu_si128((__m128i *)array[i][j]);
                    temp = _mm_set1_epi16(matrix[i][j]);
                    x1   = _mm_mulhi_epi16(x0, temp);
                    x0   = _mm_mullo_epi16(x0, temp);
                    s0   = _mm_unpacklo_epi16(x0, x1);
                    sum0 = _mm_add_epi32(sum0, s0);
                    s0   = _mm_unpackhi_epi16(x0, x1);
                    sum1 = _mm_add_epi32(sum1, s0);
                }
                sum0 = _mm_srai_epi32(sum0, 3);
                sum1 = _mm_srai_epi32(sum1, 3);

                sum0 = _mm_shufflelo_epi16(sum0, _MM_SHUFFLE(3, 1, 2, 0));
                sum0 = _mm_shufflehi_epi16(sum0, _MM_SHUFFLE(3, 1, 2, 0));
                sum1 = _mm_shufflelo_epi16(sum1, _MM_SHUFFLE(2, 0, 3, 1));
                sum1 = _mm_shufflehi_epi16(sum1, _MM_SHUFFLE(2, 0, 3, 1));
                sum0 = _mm_or_si128(sum0, sum1);
                sum0 = _mm_shuffle_epi32(sum0, _MM_SHUFFLE(3, 1, 2, 0));

                __m128i mask = _mm_cmplt_epi16(sum0, _mm_setzero_si128());
                sum1 = _mm_xor_si128(sum0, _mm_set1_epi8(0xFF)); // ~sum0
                sum1 = _mm_add_epi16(sum1, _mm_set1_epi16(0x01));  // -x == ~x + 1
                sum1 = _mm_and_si128(sum1, mask); // negative -> positive
                sum0 = _mm_andnot_si128(mask, sum0);
                sum0 = _mm_or_si128(sum0, sum1); // abs(sum0)

                sum  = _mm_adds_epu16(sum, sum0);
            }
            __m128i temp;

            __m128i th   = _mm_set1_epi16(th_min);
            MM_MAX_EPU16(sum, th, temp); //temp = max(sum, th);

            temp = _mm_cmpeq_epi16(temp, th);
            sum  = _mm_andnot_si128(temp, sum);

            th = _mm_set1_epi16(th_max);
            MM_MIN_EPU16(sum, th, temp); // temp = min(sum, th)

            temp = _mm_cmpeq_epi16(temp, th);
            __m128i val = _mm_set1_epi16(plane_max);
            th = _mm_and_si128(temp, val);
            temp = _mm_xor_si128(temp, _mm_set1_epi8(0xFF));
            sum = _mm_and_si128(sum, temp);
            sum = _mm_or_si128(sum, th);

            _mm_store_si128((__m128i *)(dstp + x), sum);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig: p2 + bstride;
    }
}


const proc_edge_detection sobel[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
