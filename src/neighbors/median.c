/*
  median.c: Copyright (C) 2012  Oka Motofumi

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


/* from 'Implementing Median Filters in XC4000E FPGAs' by John L. Smith */
#define HIGHLOW(in0, in1, low, high) {\
    low = in0; high = in1;\
    if (in0 > in1) { low = in1; high = in0;}\
}
static unsigned VS_CC get_median_9(unsigned m0, unsigned m1, unsigned m2,
                                   unsigned m3, unsigned m4, unsigned m5,
                                   unsigned m6, unsigned m7, unsigned m8)
{
    unsigned  x0, x1, x2, x3, x4, x5, x6, x7, x8, x9,
              x10, x11, x12, x13, x14, x15;
    HIGHLOW( m1,  m2,  x0,  x1);
    HIGHLOW( m4,  m5,  x2,  x3);
    HIGHLOW( m7,  m8,  x4,  x5);
    HIGHLOW( m0,  x0,  x6,  x7);
    HIGHLOW( m3,  x2,  x8,  x9);
    HIGHLOW( m6,  x4, x10, x11);
    HIGHLOW( x7,  x1, x12, x13);
    HIGHLOW( x9,  x3, x14, x15);
    HIGHLOW(x11,  x5,  x0,  x1)
    HIGHLOW(x14,  x0,  x2,  x3);
    x4 = x6 >  x8 ?  x6 :  x8;
    x5 = x4 > x10 ?  x4 : x10;
    x6 = x2 > x12 ?  x2 : x12;
    x7 = x3 >  x6 ?  x3 :  x6;
    x8 = x1 > x15 ? x15 :  x1;
    x9 = x8 > x13 ? x13 :  x8;
    HIGHLOW( x7,  x9,  x0,  x1);
    x2 = x0 > x5 ? x0 : x5;
    return x1 > x2 ? x2 : x1;
}
#undef HIGHLOW


static void VS_CC
proc_median_8bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                 VSFrameRef *dst)
{
    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    const uint8_t *r1 = vsapi->getReadPtr(src, plane);
    uint8_t *dstp = vsapi->getWritePtr(dst, plane);

    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - stride * !!y;
        const uint8_t *r2 = r1 + stride * !!(h -y);
        for (int x = 0; x <= w; x++) {
            int xl = x - !!x;
            int xr = x + !!(w - x);
            dstp[x] = get_median_9(*(r0 + xl), *(r0 + x), *(r0 + xr),
                                   *(r1 + xl), *(r1 + x), *(r1 + xr),
                                   *(r2 + xl), *(r2 + x), *(r2 + xr));
        }
        r1 += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_median_16bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                  VSFrameRef *dst)
{
    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    const uint16_t *r1 = (uint16_t *)vsapi->getReadPtr(src, plane);
    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - stride * !!y;
        const uint16_t *r2 = r1 + stride * !!(h -y);
        for (int x = 0; x <= w; x++) {
            int xl = x - !!x;
            int xr = x + !!(w - x);
            dstp[x] = get_median_9(*(r0 + xl), *(r0 + x), *(r0 + xr),
                                   *(r1 + xl), *(r1 + x), *(r1 + xr),
                                   *(r2 + xl), *(r2 + x), *(r2 + xr));
        }
        r1 += stride;
        dstp += stride;
    }
}


const proc_neighbors median[] = {
    proc_median_8bit,
    proc_median_16bit
};
