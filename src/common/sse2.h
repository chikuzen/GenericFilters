/*
  sse2.h: Copyright (C) 2012-2013  Oka Motofumi

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


#ifndef SSE2_DEFINE_HEADER
#define SSE2_DEFINE_HEADER

#include <string.h>
#include <stdint.h>
#include <emmintrin.h>

#ifdef _MSC_VER
#define GF_ALIGN __declspec(align(16))
#define GF_FUNC_ALIGN
#else
#ifdef __MINGW32__
#define GF_FUNC_ALIGN __attribute__((force_align_arg_pointer))
#else
#define GF_FUNC_ALIGN
#endif // __MINGW32__
#define GF_ALIGN __attribute__((aligned(16)))
#endif // _MSC_VER

#define MM_MAX_EPU16(X, Y) (_mm_adds_epu16(Y, _mm_subs_epu16(X, Y)))
#define MM_MIN_EPU16(X, Y) (_mm_subs_epu16(X, _mm_subs_epu16(X, Y)))


static inline __m128i mm_cast_epi32(__m128i xmm0, __m128i xmm1)
{
    xmm0 = _mm_shufflelo_epi16(xmm0, _MM_SHUFFLE(3, 1, 2, 0));
    xmm0 = _mm_shufflehi_epi16(xmm0, _MM_SHUFFLE(3, 1, 2, 0));
    xmm1 = _mm_shufflelo_epi16(xmm1, _MM_SHUFFLE(2, 0, 3, 1));
    xmm1 = _mm_shufflehi_epi16(xmm1, _MM_SHUFFLE(2, 0, 3, 1));
    xmm0 = _mm_or_si128(xmm0, xmm1);
    return _mm_shuffle_epi32(xmm0, _MM_SHUFFLE(3, 1, 2, 0));
}


static inline __m128i mm_max_epi32(__m128i xmm0, __m128i xmm1)
{
    __m128i mask = _mm_cmpgt_epi32(xmm0, xmm1);
    return _mm_or_si128(_mm_and_si128(mask, xmm0),
                         _mm_andnot_si128(mask, xmm1));
}


static inline __m128i mm_min_epi32(__m128i xmm0, __m128i xmm1)
{
    __m128i mask = _mm_cmplt_epi32(xmm0, xmm1);
    return _mm_or_si128(_mm_and_si128(mask, xmm0),
                        _mm_andnot_si128(mask, xmm1));
}


static inline void VS_CC
line_copy8(uint8_t *line, const uint8_t *srcp, int width, int mergin)
{
    memcpy(line, srcp, width);
    for (int i = mergin; i > 0; i--) {
        line[0 - i] = line[0];
        line[width - 1 + i] = line[width - 1];
    }
}


static inline void VS_CC
line_copy16(uint16_t *line, const uint16_t *srcp, int width, int mergin)
{
    memcpy(line, srcp, width * 2);
    for (int i = mergin; i > 0; i--) {
        line[0 - i] = line[0];
        line[width + i - 1] = line[width - 1];
    }
}

#endif
