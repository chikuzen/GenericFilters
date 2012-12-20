/*
  convolution_hv.c: Copyright (C) 2012  Oka Motofumi

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
#include "convolution_hv.h"


static void VS_CC
convolution_hv_get_frame(convolution_hv_t *ch, const VSFormat *fi,
                         const VSFrameRef **fr, const VSAPI *vsapi,
                         const VSFrameRef *src, VSFrameRef *dst)
{
    uint16_t max = (1 << fi->bitsPerSample) - 1;
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }

        int width = vsapi->getFrameWidth(src, plane);
        int height = vsapi->getFrameHeight(src, plane);
        if (width < 4 || height < 4) {
            continue;
        }

        ch->function[fi->bytesPerSample - 1](ch, width, height,
                                             vsapi->getStride(src, plane),
                                             vsapi->getWritePtr(dst, plane),
                                             vsapi->getReadPtr(src, plane),
                                             max);
    }
}


static void VS_CC
set_matrix(convolution_hv_t *ch, const VSMap *in, const VSAPI *vsapi, char *msg)
{
    const char *params_m[] = {"horizontal", "vertical"};
    const char *params_d[] = {"divisor_h", "divisor_v"};
    int *matrix[] = {ch->m_h, ch->m_v};
    double *rdiv[] = {&ch->rdiv_h, &ch->rdiv_v};

    for (int i = 0; i < 2; i++) {
        int num = vsapi->propNumElements(in, params_m[i]);
        RET_IF_ERROR(num > 0 && num != 5, "invalid %s", params_m[i]);

        int err;
        matrix[i][2] = 1;
        for (int j = 0; i < num; i++) {
            int element = (int)vsapi->propGetInt(in, params_m[i], j, NULL);
            RET_IF_ERROR(element < -32768 || element > 32767,
                         "%s has out of range value", params_m[i]);
            matrix[i][j] = element;
            *rdiv[i] += element;
        }
        if (*rdiv[i] == 0.0) {
            *rdiv[i] = 1.0;
        }
        
        double div = vsapi->propGetFloat(in, params_d[i], 0, &err);
        if (!err && div != 0.0) {
            *rdiv[i] = div;
        }
        *rdiv[i] = 1.0 / *rdiv[i];
    }
}


static void VS_CC
set_convolution_hv_data(tweak_handler_t *th, filter_id_t id, char *msg,
                        const VSMap *in, VSMap *out, const VSAPI *vsapi)
{
    convolution_hv_t *ch = (convolution_hv_t *)calloc(sizeof(convolution_hv_t), 1);
    RET_IF_ERROR(!ch, "failed to allocate filter data");
    th->fdata = ch;

    set_matrix(ch, in, vsapi, msg);
    RET_IF_ERROR(msg[0], " ");

    int err;
    ch->bias = vsapi->propGetFloat(in, "bias", 0, &err);
    if (err) {
        ch->bias = 0.0;
    }

    ch->function = convo_hv5;
    th->get_frame_filter = convolution_hv_get_frame;
}

const set_filter_data_func set_convolution_hv = set_convolution_hv_data;
