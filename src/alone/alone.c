#include <stdlib.h>
#include "alone.h"

typedef void (VS_CC *proc_alone)(int , uint16_t *, const uint8_t *, uint8_t *);


static void VS_CC
proc_8bit(int plane_size, uint16_t *lut, const uint8_t *srcp, uint8_t *dstp)
{
    while (plane_size--) {
        *dstp++ = lut[*srcp++];
    }
}


static void VS_CC
proc_16bit(int plane_size, uint16_t *lut, const uint8_t *s, uint8_t *d)
{
    plane_size >>= 1;
    uint16_t *dstp = (uint16_t *)d;
    const uint16_t *srcp = (uint16_t *)s;

    while (plane_size--) {
        *dstp++ = lut[*srcp++];
    }
}


static void VS_CC
alone_get_frame(alone_t *ah, const VSFormat *fi, const VSFrameRef **fr,
                const VSAPI *vsapi, const VSFrameRef *src, VSFrameRef *dst)
{
    proc_alone function[] = {proc_8bit, proc_16bit};
    int index = fi->bytesPerSample - 1;
    for (int plane = 0; plane < fi->numPlanes; plane++) {
        if (fr[plane]) {
            continue;
        }

        function[index](vsapi->getFrameHeight(src, plane) *
                        vsapi->getStride(src, plane),
                        ah->lut,
                        vsapi->getReadPtr(src, plane),
                        vsapi->getWritePtr(dst, plane));
    }
}


static void VS_CC alone_free_data(void *data)
{
    alone_t *ah = (alone_t *)data;
    if (ah->lut) {
        free(ah->lut);
        ah->lut = NULL;
    }
}


static const char * VS_CC set_alone_handler(tweak_handler_t *th)
{
    alone_t *ah = (alone_t *)calloc(sizeof(alone_t), 1);
    if (!ah) {
        return "failed to allocate handler";
    }

    th->fdata = ah;
    th->free_data = alone_free_data;
    th->get_frame_filter = alone_get_frame;
    ah->lut_size = 1 << (8 * th->vi->format->bytesPerSample);
    ah->lut = (uint16_t *)malloc(sizeof(uint16_t) * ah->lut_size);
    if (!ah->lut) {
        return "out of memory";
    }

    return NULL;
}


const set_alone_handler_func set_alone = set_alone_handler;
