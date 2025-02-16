#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


//
// workerThreadStart --
//
// Thread entrypoint.
void workerThreadStart(WorkerArgs * const args) {
    // printf("Hello world from thread %d\n", args->threadId);

    int heightChunk = args->height / args->numThreads;
    int startRow = args->threadId * heightChunk;
    mandelbrotSerial(
        args->x0,
        args->y0,
        args->x1, 
        args->y1, 
        args->width, 
        args->height, 
        startRow, 
        heightChunk, 
        args->maxIterations, 
        args->output
    );
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    int height_chunk = height / numThreads;

    for (int i=0; i<numThreads; i++) {
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
      
        args[i].threadId = i;
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStart, &args[i]);
    }
    
    workerThreadStart(&args[0]);

    int remainHeight = height - height_chunk * numThreads;
    // Remaining parts
    if (remainHeight) {
        int remainStartRow = height_chunk * numThreads;
        mandelbrotSerial(x0, y0, x1, y1, width, height, remainStartRow, remainHeight, maxIterations, output);
    }

    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();
    }
}

