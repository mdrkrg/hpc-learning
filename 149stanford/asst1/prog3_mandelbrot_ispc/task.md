## 1
1. 8 times
   According to the SIMD model of programming, if there exists a large portion of if-then-else clauses, a large portion of SIMD instruction result will be masked off and discarded, decreasing the parallelism.
   These two lines of code `if (z_re * z_re + z_im * z_im > 4.f) break;` might frequently mask of a portion of parallel tunnels of vector, which will lead to some tunnels being performed operations, while others not. This will severely affect the data on the border of the Mandelbrot Set, since some data will `break` within a few iterations, while others might have much more iterations, making the SIMD instructions perform much more branches than the data far away from the border.
   In general, if the iteration number are basically the same across the data in a vector being processed, the parallel performance will be much better.

## 2
1. When running at 32 tasks, ispc can achieve a speed up of ~40x compared to serial version, and a ~10x compared to ispc without tasks.
2. The thread number should be better when the number of rows are perfectly divided by the thread number, due to the fact that they don't have remaining part of data to deal with.
   Apart from that, the tasked ispc generally meet a performance bottleneck when the task count is at around 32 - 40 for my 16 core 24 hardware-thread PC.
3. There's obviously no explicit call to `join()` in sipc.
4. Sometimes we may want some part of code be guaranteed to not thread-level parallelized or synchronized, so if we don't explicitly declare a parallel portion of code to `launch`, the ISPC will not fork them into threads.
