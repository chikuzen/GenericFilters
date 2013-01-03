#include "edge.h"

static void VS_CC
proc_8bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *dstp, const uint8_t *srcp, edge_t *eh)
{
}

static void VS_CC
proc_16bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *dstp, const uint8_t *srcp, edge_t *eh)
{
}

const proc_edge_detection prewitt[] = {
    proc_8bit_sse2,
    proc_16bit_sse2
};
