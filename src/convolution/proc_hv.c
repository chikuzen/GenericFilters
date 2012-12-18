/*
  proc_hv.c: Copyright (C) 2012  Oka Motofumi

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

#define TMP(c) \
    ((r0[( c )] * v0 + r1[( c )] * v1 + r2[( c )] * v2 + r3[( c )] * v3 + r4[( c )] * v4) / div_v)

#define SET_CONVOLUTION_HV5(i0, i1, i2, i3, i4, fd) \
    dstp[( i2 )] = \
    clamp_##fd((r2[( i0 )] * h0 + r2[( i1 )] * h1 + TMP(( i2 )) * h2 + r2[( i3 )] * h3 + r2[( i4 )] * h4) / div_h + bias, max);

static void VS_CC
proc_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int h0 =   ch->m[0], h1 =   ch->m[1], h2 =   ch->m[2], h3 =   ch->m[3], h4 =   ch->m[4],
        v0 = ch->m_v[0], v1 = ch->m_v[1], v2 = ch->m_v[2], v3 = ch->m_v[3], v4 = ch->m_v[4];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane);
    int div_h = ch->div;
    float div_v = (float)ch->div_v;
    float bias = (float)ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r2 = vsapi->getReadPtr(src, plane);
    const uint8_t *r1 = r2;
    const uint8_t *r0 = r1;
    const uint8_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint8_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        SET_CONVOLUTION_HV5(0, 0, 0, 1, 2, f);
        SET_CONVOLUTION_HV5(0, 0, 1, 2, 3, f);
        for (int x = 2; x < w_; x++) {
            SET_CONVOLUTION_HV5(x - 2, x - 1, x, x + 1, x + 2, f);
        }
        SET_CONVOLUTION_HV5(w_ - 2, w_ - 1, w_, w, w, f);
        SET_CONVOLUTION_HV5(w_ - 1, w_ , w, w, w, f);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


static void VS_CC
proc_16bit(convolution_t *ch, int plane, const VSFrameRef *src,
             VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int h0 =   ch->m[0], h1 =   ch->m[1], h2 =   ch->m[2], h3 =   ch->m[3], h4 =   ch->m[4],
        v0 = ch->m_v[0], v1 = ch->m_v[1], v2 = ch->m_v[2], v3 = ch->m_v[3], v4 = ch->m_v[4];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    int div_h = ch->div;
    double div_v = ch->div_v;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r2 = (uint16_t *)vsapi->getReadPtr(src, plane);
    const uint16_t *r1 = r2;
    const uint16_t *r0 = r1;
    const uint16_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        SET_CONVOLUTION_HV5(0, 0, 0, 1, 2, d);
        SET_CONVOLUTION_HV5(0, 0, 1, 2, 3, d);
        for (int x = 2; x < w_; x++) {
            SET_CONVOLUTION_HV5(x - 2, x - 1, x, x + 1, x + 2, d);
        }
        SET_CONVOLUTION_HV5(w_ - 2, w_ - 1, w_, w, w, d);
        SET_CONVOLUTION_HV5(w_ - 1, w_ , w, w, w, d);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}
#undef SET_CONVOLUTION_HV5
#undef TMP


const proc_convolution convo_hv5[] = {
    proc_8bit,
    proc_16bit
};
