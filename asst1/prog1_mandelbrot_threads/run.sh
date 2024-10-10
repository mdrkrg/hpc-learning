#!/bin/bash

EXPECTED_ARGC=1
ARGC=$#
re='^[0-9]+$'

thread=$1

if [ $ARGC -ne $EXPECTED_ARGC ]; then
  printf "Usage: %s [thread_num]\n" "${0}"
  exit 1
fi

if ! [[ "$thread" =~ $re ]]; then
  echo "$thread"
  printf "Usage: %s [thread_num]\n" "${0}"
  printf "[thread_num]: min 2, max 32\n"
  exit 1
fi

if [ "$thread" -le 1 ] || [ "$thread" -gt 32 ]; then
  printf "Usage: %s [thread_num]\n" "${0}"
  printf "[thread_num]: min 2, max 32\n"
  exit 1
fi

make

for ((i = 2; i <= ${1}; i++)); do
  ./mandelbrot --threads ${i} -v 1 | tee -a ./result.txt
done
