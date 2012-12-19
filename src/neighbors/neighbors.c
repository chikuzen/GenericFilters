/*
  neighbors.c: Copyright (C) 2012  Oka Motofumi

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
#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "neighbors.h"


static void VS_CC
neighbors_get_frame(neighbors_t *nh, const VSFormat *fi, const VSFrameRef **fr,
                    const VSAPI *vsapi, const VSFrameRef *src, VSFrameRef *dst)
{
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }
        
        int w = vsapi->getFrameWidth(src, plane) - 1;
        int h = vsapi->getFrameHeight(src, plane) - 1;
        int stride = vsapi->getStride(src, plane);

        uint8_t *dstp = vsapi->getWritePtr(dst, plane);
        const uint8_t *srcp = vsapi->getReadPtr(src, plane);
        nh->proc_function[fi->bytesPerSample - 1](w, h, stride, dstp, srcp);
    }
}


static void VS_CC
set_neighbors_data(neighbors_handler_t *nh, filter_id_t id, char *msg,
                   const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    neighbors_t *nb = (neighbors_t *)calloc(sizeof(neighbors_t), 1);
    RET_IF_ERROR(!nb, "failed to allocate filter data");
    nh->fdata = nb;

    switch (id) {
    case ID_MAXIMUM:
        nb->proc_function = maximum;
        break;
    case ID_MEDIAN:
        nb->proc_function = median;
        break;
    default:
        nb->proc_function = minimum;
    }

    nh->get_frame_filter = neighbors_get_frame;
}


const set_filter_data_t set_neighbors = set_neighbors_data;
