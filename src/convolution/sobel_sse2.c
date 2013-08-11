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

    H = -v0 + v2 - 2*v3 + 2*v5 - v6 + v8
    V = -v0 - 2*v1 - v2 + v6 + 2*v7 + v8
    out = sqrt(pow(H, 2) + pow(V, 2))
        = max(H, V) * 15/16 + min(H, V) * 15/32
    largest error: 6.25% mean error: 1.88%
*/  

static const GF_ALIGN int16_t fifteens[8] = {15, 15, 15, 15, 15, 15, 15, 15};


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

    GF_ALIGN uint8_t ar_min[16];
    GF_ALIGN uint8_t ar_max[16];
    memset(ar_min, th_min, 16);
    memset(ar_max, th_max, 16);

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width, 1);
        uint8_t *array[][6] = {
            /*  -1      -1       -2       1       1       2   */
            { p0 - 1, p2 - 1, p1 - 1, p0 + 1, p2 + 1, p1 + 1 },
            { p0 - 1, p0 + 1, p0    , p2 - 1, p2 + 1, p2     }
        };

        for (int x = 0; x < width; x += 16) {
            __m128i zero = _mm_setzero_si128();
            __m128i sumlo[2], sumhi[2];
            
            __m128i xmm0 = _mm_loadu_si128((__m128i *)(p0 + x - 1));
            sumlo[0] = _mm_unpacklo_epi8(xmm0, zero);
            sumhi[0] = _mm_unpackhi_epi8(xmm0, zero);
            sumlo[1] = sumlo[0];
            sumhi[1] = sumhi[0];

            for (int i = 0; i < 2; i++) {
                __m128i xmm1, all1, one;

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][1] + x));
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sumlo[i] = _mm_add_epi16(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi16(sumhi[i], xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][2] + x));
                xmm1 = _mm_slli_epi16(_mm_unpackhi_epi8(xmm0, zero), 1);
                xmm0 = _mm_slli_epi16(_mm_unpacklo_epi8(xmm0, zero), 1);
                sumlo[i] = _mm_add_epi16(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi16(sumhi[i], xmm1);

                // -x - y - 2z = (x + y + 2z) * -1
                all1 = _mm_cmpeq_epi32(xmm0, xmm0);
                one = _mm_srli_epi16(xmm0, 15);
                sumlo[i] = _mm_add_epi16(one, _mm_xor_si128(sumlo[i], all1));
                sumhi[i] = _mm_add_epi16(one, _mm_xor_si128(sumhi[i], all1));

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][3] + x));
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sumlo[i] = _mm_add_epi16(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi16(sumhi[i], xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][4] + x));
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sumlo[i] = _mm_add_epi16(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi16(sumhi[i], xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][5] + x));
                xmm1 = _mm_slli_epi16(_mm_unpackhi_epi8(xmm0, zero), 1);
                xmm0 = _mm_slli_epi16(_mm_unpacklo_epi8(xmm0, zero), 1);
                sumlo[i] = _mm_add_epi16(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi16(sumhi[i], xmm1);

                xmm0 = _mm_add_epi16(one, _mm_xor_si128(sumlo[i], all1));
                sumlo[i] = _mm_or_si128(_mm_max_epi16(sumlo[i], zero),
                                        _mm_max_epi16(xmm0, zero));

                xmm0 = _mm_add_epi16(one, _mm_xor_si128(sumhi[i], all1));
                sumhi[i] = _mm_or_si128(_mm_max_epi16(sumhi[i], zero),
                                        _mm_max_epi16(xmm0, zero));
            }

            xmm0 = _mm_load_si128((__m128i *)fifteens);

            __m128i max = _mm_max_epi16(sumlo[0], sumlo[1]);
            __m128i min = _mm_min_epi16(sumlo[0], sumlo[1]);
            max = _mm_srli_epi16(_mm_mullo_epi16(max, xmm0), 4);
            min = _mm_srli_epi16(_mm_mullo_epi16(min, xmm0), 5);
            __m128i outlo = _mm_add_epi16(max, min);

            max = _mm_max_epi16(sumhi[0], sumhi[1]);
            min = _mm_min_epi16(sumhi[0], sumhi[1]);
            max = _mm_srli_epi16(_mm_mullo_epi16(max, xmm0), 4);
            min = _mm_srli_epi16(_mm_mullo_epi16(min, xmm0), 5);
            __m128i outhi = _mm_add_epi16(max, min);
            
            outlo = _mm_srli_epi16(outlo, eh->rshift);
            outhi = _mm_srli_epi16(outhi, eh->rshift);

            xmm0  = _mm_packus_epi16(outlo, outhi);

            max = _mm_load_si128((__m128i *)ar_max);
            __m128i temp = _mm_min_epu8(xmm0, max);
            temp = _mm_cmpeq_epi8(temp, max);
            xmm0 = _mm_or_si128(temp, xmm0);

            min = _mm_load_si128((__m128i *)ar_min);
            temp = _mm_max_epu8(xmm0, min);
            temp = _mm_cmpeq_epi8(temp, min);
            xmm0 = _mm_andnot_si128(temp, xmm0);

            _mm_store_si128((__m128i *)(dstp + x), xmm0);
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

    uint16_t th_min = eh->min > eh->plane_max ? eh->plane_max :
                                                 (uint16_t)eh->min;
    uint16_t th_max = eh->max > eh->plane_max ? eh->plane_max :
                                                 (uint16_t)eh->max;

    GF_ALIGN uint16_t ar_min[8];
    GF_ALIGN uint16_t ar_max[8];
    GF_ALIGN int16_t ar_pmax[8];
    for (int i = 0; i < 8; i++) {
        ar_min[i] = th_min;
        ar_max[i] = th_max;
        ar_pmax[i] = (int16_t)eh->plane_max;
    }

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);
        uint16_t *array[][6] = {
            /*  -1      -1       -2       1       1       2   */
            { p0 - 1, p2 - 1, p1 - 1, p0 + 1, p2 + 1, p1 + 1 },
            { p0 - 1, p0 + 1, p0    , p2 - 1, p2 + 1, p2     }
        };
        for (int x = 0; x < width; x += 8) {
            __m128i sum[2];
            sum[0] = _mm_loadu_si128((__m128i *)(p0 + x - 1));
            sum[1] = sum[0];

            for (int i = 0; i < 2; i++) {
                __m128i xmm0, xmm1, all1, one;

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][1] + x));
                sum[i] = _mm_add_epi16(sum[i], xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][2] + x));
                xmm0 = _mm_slli_epi16(xmm0, 1);
                sum[i] = _mm_add_epi16(sum[i], xmm0);

                all1 = _mm_cmpeq_epi32(xmm0, xmm0);
                one = _mm_srli_epi16(all1, 15);
                sum[i] = _mm_add_epi16(one, _mm_xor_si128(sum[i], all1));

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][3] + x));
                sum[i] = _mm_add_epi16(sum[i], xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][4] + x));
                sum[i] = _mm_add_epi16(sum[i], xmm0);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][5] + x));
                xmm0 = _mm_slli_epi16(xmm0, 1);
                sum[i]  = _mm_add_epi16(sum[i], xmm0);

                xmm0 = _mm_cmpgt_epi16(sum[i], _mm_setzero_si128());
                xmm1 = _mm_add_epi16(one, _mm_xor_si128(sum[i], all1));
                sum[i] = _mm_or_si128(_mm_and_si128(sum[i], xmm0),
                                      _mm_andnot_si128(xmm0, xmm1));
            }

            __m128i xmul = _mm_load_si128((__m128i *)fifteens);
            __m128i max = _mm_max_epi16(sum[0], sum[1]);
            __m128i min = _mm_min_epi16(sum[0], sum[1]);
            max = _mm_srli_epi16(_mm_mullo_epi16(max, xmul), 4);
            min = _mm_srli_epi16(_mm_mullo_epi16(min, xmul), 5);
            __m128i out = _mm_add_epi16(max, min);
            
            max = _mm_load_si128((__m128i *)ar_max);
            min = _mm_load_si128((__m128i *)ar_min);
            __m128i pmax = _mm_load_si128((__m128i *)ar_pmax);

            out = _mm_srli_epi16(out, eh->rshift);
            out = _mm_min_epi16(out, max);
            out = _mm_max_epi16(out, min);

            __m128i temp = _mm_cmpeq_epi16(out, max);
            temp = _mm_and_si128(temp, pmax);
            out  = _mm_or_si128(out, temp);

            temp = _mm_cmpeq_epi16(out, min);
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

