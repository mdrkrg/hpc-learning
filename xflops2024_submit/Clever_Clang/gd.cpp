#include "gd.h"
// #include <omp.h>

#define poly_gradient(x, params)                                               \
    (4 * params->a * x * x * x + 3 * params->b * x * x + 2 * params->c * x +   \
     params->d)

void gradient_descent(float *__restrict__ points, uint32_t N, uint32_t M,
                      float eta, const PolyParams *params) {
    //     omp_set_num_threads(4);
    // #pragma omp parallel for
    for (uint32_t j = 0; j < M; ++j) {
#pragma clang loop vectorize(enable) unroll(enable)
        for (uint32_t i = 0; i < N; ++i) {
            points[i] -= eta * poly_gradient(points[i], params);
        }
    }
}
