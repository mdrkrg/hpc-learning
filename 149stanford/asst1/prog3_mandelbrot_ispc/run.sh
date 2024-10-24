#!/bin/bash

EXPECTED_ARGC=1
ARGC=$#
OUTPUT_FILE='./result.txt'
re='^[0-9]+$'

thread=$1

if [ $ARGC -ne $EXPECTED_ARGC ]; then
  printf "Usage: %s [thread_num]\n" "${0}"
  exit 1
fi

if ! [[ "$thread" =~ $re ]]; then
  echo "$thread"
  printf "Usage: %s [thread_num]\n" "${0}"
  printf "[thread_num]: min 2, max 64\n"
  exit 1
fi

if [ "$thread" -le 1 ] || [ "$thread" -gt 64 ]; then
  printf "Usage: %s [thread_num]\n" "${0}"
  printf "[thread_num]: min 2, max 64\n"
  exit 1
fi

make -s
rm -f ${OUTPUT_FILE}

for ((i = 2; i <= ${1}; i++)); do
  ./mandelbrot_ispc --tasks=${i} -v 1 | tee -a ${OUTPUT_FILE}
done
