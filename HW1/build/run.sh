#!/usr/bin/env bash

ACCOUNTS_ARRAY=(100 1000 10000)
THREADS_ARRAY=(1 2 4 8 16)
ITERATIONS=100000

echo "executable,threads,accounts,result"

for ACC in "${ACCOUNTS_ARRAY[@]}"; do
  for TH in "${THREADS_ARRAY[@]}"; do

    if [ "$TH" -eq 1 ]; then
      # Run 'single' if threads=1
    #   OUTPUT=$(./single --iterations "$ITERATIONS" --accounts "$ACC")
    #   echo "single,$TH,$ACC,\"$OUTPUT\""
      OUTPUT=$(./single2 --iterations "$ITERATIONS" --accounts "$ACC")
      echo "single2,$TH,$ACC,\"$OUTPUT\""
    else
      # Run 'multi' if threads=2,4,8
      OUTPUT=$(./multi3 --threads "$TH" --accounts "$ACC" --batch_size 50000)
      echo "multi3,$TH,$ACC,\"$OUTPUT\""
    #   OUTPUT=$(./multi4 --threads "$TH" --accounts "$ACC")
    #   echo "multi4,$TH,$ACC,\"$OUTPUT\""
    fi

  done
done
