#ifndef GD_H
#define GD_H

#include <string>

struct PolyParams {
    float a, b, c, d;
    PolyParams(float a, float b, float c, float d) : a(a), b(b), c(c), d(d) {}
};



void gradient_descent(float *points, uint32_t N, uint32_t M, float eta, const PolyParams* params);

#endif // GD_H