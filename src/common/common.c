#include <stdlib.h>
#include "common.h"


static const VSFrameRef * VS_CC
get_frame_common(int n, int activation_reason, void **instance_data,
                 void **frame_data, VSFrameContext *frame_ctx, VSCore *core,
                 const VSAPI *vsapi)
{
    neighbors_handler_t *nh = (neighbors_handler_t *)*instance_data;

    if (activation_reason == arInitial) {
        vsapi->requestFrameFilter(n, nh->node, frame_ctx);
        return NULL;
    }

    if (activation_reason != arAllFramesReady) {
        return NULL;
    }

    const VSFrameRef *src = vsapi->getFrameFilter(n, nh->node, frame_ctx);
    const VSFormat *fi = vsapi->getFrameFormat(src);
    if (fi->sampleType != stInteger) {
        return src;
    }
    const int pl[] = {0, 1, 2};
    const VSFrameRef *fr[] = {nh->planes[0] ? NULL : src,
                              nh->planes[1] ? NULL : src,
                              nh->planes[2] ? NULL : src};
    VSFrameRef *dst = vsapi->newVideoFrame2(fi, vsapi->getFrameWidth(src, 0),
                                            vsapi->getFrameHeight(src, 0),
                                            fr, pl, src, core);

    nh->get_frame_filter(nh->fdata, fi, fr, vsapi, src, dst);

    vsapi->freeFrame(src);

    return dst;
}


static void VS_CC
init_handler(VSMap *in, VSMap *out, void **instance_data, VSNode *node,
             VSCore *core, const VSAPI *vsapi)
{
    neighbors_handler_t *nh = (neighbors_handler_t *)*instance_data;
    vsapi->setVideoInfo(nh->vi, 1, node);
    vsapi->clearMap(in);
}


static void VS_CC
close_handler(void *instance_data, VSCore *core, const VSAPI *vsapi)
{
    neighbors_handler_t *nh = (neighbors_handler_t *)instance_data;
    if (!nh) {
        return;
    }
    if (nh->node) {
        vsapi->freeNode(nh->node);
        nh->node = NULL;
    }
    if (nh->fdata) {
        free(nh->fdata);
        nh->fdata = NULL;
    }
    free(nh);
    nh = NULL;
}


static int VS_CC
set_proc_planes(neighbors_handler_t *nh, const VSMap *in, const VSAPI *vsapi)
{
    int num = vsapi->propNumElements(in, "planes");
    if (num < 1) {
        for (int i = 0; i < 3; nh->planes[i++] = 1);
    } else {
        for (int i = 0; i < num; i++) {
            int p = (int)vsapi->propGetInt(in, "planes", i, NULL);
            if (p < 0 || p > 2) {
                return -1;
            }
            nh->planes[p] = 1;
        }
    }

    return 0;
}


const VSFilterInit init_filter = init_handler;
const VSFilterGetFrame get_frame = get_frame_common;
const VSFilterFree free_filter = close_handler;
const set_planes_t set_planes = set_proc_planes;
