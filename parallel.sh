#!/bin/bash

PROCS=${PROCS:-16}
DIR=${1}

if [ -z "$DIR" ]; then
  echo "Usage: ${0} path/to/pool/directory";
  exit 1;
fi

for ((p=0;p<${PROCS};p++))
do
  ./pool -d ${DIR} -b ${p} -n ${PROCS} -f bin -p poss > /tmp/p${p}.txt 2>&1 &
done

echo "Submitted ${PROCS} background jobs ..."
echo "tail -f /tmp/p0.txt"
ps aux | grep pool
