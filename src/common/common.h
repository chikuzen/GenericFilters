#ifndef TWEAK_COMMON_H
#define TWEAK_COMMON_H

#include <stdio.h>
#include <stdarg.h>
#include "VapourSynth.h"

#ifdef _MSC_VER
#pragma warning(disable:4996 4244)
#define snprintf _snprintf
#endif

#define TWEAK_PLUGIN_VERSION "0.1.0"


typedef struct tweak_handler tweak_handler_t;

struct tweak_handler {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    int planes[3];
    struct filter_data *fdata;
    void (VS_CC *free_data)(void *);
    void (VS_CC *get_frame_filter)(struct filter_data *, const VSFormat *,
                                   const VSFrameRef **, const VSAPI *,
                                   const VSFrameRef *, VSFrameRef *);
};

typedef enum {
    ID_NONE,
    ID_CONVO,
    ID_CONVO_HV,
    ID_MAXIMUM,
    ID_MEDIAN,
    ID_MINIMUM,
    ID_INVERT,
    ID_LIMITER,
    ID_LEVELS,
    ID_INFLATE,
    ID_DEFLATE,
    ID_BINARIZE
} filter_id_t;


typedef void (VS_CC *set_filter_data_func)(tweak_handler_t *th,
                                            filter_id_t id, char *msg,
                                            const VSMap *in, VSMap *out,
                                            const VSAPI *vsapi);

extern const set_filter_data_func set_convolution;
extern const set_filter_data_func set_convolution_hv;
extern const set_filter_data_func set_neighbors;
extern const set_filter_data_func set_invert;
extern const set_filter_data_func set_limitter;
extern const set_filter_data_func set_levels;
extern const set_filter_data_func set_xxflate;
extern const set_filter_data_func set_binarize;


#ifdef USE_ALIGNED_MALLOC
#ifdef _WIN32
#include <malloc.h>
#else
static inline void *_aligned_malloc(size_t size, size_t alignment)
{
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : 0;
}

static inline void _aligned_free(void *p)
{
    free(p);
}
#endif // _WIN32
#endif // USE_ALIGNED_MALLOC

#define RET_IF_ERROR(cond, ...) { \
    if (cond) { \
        snprintf(msg, 240, __VA_ARGS__); \
        return; \
    } \
}

#endif // TWEAK_COMMON_H
