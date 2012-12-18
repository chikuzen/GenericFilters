/*
  proc_3x3.c: Copyright (C) 2012  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of Neighbors

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


#define PROC_CONVOLUTION
#include "convolution.h"


static void VS_CC
proc_8bit(convolution_t *ch, int plane, const VSFrameRef *src, VSFrameRef *dst,
          const VSAPI *vsapi, uint16_t max)
{
    int m00 = ch->m[0], m01 = ch->m[1], m02 = ch->m[2],
        m10 = ch->m[3], m11 = ch->m[4], m12 = ch->m[5],
        m20 = ch->m[6], m21 = ch->m[7], m22 = ch->m[8];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int w_ = w - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    float div = ch->div;
    float bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r1 = vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - stride * !!y;
        const uint8_t *r2 = r1 + stride * !!(h -y);
        int32_t value = r0[0] * m00 + r0[0] * m01 + r0[1] * m02 +
                        r1[0] * m10 + r1[0] * m11 + r1[1] * m12 +
                        r2[0] * m20 + r2[0] * m21 + r2[1] * m22;
        dstp[0] = clamp_f(value  / div + bias, max);
        for (int x = 1; x < w; x++) {
            value = r0[x - 1] * m00 + r0[x] * m01 + r0[x + 1] * m02 +
                    r1[x - 1] * m10 + r1[x] * m11 + r1[x + 1] * m12 +
                    r2[x - 1] * m20 + r2[x] * m21 + r2[x + 1] * m22;
            dstp[x] = clamp_f(value / div + bias, max);
        }
        value = r0[w_] * m00 + r0[w] * m01 + r0[w] * m02 +
                r1[w_] * m10 + r1[w] * m11 + r1[w] * m12 +
                r2[w_] * m20 + r2[w] * m21 + r2[w] * m22;
        dstp[w] = clamp_f(value / div + bias, max);
        r1 += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_16bit(convolution_t *ch, int plane, const VSFrameRef *src, VSFrameRef *dst,
           const VSAPI *vsapi, uint16_t max)
{
    int m00 = ch->m[0], m01 = ch->m[1], m02 = ch->m[2],
        m10 = ch->m[3], m11 = ch->m[4], m12 = ch->m[5],
        m20 = ch->m[6], m21 = ch->m[7], m22 = ch->m[8];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    if (w < 1) {
        return;
    }
    int w_ = w - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div = ch->div;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r1 = (uint16_t *)vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - stride * !!y;
        const uint16_t *r2 = r1 + stride * !!(h -y);
        int64_t value = r0[0] * m00 + r0[0] * m01 + r0[1] * m02 +
                        r1[0] * m10 + r1[0] * m11 + r1[1] * m12 +
                        r2[0] * m20 + r2[0] * m21 + r2[1] * m22;
        dstp[0] = clamp_d(value / div + bias, max);
        for (int x = 1; x < w; x++) {
            value = r0[x - 1] * m00 + r0[x] * m01 + r0[x + 1] * m02 +
                    r1[x - 1] * m10 + r1[x] * m11 + r1[x + 1] * m12 +
                    r2[x - 1] * m20 + r2[x] * m21 + r2[x + 1] * m22;
            dstp[x] = clamp_d(value / div + bias, max);
        }
        value = r0[w_] * m00 + r0[w] * m01 + r0[w] * m02 +
                r1[w_] * m10 + r1[w] * m11 + r1[w] * m12 +
                r2[w_] * m20 + r2[w] * m21 + r2[w] * m22;
        dstp[w] = clamp_d(value / div + bias, max);
        r1 += stride;
        dstp += stride;
    }
}

const proc_convolution convo_3x3[] = {
    proc_8bit,
    proc_16bit
};