#define ALPHA ((float)0.96043387)
#define BETA ((float)0.39782473)
static const GF_ALIGN float ar_alpha[4] = {ALPHA, ALPHA, ALPHA, ALPHA};
static const GF_ALIGN float ar_beta[4] = {BETA, BETA, BETA, BETA};
#undef ALPHA
#undef BETA

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

    GF_ALIGN uint16_t ar_min[8];
    GF_ALIGN uint16_t ar_max[8];
    for (int i = 0; i < 8; i++) {
        ar_min[i] = eh->min;
        ar_max[i] = eh->max;
    }

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);
        uint16_t *array[][6] = {
            /*  -1      -1       -2       1       1       2   */
            { p0 - 1, p2 - 1, p1 - 1, p0 + 1, p2 + 1, p1 + 1 },
            { p0 - 1, p0 + 1, p0    , p2 - 1, p2 + 1, p2     }
        };
        for (int x = 0; x < width; x += 8) {
            __m128i zero = _mm_setzero_si128();
            __m128i all1 = _mm_cmpeq_epi32(zero, zero);
            __m128i sumlo[2], sumhi[2];
            
            __m128i xmm0 = _mm_loadu_si128((__m128i *)(p0 + x - 1));
            sumlo[0] = _mm_unpacklo_epi16(xmm0, zero);
            sumhi[0] = _mm_unpackhi_epi16(xmm0, zero);
            sumlo[1] = sumlo[0];
            sumhi[1] = sumhi[0];
            
            for (int i = 0; i < 2; i++) {
                __m128i xmm1, one;

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][1] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sumlo[i] = _mm_add_epi32(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi32(sumhi[i], xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][2] + x));
                xmm1 = _mm_slli_epi32(_mm_unpackhi_epi16(xmm0, zero), 1);
                xmm0 = _mm_slli_epi32(_mm_unpacklo_epi16(xmm0, zero), 1);
                sumlo[i] = _mm_add_epi32(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi32(sumhi[i], xmm1);

                one = _mm_srli_epi32(all1, 31);
                sumlo[i] = _mm_add_epi32(one, _mm_xor_si128(sumlo[i], all1));
                sumhi[i] = _mm_add_epi32(one, _mm_xor_si128(sumhi[i], all1));

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][3] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sumlo[i] = _mm_add_epi32(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi32(sumhi[i], xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][4] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sumlo[i] = _mm_add_epi32(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi32(sumhi[i], xmm1);

                xmm0 = _mm_loadu_si128((__m128i *)(array[i][5] + x));
                xmm1 = _mm_slli_epi32(_mm_unpackhi_epi16(xmm0, zero), 1);
                xmm0 = _mm_slli_epi32(_mm_unpacklo_epi16(xmm0, zero), 1);
                sumlo[i] = _mm_add_epi32(sumlo[i], xmm0);
                sumhi[i] = _mm_add_epi32(sumhi[i], xmm1);

                xmm0 = _mm_cmpgt_epi32(sumlo[i], zero);
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(sumlo[i], all1));
                sumlo[i] = _mm_or_si128(_mm_and_si128(sumlo[i], xmm0),
                                         _mm_andnot_si128(xmm0, xmm1));

                xmm0 = _mm_cmpgt_epi32(sumhi[i], zero);
                xmm1 = _mm_add_epi32(one, _mm_xor_si128(sumhi[i], all1));
                sumhi[i] = _mm_or_si128(_mm_and_si128(sumhi[i], xmm0),
                                         _mm_andnot_si128(xmm0, xmm1));
            }

            __m128 alpha = _mm_load_ps(ar_alpha);
            __m128 beta = _mm_load_ps(ar_beta);
            
            __m128 max = _mm_cvtepi32_ps(mm_max_epi32(sumlo[0], sumlo[1]));
            __m128 min = _mm_cvtepi32_ps(mm_min_epi32(sumlo[0], sumlo[1]));
            max = _mm_mul_ps(max, alpha);
            min = _mm_mul_ps(min, beta);
            __m128i outlo = _mm_cvtps_epi32(_mm_add_ps(max, min));
            
            max = _mm_cvtepi32_ps(mm_max_epi32(sumhi[0], sumhi[1]));
            min = _mm_cvtepi32_ps(mm_min_epi32(sumhi[0], sumhi[1]));
            max = _mm_mul_ps(max, alpha);
            min = _mm_mul_ps(min, beta);
            __m128i outhi = _mm_cvtps_epi32(_mm_add_ps(max, min));
            
            outlo = _mm_srli_epi32(outlo, eh->rshift);
            outhi = _mm_srli_epi32(outhi, eh->rshift);

            xmm0 = _mm_srli_epi32(all1, 16);
            outlo = _mm_or_si128(_mm_cmpgt_epi32(outlo, xmm0), outlo);
            outlo = _mm_and_si128(outlo, xmm0);
            outhi = _mm_or_si128(_mm_cmpgt_epi32(outhi, xmm0), outhi);
            outhi = _mm_and_si128(outhi, xmm0);

            outlo = mm_cast_epi32(outlo, outhi);

            xmm0 = _mm_load_si128((__m128i *)ar_max);
            outhi = MM_MIN_EPU16(outlo, xmm0);
            outhi = _mm_cmpeq_epi16(outhi, xmm0);
            outlo = _mm_or_si128(outhi, outlo);

            xmm0 = _mm_load_si128((__m128i *)ar_min);
            outhi = MM_MAX_EPU16(outlo, xmm0);
            outhi = _mm_cmpeq_epi16(outhi, xmm0);
            outlo = _mm_andnot_si128(outhi, outlo);

            _mm_store_si128((__m128i *)(dstp + x), outlo);
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
