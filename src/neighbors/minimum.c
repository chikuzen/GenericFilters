/*
  minimum.c: Copyright (C) 2012  Oka Motofumi

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


#include <stdint.h>
#include "neighbors.h"


static uint8_t VS_CC get_min_u8(uint8_t x, uint8_t y, uint8_t z)
{
    if (x < y) {
        return x < z ? x : z;
    }
    return y < z ? y : z;
}


static uint16_t VS_CC get_min_u16(uint16_t x, uint16_t y, uint16_t z)
{
    if (x < y) {
        return x < z ? x : z;
    }
    return y < z ? y : z;
}


static void VS_CC
proc_min_tb_u8(const uint8_t *rt, const uint8_t *rb, int w, uint8_t *dstp)
{
    uint8_t mint, minb;
    mint = get_min_u8(rt[0], rt[1], rb[0]);
    dstp[0] = mint < rb[1] ? mint : rb[1];
    for (int x = 1; x < w; x++) {
        mint = get_min_u8(rt[x - 1], rt[x], rt[x + 1]);
        minb = get_min_u8(rb[x - 1], rb[x], rb[x + 1]);
        dstp[x] = mint < minb ? mint : minb;
    }
    minb = get_min_u8(rt[w - 1], rt[w], rb[w - 1]);
    dstp[w] = minb < rb[w] ? minb : rb[w];
}


static void VS_CC
proc_min_tb_u16(const uint16_t *rt, const uint16_t *rb, int w, uint16_t *dstp)
{
    uint16_t mint, minb;
    mint = get_min_u16(rt[0], rt[1], rb[0]);
    dstp[0] = mint < rb[1] ? mint : rb[1];
    for (int x = 1; x < w; x++) {
        mint = get_min_u16(rt[x - 1], rt[x], rt[x + 1]);
        minb = get_min_u16(rb[x - 1], rb[x], rb[x + 1]);
        dstp[x] = mint < minb ? mint : minb;
    }
    minb = get_min_u16(rt[w - 1], rt[w], rt[w - 1]);
    dstp[w] = minb < rb[w] ? minb : rb[w];
}


static void VS_CC
proc_minimum_8bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                  VSFrameRef *dst)
{
    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    const uint8_t *r0 = vsapi->getReadPtr(src, plane);
    const uint8_t *r1 = r0;
    const uint8_t *r2 = r1 + stride;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);

    proc_min_tb_u8(r1, r2, w, dstp);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        uint8_t min0 = get_min_u8(r0[0], r1[0], r2[0]);
        uint8_t min1 = get_min_u8(r0[1], r1[1], r2[1]);
        dstp[0] = min0 < min1 ? min0 : min1;
        for (int x = 1; x < w; x++) {
            min0 = get_min_u8(r0[x - 1], r0[x], r0[x + 1]);
            min1 = get_min_u8(r1[x - 1], r1[x], r1[x + 1]);
            uint8_t min2 = get_min_u8(r2[x - 1], r2[x], r2[x + 1]);
            dstp[x] = get_min_u8(min0, min1, min2);
        }
        min0 = get_min_u8(r0[w - 1], r1[w - 1], r2[w - 1]);
        min1 = get_min_u8(r0[w], r1[w], r2[w]);
        dstp[w] = min0 < min1 ? min0 : min1;

        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }

    proc_min_tb_u8(r0, r1, w, dstp);
}


static void VS_CC
proc_minimum_16bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                   VSFrameRef *dst)
{
    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    const uint16_t *r0 = (uint16_t *)vsapi->getReadPtr(src, plane);
    const uint16_t *r1 = r0;
    const uint16_t *r2 = r1 + stride;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);

    proc_min_tb_u16(r1, r2, w, dstp);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        uint16_t min0 = get_min_u16(r0[0], r1[0], r2[0]);
        uint16_t min1 = get_min_u16(r0[1], r1[1], r2[1]);
        dstp[0] = min0 < min1 ? min0 : min1;
        for (int x = 1; x < w; x++) {
            min0 = get_min_u16(r0[x - 1], r0[x], r0[x + 1]);
            min1 = get_min_u16(r1[x - 1], r1[x], r1[x + 1]);
            uint16_t min2 = get_min_u16(r2[x - 1], r2[x], r2[x + 1]);
            dstp[x] = get_min_u16(min0, min1, min2);
        }
        min0 = get_min_u16(r0[w - 1], r1[w - 1], r2[w - 1]);
        min1 = get_min_u16(r0[w], r1[w], r2[w]);
        dstp[w] = min0 < min1 ? min0 : min1;

        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }

    proc_min_tb_u16(r0, r1, w, dstp);
}


const proc_neighbors minimum[] = {
    proc_minimum_8bit,
    proc_minimum_16bit
};