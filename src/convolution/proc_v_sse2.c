/*
  proc_v_sse2.c: Copyright (C) 2012-2013  Oka Motofumi

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


#include "convolution.h"
#include "sse2.h"


static void GF_FUNC_ALIGN VS_CC
proc_8bit_sse2(convolution_t *ch, uint8_t *buff, int bstride, int width,
               int height, int stride, uint8_t *dstp, const uint8_t *srcp)
{
    const uint8_t *p[17] = {NULL};
    int num = ch->length;
    int border = num / 2;

    for (int i = 0; i <= border; i++) {
        p[i] = srcp;
    }

    for (int i = border + 1; i < num; i++) {
        p[i] = p[i - 1] + stride;
    }

    __m128i zero = _mm_setzero_si128();
    __m128i all1 = _mm_cmpeq_epi32(zero, zero);
    __m128i one = _mm_srli_epi16(all1, 15);
    __m128 rdiv = _mm_set1_ps((float)ch->rdiv);
    __m128 bias = _mm_set1_ps((float)ch->bias);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 16) {
            __m128i sum[4] = { zero, zero, zero, zero };

            for (int i = 0; i < num; i++) {
                __m128i matrix = _mm_unpacklo_epi16(
                    _mm_set1_epi16((int16_t)ch->m[i]), zero);
                __m128i xmm0, xmm1, xmm2;

                xmm0 = _mm_load_si128((__m128i *)(p[i] + x));
                xmm2 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);

                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sum[0] = _mm_add_epi32(sum[0], _mm_madd_epi16(xmm0, matrix));
                sum[1] = _mm_add_epi32(sum[1], _mm_madd_epi16(xmm1, matrix));

                xmm1 = _mm_unpackhi_epi16(xmm2, zero);
                xmm0 = _mm_unpacklo_epi16(xmm2, zero);
                sum[2] = _mm_add_epi32(sum[2], _mm_madd_epi16(xmm0, matrix));
                sum[3] = _mm_add_epi32(sum[3], _mm_madd_epi16(xmm1, matrix));
            }

            for (int i = 0; i < 4; i++) {
                __m128 sumfp = _mm_cvtepi32_ps(sum[i]);
                sumfp = _mm_mul_ps(sumfp, rdiv);
                sumfp = _mm_add_ps(sumfp, bias);
                sum[i] = _mm_cvttps_epi32(sumfp);
            }

            sum[0] = _mm_packs_epi32(sum[0], sum[1]);
            sum[1] = _mm_packs_epi32(sum[2], sum[3]);

            if (!ch->saturate) {
                for (int i = 0; i < 2; i++) {
                    __m128i mask = _mm_cmplt_epi16(sum[i], zero);
                    __m128i temp = _mm_add_epi16(one, _mm_xor_si128(sum[i], all1));
                    temp = _mm_and_si128(temp, mask);
                    sum[i] = _mm_andnot_si128(mask, sum[i]);
                    sum[i] = _mm_or_si128(sum[i], temp);
                }
            }

            sum[0] = _mm_packus_epi16(sum[0], sum[1]);

            _mm_store_si128((__m128i *)(dstp + x), sum[0]);
        }
        dstp += stride;
        for (int i = 0; i < num - 1; i++) {
            p[i] = p[i + 1];
        }
        p[num - 1] = (y < height - border - 1) ? p[num - 1] + stride :
                                                 p[num - 1] - stride;
    }
}


static void GF_FUNC_ALIGN VS_CC
proc_9_10_sse2(convolution_t *ch, uint8_t *buff, int bstride, int width,
               int height, int stride, uint8_t *d, const uint8_t *s)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride /= 2;

    const uint16_t *p[17] = {NULL};
    int num = ch->length;
    int border = num / 2;
    for (int i = 0; i <= border; i++) {
        p[i] = srcp;
    }
    for (int i = border + 1; i < num; i++) {
        p[i] = p[i - 1] + stride;
    }

    __m128i zero = _mm_setzero_si128();
    __m128i all1 = _mm_cmpeq_epi32(zero, zero);
    __m128i one = _mm_srli_epi16(all1, 15);
    __m128 rdiv = _mm_set1_ps((float)ch->rdiv);
    __m128 bias = _mm_set1_ps((float)ch->bias);

    for (int y = 0; y < height; y++) {

        for (int x = 0; x < width; x += 8) {
            __m128i sum[2] = { zero, zero };

            for (int i = 0; i < num; i++) {
                __m128i matrix = _mm_unpacklo_epi16(
                    _mm_set1_epi16((int16_t)ch->m[i]), zero);
                __m128i xmm0, xmm1;

                xmm0 = _mm_load_si128((__m128i *)(p[i] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sum[0] = _mm_add_epi32(sum[0], _mm_madd_epi16(xmm0, matrix));
                sum[1] = _mm_add_epi32(sum[1], _mm_madd_epi16(xmm1, matrix));
            }

            for (int i = 0; i < 2; i++) {
                __m128 sumfp = _mm_cvtepi32_ps(sum[i]);
                sumfp = _mm_mul_ps(sumfp, rdiv);
                sumfp = _mm_add_ps(sumfp, bias);
                sum[i] = _mm_cvttps_epi32(sumfp);
            }

            sum[0] = _mm_packs_epi32(sum[0], sum[1]);

            if (!ch->saturate) {
                __m128i mask = _mm_cmplt_epi16(sum[0], zero);
                __m128i temp = _mm_add_epi16(one, _mm_xor_si128(sum[0], all1));
                temp = _mm_and_si128(temp, mask);
                sum[0] = _mm_andnot_si128(mask, sum[0]);
                sum[0] = _mm_or_si128(sum[0], temp);
            }

            _mm_store_si128((__m128i *)(dstp + x), sum[0]);
        }
        dstp += stride;
        for (int i = 0; i < num - 1; i++) {
            p[i] = p[i + 1];
        }
        p[num - 1] = (y < height - border - 1) ? p[num - 1] + stride :
                                                 p[num - 1] - stride;
    }
}


static void GF_FUNC_ALIGN VS_CC
proc_16bit_sse2(convolution_t *ch, uint8_t *buff, int bstride, int width,
                  int height, int stride, uint8_t *d, const uint8_t *s)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride /= 2;

    const uint16_t *p[17] = {NULL};
    int num = ch->length;
    int border = num / 2;
    for (int i = 0; i <= border; i++) {
        p[i] = srcp;
    }
    for (int i = border + 1; i < num; i++) {
        p[i] = p[i - 1] + stride;
    }

    __m128i zero = _mm_setzero_si128();
    __m128i all1 = _mm_cmpeq_epi32(zero, zero);
    __m128i one = _mm_srli_epi32(all1, 31);
    __m128 rdiv = _mm_set1_ps((float)ch->rdiv);
    __m128 bias = _mm_set1_ps((float)ch->bias);

    for (int y = 0; y < height; y++) {

        for (int x = 0; x < width; x += 8) {
            __m128i sum[2] = { zero, zero };

            for (int i = 0; i < num; i++) {
                int sign = ch->m[i] < 0 ? -1 : 1;
                __m128i matrix = _mm_set1_epi16((int16_t)(ch->m[i] * sign));
                __m128i xmm0, xmm1, xmm2;

                xmm0 = _mm_load_si128((__m128i *)(p[i] + x));

                xmm1 = _mm_mullo_epi16(xmm0, matrix);
                xmm0 = _mm_mulhi_epu16(xmm0, matrix);
                xmm2 = _mm_unpacklo_epi16(xmm1, xmm0);
                xmm0 = _mm_unpackhi_epi16(xmm1, xmm0);

                if (sign < 0) {
                    xmm2 = _mm_add_epi32(one, _mm_xor_si128(xmm2, all1));
                    xmm0 = _mm_add_epi32(one, _mm_xor_si128(xmm0, all1));
                }
                sum[0] = _mm_add_epi32(sum[0], xmm2);
                sum[1] = _mm_add_epi32(sum[1], xmm0);
            }

            for (int i = 0; i < 2; i++) {
                __m128 sumfp;
                __m128i mask, temp;
                sumfp  = _mm_cvtepi32_ps(sum[i]);
                sumfp  = _mm_mul_ps(sumfp, rdiv);
                sumfp  = _mm_add_ps(sumfp, bias);
                sum[i] = _mm_cvttps_epi32(sumfp);

                temp = _mm_srli_epi32(all1, 16);
                mask = _mm_cmplt_epi32(sum[i], temp);
                sum[i] = _mm_or_si128(_mm_and_si128(sum[i], mask),
                                      _mm_andnot_si128(mask, temp));
                mask = _mm_cmpgt_epi32(sum[i], zero);
                if (ch->saturate) {
                    sum[i] = _mm_and_si128(mask, sum[i]);
                } else {
                    temp = _mm_add_epi32(one, _mm_xor_si128(sum[i], all1));
                    sum[i] = _mm_or_si128(_mm_and_si128(mask, sum[i]),
                                          _mm_andnot_si128(mask, temp));
                }
            }

            sum[0] = _mm_shufflelo_epi16(sum[0], _MM_SHUFFLE(3, 1, 2, 0));
            sum[0] = _mm_shufflehi_epi16(sum[0], _MM_SHUFFLE(3, 1, 2, 0));
            sum[1] = _mm_shufflelo_epi16(sum[1], _MM_SHUFFLE(2, 0, 3, 1));
            sum[1] = _mm_shufflehi_epi16(sum[1], _MM_SHUFFLE(2, 0, 3, 1));
            sum[0] = _mm_or_si128(sum[0], sum[1]);
            sum[0] = _mm_shuffle_epi32(sum[0], _MM_SHUFFLE(3, 1, 2, 0));

            _mm_store_si128((__m128i *)(dstp + x), sum[0]);
        }

        dstp += stride;
        for (int i = 0; i < num - 1; i++) {
            p[i] = p[i + 1];
        }
        p[num - 1] = (y < height - border - 1) ? p[num - 1] + stride :
                                                 p[num - 1] - stride;
    }
}


const proc_convolution convo_v[] = {
    proc_8bit_sse2,
    proc_9_10_sse2,
    proc_16bit_sse2
};
