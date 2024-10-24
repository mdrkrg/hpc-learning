#!/bin/bash

#SBATCH --job-name=hpl
#SBATCH --partition=kp920race
#SBATCH --time=1:00:00
#SBATCH -N 1
#SBATCH --ntasks-per-node=128
#SBATCH --output=hpl.out
#SBATCH --error=hpl.err

ulimit -s unlimited
ulimit -l unlimited
# export HPL_DATA_FILE=bin/xflops/HPL.dat
module load openblas/0.3.21-gcc-10.3.1
module load hypermpi/4.1.1-bisheng-2.5.0
cd ./bin/xflops
mpirun -np 128 ./xhpl &>../../myhpl.out
