/*
  neighbors.c: Copyright (C) 2012  Oka Motofumi

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


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define USE_ALIGNED_MALLOC
#include "common.h"
#include "neighbors.h"


static void VS_CC
neighbors_get_frame(neighbors_t *nh, const VSFormat *fi, const VSFrameRef **fr,
                    const VSAPI *vsapi, const VSFrameRef *src, VSFrameRef *dst)
{
    int bps = fi->bytesPerSample;
    int bstride = ((vsapi->getFrameWidth(src, 0) * bps + 32 + 15) / 16) * 16;
     uint8_t *buff = (uint8_t *)_aligned_malloc(bstride * 3, 16);
    if (!buff) {
        return;
    }

    bps--;
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }

        nh->function[bps](buff, bstride,
                          vsapi->getFrameWidth(src, plane),
                          vsapi->getFrameHeight(src, plane),
                          vsapi->getStride(src, plane),
                          vsapi->getWritePtr(dst, plane),
                          vsapi->getReadPtr(src, plane),
                          nh->th);
    }

    _aligned_free(buff);
}


static void VS_CC
set_neighbors_data(tweak_handler_t *th, filter_id_t id, char *msg,
                   const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    neighbors_t *nh = (neighbors_t *)calloc(sizeof(neighbors_t), 1);
    RET_IF_ERROR(!nh, "failed to allocate filter data");
    th->fdata = nh;

    switch (id) {
    case ID_MAXIMUM:
        nh->function = maximum;
        break;
    case ID_MEDIAN:
        nh->function = median;
        break;
    default:
        nh->function = minimum;
    }

    int err;
    nh->th = (int)vsapi->propGetInt(in, "threshold", 0, &err);
    if (err || nh->th < 0 || nh->th > 0xFF) {
        nh->th = 0xFF;
    }

    th->get_frame_filter = neighbors_get_frame;
}


const set_filter_data_func set_neighbors = set_neighbors_data;
