/*
  proc_h.c: Copyright (C) 2012  Oka Motofumi

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


#define PROC_CONVOLUTION
#include "convolution.h"


static void VS_CC
proc_3_8bit(convolution_t *ch, int w, int h, int stride, uint8_t *dstp,
            const uint8_t *r, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2];

    float div = (float)ch->div;
    float bias = (float)ch->bias;

    w--;
    int w_ = w - 1;

    for (int y = 0; y < h; y++) {
        dstp[0] = clamp_f((r[0] * m0 + r[0] * m1 + r[1] * m2) / div + bias, max);
        for (int x = 0; x < w; x++) {
            dstp[x] = clamp_f((r[x - 1] * m0 + r[x] * m1 + r[x + 1] * m2) / div + bias, max);
        }
        dstp[w] = clamp_f((r[w_] * m0 + r[w] * m1 + r[w] * m2) / div + bias, max);
        r += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_3_16bit(convolution_t *ch, int w, int h, int stride, uint8_t *d,
             const uint8_t *srcp, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2];

    double div = ch->div;
    double bias = ch->bias;

    w--; stride >>=1;
    int w_ = w - 1;
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *r = (uint16_t *)srcp;

    for (int y = 0; y < h; y++) {
        dstp[0] = clamp_d((r[0] * m0 + r[0] * m1 + r[1] * m2) / div + bias, max);
        for (int x = 0; x < w; x++) {
            dstp[x] = clamp_d((r[x - 1] * m0 + r[x] * m1 + r[x + 1] * m2) / div + bias, max);
        }
        dstp[w] = clamp_d((r[w_] * m0 + r[w] * m1 + r[w] * m2) / div + bias, max);
        r += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_5_8bit(convolution_t *ch, int w, int h, int stride, uint8_t *dstp,
            const uint8_t *r, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2], m3 = ch->m[3], m4 = ch->m[4];

    float div = (float)ch->div;
    float bias = (float)ch->bias;

    w--;
    int w_ = w - 1;

    for (int y = 0; y < h; y++) {
        int64_t value = r[0] * m0 + r[0] * m1 + r[0] * m2 + r[1] * m3 + r[2] * m4;
        dstp[0] = clamp_f(value / div + bias, max);

        value = r[0] * m0 + r[0] * m1 + r[1] * m2 + r[2] * m3 + r[3] * m4;
        dstp[1] = clamp_f(value / div + bias, max);

        for (int x = 2; x < w_; x++) {
            value = r[x - 2] * m0 + r[x - 1] * m1 + r[x] * m2 + r[x + 1] * m3 +
                    r[x + 2] * m4;
            dstp[x] = clamp_f(value / div + bias, max);
        }

        value = r[w_ - 2] * m0 + r[w_ - 1] * m1 + r[w_] * m2 + r[w] * m3 +
                r[w] * m4;
        dstp[w_] = clamp_f(value / div + bias, max);

        value = r[w_ - 1] * m0 + r[w_] * m1 + r[w] * m2 + r[w] * m3 + r[w] * m4;
        dstp[w] = clamp_f(value / div + bias, max);

        r += stride;
        dstp += stride;
    }
}


static void VS_CC
proc_5_16bit(convolution_t *ch, int w, int h, int stride, uint8_t *d,
             const uint8_t *srcp, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2], m3 = ch->m[3], m4 = ch->m[4];

    double div = ch->div;
    double bias = ch->bias;

    w--; stride >>= 1;
    int w_ = w - 1;
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *r = (uint16_t *)srcp;

    for (int y = 0; y < h; y++) {
        int64_t value = r[0] * m0 + r[0] * m1 + r[0] * m2 + r[1] * m3 + r[2] * m4;
        dstp[0] = clamp_d(value / div + bias, max);

        value = r[0] * m0 + r[0] * m1 + r[1] * m2 + r[2] * m3 + r[3] * m4;
        dstp[1] = clamp_d(value / div + bias, max);

        for (int x = 2; x < w_; x++) {
            value = r[x - 2] * m0 + r[x - 1] * m1 + r[x] * m2 + r[x + 1] * m3 +
                    r[x + 2] * m4;
            dstp[x] = clamp_d(value / div + bias, max);
        }

        value = r[w_ - 2] * m0 + r[w_ - 1] * m1 + r[w_] * m2 + r[w] * m3 +
                r[w] * m4;
        dstp[w_] = clamp_d(value / div + bias, max);

        value = r[w_ - 1] * m0 + r[w_] * m1 + r[w] * m2 + r[w] * m3 + r[w] * m4;
        dstp[w] = clamp_d(value / div + bias, max);

        r += stride;
        dstp += stride;
    }
}


const proc_convolution convo_h3[] = {
    proc_3_8bit,
    proc_3_16bit
};
const proc_convolution convo_h5[] = {
    proc_5_8bit,
    proc_5_16bit
};

