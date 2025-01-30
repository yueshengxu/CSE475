#!/usr/bin/env bash

ACCOUNTS_ARRAY=(100 1000 10000)
ITERATIONS_ARRAY=(100000 200000 300000)
THREADS_ARRAY=(1 2 4 8 16)

echo "executable,threads,accounts,iterations,result"

for ACC in "${ACCOUNTS_ARRAY[@]}"; do
  for ITER in "${ITERATIONS_ARRAY[@]}"; do
    for TH in "${THREADS_ARRAY[@]}"; do

      if [ "$TH" -eq 1 ]; then
        OUTPUT=$(./single2 --iterations "$ITER" --accounts "$ACC")
        echo "single2,$TH,$ACC,$ITER,\"$OUTPUT\""
      else
        OUTPUT=$(./multi3 --threads "$TH" --accounts "$ACC" --batch_size 5000 --iterations "$ITER")
        echo "multi3,$TH,$ACC,$ITER,\"$OUTPUT\""
      fi

    done
  done
done
