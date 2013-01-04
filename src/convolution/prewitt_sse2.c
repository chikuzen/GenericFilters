/*
  prewitt_sse2.c: Copyright (C) 2013  Oka Motofumi

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


static void GF_FUNC_ALIGN VS_CC
proc_8bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *dstp, const uint8_t *srcp, edge_t *eh)
{
    const int16_t matrix[8][9] = {
        {  1,  1,  1,  1, -2,  1, -1, -1, -1 },
        {  1,  1,  1,  1, -2, -1,  1, -1, -1 },
        {  1,  1, -1,  1, -2, -1,  1,  1, -1 },
        {  1, -1, -1,  1, -2, -1,  1,  1,  1 },
        { -1, -1, -1,  1, -2,  1,  1,  1,  1 },
        { -1, -1,  1, -1, -2,  1,  1,  1,  1 },
        { -1,  1,  1, -1, -2,  1, -1,  1,  1 },
        {  1,  1,  1, -1, -2,  1, -1, -1,  1 }
    };

    uint8_t *p0 = buff + 16;
    uint8_t *p1 = p0 + bstride;
    uint8_t *p2 = p1 + bstride;
    uint8_t *orig = p0, *end = p2;

    line_copy8(p0, srcp, width, 1);
    line_copy8(p1, srcp, width, 1);
    srcp += stride;

    uint8_t th_min = eh->min > 0xFF ? 0xFF : (uint8_t)eh->min;
    uint8_t th_max = eh->max > 0xFF ? 0xFF : (uint8_t)eh->max;

    for (int y = 0; y < height; y++) {
        line_copy8(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 16) {
            __m128i output;

            {
                __m128i out[4] = {
                    _mm_set1_epi32(0x80000000), _mm_set1_epi32(0x80000000),
                    _mm_set1_epi32(0x80000000), _mm_set1_epi32(0x80000000)
                };
                uint8_t *coordinates[] = {
                    p0 + x - 1, p0 + x, p0 + x + 1,
                    p1 + x - 1, p1 + x, p1 + x + 1,
                    p2 + x - 1, p2 + x, p2 + x + 1
                };

                for (int i = 0; i < 8; i++) {
                    __m128i sum[4] = {
                        _mm_setzero_si128(), _mm_setzero_si128(),
                        _mm_setzero_si128(), _mm_setzero_si128()
                    };
                    for (int j = 0; j < 9; j++) {
                        __m128i x0, x1, y0, y1, s0, temp;
                        x0     = _mm_loadu_si128((__m128i *)coordinates[j]);
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

                    for (int j = 0; j < 4; j++) {
                        __m128i high = _mm_cmpgt_epi32(sum[j], out[j]);
                        sum[j] = _mm_and_si128(sum[j], high);
                        out[j] = _mm_andnot_si128(high, out[j]);
                        out[j] = _mm_or_si128(out[j], sum[j]);
                    }
                }
                {
                    __m128 rdiv = _mm_set1_ps(0.10);
                    for (int j = 0; j < 4; j++) {
                        __m128 outfp = _mm_cvtepi32_ps(out[j]);
                        outfp = _mm_mul_ps(outfp, rdiv);
                        out[j] = _mm_cvttps_epi32(outfp);
                    }
                }
                out[0] = _mm_packs_epi32(out[0], out[1]);
                out[1] = _mm_packs_epi32(out[2], out[3]);
                out[2] = _mm_packus_epi16(out[0], out[1]);
                output = out[2];
            }
            __m128i all1 = _mm_set1_epi8(0xFF);
            __m128i th   = _mm_set1_epi8(th_min);
            __m128i temp = _mm_max_epu8(output, th);
            temp = _mm_cmpeq_epi8(temp, th);
            output  = _mm_andnot_si128(temp, output);
            th   = _mm_set1_epi8(th_max);
            temp = _mm_min_epu8(output, th);
            temp = _mm_cmpeq_epi8(temp, th);
            th   = _mm_and_si128(temp, all1);
            temp = _mm_xor_si128(temp, all1);
            output  = _mm_and_si128(output, temp);
            output  = _mm_or_si128(output, th);

            _mm_store_si128((__m128i *)(dstp + x), output);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}


static void GF_FUNC_ALIGN VS_CC
proc_16bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *d, const uint8_t *s, edge_t *eh)
{
    const int16_t matrix[8][9] = {
        {  1,  1,  1,  1, -2,  1, -1, -1, -1 },
        {  1,  1,  1,  1, -2, -1,  1, -1, -1 },
        {  1,  1, -1,  1, -2, -1,  1,  1, -1 },
        {  1, -1, -1,  1, -2, -1,  1,  1,  1 },
        { -1, -1, -1,  1, -2,  1,  1,  1,  1 },
        { -1, -1,  1, -1, -2,  1,  1,  1,  1 },
        { -1,  1,  1, -1, -2,  1, -1,  1,  1 },
        {  1,  1,  1, -1, -2,  1, -1, -1,  1 }
    };

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

    uint16_t th_min = eh->min;
    uint16_t th_max = eh->max;
    uint16_t plane_max = eh->plane_max;

    for (int y = 0; y < height; y++) {
        line_copy16(p2, srcp, width, 1);

        for (int x = 0; x < width; x += 8) {
            __m128i out[2] = {
                _mm_set1_epi32(0x80000000), _mm_set1_epi32(0x80000000)
            };

            uint16_t *coordinates[] = {
                p0 + x - 1, p0 + x, p0 + x + 1,
                p1 + x - 1, p1 + x, p1 + x + 1,
                p2 + x - 1, p2 + x, p2 + x + 1
            };

            for (int i = 0; i < 8; i++) {
                __m128i sum[2] = { _mm_setzero_si128(), _mm_setzero_si128() };

                for (int j = 0; j < 9; j++) {
                    __m128i x0, x1, s0, temp;
                    x0     = _mm_loadu_si128((__m128i *)coordinates[j]);
                    temp   = _mm_set1_epi16(matrix[i][j]);
                    x1     = _mm_mulhi_epi16(x0, temp);
                    x0     = _mm_mullo_epi16(x0, temp);
                    s0     = _mm_unpacklo_epi16(x0, x1);
                    sum[0] = _mm_add_epi32(sum[0], s0);
                    s0     = _mm_unpackhi_epi16(x0, x1);
                    sum[1] = _mm_add_epi32(sum[1], s0);
                }
                for (int j = 0; j < 2; j++) {
                    __m128i high = _mm_cmpgt_epi32(sum[j], out[j]);
                    sum[j] = _mm_and_si128(sum[j], high);
                    out[j] = _mm_andnot_si128(high, out[j]);
                    out[j] = _mm_or_si128(out[j], sum[j]);
                }
            }
            __m128 rdiv = _mm_set1_ps(0.10);

            for (int j = 0; j < 2; j++) {
                __m128 outfp = _mm_cvtepi32_ps(out[j]);
                outfp = _mm_mul_ps(outfp, rdiv);
                out[j] = _mm_cvttps_epi32(outfp);
            }
            out[0] = _mm_shufflelo_epi16(out[0], _MM_SHUFFLE(3, 1, 2, 0));
            out[0] = _mm_shufflehi_epi16(out[0], _MM_SHUFFLE(3, 1, 2, 0));
            out[1] = _mm_shufflelo_epi16(out[1], _MM_SHUFFLE(2, 0, 3, 1));
            out[1] = _mm_shufflehi_epi16(out[1], _MM_SHUFFLE(2, 0, 3, 1));
            out[0] = _mm_or_si128(out[0], out[1]);
            out[0] = _mm_shuffle_epi32(out[0], _MM_SHUFFLE(3, 1, 2, 0));

            __m128i temp;
            __m128i th   = _mm_set1_epi16(th_min);
            MM_MAX_EPU16(out[0], th, temp);
            temp = _mm_cmpeq_epi16(temp, th);
            out[0]  = _mm_andnot_si128(temp, out[0]);
            th = _mm_set1_epi16(th_max);
            MM_MIN_EPU16(out[0], th, temp);
            temp = _mm_cmpeq_epi16(temp, th);
            __m128i val = _mm_set1_epi16(plane_max);
            th = _mm_and_si128(temp, val);
            temp = _mm_xor_si128(temp, _mm_set1_epi8(0xFF));
            out[0] = _mm_and_si128(out[0], temp);
            out[0] = _mm_or_si128(out[0], th);

            _mm_store_si128((__m128i *)(dstp + x), out[0]);
        }

        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
}

const proc_edge_detection prewitt[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
