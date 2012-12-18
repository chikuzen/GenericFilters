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


#ifndef CONVOLUTION_FILTER_HEADER
#define CONVOLUTION_FILTER_HEADER

#include <stdint.h>
#include "common.h"

typedef void (VS_CC *proc_convolution)(struct filter_data *, int,
                                        const VSFrameRef *, VSFrameRef *,
                                        const VSAPI *, uint16_t);

typedef struct filter_data {
    const proc_convolution *proc_function;
    int m[25];
    int m_v[5];
    double div;
    double div_v;
    double bias;
} convolution_t;


extern const proc_convolution convo_h3[];
extern const proc_convolution convo_h5[];
extern const proc_convolution convo_v3[];
extern const proc_convolution convo_v5[];
extern const proc_convolution convo_3x3[];
extern const proc_convolution convo_5x5[];
extern const proc_convolution convo_hv5[];


#ifdef PROC_CONVOLUTION
static inline uint16_t VS_CC clamp_f(float val, uint16_t max)
{
    if (val < 1.0) {
        return 0;
    }
    if (val > max) {
        return max;
    }
    return (uint16_t)val;
}


static inline uint16_t VS_CC clamp_d(double val, uint16_t max)
{
    if (val < 0.0) {
        return 0;
    }
    if (val > max) {
        return max;
    }
    return (uint16_t)val;
}
#endif // PROC_CONVOLUTION

#endif // CONVOLUTION_FILTER_HEADER
