#define PROC_CONVOLUTION
#include "convolution.h"


static void VS_CC
proc_3_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2],
        m3 = ch->m[3], m4 = ch->m[4], m5 = ch->m[5];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    double div = ch->div;
    double bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r1 = vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - !!y * stride;
        const uint8_t *r2 = r1 + !!(h - y) * stride;
        int64_t value = r1[0] * m0 + r1[0] * m1 + r1[1] * m2 +
                        r0[0] * m3 + r1[0] * m4 + r2[0] * m5;
        dstp[0] = clamp(value / div + bias, 0xFF);
        for (int x = 1; x < w; x++) {
            value = r1[x - 1] * m0 + r1[x] * m1 + r1[x + 1] * m2 +
                    r0[x] * m3 + r1[x] * m4 + r2[x] * m5;
            dstp[x] = clamp(value / div + bias, 0xFF);
        }
        value = r1[w - 1] * m0 + r1[w] * m1 + r1[w] * m2 +
                r0[w] * m3 + r1[w] * m4 + r2[w] * m5;
        dstp[w] = clamp(value / div + bias, 0xFF);
        dstp += stride;
        r1 += stride;
    }
}


static void VS_CC
proc_3_16bit(convolution_t *ch, int plane, const VSFrameRef *src,
             VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2],
        m3 = ch->m[3], m4 = ch->m[4], m5 = ch->m[5];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div = ch->div;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r1 = (uint16_t *)vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - !!y * stride;
        const uint16_t *r2 = r1 + !!(h - y) * stride;
        int64_t value = r1[0] * m0 + r1[0] * m1 + r1[1] * m2 +
                        r0[0] * m3 + r1[0] * m4 + r2[0] * m5;
        dstp[0] = clamp(value / div + bias, max);
        for (int x = 1; x < w; x++) {
            value = r1[x - 1] * m0 + r1[x] * m1 + r1[x + 1] * m2 +
                    r0[x] * m3 + r1[x] * m4 + r2[x] * m5;
            dstp[x] = clamp(value / div + bias, max);
        }
        value = r1[w - 1] * m0 + r1[w] * m1 + r1[w] * m2 +
                r0[w] * m3 + r1[w] * m4 + r2[w] * m5;
        dstp[w] = clamp(value / div + bias, max);
        dstp += stride;
        r1 += stride;
    }
}


static void VS_CC
proc_5_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2], m3 = ch->m[3], m4 = ch->m[4],
        m5 = ch->m[5], m6 = ch->m[6], m7 = ch->m[7], m8 = ch->m[8], m9 = ch->m[9];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane);
    double div = ch->div;
    double bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r2 = vsapi->getReadPtr(src, plane);
    const uint8_t *r1 = r2;
    const uint8_t *r0 = r1;
    const uint8_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint8_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        int64_t value = r2[0] * m0 + r2[0] * m1 + r2[0] * m2 + r2[1] * m3 + r2[2] * m4 +
                        r0[0] * m5 + r1[0] * m6 + r2[0] * m7 + r3[0] * m8 + r4[0] * m5;
        dstp[0] = clamp(value / div + bias, 0xFF);
        value = r2[0] * m0 + r2[0] * m1 + r2[1] * m2 + r2[2] * m3 + r2[3] * m4 +
                r0[1] * m5 + r1[1] * m6 + r2[1] * m7 + r3[1] * m8 + r4[1] * m5;
        dstp[1] = clamp(value / div + bias, 0XFF);
        for (int x = 2; x < w_; x++) {
            value = r2[x - 2] * m0 + r2[x - 1] * m1 + r2[x] * m2 + r2[x + 1] * m3 + r2[x + 2] * m4 +
                    r0[x] * m5 + r1[x] * m6 + r2[x] * m7 + r3[x] * m8 + r4[x] * m9;
            dstp[x] = clamp(value / div + bias, 0xFF);
        }
        value = r2[w_ - 2] * m0 + r2[w_ - 1] * m1 + r2[w_] * m2 + r2[w] * m3 + r2[w] * m4 +
                r0[w_] * m5 + r1[w_] * m6 + r2[w_] * m7 + r3[w_] * m8 + r4[w_] * m9;
        dstp[w_] = clamp(value / div + bias, 0xFF);
        value = r2[w_ - 1] * m0 + r2[w_] * m1 + r2[w] * m2 + r2[w] * m3 + r2[w] * m4 +
                r0[w] * m5 + r1[w] * m6 + r2[w] * m7 + r3[w] * m8 + r4[w] * m9;
        dstp[w] = clamp(value / div + bias, 0xFF);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


static void VS_CC
proc_5_16bit(convolution_t *ch, int plane, const VSFrameRef *src,
             VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 = ch->m[0], m1 = ch->m[1], m2 = ch->m[2], m3 = ch->m[3], m4 = ch->m[4],
        m5 = ch->m[5], m6 = ch->m[6], m7 = ch->m[7], m8 = ch->m[8], m9 = ch->m[9];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div = ch->div;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r2 = vsapi->getReadPtr(src, plane);
    const uint16_t *r1 = r2;
    const uint16_t *r0 = r1;
    const uint16_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        int64_t value = r2[0] * m0 + r2[0] * m1 + r2[0] * m2 + r2[1] * m3 + r2[2] * m4 +
                        r0[0] * m5 + r1[0] * m6 + r2[0] * m7 + r3[0] * m8 + r4[0] * m5;
        dstp[0] = clamp(value / div + bias, max);
        value = r2[0] * m0 + r2[0] * m1 + r2[1] * m2 + r2[2] * m3 + r2[3] * m4 +
                r0[1] * m5 + r1[1] * m6 + r2[1] * m7 + r3[1] * m8 + r4[1] * m5;
        dstp[1] = clamp(value / div + bias, max);
        for (int x = 2; x < w_; x++) {
            value = r2[x - 2] * m0 + r2[x - 1] * m1 + r2[x] * m2 + r2[x + 1] * m3 + r2[x + 2] * m4 +
                    r0[x] * m5 + r1[x] * m6 + r2[x] * m7 + r3[x] * m8 + r4[x] * m9;
            dstp[x] = clamp(value / div + bias, max);
        }
        value = r2[w_ - 2] * m0 + r2[w_ - 1] * m1 + r2[w_] * m2 + r2[w] * m3 + r2[w] * m4 +
                r0[w_] * m5 + r1[w_] * m6 + r2[w_] * m7 + r3[w_] * m8 + r4[w_] * m9;
        dstp[w_] = clamp(value / div + bias, max);
        value = r2[w_ - 1] * m0 + r2[w_] * m1 + r2[w] * m2 + r2[w] * m3 + r2[w] * m4 +
                r0[w] * m5 + r1[w] * m6 + r2[w] * m7 + r3[w] * m8 + r4[w] * m9;
        dstp[w] = clamp(value / div + bias, max);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}


const proc_convolution convo_hv3[] = {
    proc_3_8bit,
    proc_3_16bit
};
const proc_convolution convo_hv5[] = {
    proc_5_8bit,
    proc_5_16bit
};
