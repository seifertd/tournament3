#!/bin/bash

PROCS=16

for ((p=0;p<${PROCS};p++))
do
  ./pool -d 2023 -b ${p} -n ${PROCS} -f bin -p poss > /tmp/p${p}.txt 2>&1 &
done

echo "Submitted ${PROCS} background jobs ..."
echo "tail -f /tmp/p0.txt"
ps aux | grep pool
