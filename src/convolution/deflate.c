/*
  deflate.c: Copyright (C) 2012  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of Tweak.

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


#include "xxflate.h"


#define cond_add(cond, num) {\
    if (th > cond) { \
        value += cond * num; \
        div += num;\
    } \
}

static void VS_CC
proc_8bit(int w, int h, int stride, uint8_t *dstp, const uint8_t *r1, int th)
{
    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - stride * !!y;
        const uint8_t *r2 = r1 + stride * !!(h - y);
        int value = r1[0], div = 1;
        cond_add(r0[0], 2);
        cond_add(r0[1], 1);
        cond_add(r1[0], 1);
        cond_add(r1[1], 1);
        cond_add(r2[0], 2);
        cond_add(r2[1], 1);
        dstp[0] = (double)value / div;
        for (int x = 1; x < w; x++) {
            value = r1[x]; div = 1;
            cond_add(r0[x - 1], 1);
            cond_add(r0[x], 1);
            cond_add(r0[x + 1], 1);
            cond_add(r1[x - 1], 1);
            cond_add(r1[x + 1], 1);
            cond_add(r2[x - 1], 1);
            cond_add(r2[x], 1);
            cond_add(r2[x + 1], 1);
            dstp[x] = (double)value / div;
        }
        value = r1[w]; div = 1;
        cond_add(r0[w - 1], 1);
        cond_add(r0[w], 2);
        cond_add(r1[w - 1], 1);
        cond_add(r1[w], 1);
        cond_add(r2[w - 1], 1);
        cond_add(r2[w], 2);
        dstp[w] = (double)value / div;
        r1 += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_16bit(int w, int h, int stride, uint8_t *d, const uint8_t *srcp, int th)
{
    stride >>= 1;
    uint8_t *dstp = (uint8_t *)d;
    const uint16_t *r1 = (uint16_t *)srcp;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - stride * !!y;
        const uint16_t *r2 = r1 + stride * !!(h - y);
        int value = r1[0], div = 1;
        cond_add(r0[0], 2);
        cond_add(r0[1], 1);
        cond_add(r1[0], 1);
        cond_add(r1[1], 1);
        cond_add(r2[0], 2);
        cond_add(r2[1], 1);
        dstp[0] = (double)value / div;
        for (int x = 1; x < w; x++) {
            value = r1[x]; div = 1;
            cond_add(r0[x - 1], 1);
            cond_add(r0[x], 1);
            cond_add(r0[x + 1], 1);
            cond_add(r1[x - 1], 1);
            cond_add(r1[x + 1], 1);
            cond_add(r2[x - 1], 1);
            cond_add(r2[x], 1);
            cond_add(r2[x + 1], 1);
            dstp[x] = (double)value / div;
        }
        value = r1[w]; div = 1;
        cond_add(r0[w - 1], 1);
        cond_add(r0[w], 2);
        cond_add(r1[w - 1], 1);
        cond_add(r1[w], 1);
        cond_add(r2[w - 1], 1);
        cond_add(r2[w], 2);
        dstp[w] = (double)value / div;
        r1 += stride;
        dstp += stride;
    }
}


const proc_xxflate deflate[] = {
    proc_8bit,
    proc_16bit
};
