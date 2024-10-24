# Clever Clang

## Overview

初次看到本题的 specs，我就觉得这个可以用 SIMD 做相对简单熟悉的暴力优化。一开始我就有疑问考核是否能用 SIMD，从这道题的 README 来看确实是需要用。因此我也把这道题当作了第一道练手题。

此外的话，能够在别的架构的机器上编译运行独有的 intrinsics，这种体验还是挺令人高兴的吧。

## ~~Implementation Notes (Stupid Assembly)~~

### SIMD

The SIMD itself is not so worth mentioning, just read up the [manual](https://developer.arm.com/architectures/instruction-sets/intrinsics). What is worth mentioning though, is dealing with corner case.

```c++
uint32_t remain = N % 4;
if (remain) {
  // logic to deal with corner case
}
```

To deal with the corner case and not reading and writing things out of bound, we should use `vld_lane_f32` and `vst_lane_f32`, note the `lane` means only operating on one index of the vector register. They work like a mask and only permit operation on certain element, but ARM doesn't support a mask like AVX so we need to manually perform operation on each lane.

There is one more thing that is worth mentioning, is that the `lane` argument for intrinsics like `vld_lane_f32` need to be a compile time constant, so there will be some loop unrolling of the corner case.

```cpp
/*
 * storing the corner case to points[]
 */

// this won't work
for (size_t i = 1; i < remain; i++) {
    vst1q_lane_f32(points + remain_start, vec_remain, i);
}

// correct version
switch (remain) {
    case 3:
        vst1q_lane_f32(points + remain_start, vec_remain, 3);
    case 2:
        vst1q_lane_f32(points + remain_start, vec_remain, 2);
    case 1:
        vst1q_lane_f32(points + remain_start, vec_remain, 1);
}
```

I've come across some floating point precision problem on the vectorized `poly_gradient()`, this might or might not be resolved by changing the calculation precedence, I'll come back to that later.

### Parallelism

I simply let `omp parallel for` handle the work distribution, and did a bit test and decided to use 4 threads.

## Implementation (Clever Clang)

### Using the Clever Clang

One can identify how and why a loop can or cannot be optimized with vectorization by running the `clang` with these flags.

> `-Rpass=loop-vectorize` identifies loops that were successfully vectorized.
> 
> `-Rpass-missed=loop-vectorize` identifies loops that failed vectorization and indicates if vectorization was specified.
> 
> `-Rpass-analysis=loop-vectorize` identifies the statements that caused vectorization to fail. If in addition -fsave-optimization-record is provided, multiple causes of vectorization failure may be listed (this behavior might change in the future).

### Identifying the Data Dependency

The `x` have data dependency when it is used in `poly_gradient` and also subtracted by the result of that call, which is not ideal for optimization. The `-Rpass-analysis=loop-vectorize` also warn that there're some problem with data dependency, but the warning message is not very hintful, so I decided to handle it myself.

The key idea is to swap the inner and outer loop, each iteration of inner loop performs operation on every item of `points` once, and then the outer loop handles the number of iteration to be performed.

Some terms about optimization, they are different:
- Loop vectorization
- Loop interleaving
- Loop unroll

And also, turning off OMP's `schedule(dynamic)` can further optimize the performance. This is because the calculation among all processes are identical and basically won't let one processor to idle for too long. Turning on dynamic scheduling will introduce extra cost but gain little effect.

### Resolving Data Race

My implementation of Clever Clang has gone through data race, which will lead to wrong answer. I still can't find the proper solution to this situation. So... Let's just submit it this way.

## Reflections

~~我一开始想试试看 ISPC 能不能用，但是提交需要用 `cpp` 而作罢。不过也许看一看 ISPC 编译的 SIMD 代码也许可以发掘潜在的性能提升点。~~ 不要折磨自己。

这道题的关键在于发现数据依赖，并调换循环的内外顺序。以及经典的并行程序如何处理数据竞争的问题。
