/*
  convolution.c: Copyright (C) 2012  Oka Motofumi

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
#include <string.h>

#include "common.h"
#include "convolution.h"


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

        int width = vsapi->getFrameWidth(src, plane);
        if (width < 2 &&
            ch->proc_function != convo_v3 &&
            ch->proc_function != convo_v5) {
            continue;
        }
        if (width < 4) {
            if (ch->proc_function == convo_h5 ||
                ch->proc_function == convo_hv5 ||
                ch->proc_function == convo_5x5) {
                continue;
            }
        }

        int height = vsapi->getFrameHeight(src, plane);
        if (height < 2 &&
            ch->proc_function != convo_h3 &&
            ch->proc_function != convo_h5) {
            continue;
        }
        if (height < 4) {
            if (ch->proc_function == convo_v5 ||
                ch->proc_function == convo_hv5 ||
                ch->proc_function == convo_5x5) {
                continue;
            }
        }

        int stride = vsapi->getStride(src, plane);
        uint8_t * dstp = vsapi->getWritePtr(dst, plane);
        const uint8_t *srcp = vsapi->getReadPtr(src, plane);

        ch->proc_function[fi->bytesPerSample - 1](ch, width, height, stride,
                                                  dstp, srcp, max);
    }
}


static void VS_CC
set_matrix_and_proc_function(convolution_t *ch, filter_id_t id, const VSMap *in,
                             const VSAPI *vsapi, char *msg)
{
    const char *param = id == ID_CONVO_HV ? "horizontal" : "matrix";
    int num = vsapi->propNumElements(in, param);
    RET_IF_ERROR((id == ID_CONVO_HV && num > 0 && num != 5) ||
                 (num > 0 && num != 3 && num != 5 && num != 9 && num != 25),
                 "invalid %s", param);

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
        if (id == ID_CONVO) {
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
        ch->proc_function = id == ID_CONVO_HV ? convo_hv5 : convo_3x3;
    }

    ch->m[4] = ch->m_v[1] = 1;
    param = "matrix";
    if (id == ID_CONVO_HV) {
        ch->m[2] = 1;
        param = "horizontal";
    }

    for (int i = 0; i < num; i++) {
        int element = (int)vsapi->propGetInt(in, param, i, NULL);
        RET_IF_ERROR(element < -32768 || element > 32767,
                     "%s has out of range value", param);
        ch->m[i] = element;
        ch->rdiv += element;
    }
    if (ch->rdiv == 0.0) {
        ch->rdiv = 1.0;
    }

    if (id == ID_CONVO) {
        return;
    }

    num = vsapi->propNumElements(in, "vertical");
    RET_IF_ERROR(num > 0 && num != 5, "invalid vertical");
    ch->m_v[2] = 1;
    for (int i = 0; i < num; i++) {
        int element = (int)vsapi->propGetInt(in, "vertical", i, NULL);
        RET_IF_ERROR(element < -32768 || element >32767,
                     "vertical has out of range value");
        ch->m_v[i] = element;
        ch->rdiv_v += element;
    }
    if (ch->rdiv_v == 0.0) {
        ch->rdiv_v = 1.0;
    }
}


static void VS_CC
set_convolution_data(tweak_handler_t *th, filter_id_t id, char *msg,
                     const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    convolution_t *ch = (convolution_t *)calloc(sizeof(convolution_t), 1);
    RET_IF_ERROR(!ch, "failed to allocate filter data");
    th->fdata = ch;

    set_matrix_and_proc_function(ch, id, in, vsapi, msg);
    RET_IF_ERROR(msg[0], " ");

    int err;
    ch->bias = vsapi->propGetFloat(in, "bias", 0, &err);
    if (err) {
        ch->bias = 0.0;
    }

    double div = vsapi->propGetFloat(
        in, id == ID_CONVO_HV ? "divisor_h" : "divisor", 0, &err);
    if (!err && div != 0.0) {
        ch->rdiv = div;
    }
    ch->rdiv = 1.0 / ch->rdiv;

    if (id == ID_CONVO_HV) {
        div = vsapi->propGetFloat(in, "divisor_v", 0, &err);
        if (!err && div != 0.0) {
            ch->rdiv_v = div;
        }
        ch->rdiv_v = 1.0 / ch->rdiv_v;
    }

    th->get_frame_filter = convolution_get_frame;
}
#undef RET_IF_ERROR


const set_filter_data_func set_convolution = set_convolution_data;
