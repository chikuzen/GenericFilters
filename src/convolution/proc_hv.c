#define PROC_CONVOLUTION
#include "convolution.h"


#define SET_CONVOLUTION_HV3(l, c, r) \
    tmp = ((r0[( c )] * m3 + r1[( c )] * m4 + r2[( c )] * m5) / div_v);\
    dstp[( c )] = clamp((r1[( l )] * m0 + tmp * m1 + r1[( r )] * m2) / div_h + bias, max);

static void VS_CC
proc_3_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 =   ch->m[0], m1 =   ch->m[1], m2 =   ch->m[2],
        m3 = ch->m_v[0], m4 = ch->m_v[1], m5 = ch->m_v[2];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane);
    double div_h = ch->div;
    double div_v = ch->div_v;
    double bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r1 = vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint8_t *r0 = r1 - !!y * stride;
        const uint8_t *r2 = r1 + !!(h - y) * stride;
        double tmp;
        SET_CONVOLUTION_HV3(0, 0, 1);
        for (int x = 1; x < w; x++) {
            SET_CONVOLUTION_HV3(x - 1, x, x + 1);
        }
        
        SET_CONVOLUTION_HV3(w - 1, w, w);
        dstp += stride;
        r1 += stride;
    }
}


static void VS_CC
proc_3_16bit(convolution_t *ch, int plane, const VSFrameRef *src,
             VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 =   ch->m[0], m1 =   ch->m[1], m2 =   ch->m[2],
        m3 = ch->m_v[0], m4 = ch->m_v[1], m5 = ch->m_v[2];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div_h = ch->div;
    double div_v = ch->div_v;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r1 = (uint16_t *)vsapi->getReadPtr(src, plane);

    for (int y = 0; y <= h; y++) {
        const uint16_t *r0 = r1 - !!y * stride;
        const uint16_t *r2 = r1 + !!(h - y) * stride;
        double tmp;
        SET_CONVOLUTION_HV3(0, 0, 1);
        for (int x = 1; x < w; x++) {
            SET_CONVOLUTION_HV3(x - 1, x, x + 1);
        }
        
        SET_CONVOLUTION_HV3(w - 1, w, w);
        dstp += stride;
        r1 += stride;
    }
}
#undef SET_CONVOLUTION_HV3


#define SET_CONVOLUTION_HV5(i0, i1, i2, i3, i4) \
    tmp = ((r0[( i2 )] * m5 + r1[( i2 )] * m6 + r2[( i2 )] * m7 + r3[( i2 )] * m8 + r4[( i2 )] * m9) / div_v);\
    dstp[( i2 )] = clamp((r2[( i0 )] * m0 + r2[( i1 )] * m1 + tmp * m2 + r2[( i3 )] * m3 + r2[( i4 )] * m4) / div_h + bias, max);

static void VS_CC
proc_5_8bit(convolution_t *ch, int plane, const VSFrameRef *src,
            VSFrameRef *dst, const VSAPI *vsapi, uint16_t max)
{
    int m0 =   ch->m[0], m1 =   ch->m[1], m2 =   ch->m[2], m3 =   ch->m[3], m4 =   ch->m[4],
        m5 = ch->m_v[0], m6 = ch->m_v[1], m7 = ch->m_v[2], m8 = ch->m_v[3], m9 = ch->m_v[4];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane);
    double div_h = ch->div;
    double div_v = ch->div_v;
    double bias = ch->bias;

    uint8_t *dstp = vsapi->getWritePtr(dst, plane);
    const uint8_t *r2 = vsapi->getReadPtr(src, plane);
    const uint8_t *r1 = r2;
    const uint8_t *r0 = r1;
    const uint8_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint8_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        double tmp;
        SET_CONVOLUTION_HV5(0, 0, 0, 1, 2);
        SET_CONVOLUTION_HV5(0, 0, 1, 2, 3);
        for (int x = 2; x < w_; x++) {
            SET_CONVOLUTION_HV5(x - 2, x - 1, x, x + 1, x + 2);
        }
        SET_CONVOLUTION_HV5(w_ - 2, w_ - 1, w_, w, w);
        SET_CONVOLUTION_HV5(w_ - 1, w_ , w, w, w);
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
    int m0 =   ch->m[0], m1 =   ch->m[1], m2 =   ch->m[2], m3 =   ch->m[3], m4 =   ch->m[4],
        m5 = ch->m_v[0], m6 = ch->m_v[1], m7 = ch->m_v[2], m8 = ch->m_v[3], m9 = ch->m_v[4];

    int w = vsapi->getFrameWidth(src, plane) - 1;
    int h = vsapi->getFrameHeight(src, plane) - 1;
    if (w < 3 || h < 3) {
        return;
    }
    int w_ = w - 1;
    int stride = vsapi->getStride(src, plane) / 2;
    double div_h = ch->div;
    double div_v = ch->div_v;
    double bias = ch->bias;

    uint16_t *dstp = (uint16_t *)vsapi->getWritePtr(dst, plane);
    const uint16_t *r2 = (uint16_t *)vsapi->getReadPtr(src, plane);
    const uint16_t *r1 = r2;
    const uint16_t *r0 = r1;
    const uint16_t *r3 = r2 + stride;

    for (int y = 0; y <= h; y++) {
        const uint16_t *r4 = r3 + (!!(h - y - 1) - !(h - y)) * stride;
        double tmp;
        SET_CONVOLUTION_HV5(0, 0, 0, 1, 2);
        SET_CONVOLUTION_HV5(0, 0, 1, 2, 3);
        for (int x = 2; x < w_; x++) {
            SET_CONVOLUTION_HV5(x - 2, x - 1, x, x + 1, x + 2);
        }
        SET_CONVOLUTION_HV5(w_ - 2, w_ - 1, w_, w, w);
        SET_CONVOLUTION_HV5(w_ - 1, w_ , w, w, w);
        dstp += stride;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
    }
}
#undef SET_CONVOLUTION_HV5


const proc_convolution convo_hv3[] = {
    proc_3_8bit,
    proc_3_16bit
};
const proc_convolution convo_hv5[] = {
    proc_5_8bit,
    proc_5_16bit
};
