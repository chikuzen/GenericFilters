/*
  proc_v.c: Copyright (C) 2012  Oka Motofumi

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
proc_3_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2];

    int w = vsapi->getFrameWidth(src, plane);
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    double div = ch->div;
    double bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r1 = vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - !!(h - y) * stride;
        const uint8_t *r2 = r1 + !!(h - y) * stride;
        for (int x = 0; x < w; x++) {
            int64_t value = r0[x] * m0 + r1[x] * m1 + r2[x] * m2;
            dstp[x] = clamp(value / div + bias, max);
        }
        dstp += stride;
        r1 += stride;
    }
}


static void VS_CC
proc_3_16bit(convolution_t *ch, int plane, const VSFrameRef *src,
             VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2];

    int w = vsapi->getFrameWidth(src, plane);
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div = ch->div;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r1 = (uint16_t *)vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - !!(h - y) * stride;
        const uint16_t *r2 = r1 + !!(h - y) * stride;
        for (int x = 0; x < w; x++) {
            int64_t value = *(r0 + x) * m0 + *(r1 + x) * m1 + *(r2 + x) * m2;
            dstp[x] = clamp(value / div + bias, max);
        }
        dstp += stride;
        r1 += stride;
    }
}


static void VS_CC
proc_5_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2], m3 = ch->m[3], m4 = ch->m[4];

    int w = vsapi->getFrameWidth(src, plane);
    int h = vsapi->getFrameHeight(src, plane) - 1;
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
        for (int x = 0; x < w; x++) {
            int64_t value = *(r0 + x) * m0 + *(r1 + x) * m1 + *(r2 + x) * m2 +
                            *(r3 + x) * m3 + *(r4 + x) * m4;
            dstp[x] = clamp(value / div + bias, max);
        }
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


static void VS_CC
proc_5_16bit(convolution_t *ch, int plane, const VSFrameRef *src,
             VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2], m3 = ch->m[3], m4 = ch->m[4];

    int w = vsapi->getFrameWidth(src, plane);
    int h = vsapi->getFrameHeight(src, plane) - 1;
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
        for (int x = 0; x < w; x++) {
            int64_t value = *(r0 + x) * m0 + *(r1 + x) * m1 + *(r2 + x) * m2 +
                            *(r3 + x) * m3 + *(r4 + x) * m4;
            dstp[x] = clamp(value / div + bias, max);
        }
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}

const proc_convolution convo_v3[] = {
    proc_3_8bit,
    proc_3_16bit
};
const proc_convolution convo_v5[] = {
    proc_5_8bit,
    proc_5_16bit
};
