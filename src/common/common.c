/*
  common.c: Copyright (C) 2012  Oka Motofumi

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
#include <string.h>

#include "common.h"


static const VSFrameRef * VS_CC
get_frame_common(int n, int activation_reason, void **instance_data,
                 void **frame_data, VSFrameContext *frame_ctx, VSCore *core,
                 const VSAPI *vsapi)
{
    neighbors_handler_t *nh = (neighbors_handler_t *)*instance_data;

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

    nh->get_frame_filter(nh->fdata, fi, fr, vsapi, src, dst);

    vsapi->freeFrame(src);

    return dst;
}


static void VS_CC
vs_init(VSMap *in, VSMap *out, void **instance_data, VSNode *node,
        VSCore *core, const VSAPI *vsapi)
{
    neighbors_handler_t *nh = (neighbors_handler_t *)*instance_data;
    vsapi->setVideoInfo(nh->vi, 1, node);
    vsapi->clearMap(in);
}


static void VS_CC
close_handler(void *instance_data, VSCore *core, const VSAPI *vsapi)
{
    neighbors_handler_t *nh = (neighbors_handler_t *)instance_data;
    if (!nh) {
        return;
    }
    if (nh->node) {
        vsapi->freeNode(nh->node);
        nh->node = NULL;
    }
    if (nh->fdata) {
        if (nh->free_data) {
            nh->free_data(nh->fdata);
        }
        free(nh->fdata);
        nh->fdata = NULL;
    }
    free(nh);
    nh = NULL;
}


static int VS_CC
set_planes(neighbors_handler_t *nh, const VSMap *in, const VSAPI *vsapi)
{
    int num = vsapi->propNumElements(in, "planes");
    if (num < 1) {
        for (int i = 0; i < 3; nh->planes[i++] = 1);
        return 0;
    }

    for (int i = 0; i < num; i++) {
        int p = (int)vsapi->propGetInt(in, "planes", i, NULL);
        if (p < 0 || p > 2) {
            return -1;
        }
        nh->planes[p] = 1;
    }

    return 0;
}


typedef struct {
    set_filter_data_func function;
    filter_id_t id;
} setter_t;


static setter_t get_setter(const char *filter_name)
{
    struct {
        const char *name;
        setter_t setter;
    } table[] = {
        { "Convolution",   { set_convolution, ID_CONVO    } },
        { "ConvolutionHV", { set_convolution, ID_CONVO_HV } },
        { "Maximum",       { set_neighbors,   ID_MAXIMUM  } },
        { "Median",        { set_neighbors,   ID_MEDIAN   } },
        { "Minimum",       { set_neighbors,   ID_MINIMUM  } },
        { "Invert",        { set_invert,      ID_INVERT   } },
        { "Limitter",      { set_limitter,    ID_LIMITTER } },
        { "Levels",        { set_levels,      ID_LEVELS   } },
        { filter_name,     { NULL,            ID_NONE     } }
    };

    int i = 0;
    while (strcmp(filter_name, table[i].name) != 0) i++;

    return table[i].setter;
}


#ifdef RET_IF_ERROR
#undef RET_IF_ERROR
#endif

#define RET_IF_ERROR(cond, ...) { \
    if (cond) { \
        close_handler(nh, core, vsapi); \
        snprintf(msg, 240, __VA_ARGS__); \
        vsapi->setError(out, msg_buff); \
        return; \
    } \
}

static void VS_CC
create_filter_common(const VSMap *in, VSMap *out, void *user_data, VSCore *core,
                     const VSAPI *vsapi)
{
    const char *filter_name = (char *)user_data;
    char msg_buff[256] = { 0 };
    snprintf(msg_buff, 256, "%s: ", filter_name);
    char *msg = msg_buff + strlen(msg_buff);

    neighbors_handler_t *nh =
        (neighbors_handler_t *)calloc(sizeof(neighbors_handler_t), 1);
    RET_IF_ERROR(!nh, "failed to allocate handler");

    nh->node = vsapi->propGetNode(in, "clip", 0, 0);
    nh->vi = vsapi->getVideoInfo(nh->node);
    RET_IF_ERROR(set_planes(nh, in, vsapi), "planes index out of range");

    setter_t setter = get_setter(filter_name);
    RET_IF_ERROR(setter.id == ID_NONE, "initialize failed");

    setter.function(nh, setter.id, msg, in, out, vsapi);
    RET_IF_ERROR(msg[0], " ");

    vsapi->createFilter(in, out, filter_name, vs_init, get_frame_common,
                        close_handler, fmParallel, 0, nh, core);
}
#undef RET_IF_ERROR


VS_EXTERNAL_API(void)
VapourSynthPluginInit(VSConfigPlugin conf, VSRegisterFunction reg,
                      VSPlugin *plugin)
{
    conf("chikuzen.does.not.have.his.own.domain.neighbors", "neighbors",
         "Pixel value modifier with reference to the neighbor pixels v"
         NEIGHBORS_VERSION, VAPOURSYNTH_API_VERSION, 1, plugin);
    reg("Convolution",
        "clip:clip;matrix:int[]:opt;bias:float:opt;divisor:float:opt;"
        "planes:int[]:opt;mode:data:opt;",
        create_filter_common, (void *)"Convolution", plugin);
    reg("ConvolutionHV",
        "clip:clip;horizontal:int[]:opt;vertical:int[]:opt;bias:float:opt;"
        "divisor_h:float:opt;divisor_v:float:opt;planes:int[]:opt;",
        create_filter_common, (void *)"ConvolutionHV", plugin);
    reg("Minimum", "clip:clip;planes:int[]:opt;",
        create_filter_common, (void *)"Minimum", plugin);
    reg("Maximum", "clip:clip;planes:int[]:opt;",
        create_filter_common, (void *)"Maximum", plugin);
    reg("Median", "clip:clip;planes:int[]:opt;",
        create_filter_common, (void *)"Median", plugin);
    reg("Invert", "clip:clip;planes:int[]:opt;",
        create_filter_common, (void *)"Invert", plugin);
    reg("Limitter", "clip:clip;min:int:opt;max:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Limitter", plugin);
    reg("Levels",
        "clip:clip;min_in:int[]:opt;max_in:int[]:opt;gamma:float:opt;"
        "min_out:int:opt;max_out:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Levels", plugin);
}
