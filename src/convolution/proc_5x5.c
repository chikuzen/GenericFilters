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

#define SET_MATRIX() \
    int m00 = ch->m[ 0], m01 = ch->m[ 1], m02 = ch->m[ 2], m03 = ch->m[ 3], m04 = ch->m[ 4],\
        m10 = ch->m[ 5], m11 = ch->m[ 6], m12 = ch->m[ 7], m13 = ch->m[ 8], m14 = ch->m[ 9],\
        m20 = ch->m[10], m21 = ch->m[11], m22 = ch->m[12], m23 = ch->m[13], m24 = ch->m[14],\
        m30 = ch->m[15], m31 = ch->m[16], m32 = ch->m[17], m33 = ch->m[18], m34 = ch->m[19],\
        m40 = ch->m[20], m41 = ch->m[21], m42 = ch->m[22], m43 = ch->m[23], m44 = ch->m[24];

#define GET_CONVOLUTION(i0, i1, i2, i3, i4) \
    (r0[i0] * m00 + r0[i1] * m01 + r0[i2] * m02 + r0[i3] * m03 + r0[i4] * m04 +\
     r1[i0] * m10 + r1[i1] * m11 + r1[i2] * m12 + r1[i3] * m13 + r1[i4] * m14 +\
     r2[i0] * m20 + r2[i1] * m21 + r2[i2] * m22 + r2[i3] * m23 + r2[i4] * m24 +\
     r3[i0] * m30 + r3[i1] * m31 + r3[i2] * m32 + r3[i3] * m33 + r3[i4] * m34 +\
     r4[i0] * m40 + r4[i1] * m41 + r4[i2] * m42 + r4[i3] * m43 + r4[i4] * m44)

static void VS_CC
proc_8bit(convolution_t *ch, int w, int h, int stride, uint8_t *dstp,
          const uint8_t *r2, uint16_t max)
{
    SET_MATRIX();
    float rdiv = (float)ch->rdiv;
    float bias = (float)ch->bias;

    w--; h--;
    int w_ = w - 1;
    const uint8_t *r1 = r2;
    const uint8_t *r0 = r1;
    const uint8_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint8_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;

        int32_t value = GET_CONVOLUTION(0, 0, 0, 1, 2);
        dstp[0] = clamp_f( value * rdiv + bias, 0xFF);

        value = GET_CONVOLUTION(0, 0, 1, 2, 3);
        dstp[1] = clamp_f(value * rdiv + bias, 0XFF);

        for (int x = 0; x <= w; x++) {
            value = GET_CONVOLUTION(x - 2, x - 1, x, x + 1, x + 2);
            dstp[x] = clamp_f(value * rdiv + bias, 0xFF);
        }

        value = GET_CONVOLUTION(w_ - 2, w_ -1, w_, w, w);
        dstp[w_] = clamp_f(value * rdiv + bias, 0xFF);

        value = GET_CONVOLUTION(w_ - 1, w_, w, w, w);
        dstp[w] = clamp_f(value * rdiv + bias, 0XFF);

        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


static void VS_CC
proc_16bit(convolution_t *ch, int w, int h, int stride, uint8_t *d,
           const uint8_t *srcp, uint16_t max)
{
    SET_MATRIX();
    double rdiv = ch->rdiv;
    double bias = ch->bias;

    w--; h--; stride >>= 1;
    int w_ = w - 1;
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *r2 = (uint16_t *)srcp;
    const uint16_t *r1 = r2;
    const uint16_t *r0 = r1;
    const uint16_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;

        int64_t value = GET_CONVOLUTION(0, 0, 0, 1, 2);
        dstp[0] = clamp_d(value * rdiv + bias, 0xFF);

        value = GET_CONVOLUTION(0, 0, 1, 2, 3);
        dstp[1] = clamp_d(value * rdiv + bias, 0XFF);

        for (int x = 0; x <= w; x++) {
            value = GET_CONVOLUTION(x - 2, x - 1, x, x + 1, x + 2);
            dstp[x] = clamp_d(value * rdiv + bias, 0xFF);
        }

        value = GET_CONVOLUTION(w_ - 2, w_ -1, w_, w, w);
        dstp[w_] = clamp_d(value * rdiv + bias, 0xFF);

        value = GET_CONVOLUTION(w_ - 1, w_, w, w, w);
        dstp[w] = clamp_d(value * rdiv + bias, 0XFF);

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
