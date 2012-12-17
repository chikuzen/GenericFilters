/*
  convolution.c: Copyright (C) 2012  Oka Motofumi

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
#include <stdarg.h>

#include "convolution.h"

#ifdef _MSC_VER
#pragma warning(disable:4996)
#define snprintf _snprintf
#endif


static const VSFrameRef * VS_CC
convolution_get_frame(int n, int activation_reason, void **instance_data,
                      void **frame_data, VSFrameContext *frame_ctx,
                      VSCore *core, const VSAPI *vsapi)
{
    convolution_t *ch = (convolution_t *)*instance_data;

    if (activation_reason == arInitial) {
        vsapi->requestFrameFilter(n, ch->node, frame_ctx);
        return NULL;
    }

    if (activation_reason != arAllFramesReady) {
        return NULL;
    }

    const VSFrameRef *src = vsapi->getFrameFilter(n, ch->node, frame_ctx);
    const VSFormat *fi = vsapi->getFrameFormat(src);
    if (fi->sampleType != stInteger) {
        return src;
    }

    const int pl[] = {0, 1, 2};
    const VSFrameRef *fr[] = {ch->planes[0] ? NULL : src,
                              ch->planes[1] ? NULL : src,
                              ch->planes[2] ? NULL : src};
    VSFrameRef *dst = vsapi->newVideoFrame2(fi, vsapi->getFrameWidth(src, 0),
                                            vsapi->getFrameHeight(src, 0),
                                            fr, pl, src, core);

    uint16_t max = (1 << fi->bitsPerSample) - 1;
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }
        ch->proc_function[fi->bytesPerSample - 1](ch, plane, src, dst, vsapi, max);
    }

    vsapi->freeFrame(src);
    return dst;
}


static void VS_CC
init_convolution(VSMap *in, VSMap *out, void **instance_data, VSNode *node,
                 VSCore *core, const VSAPI *vsapi)
{
    convolution_t *ch = (convolution_t *)*instance_data;
    vsapi->setVideoInfo(ch->vi, 1, node);
    vsapi->clearMap(in);
}



static void VS_CC
close_convolution(void *instance_data, VSCore *core, const VSAPI *vsapi)
{
    convolution_t *ch = (convolution_t *)instance_data;
    if (!ch) {
        return;
    }
    if (ch->node) {
        vsapi->freeNode(ch->node);
        ch->node = NULL;
    }
    free(ch);
    ch = NULL;
}


#define RET_IF_ERROR(cond, ...) \
{\
    if (cond) {\
        close_convolution(ch, core, vsapi);\
        snprintf(msg, 240, __VA_ARGS__);\
        vsapi->setError(out, msg_buff);\
        return;\
    }\
}

static void VS_CC
create_convolution(const VSMap *in, VSMap *out, void *user_data, VSCore *core,
               const VSAPI *vsapi)
{
    char msg_buff[256] = "Convolution: ";
    char *msg = msg_buff + strlen(msg_buff);
    int err;

    convolution_t *ch = (convolution_t *)calloc(sizeof(convolution_t), 1);
    RET_IF_ERROR(!ch, "failed to allocate handler");

    ch->node = vsapi->propGetNode(in, "clip", 0, 0);
    ch->vi = vsapi->getVideoInfo(ch->node);

    int num = vsapi->propNumElements(in, "planes");
    if (num < 1) {
        for (int i = 0; i < 3; ch->planes[i++] = 1);
    } else {
        for (int i = 0; i < num; i++) {
            int p = (int)vsapi->propGetInt(in, "planes", i, NULL);
            RET_IF_ERROR(p < 0 || p > 2, "planes index out of range");
            ch->planes[p] = 1;
        }
    }

    num = vsapi->propNumElements(in, "matrix");
    RET_IF_ERROR(num > 0 && num != 3 && num != 5 && num != 6 && num != 9 &&
                 num != 10 && num != 25, "invalid matrix");
    switch (num) {
    case 3:
    case 5:
        ch->proc_function = num == 3 ? convo_h3 : convo_h5;
        const char *mode = vsapi->propGetData(in, "mode", 0, &err);
        if (!err && mode[0] == 'v') {
            ch->proc_function = num == 3 ? convo_v3 : convo_v5;
        }
        break;
    case 6:
        ch->proc_function = convo_hv3;
        break;
    case 10:
        ch->proc_function = convo_hv5;
        break;
    case 25:
        ch->proc_function = convo_5x5;
        break;
    default:
        ch->proc_function = convo_3x3;
    }

    ch->m[4] = 1;
    for (int i = 0; i < num; i++) {
        int element = (int)vsapi->propGetInt(in, "matrix", i, NULL);
        ch->m[i] = element;
        ch->div += element;
    }
    if (ch->div == 0.0) {
        ch->div = 1.0;
    }

    ch->bias = vsapi->propGetFloat(in, "bias", 0, &err);
    if (err) {
        ch->bias = 0.0;
    }
    ch->bias += 0.5;

    double div = vsapi->propGetFloat(in, "divisor", 0, &err);
    if (!err && div != 0.0) {
        ch->div = div;
    }
    vsapi->createFilter(in, out, "Convolution", init_convolution,
                        convolution_get_frame, close_convolution, fmParallel,
                        0, ch, core);
}
#undef RET_IF_ERROR


const VSPublicFunction public_convolution = create_convolution;
