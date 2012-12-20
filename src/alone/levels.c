/*
  levels.c: Copyright (C) 2012  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of Tweak

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

#include <stdlib.h>
#include <math.h>
#include "common.h"


typedef struct filter_data {
    void (VS_CC *function)(int, const uint8_t *, uint8_t *, uint16_t *);
    uint16_t *lut;
} levels_t;


static void VS_CC
proc_8bit(int plane_size, const uint8_t *srcp, uint8_t *dstp, uint16_t *lut)
{
    while (plane_size--) {
        *dstp++ = (uint8_t)lut[*srcp++];
    }
}


static void VS_CC
proc_16bit(int plane_size, const uint8_t *s, uint8_t *d, uint16_t *lut)
{
    plane_size >>= 1;
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *srcp = (uint16_t *)s;

    while (plane_size--) {
        *dstp++ = lut[*srcp++];
    }
}


static void VS_CC
levels_get_frame(levels_t *lh, const VSFormat *fi, const VSFrameRef **fr,
                   const VSAPI *vsapi, const VSFrameRef *src, VSFrameRef *dst)
{
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }

        lh->function(vsapi->getStride(src, plane) *
                     vsapi->getFrameHeight(src, plane),
                     vsapi->getReadPtr(src, plane),
                     vsapi->getWritePtr(dst, plane),
                     lh->lut);
    }
}


static void VS_CC
free_levels_data(void *data)
{
    levels_t *lh = (levels_t *)data;
    if (lh && lh->lut) {
        free(lh->lut);
        lh->lut = NULL;
    }
}


static void VS_CC
set_lut(int imin, int imax, int omin, int omax, double gamma, int size,
        uint16_t *lut)
{
    int c0 = omax - omin;
    double c1 = 1.0 / (imax - imin);
    double rgamma = 1.0 / gamma;
    for (int pix = 0; pix < size; pix++) {
        lut[pix] = (uint16_t)((int)(pow(((pix - imin) * c1), rgamma) * c0 + 0.5) + omin);
    }
}


static void VS_CC
set_levels_data(tweak_handler_t *th, filter_id_t id, char *msg,
                const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    RET_IF_ERROR(!th->vi->format, "format is not constant");

    levels_t *lh = (levels_t *)calloc(sizeof(levels_t), 1);
    RET_IF_ERROR(!lh, "failed to allocate filter data");
    th->fdata = lh;

    // Is maximum of input really 511/1023 in the case of 9/10 bits ?
    lh->lut = (uint16_t *)calloc(sizeof(uint16_t),
                                 1 << (8 * th->vi->format->bytesPerSample));
    RET_IF_ERROR(!lh->lut, "out of memory");
    th->free_data = free_levels_data;

    int err;
    int bps = th->vi->format->bitsPerSample;
    int size = 1 << bps;
    int imin = (int)vsapi->propGetInt(in, "min_in", 0, &err);
    if (err || imin < 0) {
        imin = 0;
    }
    int imax = (int)vsapi->propGetInt(in, "max_in", 0, &err);
    if (err || imax > size - 1) {
        imax = 0xFF << (bps - 8);
    }
    int omin = (int)vsapi->propGetInt(in, "min_out", 0, &err);
    if (err || omin < 0) {
        omin = 0;
    }
    int omax = (int)vsapi->propGetInt(in, "max_out", 0, &err);
    if (err || omax > size - 1) {
        omax = 0xFF << (bps - 8);
    }
    double gamma = vsapi->propGetFloat(in, "gamma", 0, &err);
    if (err || gamma <= 0.0f) {
        gamma = 1.0;
    }

    set_lut(imin, imax, omin, omax, gamma, size, lh->lut);

    lh->function = bps > 8 ? proc_16bit : proc_8bit;
    th->get_frame_filter = levels_get_frame;
}


const set_filter_data_func set_levels = set_levels_data;
