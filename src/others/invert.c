#include <stdint.h>
#include "others.h"

static void VS_CC
proc_8bit_or_16bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                   VSFrameRef *dst, int bits_per_sample)
{
    int plane_size = (vsapi->getStride(src, plane) / sizeof(unsigned)) * 
                     vsapi->getFrameHeight(src, plane);
    const unsigned *srcp = (unsigned *)vsapi->getReadPtr(src, plane);
    unsigned *dstp = (unsigned *)vsapi->getWritePtr(dst, plane);

    while (plane_size--) {
        *dstp++ = ~(*srcp++);
    }
}


static void VS_CC
proc_9bit_or_10bit(int plane, const VSFrameRef *src, const VSAPI *vsapi,
                   VSFrameRef *dst, int bits_per_sample)
{
    int plane_size = (vsapi->getStride(src, plane) / sizeof(unsigned)) * 
                     vsapi->getFrameHeight(src, plane);
    const unsigned *srcp = (unsigned *)vsapi->getReadPtr(src, plane);
    unsigned *dstp = (unsigned *)vsapi->getWritePtr(dst, plane);

    unsigned mask = (1 << bits_per_sample) - 1;
    for (size_t i = 1, usize = sizeof(unsigned) / sizeof(uint16_t); i < usize; i++) {
        mask = (mask << 16) | mask;
    }

    while (plane_size--) {
        *dstp++ = (*srcp++ ^ mask);
    }
}


const proc_others invert[] = {
    proc_8bit_or_16bit,
    proc_9bit_or_10bit
};
