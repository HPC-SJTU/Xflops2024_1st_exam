#include "gd.h"

inline float poly_gradient(float x, const PolyParams* params) {
    return 4 * params->a * x * x * x + 3 * params->b * x * x + 2 * params->c * x + params->d;
}

void gradient_descent(float *points, uint32_t N, uint32_t M, float eta, const PolyParams* params) {
    for (uint32_t i = 0; i < N; ++i) {
        float x = points[i];
        for (uint32_t j = 0; j < M; ++j) {
            float grad = poly_gradient(x, params);
            x -= eta * grad;
        }
        points[i] = x;
    }
}