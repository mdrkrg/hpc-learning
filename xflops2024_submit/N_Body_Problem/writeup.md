# N Body Problem

这个是我做的第三道题，第二道题有个库死活链接不上去，遂开启下一道。看上去前一部分就是简单的并行优化题，而 MPI 我还不是很熟悉。

## Implementation Notes

### OpenMP

At first I want to implement with SIMD, but the workload is too heavy, so I decided to first make a test run with simple OMP parallelism, since the cluster with 128 cores is really a performance monster.

I've replaced `sqrt` with `pow` in `compute_forces`, and enabled flag `-ffast-math`. This may have some optimization but not much, but not dividing anything can avoid loss of precision, which is the main reason I've done this.

Also there're some **loop unrolling** in `compute_forces` but the effect will be subtle.

In `compute_total_energy`, the readme says *feel free to optimize with your smart* so I initially decided to use the famous Quake III `Q_rsqrt` to compute 1/r, which involves some floating point representation and bit hack. But I later discovered the ARM intrinsic `vrsqrted_f64`, so no need to bother now.

But, why would it be slower than that? Wikipedia says the Intel AVX is faster than Q_rsqrt, so why Arm's vrsqrted slower? Is it the problem of precision (fp64 rather than fp32)?

One final subtle thing I've learnt is OMP's `schedule(dynamic)`, this let the system balance share of work among threads dynamically and avoids faster threads waiting for the slowest one.

### OpenMPI

Including headers to clangd:
```yaml
CompileFlags:
  Add:
  - -I/usr/lib/x86_64-linux-gnu/openmpi/include
```

#### First TODO: Broadcasting

This part I initially want to use `MPI_Send` and `MPI_Recv` pairs, but this is too redundant, so I search the web and found `MPI_Bcast`. This communicator accept a `root` parameter, as the root process to broadcast, and the other processes automatically receive them.

#### Second TODO: Synchronize with other processes

~~The momentum and energy have to be computed after each iteration of `step`, when all processes finish their computation in this iteration. This is done by calling `MPI_Barrier`, which will block execution of all processes until all processes have reached this point.~~

This is not correct. The global `particles` need to be updated before the next iteration. We have to figure out how to write the `local_particles` into the global `particles`

`MPI_Allgather` might be what I'm looking for, but how to let it fill into this offset `rank * local_n`?

It turns out we don't need.

#### Third TODO: Reduce the local result to 0th process

After all parallel processes have done computing their own result in `local_*`, we have to reduce them to the 0th process. This is like OpenMP's `reduction(+: global_*)`, done by calling `MPI_Reduce`. The `op` parameter needs to be `MPI_SUM` since we are summing up the results of other processes.

That's it, we're done!
