#ifndef NO_SIMD_H
#define NO_SIMD_H

#include <string.h>
#include <stdint.h>

#define GF_FUNC_ALIGN
#define GF_ALIGN

static inline void
line_copy8(uint8_t *line, const uint8_t *srcp, int width, int mergin)
{
    memcpy(line, srcp, width);
    for (int i = mergin; i > 0; i--) {
        line[-i] = line[i];
        line[width - 1 + i] = line[width - 1 - i];
    }
}


static inline void
line_copy16(uint16_t *line, const uint16_t *srcp, int width, int mergin)
{
    memcpy(line, srcp, width * 2);
    for (int i = mergin; i > 0; i--) {
        line[-i] = line[i];
        line[width - 1 + i] = line[width - 1 - i];
    }
}


static inline void
line_copyf(float *line, const float *srcp, int width, int mergin)
{
    memcpy(line, srcp, width * sizeof(float));
    for (int i = mergin; i > 0; i--) {
        line[-i] = line[i];
        line[width - 1 + i] = line[width - 1 - i];
    }
}


static inline int max_int(int x, int y)
{
    return x > y ? x : y;
}


static inline float max_float(float x, float y)
{
    return x > y ? x : y;
}


static inline int min_int(int x, int y)
{
    return x < y ? x : y;
}


static inline float min_float(float x, float y)
{
    return x < y ? x : y;
}
#endif