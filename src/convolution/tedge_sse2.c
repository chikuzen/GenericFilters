#include "edge.h"
#include "sse2.h"


static const GF_ALIGN int16_t fifteens[] = {15, 15, 15, 15, 15, 15, 15, 15};

// 12:74 -> (int)(12.0/3):(int)(74.0/3+0.5)
static const GF_ALIGN int16_t ar_mulx[][8] = {
    {  4,   4,   4,   4,   4,   4,   4,   4},
    {-25, -25, -25, -25, -25, -25, -25, -25},
    { 25,  25,  25,  25,  25,  25,  25,  25},
    { -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4}
};
static const GF_ALIGN int16_t ar_muly[][8] = {
    { -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4},
    { 25,  25,  25,  25,  25,  25,  25,  25},
    {-25, -25, -25, -25, -25, -25, -25, -25},
    {  4,   4,   4,   4,   4,   4,   4,   4}
};


static void GF_FUNC_ALIGN VS_CC
proc_8bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *dstp, const uint8_t *srcp, edge_t *eh)
{
    uint8_t* p0 = buff + 16;
    uint8_t* p1 = p0 + bstride;
    uint8_t* p2 = p1 + bstride;
    uint8_t* p3 = p2 + bstride;
    uint8_t* p4 = p3 + bstride;
    uint8_t* orig = p0;
    uint8_t* end = p4;

    line_copy8(p0, srcp, width, 2);
    line_copy8(p1, srcp, width, 2);
    line_copy8(p2, srcp, width, 2);
    srcp += stride;
    line_copy8(p3, srcp, width, 2);
    srcp += stride;

    uint8_t th_min = eh->min > 0xFF ? 0xFF : (uint8_t)eh->min;
    uint8_t th_max = eh->max > 0xFF ? 0xFF : (uint8_t)eh->max;
    GF_ALIGN uint8_t ar_min[16];
    GF_ALIGN uint8_t ar_max[16];
    memset(ar_min, th_min, 16);
    memset(ar_max, th_max, 16);

    for (int y = 0; y < height; y++) {
        line_copy8(p4, srcp, width, 2);
        uint8_t* posh[] = {p2 - 2, p2 - 1, p2 + 1, p2 + 2};
        uint8_t* posv[] = {p0, p1, p3, p4};

        for (int x = 0; x < width; x += 16) {

            __m128i zero = _mm_setzero_si128();
            __m128i sumx[2] = {zero, zero};
            __m128i sumy[2] = {zero, zero};

            for (int i = 0; i < 4; i++) {
                __m128i xmm0, xmm1, xmul;
                xmul = _mm_load_si128((__m128i *)ar_mulx[i]);
                xmm0 = _mm_loadu_si128((__m128i *)(posh[i] + x));
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sumx[0] = _mm_add_epi16(sumx[0], _mm_mullo_epi16(xmm0, xmul));
                sumx[1] = _mm_add_epi16(sumx[1], _mm_mullo_epi16(xmm1, xmul));

                xmul = _mm_load_si128((__m128i *)ar_muly[i]);
                xmm0 = _mm_load_si128((__m128i *)(posv[i] + x));
                xmm1 = _mm_unpackhi_epi8(xmm0, zero);
                xmm0 = _mm_unpacklo_epi8(xmm0, zero);
                sumy[0] = _mm_add_epi16(sumy[0], _mm_mullo_epi16(xmm0, xmul));
                sumy[1] = _mm_add_epi16(sumy[1], _mm_mullo_epi16(xmm1, xmul));
            }

            __m128i ab = _mm_load_si128((__m128i*)fifteens);
            __m128i max, min;
            for (int i = 0; i < 2; i++) {
                __m128i mull, mulh;
                sumx[i] = mm_abs_epi16(sumx[i]);
                sumy[i] = mm_abs_epi16(sumy[i]);
                max = _mm_max_epi16(sumx[i], sumy[i]);
                min = _mm_min_epi16(sumx[i], sumy[i]);

                mull = _mm_srli_epi32(_mm_madd_epi16(ab, _mm_unpacklo_epi16(max, zero)), 4);
                mulh = _mm_srli_epi32(_mm_madd_epi16(ab, _mm_unpackhi_epi16(max, zero)), 4);
                max = mm_cast_epi32(mull, mulh);

                mull = _mm_srli_epi32(_mm_madd_epi16(ab, _mm_unpacklo_epi16(min, zero)), 5);
                mulh = _mm_srli_epi32(_mm_madd_epi16(ab, _mm_unpackhi_epi16(min, zero)), 5);
                min = mm_cast_epi32(mull, mulh);

                sumx[i] = _mm_adds_epu16(max, min);
                sumx[i] = _mm_srli_epi16(sumx[i], eh->rshift);
            }

            __m128i out = _mm_packus_epi16(sumx[0], sumx[1]);
            max = _mm_load_si128((__m128i *)ar_max);
            __m128i temp = _mm_min_epu8(out, max);
            temp = _mm_cmpeq_epi8(temp, max);
            out = _mm_or_si128(temp, out);

            min = _mm_load_si128((__m128i *)ar_min);
            temp = _mm_max_epu8(out, min);
            temp = _mm_cmpeq_epi8(temp, min);
            out = _mm_andnot_si128(temp, out);

            _mm_store_si128((__m128i*)(dstp + x), out);
        }
        srcp += stride * (y < height - 3);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = p3;
        p3 = p4;
        p4 = (p4 == end) ? orig : p4 + bstride;
    }
}


