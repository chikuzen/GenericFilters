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


static unsigned VS_CC get_min(unsigned x, unsigned y, unsigned z)
{
    if (y > x) {
        return z > x ? x : z;
    }
    return z > y ? y : z;
}


static void VS_CC
proc_tb_u8(const uint8_t *rt, const uint8_t *rb, int w, uint8_t *dstp)
{
    uint8_t mint, minb;
    mint = get_min(rt[0], rt[1], rb[0]);
    dstp[0] = mint < rb[1] ? mint : rb[1];
    for (int x = 1; x < w; x++) {
        mint = get_min(rt[x - 1], rt[x], rt[x + 1]);
        minb = get_min(rb[x - 1], rb[x], rb[x + 1]);
        dstp[x] = mint < minb ? mint : minb;
    }
    minb = get_min(rt[w - 1], rt[w], rb[w - 1]);
    dstp[w] = minb < rb[w] ? minb : rb[w];
}


static void VS_CC
proc_tb_u16(const uint16_t *rt, const uint16_t *rb, int w, uint16_t *dstp)
{
    uint16_t mint, minb;
    mint = get_min(rt[0], rt[1], rb[0]);
    dstp[0] = mint < rb[1] ? mint : rb[1];
    for (int x = 1; x < w; x++) {
        mint = get_min(rt[x - 1], rt[x], rt[x + 1]);
        minb = get_min(rb[x - 1], rb[x], rb[x + 1]);
        dstp[x] = mint < minb ? mint : minb;
    }
    minb = get_min(rt[w - 1], rt[w], rt[w - 1]);
    dstp[w] = minb < rb[w] ? minb : rb[w];
}


static void VS_CC
proc_8bit(int w, int h, int stride, uint8_t *dstp, const uint8_t *r1)
{
    const uint8_t *r0 = r1;
    const uint8_t *r2 = r1 + stride;

    proc_tb_u8(r1, r2, w, dstp);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        uint8_t min0 = get_min(r0[0], r1[0], r2[0]);
        uint8_t min1 = get_min(r0[1], r1[1], r2[1]);
        dstp[0] = min0 < min1 ? min0 : min1;

        for (int x = 1; x < w; x++) {
            min0 = get_min(r0[x - 1], r0[x], r0[x + 1]);
            min1 = get_min(r1[x - 1], r1[x], r1[x + 1]);
            uint8_t min2 = get_min(r2[x - 1], r2[x], r2[x + 1]);
            dstp[x] = get_min(min0, min1, min2);
        }

        min0 = get_min(r0[w - 1], r1[w - 1], r2[w - 1]);
        min1 = get_min(r0[w], r1[w], r2[w]);
        dstp[w] = min0 < min1 ? min0 : min1;

        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }

    proc_tb_u8(r0, r1, w, dstp);
}


static void VS_CC
proc_16bit(int w, int h, int stride, uint8_t *d, const uint8_t *srcp)
{
    stride >>= 1;
    const uint16_t *r1 = (uint16_t *)srcp;
    const uint16_t *r0 = r1;
    const uint16_t *r2 = r1 + stride;

    uint16_t *dstp = (uint16_t *)d;

    proc_tb_u16(r1, r2, w, dstp);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        uint16_t min0 = get_min(r0[0], r1[0], r2[0]);
        uint16_t min1 = get_min(r0[1], r1[1], r2[1]);
        dstp[0] = min0 < min1 ? min0 : min1;

        for (int x = 1; x < w; x++) {
            min0 = get_min(r0[x - 1], r0[x], r0[x + 1]);
            min1 = get_min(r1[x - 1], r1[x], r1[x + 1]);
            uint16_t min2 = get_min(r2[x - 1], r2[x], r2[x + 1]);
            dstp[x] = get_min(min0, min1, min2);
        }

        min0 = get_min(r0[w - 1], r1[w - 1], r2[w - 1]);
        min1 = get_min(r0[w], r1[w], r2[w]);
        dstp[w] = min0 < min1 ? min0 : min1;

        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }

    proc_tb_u16(r0, r1, w, dstp);
}


const proc_neighbors minimum[] = {
    proc_8bit,
    proc_16bit
};
