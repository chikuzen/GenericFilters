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
           abs(-v0 - 2*v1 - v2 + v6 + 2*v7 + v8))
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

    uint8_t th_min = eh->min > 0xFF ? 0xFF : (uint8_t)eh->min;
    uint8_t th_max = eh->max > 0xFF ? 0xFF : (uint8_t)eh->max;

    __m128i zero = _mm_setzero_si128();
    __m128i all1 = _mm_cmpeq_epi32(zero, zero);
    __m128i one = _mm_srli_epi16(all1, 15);
    __m128i xmin = _mm_set1_epi8((int8_t)th_min);
    __m128i xmax = _mm_set1_epi8((int8_t)th_max);

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 16) {
            __m128i abs_lo = zero;
            __m128i abs_hi = zero;

            uint8_t *array[2][6] = {
                /*    -1        -1          -2          1           1           2       */
                { p0 + x - 1, p2 + x - 1, p1 + x - 1, p1 + x + 1, p2 + x - 1, p2 + x + 1 },
                { p0 + x - 1, p0 + x + 1, p2 + x    , p2 + x - 1, p2 + x + 1, p2 + x     }
            };

            for (int i = 0; i < 2; i++) {
                __m128i sum_lo, sum_hi, xmm0, xmm1;

                xmm0 = _mm_loadu_si128((__m128i *)array[i][0]);
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                xmm0 = _mm_add_epi16(one, _mm_xor_si128(xmm0, all1));
                xmm1 = _mm_add_epi16(one, _mm_xor_si128(xmm1, all1));
                sum_lo = xmm0;
                sum_hi = xmm1;

                xmm0 = _mm_loadu_si128((__m128i *)array[i][1]);
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                xmm0 = _mm_add_epi16(one, _mm_xor_si128(xmm0, all1));
                xmm1 = _mm_add_epi16(one, _mm_xor_si128(xmm1, all1));
                sum_lo = _mm_add_epi16(sum_lo, xmm0);
                sum_hi = _mm_add_epi16(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][2]);
                xmm1 = _mm_slli_epi16(_mm_unpackhi_epi8(xmm0, zero), 1);
                xmm0 = _mm_slli_epi16(_mm_unpacklo_epi8(xmm0, zero), 1);
                xmm0 = _mm_add_epi16(one, _mm_xor_si128(xmm0, all1));
                xmm1 = _mm_add_epi16(one, _mm_xor_si128(xmm1, all1));
                sum_lo = _mm_add_epi16(sum_lo, xmm0);
                sum_hi = _mm_add_epi16(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][3]);
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sum_lo = _mm_add_epi16(sum_lo, xmm0);
                sum_hi = _mm_add_epi16(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][4]);
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sum_lo = _mm_add_epi16(sum_lo, xmm0);
                sum_hi = _mm_add_epi16(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][5]);
                xmm1 = _mm_slli_epi16(_mm_unpackhi_epi8(xmm0, zero), 1);
                xmm0 = _mm_slli_epi16(_mm_unpacklo_epi8(xmm0, zero), 1);
                sum_lo = _mm_add_epi16(sum_lo, xmm0);
                sum_hi = _mm_add_epi16(sum_hi, xmm1);

                xmm0 = _mm_add_epi16(one, _mm_xor_si128(sum_lo, all1));
                xmm1 = _mm_or_si128(_mm_max_epi16(sum_lo, zero),
                                    _mm_max_epi16(xmm0, zero));
                abs_lo = _mm_add_epi16(abs_lo, xmm1);

                xmm0 = _mm_add_epi16(one, _mm_xor_si128(sum_hi, all1));
                xmm1 = _mm_or_si128(_mm_max_epi16(sum_hi, zero),
                                    _mm_max_epi16(xmm0, zero));
                abs_hi = _mm_add_epi16(abs_hi, xmm1);
            }

            abs_lo = _mm_srli_epi16(abs_lo, eh->rshift);
            abs_hi = _mm_srli_epi16(abs_hi, eh->rshift);

            __m128i out  = _mm_packus_epi16(abs_lo, abs_hi);

            __m128i temp = _mm_min_epu8(out, xmax);
            temp = _mm_cmpeq_epi8(temp, xmax);
            out = _mm_or_si128(temp, out);

            temp = _mm_max_epu8(out, xmin);
            temp = _mm_cmpeq_epi8(temp, xmin);
            out = _mm_andnot_si128(temp, out);

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
proc_9_10_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *d, const uint8_t *s, edge_t *eh)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride /= 2;
    bstride /= 2;

    uint16_t *p0 = (uint16_t *)buff + 8;
    uint16_t *p1 = p0 + bstride;
    uint16_t *p2 = p1 + bstride;
    uint16_t *orig = p0, *end = p2;

    line_copy16(p0, srcp, width, 1);
    line_copy16(p1, srcp, width, 1);
    srcp += stride;

    uint16_t th_min = eh->min > eh->plane_max ? eh->plane_max : (uint16_t)eh->min;
    uint16_t th_max = eh->max > eh->plane_max ? eh->plane_max : (uint16_t)eh->max;

    __m128i xmin = _mm_set1_epi16((int16_t)th_min);
    __m128i xmax = _mm_set1_epi16((int16_t)th_max);
    __m128i pmax = _mm_set1_epi16((int16_t)eh->plane_max);
    __m128i all1 = _mm_cmpeq_epi32(xmin, xmin);
    __m128i one = _mm_srli_epi16(all1, 15);
    __m128i zero = _mm_setzero_si128();

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 8) {
            __m128i abs = zero;

            uint16_t *array[2][6] = {
                /*    -1        -1          -2          1           1           2       */
                { p0 + x - 1, p2 + x - 1, p1 + x - 1, p1 + x + 1, p2 + x - 1, p2 + x + 1 },
                { p0 + x - 1, p0 + x + 1, p2 + x    , p2 + x - 1, p2 + x + 1, p2 + x     }
            };

            for (int i = 0; i < 2; i++) {
                __m128i sum, xmm0;

                xmm0 = _mm_loadu_si128((__m128i *)array[i][0]);
                xmm0 = _mm_add_epi16(one, _mm_xor_si128(xmm0, all1));
                sum  = xmm0;

                xmm0 = _mm_loadu_si128((__m128i *)array[i][1]);
                xmm0 = _mm_add_epi16(one, _mm_xor_si128(xmm0, all1));
                sum  = _mm_add_epi16(sum, xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][2]);
                xmm0 = _mm_slli_epi16(xmm0, 1);
                xmm0 = _mm_add_epi16(one, _mm_xor_si128(xmm0, all1));
                sum  = _mm_add_epi16(sum, xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][3]);
                sum  = _mm_add_epi16(sum, xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][4]);
                sum  = _mm_add_epi16(sum, xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][5]);
                xmm0 = _mm_slli_epi16(xmm0, 1);
                sum  = _mm_add_epi16(sum, xmm0);

                xmm0 = _mm_add_epi16(one, _mm_xor_si128(sum, all1));
                sum  = _mm_max_epi16(sum, zero),
                xmm0 = _mm_max_epi16(xmm0, zero);
                sum  = _mm_or_si128(sum, xmm0);
                abs  = _mm_add_epi16(abs, sum);
            }

            abs = _mm_srli_epi16(abs, eh->rshift);
            abs = _mm_min_epi16(abs, xmax);
            abs = _mm_max_epi16(abs, xmin);

            __m128i temp = _mm_cmpeq_epi16(abs, xmax);
            temp = _mm_and_si128(temp, pmax);
            abs  = _mm_or_si128(abs, temp);

            temp = _mm_cmpeq_epi16(abs, xmin);
            abs = _mm_andnot_si128(temp, abs);

            _mm_store_si128((__m128i *)(dstp + x), abs);
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
    stride /= 2;
    bstride /= 2;

    uint16_t *p0 = (uint16_t *)buff + 8;
    uint16_t *p1 = p0 + bstride;
    uint16_t *p2 = p1 + bstride;
    uint16_t *orig = p0, *end = p2;

    line_copy16(p0, srcp, width, 1);
    line_copy16(p1, srcp, width, 1);
    srcp += stride;

    uint16_t th_min = eh->min;
    uint16_t th_max = eh->max;

    __m128i zero = _mm_setzero_si128();
    __m128i all1 = _mm_cmpeq_epi32(zero, zero);
    __m128i one = _mm_srli_epi32(all1, 31);
    __m128i xmin = _mm_set1_epi16((int16_t)th_min);
    __m128i xmax = _mm_set1_epi16((int16_t)th_max);

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 8) {
            __m128i abs_lo = zero;
            __m128i abs_hi = zero;

            uint16_t *array[2][6] = {
                /*    -1        -1          -2          1           1           2       */
                { p0 + x - 1, p2 + x - 1, p1 + x - 1, p1 + x + 1, p2 + x - 1, p2 + x + 1 },
                { p0 + x - 1, p0 + x + 1, p2 + x    , p2 + x - 1, p2 + x + 1, p2 + x     }
            };

            for (int i = 0; i < 2; i++) {
                __m128i sum_lo, sum_hi, xmm0, xmm1;

                xmm0 = _mm_loadu_si128((__m128i *)array[i][0]);
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                xmm0 = _mm_add_epi32(one, _mm_xor_si128(xmm0, all1));
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(xmm1, all1));
                sum_lo = xmm0;
                sum_hi = xmm1;

                xmm0 = _mm_loadu_si128((__m128i *)array[i][1]);
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                xmm0 = _mm_add_epi32(one, _mm_xor_si128(xmm0, all1));
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(xmm1, all1));
                sum_lo = _mm_add_epi32(sum_lo, xmm0);
                sum_hi = _mm_add_epi32(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][2]);
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm1 = _mm_slli_epi32(xmm1, 1);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                xmm0 = _mm_slli_epi32(xmm0, 1);
                xmm0 = _mm_add_epi32(one, _mm_xor_si128(xmm0, all1));
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(xmm1, all1));
                sum_lo = _mm_add_epi32(sum_lo, xmm0);
                sum_hi = _mm_add_epi32(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][3]);
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sum_lo = _mm_add_epi32(sum_lo, xmm0);
                sum_hi = _mm_add_epi32(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][4]);
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sum_lo = _mm_add_epi32(sum_lo, xmm0);
                sum_hi = _mm_add_epi32(sum_hi, xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)array[i][5]);
                xmm1 = _mm_slli_epi32(_mm_unpackhi_epi16(xmm0, zero), 1);
                xmm0 = _mm_slli_epi32(_mm_unpacklo_epi16(xmm0, zero), 1);
                sum_lo = _mm_add_epi32(sum_lo, xmm0);
                sum_hi = _mm_add_epi32(sum_hi, xmm1);

                xmm0 = _mm_cmpgt_epi32(sum_lo, zero);
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(sum_lo, all1));
                sum_lo = _mm_or_si128(_mm_and_si128(sum_lo, xmm0),
                                      _mm_andnot_si128(xmm0, xmm1));

                xmm0 = _mm_cmpgt_epi32(sum_hi, zero);
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(sum_hi, all1));
                sum_hi = _mm_or_si128(_mm_and_si128(sum_hi, xmm0),
                                      _mm_andnot_si128(xmm0, xmm1));

                abs_lo = _mm_add_epi32(abs_lo, sum_lo);
                abs_hi = _mm_add_epi32(abs_hi, sum_hi);
            }

            abs_lo = _mm_srli_epi32(abs_lo, eh->rshift);
            abs_hi = _mm_srli_epi32(abs_hi, eh->rshift);

            __m128i temp = _mm_srli_epi32(all1, 16);
            abs_lo = _mm_or_si128(_mm_cmpgt_epi32(abs_lo, temp), abs_lo);
            abs_lo = _mm_and_si128(abs_lo, temp);
            abs_hi = _mm_or_si128(_mm_cmpgt_epi32(abs_hi, temp), abs_hi);
            abs_hi = _mm_and_si128(abs_hi, temp);

            abs_lo = _mm_shufflelo_epi16(abs_lo, _MM_SHUFFLE(3, 1, 2, 0));
            abs_lo = _mm_shufflehi_epi16(abs_lo, _MM_SHUFFLE(3, 1, 2, 0));
            abs_hi = _mm_shufflelo_epi16(abs_hi, _MM_SHUFFLE(2, 0, 3, 1));
            abs_hi = _mm_shufflehi_epi16(abs_hi, _MM_SHUFFLE(2, 0, 3, 1));
            abs_lo = _mm_or_si128(abs_lo, abs_hi);
            abs_lo = _mm_shuffle_epi32(abs_lo, _MM_SHUFFLE(3, 1, 2, 0));

            temp = MM_MIN_EPU16(abs_lo, xmax);
            temp = _mm_cmpeq_epi16(temp, xmax);
            abs_lo = _mm_or_si128(temp, abs_lo);

            temp = MM_MAX_EPU16(abs_lo, xmin);
            temp = _mm_cmpeq_epi16(temp, xmin);
            abs_lo = _mm_andnot_si128(temp, abs_lo);

            _mm_store_si128((__m128i *)(dstp + x), abs_lo);
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
    proc_9_10_sse2,
    proc_16bit_sse2
};
