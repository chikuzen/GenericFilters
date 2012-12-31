#ifndef CONVOLUTION_HV_FILTER_HEADER
#define CONVOLUTION_HV_FILTER_HEADER


#include <stdint.h>
#include "VapourSynth.h"

typedef struct filter_data convolution_hv_t;

typedef void (VS_CC *proc_convo_hv)(convolution_hv_t *, uint8_t *, int, int,
                                     int, int, uint8_t *, const uint8_t *);

struct filter_data {
    int m_h[5];
    int m_v[5];
    double rdiv_h;
    double rdiv_v;
    double bias;
    const proc_convo_hv *function;
};


extern const proc_convo_hv convo_hv5[];

#endif
