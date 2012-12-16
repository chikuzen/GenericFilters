/*
  median.c: Copyright (C) 2012  Oka Motofumi

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


#include <stdint.h>
#include "neighbors.h"

static void VS_CC swap_u8(uint8_t *x, uint8_t *y)
{
    uint8_t tmp = *x;
    *x = *y;
    *y = tmp;
}


static void VS_CC swap_u16(uint16_t *x, uint16_t *y)
{
    uint16_t tmp = *x;
    *x = *y;
    *y = tmp;
}


static void VS_CC swap_array_u8(uint8_t **x, uint8_t **y)
{
    uint8_t *tmp = *x;
    *x = *y;
    *y = tmp;
}


static void VS_CC swap_array_u16(uint16_t **x, uint16_t **y)
{
    uint16_t *tmp = *x;
    *x = *y;
    *y = tmp;
}


static void VS_CC sort_array_u8(uint8_t *array)
{
    if (array[0] > array[1]) {
        swap_u8(array, array + 1);
    }
    if (array[1] > array[2]) {
        swap_u8(array + 1, array + 2);
    }
    if (array[0] > array[1]) {
        swap_u8(array, array + 1);
    }
}


static void VS_CC sort_array_u16(uint16_t *array)
{
    if (array[0] > array[1]) {
        swap_u16(array, array + 1);
    }
    if (array[1] > array[2]) {
        swap_u16(array + 1, array + 2);
    }
    if (array[0] > array[1]) {
        swap_u16(array, array + 1);
    }
}


static uint8_t VS_CC get_median_3_u8(uint8_t a, uint8_t b, uint8_t c)
{
    if (a <= b) {
        if (a >= c) {
            return a;
        }
        return b >= c ? c : b;
    }
    if (b >= c) {
        return b;
    }
    return a >= c ? c : a;
}


static uint16_t VS_CC get_median_3_u16(uint16_t a, uint16_t b, uint16_t c)
{
    if (a <= b) {
        if (a >= c) {
            return a;
        }
        return b >= c ? c : b;
    }
    if (b >= c) {
        return b;
    }
    return a >= c ? c : a;
}


static uint8_t VS_CC
get_median_3x3_u8(uint8_t *array0, uint8_t *array1, uint8_t *array2)
{
    sort_array_u8(array0);
    sort_array_u8(array1);
    sort_array_u8(array2);
    if (array0[1] > array1[1]) {
        swap_array_u8(&array0, &array1);
    }
    if (array1[1] > array2[1]) {
        swap_array_u8(&array1, &array2);
    }
    if (array0[1] > array1[1]) {
        swap_array_u8(&array0, &array1);
    }
    return get_median_3_u8(array0[2], array1[1], array2[0]);
}


static uint16_t VS_CC
get_median_3x3_u16(uint16_t *array0, uint16_t *array1, uint16_t *array2)
{
    sort_array_u16(array0);
    sort_array_u16(array1);
    sort_array_u16(array2);
    if (array0[1] > array1[1]) {
        swap_array_u16(&array0, &array1);
    }
    if (array1[1] > array2[1]) {
        swap_array_u16(&array1, &array2);
    }
    if (array0[1] > array1[1]) {
        swap_array_u16(&array0, &array1);
    }
    return get_median_3_u16(array0[2], array1[1], array2[0]);
}

static void VS_CC
proc_median_3x3_u8(int left, int center, int right, const uint8_t *top,
                   const uint8_t *mdl, const uint8_t *btm, uint8_t *dstp)
{
    uint8_t a0[] = {top[left], top[center], top[right]};
    uint8_t a1[] = {mdl[left], mdl[center], mdl[right]};
    uint8_t a2[] = {btm[left], btm[center], btm[right]};
    dstp[center] = get_median_3x3_u8(a0, a1, a2);
}


static void VS_CC
proc_median_3x3_u16(int left, int center, int right, const uint16_t *top,
                    const uint16_t *mdl, const uint16_t *btm, uint16_t *dstp)
{
    uint16_t a0[] = {top[left], top[center], top[right]};
    uint16_t a1[] = {mdl[left], mdl[center], mdl[right]};
    uint16_t a2[] = {btm[left], btm[center], btm[right]};
    dstp[center] = get_median_3x3_u16(a0, a1, a2);
}


static void VS_CC
proc_median_line_u8(const uint8_t *r0, const uint8_t *r1, const uint8_t *r2,
                      uint8_t *dstp, int w)
{
    proc_median_3x3_u8(0, 0, 1, r0, r1, r2, dstp);
    for (int x = 0; x < w; x++) {
        proc_median_3x3_u8(x - 1, x, x + 1, r0, r1, r2, dstp);
    }
    proc_median_3x3_u8(w - 1, w, w, r0, r1, r2, dstp);
}


static void VS_CC
proc_median_line_u16(const uint16_t *r0, const uint16_t *r1, const uint16_t *r2,
                       uint16_t *dstp, int w)
{
    proc_median_3x3_u16(0, 0, 1, r0, r1, r2, dstp);
    for (int x = 0; x < w; x++) {
        proc_median_3x3_u16(x - 1, x, x + 1, r0, r1, r2, dstp);
    }
    proc_median_3x3_u16(w - 1, w, w, r0, r1, r2, dstp);
}


static void VS_CC
proc_median_8bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                 VSFrameRef *dst)
{
    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    const uint8_t *r0 = vsapi->getReadPtr(src, plane);
    const uint8_t *r1 = r0;
    const uint8_t *r2 = r1 + stride;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    
    proc_median_line_u8(r0, r1, r2, dstp, w);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        proc_median_line_u8(r0, r1, r2, dstp, w);
        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }
    
    proc_median_line_u8(r0, r1, r1, dstp, w);
}


static void VS_CC
proc_median_16bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                  VSFrameRef *dst)
{
    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    const uint16_t *r0 = (uint16_t *)vsapi->getReadPtr(src, plane);
    const uint16_t *r1 = r0;
    const uint16_t *r2 = r1 + stride;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    
    proc_median_line_u16(r0, r1, r2, dstp, w);
    r1 += stride;
    r2 += stride;
    dstp += stride;

    for (int y = 1; y < h; y++) {
        proc_median_line_u16(r0, r1, r2, dstp, w);
        r0 += stride;
        r1 += stride;
        r2 += stride;
        dstp += stride;
    }
    
    proc_median_line_u16(r0, r1, r1, dstp, w);
}

const proc_neighbors median[] = {
    proc_median_8bit,
    proc_median_16bit
};
