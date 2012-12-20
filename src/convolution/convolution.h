/*
  convolution.h: Copyright (C) 2012  Oka Motofumi

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


#ifndef CONVOLUTION_FILTER_HEADER
#define CONVOLUTION_FILTER_HEADER

#include <stdint.h>
#include "VapourSynth.h"

#ifdef _MSV_VER
#pragma warning(disable: 4244)
#endif


typedef struct filter_data convolution_t;

typedef void (VS_CC *proc_convolution)(convolution_t *, int, int, int,
                                        uint8_t *, const uint8_t *, uint16_t);

struct filter_data {
    int m[25];
    int m_v[5];
    double rdiv;
    double rdiv_v;
    double bias;
    const proc_convolution *proc_function;
};


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