#define ALPHA ((float)0.96043387)
#define BETA ((float)0.39782473)
static const GF_ALIGN float ar_alpha[4] = {ALPHA, ALPHA, ALPHA, ALPHA};
static const GF_ALIGN float ar_beta[4] = {BETA, BETA, BETA, BETA};
#undef ALPHA
#undef BETA

static void GF_FUNC_ALIGN VS_CC
proc_9_10_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
               uint8_t *d, const uint8_t *s, edge_t *eh)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride /= 2;
    bstride /= 2;

    uint16_t* p0 = (uint16_t *)buff + 8;
    uint16_t* p1 = p0 + bstride;
    uint16_t* p2 = p1 + bstride;
    uint16_t* p3 = p2 + bstride;
    uint16_t* p4 = p3 + bstride;
    uint16_t *orig = p0, *end = p4;

    line_copy16(p0, srcp, width, 2);
    line_copy16(p1, srcp, width, 2);
    line_copy16(p2, srcp, width, 2);
    srcp += stride;
    line_copy16(p3, srcp, width, 2);
    srcp += stride;

    uint16_t th_min = eh->min > eh->plane_max ? eh->plane_max :
                                                (uint16_t)eh->min;
    uint16_t th_max = eh->max > eh->plane_max ? eh->plane_max :
                                                (uint16_t)eh->max;

    GF_ALIGN uint16_t ar_min[8];
    GF_ALIGN uint16_t ar_max[8];
    GF_ALIGN int16_t ar_pmax[8];
    for (int i = 0; i < 8; i++) {
        ar_min[i] = th_min;
        ar_max[i] = th_max;
        ar_pmax[i] = (int16_t)eh->plane_max;
    }

    for (int y = 0; y < height; y++) {
        line_copy16(p4, srcp, width, 2);
        uint16_t* posh[] = {p2 - 2, p2 - 1, p2 + 1, p2 + 2};
        uint16_t* posv[] = {p0, p1, p3, p4};

        for (int x = 0; x < width; x += 8) {

            __m128i zero = _mm_setzero_si128();
            __m128i sumx[2] = {zero, zero};
            __m128i sumy[2] = {zero, zero};

            for (int i = 0; i < 4; i++) {
                __m128i xmm0, xmm1, xmul;
                xmul = _mm_load_si128((__m128i *)ar_mulx[i]);
                xmm0 = _mm_loadu_si128((__m128i *)(posh[i] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sumx[0] = _mm_add_epi32(sumx[0], _mm_madd_epi16(xmul, xmm0));
                sumx[1] = _mm_add_epi32(sumx[1], _mm_madd_epi16(xmul, xmm1));

                xmul = _mm_load_si128((__m128i *)ar_muly[i]);
                xmm0 = _mm_load_si128((__m128i *)(posv[i] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zero);
                xmm0 = _mm_unpacklo_epi16(xmm0, zero);
                sumy[0] = _mm_add_epi32(sumy[0], _mm_madd_epi16(xmul, xmm0));
                sumy[1] = _mm_add_epi32(sumy[1], _mm_madd_epi16(xmul, xmm1));
            }

            __m128 alpha = _mm_load_ps(ar_alpha);
            __m128 beta = _mm_load_ps(ar_beta);
            for (int i = 0; i < 2; i++) {
                sumx[i] = mm_abs_epi32(sumx[i]);
                sumy[i] = mm_abs_epi32(sumy[i]);
                __m128 t0 = _mm_cvtepi32_ps(mm_max_epi32(sumx[i], sumy[i]));
                __m128 t1 = _mm_cvtepi32_ps(mm_min_epi32(sumx[i], sumy[i]));
                t0 = _mm_add_ps(_mm_mul_ps(alpha, t0), _mm_mul_ps(beta, t1));
                sumx[i] = _mm_cvtps_epi32(t0);
                sumx[i] = _mm_srli_epi32(sumx[i], eh->rshift);
            }

            __m128i out = _mm_packs_epi32(sumx[0], sumx[1]);
            __m128i min = _mm_load_si128((__m128i *)ar_min);
            __m128i max = _mm_load_si128((__m128i *)ar_max);
            __m128i pmax = _mm_load_si128((__m128i *)ar_pmax);

            out = _mm_min_epi16(out, max);
            out = _mm_max_epi16(out, min);

            __m128i temp = _mm_cmpeq_epi16(out, max);
            temp = _mm_and_si128(temp, pmax);
            out  = _mm_or_si128(out, temp);

            temp = _mm_cmpeq_epi16(out, min);
            out = _mm_andnot_si128(temp, out);

            _mm_store_si128((__m128i *)(dstp + x), out);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = p3;
        p3 = p4;
        p4 = (p4 == end) ? orig : p4 + bstride;
    }
}

static const GF_ALIGN float ar_mulxf[][4] = {
    {  4.0,  4.0,  4.0,  4.0}, { -25.0, -25.0, -25.0, -25.0},
    { 25.0, 25.0, 25.0, 25.0}, {  -4.0,  -4.0,  -4.0,  -4.0}
};
static const GF_ALIGN float ar_mulyf[][4] = {
    {  -4.0,  -4.0,  -4.0,  -4.0}, { 25.0, 25.0, 25.0, 25.0},
    { -25.0, -25.0, -25.0, -25.0}, {  4.0,  4.0,  4.0,  4.0}
};

static void GF_FUNC_ALIGN VS_CC
proc_16bit_sse2(uint8_t *buff, int bstride, int width, int height, int stride,
                uint8_t *d, const uint8_t *s, edge_t *eh)
{
    const uint16_t *srcp = (uint16_t *)s;
    uint16_t *dstp = (uint16_t *)d;
    stride /= 2;
    bstride /= 2;

    uint16_t* p0 = (uint16_t *)buff + 8;
    uint16_t* p1 = p0 + bstride;
    uint16_t* p2 = p1 + bstride;
    uint16_t* p3 = p2 + bstride;
    uint16_t* p4 = p3 + bstride;
    uint16_t *orig = p0, *end = p4;

    line_copy16(p0, srcp, width, 2);
    line_copy16(p1, srcp, width, 2);
    line_copy16(p2, srcp, width, 2);
    srcp += stride;
    line_copy16(p3, srcp, width, 2);
    srcp += stride;
    
    GF_ALIGN int32_t ar_min[4];
    GF_ALIGN int32_t ar_max[4];
    for (int i = 0; i < 4; i++) {
        ar_min[i] = eh->min;
        ar_max[i] = eh->max;
    }
    
    for (int y = 0; y < height; y++) {
        line_copy16(p4, srcp, width, 2);
        uint16_t* posh[] = {p2 - 2, p2 - 1, p2 + 1, p2 + 2};
        uint16_t* posv[] = {p0, p1, p3, p4};
        
        for (int x = 0; x < width; x += 8) {
            __m128 zero = _mm_setzero_ps();
            __m128 sumx[2] = {zero, zero};
            __m128 sumy[2] = {zero, zero};

            for (int i = 0; i < 4; i++) {
                __m128i zeroi = _mm_setzero_si128();

                __m128 xmul = _mm_load_ps(ar_mulxf[i]);
                __m128i xmm0 = _mm_loadu_si128((__m128i *)(posh[i] + x));
                __m128i xmm1 = _mm_unpackhi_epi16(xmm0, zeroi);
                xmm0 = _mm_unpacklo_epi16(xmm0, zeroi);
                sumx[0] = _mm_add_ps(sumx[0], _mm_mul_ps(_mm_cvtepi32_ps(xmm0), xmul));
                sumx[1] = _mm_add_ps(sumx[1], _mm_mul_ps(_mm_cvtepi32_ps(xmm1), xmul));

                xmul = _mm_load_ps(ar_mulyf[i]);
                xmm0 = _mm_load_si128((__m128i *)(posv[i] + x));
                xmm1 = _mm_unpackhi_epi16(xmm0, zeroi);
                xmm0 = _mm_unpacklo_epi16(xmm0, zeroi);
                sumy[0] = _mm_add_ps(sumy[0], _mm_mul_ps(_mm_cvtepi32_ps(xmm0), xmul));
                sumy[1] = _mm_add_ps(sumy[1], _mm_mul_ps(_mm_cvtepi32_ps(xmm1), xmul));
            }
            
            __m128 alpha = _mm_load_ps(ar_alpha);
            __m128 beta = _mm_load_ps(ar_beta);
            __m128i min = _mm_load_si128((__m128i *)ar_min);
            __m128i max = _mm_load_si128((__m128i *)ar_max);
            __m128i out[2];
            for (int i = 0; i < 2; i++) {
                sumx[i] = mm_abs_ps(sumx[i]);
                sumy[i] = mm_abs_ps(sumy[i]);
                __m128 t0 = _mm_max_ps(sumx[i], sumy[i]);
                __m128 t1 = _mm_min_ps(sumx[i], sumy[i]);
                t0 = _mm_add_ps(_mm_mul_ps(alpha, t0), _mm_mul_ps(beta, t1));
                out[i] = _mm_srli_epi32(_mm_cvtps_epi32(t0), eh->rshift);
                out[i] = mm_min_epi32(out[i], max);
                out[i] = mm_max_epi32(out[i], min);
            }
            out[0] = mm_cast_epi32(out[0], out[1]);
            min = _mm_or_si128(min, _mm_slli_epi32(min, 16));
            max = _mm_or_si128(max, _mm_slli_epi32(max, 16));
            
            out[1] = _mm_cmpeq_epi16(out[0], max);
            out[0] = _mm_or_si128(out[0], out[1]);
            
            out[1] = _mm_cmpeq_epi16(out[0], min);
            out[0] = _mm_andnot_si128(out[1], out[0]);
            
            _mm_store_si128((__m128i *)(dstp + x), out[0]);
        }
        srcp += stride * (y < height - 2);
        dstp += stride;
        p0 = p1;
        p1 = p2;
        p2 = p3;
        p3 = p4;
        p4 = (p4 == end) ? orig : p4 + bstride;
    }
}


const proc_edge_detection tedge[] = {
    proc_8bit_sse2,
    proc_9_10_sse2,
    proc_16bit_sse2
};