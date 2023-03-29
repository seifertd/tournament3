#!/bin/bash -xe

CC=${CC:-cc}

rm -rf pool testpool benchmark
${CC} -O3 -o pool pool.c
${CC} -O3 -o testpool testpool.c
${CC} -O3 -o benchmark benchmark.c


