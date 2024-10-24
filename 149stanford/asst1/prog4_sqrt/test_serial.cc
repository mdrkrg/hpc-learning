#include <math.h>
#include <stdio.h>
#include "../common/CycleTimer.h"

extern void sqrtSerial(int N,
                float initialGuess,
                float values[],
                float output[]);

int main(int argc, char **argv) {
    if (argc != 2) return 1;
    float num = atof(argv[1]);
    const unsigned int N = 20 * 1000 * 1000;
    const float initialGuess = 1.0f;

    float* values = new float[N];
    float* output = new float[N];

    for (unsigned int i=0; i<N; i++)
    {
        values[i] = num;
    }

    double minSerial = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrtSerial(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minSerial = std::min(minSerial, endTime - startTime);
    }

    printf("[sqrt ispc]:\t\t[%.3f] ms\n", minSerial * 1000);
    return 0;
}
