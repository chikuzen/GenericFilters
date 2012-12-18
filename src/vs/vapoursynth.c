/*
  vapoursynth.c: Copyright (C) 2012  Oka Motofumi

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


#include "VapourSynth.h"

#define NEIGHBORS_VERSION "0.1.0"

extern const VSPublicFunction public_convolution;
extern const VSPublicFunction public_neighbors;


VS_EXTERNAL_API(void)
VapourSynthPluginInit(VSConfigPlugin conf, VSRegisterFunction reg,
                      VSPlugin *plugin)
{
    conf("chikuzen.does.not.have.his.own.domain.neighbors", "neighbors",
         "Pixel value modifier with reference to the neighbor pixels v"
         NEIGHBORS_VERSION, VAPOURSYNTH_API_VERSION, 1, plugin);
    reg("Convolution",
        "clip:clip;matrix:int[]:opt;bias:float:opt;divisor:float:opt;"
        "planes:int[]:opt;mode:data:opt;",
        public_convolution, (void *)"Convolution", plugin);
    reg("ConvolutionHV",
        "clip:clip;horizontal:int[]:opt;vertical:int[]:opt;bias:float:opt;"
        "divisor_h:float:opt;divisor_v:float:opt;planes:int[]:opt;",
        public_convolution, (void *)"ConvolutionHV", plugin);
    reg("Minimum", "clip:clip;planes:int[]:opt;",
        public_neighbors, (void *)"Minimum", plugin);
    reg("Maximum", "clip:clip;planes:int[]:opt;",
        public_neighbors, (void *)"Maximum", plugin);
    reg("Median", "clip:clip;planes:int[]:opt;",
        public_neighbors, (void *)"Median", plugin);
}
