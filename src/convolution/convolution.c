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

#include "common.h"
#include "convolution.h"


static void VS_CC
convolution_get_frame(convolution_t *ch, const VSFormat *fi,
                      const VSFrameRef **fr, const VSAPI *vsapi,
                      const VSFrameRef *src, VSFrameRef *dst)
{
    uint16_t max = (1 << fi->bitsPerSample) - 1;
    int index = fi->bytesPerSample - 1;
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }

        int width = vsapi->getFrameWidth(src, plane);
        if (width < 2 &&
            ch->function != convo_v3 && ch->function != convo_v5) {
            continue;
        }
        if (width < 4 &&
           (ch->function == convo_h5 || ch->function == convo_5x5)) {
            continue;
        }

        int height = vsapi->getFrameHeight(src, plane);
        if (height < 2 &&
            ch->function != convo_h3 &&
            ch->function != convo_h5) {
            continue;
        }
        if (height < 4 &&
            (ch->function == convo_v5 || ch->function == convo_5x5)) {
            continue;
        }

        ch->function[index](ch, width, height,
                            vsapi->getStride(src, plane),
                            vsapi->getWritePtr(dst, plane),
                            vsapi->getReadPtr(src, plane),
                            max);
    }
}


static void VS_CC
set_matrix_and_proc_function(convolution_t *ch, const VSMap *in,
                             const VSAPI *vsapi, char *msg)
{
    const char *param = "matrix";
    int num = vsapi->propNumElements(in, param);
    RET_IF_ERROR(num > 0 && num != 3 && num != 5 && num != 9 && num != 25,
                 "invalid %s", param);

    switch (num) {
    case 3:
        ch->function = convo_h3;
        break;
    case 5:
        ch->function = convo_h5;
        break;
    case 25:
        ch->function = convo_5x5;
        break;
    default:
        ch->function = convo_3x3;
    }

    int err;
    const char *mode = vsapi->propGetData(in, "mode", 0, &err);
    if (!err && mode[0] == 'v' && (num == 3 || num == 5)) {
        ch->function = num == 3 ? convo_v3 : convo_v5;
    }

    ch->m[4] = 1;
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
}


static void VS_CC
set_convolution_data(tweak_handler_t *th, filter_id_t id, char *msg,
                     const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    convolution_t *ch = (convolution_t *)calloc(sizeof(convolution_t), 1);
    RET_IF_ERROR(!ch, "failed to allocate filter data");
    th->fdata = ch;

    set_matrix_and_proc_function(ch, in, vsapi, msg);
    RET_IF_ERROR(msg[0], " ");

    int err;
    ch->bias = vsapi->propGetFloat(in, "bias", 0, &err);
    if (err) {
        ch->bias = 0.0;
    }

    double div = vsapi->propGetFloat(in, "divisor", 0, &err);
    if (!err && div != 0.0) {
        ch->rdiv = div;
    }
    ch->rdiv = 1.0 / ch->rdiv;

    th->get_frame_filter = convolution_get_frame;
}


const set_filter_data_func set_convolution = set_convolution_data;
