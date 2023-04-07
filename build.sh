#!/bin/bash -xe

CC=${CC:-cc}
CFLAGS="-Wall -Wextra -Wno-stringop-truncation -pedantic -std=c99 -O3"

rm -rf pool testpool benchmark
${CC} ${CFLAGS} -o pool pool.c
${CC} ${CFLAGS} -o testpool testpool.c
${CC} ${CFLAGS} -o benchmark benchmark.c


