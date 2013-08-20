/*
  edge_detect_canny_sse2.h: Copyright (C) 2013  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of Generic.

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


#include <float.h>
#include <math.h>
#include "sse2.h"
#include "canny.h"


static inline __m128i calc_direction(const __m128 gx, const __m128 gy)
{
    __m128 t0225 = _mm_set1_ps((float)(sqrt(2.0) - 1.0)); // tan(pi/8)
    __m128 t0675 = _mm_set1_ps((float)(sqrt(2.0) + 1.0)); // tan(3*pi/8)
    __m128 t1125 = mm_ivtsign_ps(t0675);
    __m128 t1575 = mm_ivtsign_ps(t0225);

    __m128 th0 = _mm_setzero_ps();
    __m128 th1 = _mm_set1_ps(90.0f);

    __m128 mask = _mm_cmplt_ps(gy, th0);
    __m128 gx2 = _mm_or_ps(_mm_and_ps(mask, mm_ivtsign_ps(gx)),
                           _mm_andnot_ps(mask, gx));

    __m128 tan = _mm_mul_ps(mm_rcp_hq_ps(gx2), mm_abs_ps(gy));
    mask = _mm_cmpord_ps(tan, tan);
    tan = _mm_or_ps(_mm_and_ps(mask, tan), _mm_andnot_ps(mask, th1));

    __m128i d0 = _mm_castps_si128(_mm_and_ps(_mm_cmpge_ps(tan, t1575),
                                             _mm_cmplt_ps(tan, t0225)));
    __m128i d1 = _mm_castps_si128(_mm_and_ps(_mm_cmpge_ps(tan, t0225),
                                             _mm_cmplt_ps(tan, t0675)));
    __m128i d2 = _mm_castps_si128(_mm_or_ps(_mm_cmpge_ps(tan, t0675),
                                            _mm_cmplt_ps(tan, t1125)));
    __m128i d3 = _mm_castps_si128(_mm_and_ps(_mm_cmpge_ps(tan, t1125),
                                             _mm_cmplt_ps(tan, t1575)));

    d0 = _mm_srli_epi32(d0, 31); // 1
    d1 = _mm_srli_epi32(d1, 30); // 3
    d2 = _mm_srli_epi32(d2, 29); // 7
    d3 = _mm_srli_epi32(d3, 28); // 15
    return _mm_or_si128(_mm_or_si128(d0, d1), _mm_or_si128(d2, d3));
}


static void GF_FUNC_ALIGN VS_CC
proc_edge_detect(float *buff, int bstride, const float *srcp, float *dstp,
                 uint8_t *direction,int width, int height, int stride)
{
    float *p0 = buff;
    float *p1 = p0 + bstride;
    float *p2 = p1 + bstride;
    float *orig = p0, *end = p2;
    
    line_copyf(p0, srcp, width, 1);
    srcp += stride;
    line_copyf(p1, srcp, width, 1);
    memset(dstp, 0, stride * sizeof(float));

    for (int y = 1; y < height - 1; y++) {
        srcp += stride;
        line_copyf(p2, srcp, width, 1);
        for (int x = 0; x < width; x += 16) {
            __m128i d[4];
            for (int i = 0; i < 4; i++) {
                int pos = x + i * 4;
                __m128 gx = _mm_sub_ps(_mm_loadu_ps(p1 + pos + 1),
                                       _mm_loadu_ps(p1 + pos - 1));
                __m128 gy = _mm_sub_ps(_mm_load_ps(p0 + pos),
                                       _mm_load_ps(p2 + pos));
                d[i] = calc_direction(gx, gy);
                
                gx = _mm_add_ps(_mm_mul_ps(gx, gx), _mm_mul_ps(gy, gy));
                gx = _mm_sqrt_ps(gx);
                _mm_store_ps(dstp + pos, gx);
            }
            d[0] = _mm_packs_epi32(d[0], d[1]);
            d[1] = _mm_packs_epi32(d[2], d[3]);
            d[0] = _mm_packs_epi16(d[0], d[1]);
            _mm_store_si128((__m128i *)(direction + x), d[0]);
        }
        dstp += stride;
        direction += stride;
        p0 = p1;
        p1 = p2;
        p2 = (p2 == end) ? orig : p2 + bstride;
    }
    memset(dstp, 0, stride * sizeof(float));
}


static void VS_CC
proc_non_max_suppress(const float *srcp, float *dstp, const uint8_t *direction,
                      int width, int height, int stride)
{
    memset(dstp, 0, stride * sizeof(float));
    srcp += stride;
    dstp += stride;
    memcpy(dstp, srcp, stride * sizeof(float) * (height - 2));

    int pos[8] = {1, 1 - stride, 0, stride, 0, 0, 0, 1 + stride};

    for (int y = 1; y < height - 1; y++) {
        direction += stride;
        dstp[0] = 0.0f;
        
        for (int x = 1; x < width - 1; x++) {
            int p = pos[(direction[x] + 1) / 2 - 1];
            if (srcp[x] < srcp[x + p] || srcp[x] < srcp[x - p]) {
                dstp[x] = 0.0f;
            }
        }
        dstp[width - 1] = 0.0f;
        srcp += stride;
        dstp += stride;
    }
    memset(dstp, 0, stride * sizeof(float));
}


static void reset(stack_t *s, int size)
{
    s->index = -1;
    memset(s->map, 0, size);
}


static void push(stack_t *s, int x, int y)
{
    s->pos[++s->index] = (int32_t)((x << 16) | y);
}


static int32_t pop(stack_t *s)
{
    return s->pos[s->index--];
}


static void VS_CC
proc_hysteresis(float *edge, int width, int height, int stride, float tmax,
                float tmin, stack_t *stack)
{
    reset(stack, width * height);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (edge[x + y * stride] < tmax || stack->map[x + y * width]) {
                continue;
            }
            edge[x + y * stride] = FLT_MAX;
            stack->map[x + y * width] = 0xFF;
            push(stack, x, y);
            
            while (stack->index > -1) {
                int32_t posx = pop(stack);
                int32_t posy = posx & 0xFFFF;
                posx >>= 16;
                int32_t xmin = posx > 1 ? posx - 1 : 1;
                int32_t xmax = posx < width - 2 ? posx + 1 : posx;
                int32_t ymin = posy > 1 ? posy - 1 : 1;
                int32_t ymax = posy < height - 2 ? posy + 1 : posy;
                for (int yy = ymin; yy <= ymax; yy++) {
                    for (int xx = xmin; xx <= xmax; xx++) {
                        if (edge[xx + yy * stride] > tmin
                            && !stack->map[xx + yy * width]) {
                            edge[xx + yy * stride] = FLT_MAX;
                            stack->map[xx + yy * width] = 0xFF;
                            push(stack, xx, yy);
                        }
                    }
                }
            }
        }
    }
}


const proc_edetect edge_detect = proc_edge_detect;
const proc_nms  non_max_suppress = proc_non_max_suppress;
const proc_hyst hysteresis = proc_hysteresis;
