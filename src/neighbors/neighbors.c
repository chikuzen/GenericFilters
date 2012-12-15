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

#include "neighbors.h"

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif


typedef struct {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    int planes[3];
    const proc_neighbors *proc_function;
} neighbors_t;


static const VSFrameRef * VS_CC
neighbors_get_frame(int n, int activation_reason, void **instance_data,
                   void **frame_data, VSFrameContext *frame_ctx,
                   VSCore *core, const VSAPI *vsapi)
{
    neighbors_t *nh = (neighbors_t *)*instance_data;

    if (activation_reason == arInitial) {
        vsapi->requestFrameFilter(n, nh->node, frame_ctx);
        return NULL;
    }

    if (activation_reason != arAllFramesReady) {
        return NULL;
    }

    const VSFrameRef *src = vsapi->getFrameFilter(n, nh->node, frame_ctx);
    const VSFormat *fi = vsapi->getFrameFormat(src);
    if (fi->sampleType != stInteger) {
        return src;
    }
    const int pl[] = {0, 1, 2};
    const VSFrameRef *fr[] = {nh->planes[0] ? NULL : src,
                              nh->planes[1] ? NULL : src,
                              nh->planes[2] ? NULL : src};
    VSFrameRef *dst = vsapi->newVideoFrame2(fi, vsapi->getFrameWidth(src, 0),
                                            vsapi->getFrameHeight(src, 0),
                                            fr, pl, src, core);

    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }
        nh->proc_function[fi->bytesPerSample - 1](plane, src, vsapi, dst);
    }

    vsapi->freeFrame(src);
    return dst;
}


static void VS_CC
init_neighbors(VSMap *in, VSMap *out, void **instance_data, VSNode *node,
              VSCore *core, const VSAPI *vsapi)
{
    neighbors_t *nh = (neighbors_t *)*instance_data;
    vsapi->setVideoInfo(nh->vi, 1, node);
    vsapi->clearMap(in);
}


static void VS_CC
close_neighbors(void *instance_data, VSCore *core, const VSAPI *vsapi)
{
    neighbors_t *nh = (neighbors_t *)instance_data;
    if (!nh) {
        return;
    }
    if (nh->node) {
        vsapi->freeNode(nh->node);
        nh->node = NULL;
    }
    free(nh);
    nh = NULL;
}


#define RET_IF_ERROR(cond, message) \
{\
    if (cond) {\
        close_neighbors(nh, core, vsapi);\
        strcat(msg_buff, message);\
        vsapi->setError(out, msg_buff);\
        return;\
    }\
}

static void VS_CC
create_neighbors(const VSMap *in, VSMap *out, void *user_data, VSCore *core,
                 const VSAPI *vsapi)
{
    char msg_buff[256] = {0};
    neighbors_t *nh = (neighbors_t *)calloc(sizeof(neighbors_t), 1);
    RET_IF_ERROR(!nh, "neighbors: failed to allocate handler");

    const char *proc = (const char *)user_data;
    const char *name;
    switch (proc[0]) {
    case '0':
        nh->proc_function = minimum;
        name = "Minimum";
        break;
    case '1':
        nh->proc_function = maximum;
        name = "Maximum";
        break;
    default:
        nh->proc_function = median;
        name = "Median";
    }
    strcat(msg_buff, name);

    nh->node = vsapi->propGetNode(in, "clip", 0, 0);
    nh->vi = vsapi->getVideoInfo(nh->node);

    int num = vsapi->propNumElements(in, "planes");
    if (num < 1) {
        for (int i = 0; i < 3; nh->planes[i++] = 1);
    } else {
        for (int i = 0; i < num; i++) {
            int p = (int)vsapi->propGetInt(in, "planes", i, NULL);
            RET_IF_ERROR(p < 0 || p > 2, ": planes index out of range");
            nh->planes[p] = 1;
        }
    }

    vsapi->createFilter(in, out, name, init_neighbors, neighbors_get_frame,
                        close_neighbors, fmParallel, 0, nh, core);
}


const VSPublicFunction public_neighbors = create_neighbors;
