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

#include "common.h"
#include "convolution.h"


typedef enum {
    FT_CONVO,
    FT_CONVO_HV
} filter_type;


static void VS_CC
convolution_get_frame(convolution_t *ch, const VSFormat *fi,
                      const VSFrameRef **fr, const VSAPI *vsapi,
                      const VSFrameRef *src, VSFrameRef *dst)
{
    uint16_t max = (1 << fi->bitsPerSample) - 1;
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }
        ch->proc_function[fi->bytesPerSample - 1](ch, plane, src, dst, vsapi, max);
    }
}


static void VS_CC
set_matrix_and_proc_function(convolution_t *ch, filter_type ft, const VSMap *in,
                            const VSAPI *vsapi, char *msg)
{
    const char *param = ft == FT_CONVO_HV ? "horizontal" : "matrix";
    int num = vsapi->propNumElements(in, param);
    if ((ft == FT_CONVO_HV && num > 0 && num != 5) ||
        (num > 0 && num != 3 && num != 5 && num != 9 && num != 25)) {
        snprintf(msg, 240, "invalid %s", param);
        return;
    }

    int err;
    switch (num) {
    case 3:
        ch->proc_function = convo_h3;
        param = vsapi->propGetData(in, "mode", 0, &err);
        if (!err && param[0] == 'v') {
            ch->proc_function = convo_v3;
        }
        break;
    case 5:
        ch->proc_function = convo_hv5;
        if (ft == FT_CONVO) {
            ch->proc_function = convo_h5;
            param = vsapi->propGetData(in, "mode", 0, &err);
            if (!err && param[0] == 'v') {
                ch->proc_function = convo_v5;
            }
        }
        break;
    case 25:
        ch->proc_function = convo_5x5;
        break;
    default:
        ch->proc_function = ft == FT_CONVO_HV ? convo_hv5 : convo_3x3;
    }

    ch->m[4] = ch->m_v[1] = 1;
    ch->m[2] = ft == FT_CONVO_HV ? 1 : 0;
    param = ft == FT_CONVO_HV ? "horizontal" : "matrix";
    for (int i = 0; i < num; i++) {
        int element = (int)vsapi->propGetInt(in, param, i, NULL);
        if (element < -32768 || element > 32767) {
            snprintf(msg, 240, "%s has out of range value", param);
            return;
        }
        ch->m[i] = element;
        ch->div += element;
    }
    if (ch->div == 0.0) {
        ch->div = 1.0;
    }
    if (ft == FT_CONVO_HV) {
        num = vsapi->propNumElements(in, "vertical");
        if (num > 0 && num != 5) {
            snprintf(msg, 240, "invalid vertical");
            return;
        }
        ch->m_v[2] = 1;
        for (int i = 0; i < num; i++) {
            int element = (int)vsapi->propGetInt(in, "vertical", i, NULL);
            if (element < -32768 || element >32767) {
                snprintf(msg, 240, "vertical has out of range value");
                return;
            }
            ch->m_v[i] = element;
            ch->div_v += element;
        }
        if (ch->div_v == 0.0) {
            ch->div_v = 1.0;
        }
    }
}


static void VS_CC
create_convolution(const VSMap *in, VSMap *out, void *user_data, VSCore *core,
                   const VSAPI *vsapi)
{
    const char *filter_name = (char *)user_data;
    char msg_buff[256] = { 0 };
    snprintf(msg_buff, 256, "%s: ", filter_name);
    char *msg = msg_buff + strlen(msg_buff);
    int err;

    neighbors_handler_t *nh =
        (neighbors_handler_t *)calloc(sizeof(neighbors_handler_t), 1);
    RET_IF_ERROR(!nh, "failed to allocate handler");
    nh->fdata = (convolution_t *)calloc(sizeof(convolution_t), 1);
    RET_IF_ERROR(!nh->fdata, "failed to allocate filter data");

    nh->node = vsapi->propGetNode(in, "clip", 0, 0);
    nh->vi = vsapi->getVideoInfo(nh->node);
    RET_IF_ERROR(set_planes(nh, in, vsapi), "planes index out of range");

    filter_type ft = FT_CONVO;
    if (strcmp(filter_name, "ConvolutionHV") == 0) {
        ft =  FT_CONVO_HV;
    }

    set_matrix_and_proc_function(nh->fdata, ft, in, vsapi, msg);
    RET_IF_ERROR(msg[0], " ");

    nh->fdata->bias = vsapi->propGetFloat(in, "bias", 0, &err);
    if (err) {
        nh->fdata->bias = 0.0;
    }

    double div = vsapi->propGetFloat(
        in, ft == FT_CONVO_HV ? "divisor_h" : "divisor", 0, &err);
    if (!err && div != 0.0) {
        nh->fdata->div = div;
    }

    if (ft == FT_CONVO_HV) {
        div = vsapi->propGetFloat(in, "divisor_v", 0, &err);
        if (!err && div != 0.0) {
            nh->fdata->div_v = div;
        }
    }

    nh->get_frame_filter = convolution_get_frame;

    vsapi->createFilter(in, out, filter_name, init_filter, get_frame,
                        free_filter, fmParallel, 0, nh, core);
}
#undef RET_IF_ERROR

const VSPublicFunction public_convolution = create_convolution;
