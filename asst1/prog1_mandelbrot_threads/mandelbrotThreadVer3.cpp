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
    bool isRemain;
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
void workerThreadStartVer3(WorkerArgs * const args) {
    // printf("Hello world from thread %d\n", args->threadId);

    int heightChunk = args->height / args->numThreads;
    int startRow = args->threadId * heightChunk;
    if (args->isRemain) {
      // by now heightChunk is still for full parts
      startRow = heightChunk * args->numThreads;
      // now it's remaining part
      heightChunk = args->height - heightChunk * args->numThreads;
    }
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
void mandelbrotThreadVer3(
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
    std::thread workers[MAX_THREADS + 1];
    WorkerArgs args[MAX_THREADS + 1];

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
        args[i].isRemain = false;
      
        args[i].threadId = i;
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStartVer3, &args[i]);
    }
    
    int remainHeight = height - height_chunk * numThreads;
    if (remainHeight) {
      // first start the zeroth worker
      workers[0] = std::thread(workerThreadStartVer3, &args[0]);

      // The arg for the remaining part
      args[numThreads] = {
        .x0 = x0,
        .x1 = x1,
        .y0 = y0,
        .y1 = y1,
        .width = static_cast<unsigned>(width),
        .height = static_cast<unsigned>(height),
        .maxIterations = maxIterations,
        .output = output,
        .threadId = numThreads, // this term is don't care
                                // as long as isRemain == true
        .numThreads = numThreads,
        .isRemain = true,
      };

      // Notice that this **remaining part might be relative small**,
      // compare to the workload of each worker thread that deal with the main part.
      // So a naive approach to launch a new thead to handle the remaining part
      // may even draw back the performance.

      // start the remain part
      workerThreadStartVer3(&args[numThreads]);
    } else {
      workerThreadStartVer3(&args[0]);
    }

    // join worker threads, conditionally join zeroth
    for (int i=remainHeight ? 0 : 1; i<numThreads; i++) {
        workers[i].join();
    }
}

