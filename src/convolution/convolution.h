/*
  convolution.h: Copyright (C) 2012  Oka Motofumi

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


#ifndef CONVOLUTION_HEADER
#define CONVOLUTION_HEADER

#include <stdint.h>
#include "VapourSynth.h"

typedef struct convolution_handle convolution_t;

typedef void (VS_CC *proc_convolution)(convolution_t *ch, int plane,
                                        const VSFrameRef *src, VSFrameRef *dst,
                                        const VSAPI *vsapi, uint16_t max);

struct convolution_handle {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    int planes[3];
    const proc_convolution *proc_function;
    int m[25];
    double div;
    double bias;
};


extern const proc_convolution convo_h3[];
extern const proc_convolution convo_h5[];
extern const proc_convolution convo_v3[];
extern const proc_convolution convo_v5[];
extern const proc_convolution convo_3x3[];
extern const proc_convolution convo_5x5[];
extern const proc_convolution convo_hv3[];
extern const proc_convolution convo_hv5[];


#ifdef PROC_CONVOLUTION
static inline uint16_t VS_CC clamp(double val, uint16_t max)
{
    if (val < 0) {
        return 0;
    }
    if (val > max) {
        return max;
    }
    return (uint16_t)val;
}
#endif // PROC_CONVOLUTION

#endif // CONVOLUTION_HEADER
