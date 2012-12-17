/*
  proc_5x5.c: Copyright (C) 2012  Oka Motofumi

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
    int m00 = ch->m[ 0], m01 = ch->m[ 1], m02 = ch->m[ 2], m03 = ch->m[ 3], m04 = ch->m[ 4],
        m10 = ch->m[ 5], m11 = ch->m[ 6], m12 = ch->m[ 7], m13 = ch->m[ 8], m14 = ch->m[ 9],
        m20 = ch->m[10], m21 = ch->m[11], m22 = ch->m[12], m23 = ch->m[13], m24 = ch->m[14],
        m30 = ch->m[15], m31 = ch->m[16], m32 = ch->m[17], m33 = ch->m[18], m34 = ch->m[19],
        m40 = ch->m[20], m41 = ch->m[21], m42 = ch->m[22], m43 = ch->m[23], m44 = ch->m[24];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane);
    double div = ch->div;
    double bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r2 = vsapi->getReadPtr(src, plane);
    const uint8_t *r1 = r2;
    const uint8_t *r0 = r1;
    const uint8_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint8_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        int64_t value = r0[0] * m00 + r0[0] * m01 + r0[0] * m02 + r0[1] * m03 + r0[2] * m04 +
                        r1[0] * m10 + r1[0] * m11 + r1[0] * m12 + r1[1] * m13 + r1[2] * m14 +
                        r2[0] * m20 + r2[0] * m21 + r2[0] * m22 + r2[1] * m23 + r2[2] * m24 +
                        r3[0] * m30 + r3[0] * m31 + r3[0] * m32 + r3[1] * m33 + r3[2] * m34 +
                        r4[0] * m40 + r4[0] * m41 + r4[0] * m42 + r4[1] * m43 + r4[2] * m44;
        dstp[0] = clamp(value / div + bias, 0xFF);
        value = r0[0] * m00 + r0[0] * m01 + r0[1] * m02 + r0[2] * m03 + r0[3] * m04 +
                r1[0] * m10 + r1[0] * m11 + r1[1] * m12 + r1[2] * m13 + r1[3] * m14 +
                r2[0] * m20 + r2[0] * m21 + r2[1] * m22 + r2[2] * m23 + r2[3] * m24 +
                r3[0] * m30 + r3[0] * m31 + r3[1] * m32 + r3[2] * m33 + r3[3] * m34 +
                r4[0] * m40 + r4[0] * m41 + r4[1] * m42 + r4[2] * m43 + r4[3] * m44;
        dstp[1] = clamp(value / div + bias, 0XFF);
        for (int x = 0; x <= w; x++) {
            value = r0[x - 2] * m00 + r0[x - 1] * m01 + r0[x] * m02 + r0[x + 1] * m03 + r0[x + 2] * m04 +
                    r1[x - 2] * m10 + r1[x - 1] * m11 + r1[x] * m12 + r1[x + 1] * m13 + r1[x + 2] * m14 +
                    r2[x - 2] * m20 + r2[x - 1] * m21 + r2[x] * m22 + r2[x + 1] * m23 + r2[x + 2] * m24 +
                    r3[x - 2] * m30 + r3[x - 1] * m31 + r3[x] * m32 + r3[x + 1] * m33 + r3[x + 2] * m34 +
                    r4[x - 2] * m40 + r4[x - 1] * m41 + r4[x] * m42 + r4[x + 1] * m43 + r4[x + 2] * m44;
            dstp[x] = clamp(value / div + bias, 0xFF);
        }
        value = r0[w_ - 2] * m00 + r0[w_ - 1] * m01 + r0[w_] * m02 + r0[w] * m03 + r0[w] * m04 +
                r1[w_ - 2] * m10 + r1[w_ - 1] * m11 + r1[w_] * m12 + r1[w] * m13 + r1[w] * m14 +
                r2[w_ - 2] * m20 + r2[w_ - 1] * m21 + r2[w_] * m22 + r2[w] * m23 + r2[w] * m24 +
                r3[w_ - 2] * m30 + r3[w_ - 1] * m31 + r3[w_] * m32 + r3[w] * m33 + r3[w] * m34 +
                r4[w_ - 2] * m40 + r4[w_ - 1] * m41 + r4[w_] * m42 + r4[w] * m43 + r4[w] * m44;
        dstp[0] = clamp(value / div + bias, 0xFF);
        value = r0[w_ - 1] * m00 + r0[w_] * m01 + r0[w] * m02 + r0[w] * m03 + r0[w] * m04 +
                r1[w_ - 1] * m10 + r1[w_] * m11 + r1[w] * m12 + r1[w] * m13 + r1[w] * m14 +
                r2[w_ - 1] * m20 + r2[w_] * m21 + r2[w] * m22 + r2[w] * m23 + r2[w] * m24 +
                r3[w_ - 1] * m30 + r3[w_] * m31 + r3[w] * m32 + r3[w] * m33 + r3[w] * m34 +
                r4[w_ - 1] * m40 + r4[w_] * m41 + r4[w] * m42 + r4[w] * m43 + r4[w] * m44;
        dstp[1] = clamp(value / div + bias, 0XFF);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


static void VS_CC
proc_16bit(convolution_t *ch, int plane, const VSFrameRef *src, VSFrameRef *dst,
           const VSAPI *vsapi, uint16_t max)
{
    int m00 = ch->m[ 0], m01 = ch->m[ 1], m02 = ch->m[ 2], m03 = ch->m[ 3], m04 = ch->m[ 4],
        m10 = ch->m[ 5], m11 = ch->m[ 6], m12 = ch->m[ 7], m13 = ch->m[ 8], m14 = ch->m[ 9],
        m20 = ch->m[10], m21 = ch->m[11], m22 = ch->m[12], m23 = ch->m[13], m24 = ch->m[14],
        m30 = ch->m[15], m31 = ch->m[16], m32 = ch->m[17], m33 = ch->m[18], m34 = ch->m[19],
        m40 = ch->m[20], m41 = ch->m[21], m42 = ch->m[22], m43 = ch->m[23], m44 = ch->m[24];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div = ch->div;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r2 = (uint16_t *)vsapi->getReadPtr(src, plane);
    const uint16_t *r1 = r2;
    const uint16_t *r0 = r1;
    const uint16_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        int64_t value = r0[0] * m00 + r0[0] * m01 + r0[0] * m02 + r0[1] * m03 + r0[2] * m04 +
                        r1[0] * m10 + r1[0] * m11 + r1[0] * m12 + r1[1] * m13 + r1[2] * m14 +
                        r2[0] * m20 + r2[0] * m21 + r2[0] * m22 + r2[1] * m23 + r2[2] * m24 +
                        r3[0] * m30 + r3[0] * m31 + r3[0] * m32 + r3[1] * m33 + r3[2] * m34 +
                        r4[0] * m40 + r4[0] * m41 + r4[0] * m42 + r4[1] * m43 + r4[2] * m44;
        dstp[0] = clamp(value / div + bias, max);
        value = r0[0] * m00 + r0[0] * m01 + r0[1] * m02 + r0[2] * m03 + r0[3] * m04 +
                r1[0] * m10 + r1[0] * m11 + r1[1] * m12 + r1[2] * m13 + r1[3] * m14 +
                r2[0] * m20 + r2[0] * m21 + r2[1] * m22 + r2[2] * m23 + r2[3] * m24 +
                r3[0] * m30 + r3[0] * m31 + r3[1] * m32 + r3[2] * m33 + r3[3] * m34 +
                r4[0] * m40 + r4[0] * m41 + r4[1] * m42 + r4[2] * m43 + r4[3] * m44;
        dstp[1] = clamp(value / div + bias, max);
        for (int x = 0; x <= w; x++) {
            value = r0[x - 2] * m00 + r0[x - 1] * m01 + r0[x] * m02 + r0[x + 1] * m03 + r0[x + 2] * m04 +
                    r1[x - 2] * m10 + r1[x - 1] * m11 + r1[x] * m12 + r1[x + 1] * m13 + r1[x + 2] * m14 +
                    r2[x - 2] * m20 + r2[x - 1] * m21 + r2[x] * m22 + r2[x + 1] * m23 + r2[x + 2] * m24 +
                    r3[x - 2] * m30 + r3[x - 1] * m31 + r3[x] * m32 + r3[x + 1] * m33 + r3[x + 2] * m34 +
                    r4[x - 2] * m40 + r4[x - 1] * m41 + r4[x] * m42 + r4[x + 1] * m43 + r4[x + 2] * m44;
            dstp[x] = clamp(value / div + bias, max);
        }
        value = r0[w_ - 2] * m00 + r0[w_ - 1] * m01 + r0[w_] * m02 + r0[w] * m03 + r0[w] * m04 +
                r1[w_ - 2] * m10 + r1[w_ - 1] * m11 + r1[w_] * m12 + r1[w] * m13 + r1[w] * m14 +
                r2[w_ - 2] * m20 + r2[w_ - 1] * m21 + r2[w_] * m22 + r2[w] * m23 + r2[w] * m24 +
                r3[w_ - 2] * m30 + r3[w_ - 1] * m31 + r3[w_] * m32 + r3[w] * m33 + r3[w] * m34 +
                r4[w_ - 2] * m40 + r4[w_ - 1] * m41 + r4[w_] * m42 + r4[w] * m43 + r4[w] * m44;
        dstp[0] = clamp(value / div + bias, max);
        value = r0[w_ - 1] * m00 + r0[w_] * m01 + r0[w] * m02 + r0[w] * m03 + r0[w] * m04 +
                r1[w_ - 1] * m10 + r1[w_] * m11 + r1[w] * m12 + r1[w] * m13 + r1[w] * m14 +
                r2[w_ - 1] * m20 + r2[w_] * m21 + r2[w] * m22 + r2[w] * m23 + r2[w] * m24 +
                r3[w_ - 1] * m30 + r3[w_] * m31 + r3[w] * m32 + r3[w] * m33 + r3[w] * m34 +
                r4[w_ - 1] * m40 + r4[w_] * m41 + r4[w] * m42 + r4[w] * m43 + r4[w] * m44;
        dstp[1] = clamp(value / div + bias, max);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


const proc_convolution convo_5x5[] = {
    proc_8bit,
    proc_16bit
};
