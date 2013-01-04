/*
  common.c: Copyright (C) 2012-2013  Oka Motofumi

  Author: Oka Motofumi (chikuzen.mo at gmail dot com)

  This file is part of GenericFilters.

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
    generic_handler_t *gh = (generic_handler_t *)*instance_data;

    if (activation_reason == arInitial) {
        vsapi->requestFrameFilter(n, gh->node, frame_ctx);
        return NULL;
    }

    if (activation_reason != arAllFramesReady) {
        return NULL;
    }

    const VSFrameRef *src = vsapi->getFrameFilter(n, gh->node, frame_ctx);
    const VSFormat *fi = vsapi->getFrameFormat(src);
    if (fi->sampleType != stInteger) {
        return src;
    }
    const int pl[] = {0, 1, 2};
    const VSFrameRef *fr[] = {gh->planes[0] ? NULL : src,
                              gh->planes[1] ? NULL : src,
                              gh->planes[2] ? NULL : src};
    VSFrameRef *dst = vsapi->newVideoFrame2(fi, vsapi->getFrameWidth(src, 0),
                                            vsapi->getFrameHeight(src, 0),
                                            fr, pl, src, core);

    gh->get_frame_filter(gh->fdata, fi, fr, vsapi, src, dst);

    vsapi->freeFrame(src);

    return dst;
}


static void VS_CC
vs_init(VSMap *in, VSMap *out, void **instance_data, VSNode *node,
        VSCore *core, const VSAPI *vsapi)
{
    generic_handler_t *gh = (generic_handler_t *)*instance_data;
    vsapi->setVideoInfo(gh->vi, 1, node);
    vsapi->clearMap(in);
}


static void VS_CC
close_handler(void *instance_data, VSCore *core, const VSAPI *vsapi)
{
    generic_handler_t *gh = (generic_handler_t *)instance_data;
    if (!gh) {
        return;
    }
    if (gh->node) {
        vsapi->freeNode(gh->node);
        gh->node = NULL;
    }
    if (gh->fdata) {
        if (gh->free_data) {
            gh->free_data(gh->fdata);
        }
        free(gh->fdata);
        gh->fdata = NULL;
    }
    free(gh);
    gh = NULL;
}


static int VS_CC
set_planes(generic_handler_t *gh, const VSMap *in, const VSAPI *vsapi)
{
    int num = vsapi->propNumElements(in, "planes");
    if (num < 1) {
        for (int i = 0; i < 3; gh->planes[i++] = 1);
        return 0;
    }

    for (int i = 0; i < num; i++) {
        int p = (int)vsapi->propGetInt(in, "planes", i, NULL);
        if (p < 0 || p > 2) {
            return -1;
        }
        gh->planes[p] = 1;
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
        { "Convolution",   { set_convolution,    ID_CONVO    } },
        { "ConvolutionHV", { set_convolution_hv, ID_CONVO_HV } },
        { "Sobel",         { set_edge,           ID_SOBEL    } },
        { "Prewitt",       { set_edge,           ID_PREWITT  } },
        { "ConvolutionHV", { set_convolution_hv, ID_CONVO_HV } },
        { "Maximum",       { set_neighbors,      ID_MAXIMUM  } },
        { "Median",        { set_neighbors,      ID_MEDIAN   } },
        { "Minimum",       { set_neighbors,      ID_MINIMUM  } },
        { "Invert",        { set_invert,         ID_INVERT   } },
        { "Limiter",       { set_limiter,        ID_LIMITER  } },
        { "Levels",        { set_levels,         ID_LEVELS   } },
        { "Inflate",       { set_xxflate,        ID_INFLATE  } },
        { "Deflate",       { set_xxflate,        ID_DEFLATE  } },
        { "Binarize",      { set_binarize,       ID_BINARIZE } },
        { filter_name,     { NULL,               ID_NONE     } }
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
        close_handler(gh, core, vsapi); \
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

    generic_handler_t *gh =
        (generic_handler_t *)calloc(sizeof(generic_handler_t), 1);
    RET_IF_ERROR(!gh, "failed to allocate handler");

    gh->node = vsapi->propGetNode(in, "clip", 0, 0);
    gh->vi = vsapi->getVideoInfo(gh->node);
    RET_IF_ERROR(set_planes(gh, in, vsapi), "planes index out of range");

    setter_t setter = get_setter(filter_name);
    RET_IF_ERROR(setter.id == ID_NONE, "initialize failed");
    setter.function(gh, setter.id, msg, in, out, vsapi);
    RET_IF_ERROR(msg[0], " ");

    vsapi->createFilter(in, out, filter_name, vs_init, get_frame_common,
                        close_handler, fmParallel, 0, gh, core);
}
#undef RET_IF_ERROR


VS_EXTERNAL_API(void)
VapourSynthPluginInit(VSConfigPlugin conf, VSRegisterFunction reg,
                      VSPlugin *plugin)
{
    conf("chikuzen.does.not.have.his.own.domain.genericfilters", "generic",
         "Set of common image-processing filters v"
         GENERIC_FILTERS_VERSION, VAPOURSYNTH_API_VERSION, 1, plugin);
    reg("Convolution",
        "clip:clip;matrix:int[]:opt;bias:float:opt;divisor:float:opt;"
        "planes:int[]:opt;saturate:int:opt;mode:data:opt;",
        create_filter_common, (void *)"Convolution", plugin);
    reg("ConvolutionHV",
        "clip:clip;horizontal:int[]:opt;vertical:int[]:opt;bias:float:opt;"
        "divisor_h:float:opt;divisor_v:float:opt;planes:int[]:opt;"
        "saturate:int:opt;",
        create_filter_common, (void *)"ConvolutionHV", plugin);
    reg("Sobel", "clip:clip;min:int:opt;max:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Sobel", plugin);
    reg("Prewitt", "clip:clip;min:int:opt;max:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Prewitt", plugin);
    reg("Minimum",
        "clip:clip;planes:int[]:opt;threshold:int:opt;coordinates:int[]:opt",
        create_filter_common, (void *)"Minimum", plugin);
    reg("Maximum",
        "clip:clip;planes:int[]:opt;threshold:int:opt;coordinates:int[]:opt",
        create_filter_common, (void *)"Maximum", plugin);
    reg("Median", "clip:clip;planes:int[]:opt;",
        create_filter_common, (void *)"Median", plugin);
    reg("Invert", "clip:clip;planes:int[]:opt;",
        create_filter_common, (void *)"Invert", plugin);
    reg("Limiter",
        "clip:clip;min:int:opt;max:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Limiter", plugin);
    reg("Levels",
        "clip:clip;min_in:int[]:opt;max_in:int[]:opt;gamma:float:opt;"
        "min_out:int:opt;max_out:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Levels", plugin);
    reg("Inflate", "clip:clip;planes:int[]:opt;threshold:int:opt;",
        create_filter_common, (void *)"Inflate", plugin);
    reg("Deflate", "clip:clip;planes:int[]:opt;threshold:int:opt;",
        create_filter_common, (void *)"Deflate", plugin);
    reg("Binarize",
        "clip:clip;threshold:int:opt;v0:int:opt;v1:int:opt;planes:int[]:opt;",
        create_filter_common, (void *)"Binarize", plugin);
}
