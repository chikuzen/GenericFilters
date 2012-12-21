#ifndef ALONE_COMMON_HEADER
#define ALONE_COMMON_HEADER

#include <stdint.h>
#include "common.h"

typedef struct filter_data alone_t;

typedef const char * (VS_CC *set_alone_handler_func)(tweak_handler_t *);

struct filter_data {
    size_t lut_size;
    uint16_t *lut;
};


extern const set_alone_handler_func set_alone;


#endif
