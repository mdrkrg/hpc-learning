# HPL

## Writing Makefile

Examining the `Make.LINUX_Intel64`, there're basically three things to configure: the HPL itself, the Message Passing library, and the Linear Algebra library. Plus, there some extra options like the compiler and compile flags.

I only have the modules provided by the lustre modules, so here are my options:
- ~~gcc/10.3.0-gcc-4.8.5~~ bisheng/2.5.0
- ~~openmpi/4.1.4-gcc-10.3.0~~ hypermpi/4.1.1-bisheng-2.5.0
  - directory `/lustre/opt/linux-openeuler22-aarch64/linux-openeuler22-aarch64/hpckit/HPCKit/24.6.30/hmpi/bisheng/hmpi`
- openblas/0.3.21-gcc-10.3.1
  - directory `/lustre/opt/linux-openeuler22-aarch64/linux-openeuler22-aarch64/gcc-10.3.1/openblas-0.3.21-bjmrioelsunu5hmwor47k5yihqraxunb`
- hpl/2.3-gcc-10.3.0-openblas (I'm not sure whether it's necessary)

### Linking Problem...

I spent a night fighting with `ld`.

Conclusion after numerous attempts to find linking problem: I love gcc.

One day I decided to try out the Wahwei's HyperMPI, and it is compiled by BiSheng compiler (which is a modified version of clang). When I use OpenMPI with gcc versioned mpicc, ld always tell me `-llustreapi`, `-lpmi`, `-lpmi2` cannot found. Even if I have added `-L/usr/lib64 -llustreapi` and LLM's idea `-Wl,-rpath=/usr/lib64 -L/usr/lib64 -l/usr/lib64/liblustreapi.so`, the problem is just there.

I tried to follow the problem. There is one top level Makefile, and it leads the way into the Makefiles of some subdirectories, which also contains their own Makefiles. The problem occurs when `make` is inside `src/pauxil/$(arch)`, so I checkout the corresponding Makefile and tried to `make` manually, then spotted the line of Makefile command when error occurs: There is no `-l` library include after the source code! It was then when I realized I might be using the wrong type of compiler. (I'm not sure is this true.)

The things happened afterwards made the problem even clearer. I tried making with HyperMPI loaded, but it told me `mpicc` not found, then I realized I have to load another compiler. Yeah, it was clang (bisheng), clang doesn't require `-l` to be at the end of source file.

So... Problem is solved, let's checkout what to do next.

## Writing SBATCH and Running the Test

The `HPL.dat` is the test parameter of the HPL test. You can configure it manually to get the most suitable configure of a particular setup of cluster.

Some [info](https://www.netlib.org/benchmark/hpl/faqs.html) about [tuning the `HPL.dat`](https://www.netlib.org/benchmark/hpl/tuning.html):
- `Ns` the problem size, N^2 is the number of FP64s.
  - "the largest problem size fitting in memory is what you should aim for"
  - 80 % of the total amount of memory is a good guess.
    - 256 GiB is 2^38 bytes, 2^35 FP64, so 80% of N is about 148291
    - I'm trying at 2^17 = 131072
- `NBs` the block sizes, for the data distribution as well as for the computational granularity.
  - usually in the \[32 .. 256\] interval
  - larger block sizes may be better for larger N
  - kp960 has three level cache of size 8, 64, 256 MiB, and L1, L2 are local to every core (128 instances), L3 has 4 instances
  - If I use 2^34 FP64, that is 2^37 Bytes, each block have 2^29 = 512 MiB
  - But if you decrease block size, NB will be too high (not ideal)
- `P` and `Q`, process grid ration

