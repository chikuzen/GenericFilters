/*
  neighbors.h: Copyright (C) 2012  Oka Motofumi

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


#ifndef NEIGHBORS_FILETR_H
#define NEIGHBORS_FILETR_H

#include <stdint.h>
#include <string.h>
#include <emmintrin.h>
#include "VapourSynth.h"

#ifdef _MSC_VER
#define NBS_ALIGN __declspec(align(16))
#define NBS_FUNC_ALIGN
#else
#define NBS_ALIGN __attribute__((aligned(16)))
#define NBS_FUNC_ALIGN __attribute__((force_align_arg_pointer))
#endif

typedef void (VS_CC *proc_neighbors)(uint8_t *, int, int, int, int, uint8_t *,
                                      const uint8_t *);

typedef struct filter_data {
    const proc_neighbors *function;
} neighbors_t;


extern const proc_neighbors minimum[];
extern const proc_neighbors median[];
extern const proc_neighbors maximum[];


#ifdef PROC_NEIGHBORS
static void VS_CC
line_copy8(uint8_t *line, const uint8_t *srcp, int width)
{
    memcpy(line, srcp, width);
    line[- 1] = line[0];
    line[width] = line[width - 1]; 
}


static void VS_CC
line_copy16(uint16_t *line, const uint16_t *srcp, int width)
{
    memcpy(line, srcp, width * 2);
    line[-1] = line[0];
    line[width] = line[width - 1];
}
#endif // PROC_NEUGHBORS

#endif
