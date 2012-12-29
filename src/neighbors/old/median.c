/*
  median.c: Copyright (C) 2012  Oka Motofumi

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


#include <stdint.h>
#include "neighbors.h"


/* from 'Implementing Median Filters in XC4000E FPGAs' by John L. Smith */
#define LOWHIGH(low, high) {\
    if (low > high) {\
        unsigned t = low;\
        low = high;\
        high = t;\
    }\
}\

static unsigned VS_CC get_median_9(unsigned m0, unsigned m1, unsigned m2,
                                   unsigned m3, unsigned m4, unsigned m5,
                                   unsigned m6, unsigned m7, unsigned m8)
{
    unsigned  x0, x1, x2, x3, x4, x5, x6, x7;

    x0 = m1;
    x1 = m2;
    LOWHIGH(x0, x1);
    x2 = m0;
    LOWHIGH(x0, x2);
    LOWHIGH(x1, x2);
    x3 = m4;
    x4 = m5;
    LOWHIGH(x3, x4);
    x5 = m3;
    LOWHIGH(x3, x5);
    LOWHIGH(x4, x5);
    if (x0 < x3) x0 = x3;
    x3 = m7;
    x6 = m8;
    LOWHIGH(x3, x6);
    x7 = m6;
    LOWHIGH(x3, x7);
    if (x0 < x3) x0 = x3;
    LOWHIGH(x6, x7);
    LOWHIGH(x4, x6);
    if (x1 < x4) x1 = x4;
    if (x1 > x6) x1 = x6;
    if (x5 > x7) x5 = x7;
    if (x2 > x5) x2 = x5;
    LOWHIGH(x1, x2);
    if (x0 < x1) x0 = x1;
    return x0 > x2 ? x2 : x0;
}
#undef LOWHIGH

static void VS_CC
proc_8bit(int w, int h, int stride, uint8_t *dstp, const uint8_t *r1)
{
    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - stride * !!y;
        const uint8_t *r2 = r1 + stride * !!(h -y);

        for (int x = 0; x <= w; x++) {
            int xl = x - !!x;
            int xr = x + !!(w - x);
            dstp[x] = get_median_9(r0[xl], r0[x], r0[xr],
                                   r1[xl], r1[x], r1[xr],
                                   r2[xl], r2[x], r2[xr]);
        }

        r1 += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_16bit(int w, int h, int stride, uint8_t *d, const uint8_t *srcp)
{
    stride >>= 1;
    const uint16_t *r1 = (uint16_t *)srcp;
    uint16_t *dstp = (uint16_t *)d;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - stride * !!y;
        const uint16_t *r2 = r1 + stride * !!(h -y);

        for (int x = 0; x <= w; x++) {
            int xl = x - !!x;
            int xr = x + !!(w - x);
            dstp[x] = get_median_9(r0[xl], r0[x], r0[xr],
                                   r1[xl], r1[x], r1[xr],
                                   r2[xl], r2[x], r2[xr]);
        }

        r1 += stride;
        dstp += stride;
    }
}


const proc_neighbors median[] = {
    proc_8bit,
    proc_16bit
};
