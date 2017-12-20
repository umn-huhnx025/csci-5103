#!/bin/bash

set -e

echo "Recompiling with benchmark output enabled..."
make -B benchmark > /dev/null
echo "Compiled successfully"

out_file="benchmarks.csv"

# truncate file, don't want old results
> $out_file

echo "Putting results in $out_file"

declare -a methods=("rand" "fifo" "custom")
declare -a programs=("sort" "scan" "focus")

nframes=100

echo "Running benchmarks..."
# echo "npages,nframes,method,program,num_page_faults,num_disk_reads,num_disk_writes" >> $out_file
for method in "${methods[@]}"; do
  for program in "${programs[@]}"; do
    for i in {1..10}; do
      npages="$((10 * $i))"
      ./virtmem $nframes $npages $method $program >> $out_file
    done
  done
done

