#ifndef NEIGHBORS_COMMON_H
#define NEIGHBORS_COMMON_H

#include <stdio.h>
#include <stdarg.h>
#include "VapourSynth.h"

#ifdef _MSC_VER
#pragma warning(disable:4996)
#define snprintf _snprintf
#endif


typedef struct neighbors_handler neighbors_handler_t;

struct neighbors_handler {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    int planes[3];
    struct filter_data *fdata;
    void (VS_CC *get_frame_filter)(struct filter_data *, const VSFormat *,
                                   const VSFrameRef **, const VSAPI *,
                                   const VSFrameRef *, VSFrameRef *);
};


typedef int (VS_CC *set_planes_t)(neighbors_handler_t *nh, const VSMap *in,
                                   const VSAPI *vsapi);


extern const VSFilterInit init_filter;
extern const VSFilterGetFrame get_frame;
extern const VSFilterFree free_filter;
extern const set_planes_t set_planes;


#define RET_IF_ERROR(cond, ...) { \
    if (cond) { \
        free_filter(nh, core, vsapi); \
        snprintf(msg, 240, __VA_ARGS__); \
        vsapi->setError(out, msg_buff); \
        return; \
    } \
}

#endif
