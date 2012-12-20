/*
  limitter.c: Copyright (C) 2012  Oka Motofumi

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

#include <stdlib.h>
#include "common.h"

typedef void (VS_CC *proc_limitter)(int, int, int, const uint8_t *, uint8_t *,
                                    uint16_t, uint16_t);

typedef struct filter_data {
    proc_limitter function[2];
    int th_min;
    int th_max;
} limitter_t;


static void VS_CC
proc_8bit(int width, int height, int stride, const uint8_t *srcp, uint8_t *dstp,
          uint16_t th_min, uint16_t th_max)
{
    uint8_t min = th_min > 0x00FF ? 0xFF : th_min;
    uint8_t max = th_max > 0x00FF ? 0xFF : th_max;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t val = srcp[x];
            if (val < min) {
                dstp[x] = min;
                continue;
            }
            if (val > max) {
                dstp[x] = max;
                continue;
            }
            dstp[x] = val;
        }
        srcp += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_16bit(int width, int height, int stride, const uint8_t *s, uint8_t *d,
           uint16_t min, uint16_t max)
{
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *srcp = (uint16_t *)s;
    stride >>= 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t val = srcp[x];
            if (val < min) {
                dstp[x] = min;
                continue;
            }
            if (val > max) {
                dstp[x] = max;
                continue;
            }
            dstp[x] = val;
        }
        srcp += stride;
        dstp += stride;
    }
}


static void VS_CC
limitter_get_frame(limitter_t *lh, const VSFormat *fi, const VSFrameRef **fr,
                   const VSAPI *vsapi, const VSFrameRef *src, VSFrameRef *dst)
{
    uint16_t min = lh->th_min;
    uint16_t max = lh->th_max;

    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }

        lh->function[fi->bytesPerSample - 1](vsapi->getFrameWidth(src, plane),
                                             vsapi->getFrameHeight(src, plane),
                                             vsapi->getStride(src, plane),
                                             vsapi->getReadPtr(src, plane),
                                             vsapi->getWritePtr(dst, plane),
                                             min, max);
    }
}


static void VS_CC
set_limitter_data(neighbors_handler_t *nh, filter_id_t id, char *msg,
                  const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    limitter_t *lh = (limitter_t *)calloc(sizeof(limitter_t), 1);
    RET_IF_ERROR(!lh, "failed to allocate filter data");
    nh->fdata = lh;

    int err;
    lh->th_min = (int)vsapi->propGetInt(in, "min", 0, &err);
    if (err || lh->th_min < 0) {
        lh->th_min = 0;
    }
    lh->th_max = (int)vsapi->propGetInt(in, "max", 0, &err);
    if (err || lh->th_max > 0xFFFF) {
        lh->th_min = 0xFFFF;
    }
    RET_IF_ERROR(lh->th_min > lh->th_max, "min is larger than max");

    lh->function[0] = proc_8bit;
    lh->function[1] = proc_16bit;

    nh->get_frame_filter = limitter_get_frame;
}


const set_filter_data_func set_limitter = set_limitter_data;
