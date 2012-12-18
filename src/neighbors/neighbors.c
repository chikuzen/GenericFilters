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
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "common.h"
#include "neighbors.h"


typedef struct filter_data {
    const proc_neighbors *proc_function;
} neighbors_t;


static void VS_CC
neighbors_get_frame(neighbors_t *nh, const VSFormat *fi, const VSFrameRef **fr,
                    const VSAPI *vsapi, const VSFrameRef *src, VSFrameRef *dst)
{
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }
        nh->proc_function[fi->bytesPerSample - 1](plane, src, vsapi, dst);
    }
}


static void VS_CC
create_neighbors(const VSMap *in, VSMap *out, void *user_data, VSCore *core,
                 const VSAPI *vsapi)
{
    const char *filter_name = (char *)user_data;
    char msg_buff[256] = {0};
    snprintf(msg_buff, 256, "%s: ", filter_name);
    char *msg = msg_buff + strlen(filter_name);

    neighbors_handler_t *nh = 
        (neighbors_handler_t *)calloc(sizeof(neighbors_handler_t), 1);
    RET_IF_ERROR(!nh, "failed to allocate handler");
    nh->fdata = (neighbors_t *)calloc(sizeof(neighbors_t), 1);
    RET_IF_ERROR(!nh->fdata, "failed to allocate filter data");

    switch (filter_name[1]) {
    case 'a':
        nh->fdata->proc_function = maximum;
        break;
    case 'e':
        nh->fdata->proc_function = median;
        break;
    default:
        nh->fdata->proc_function = minimum;
    }

    nh->node = vsapi->propGetNode(in, "clip", 0, 0);
    nh->vi = vsapi->getVideoInfo(nh->node);
    RET_IF_ERROR(set_planes(nh, in, vsapi), "planes index out of range");

    nh->get_frame_filter = neighbors_get_frame;

    vsapi->createFilter(in, out, filter_name, init_filter, get_frame,
                        free_filter, fmParallel, 0, nh, core);
}


const VSPublicFunction public_neighbors = create_neighbors;
