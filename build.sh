#!/bin/bash -xe

CC=${CC:-cc}

if [ "${CC}" = "clang" ]; then
  CFLAGS="-Wall -Wextra -Wno-string-concatenation -pedantic -std=c99 -O3"
else
  CFLAGS="-Wall -Wextra -Wunused-variable -Wno-stringop-truncation -pedantic -std=c99 -O3"
fi

rm -rf pool testpool benchmark scoredetail
${CC} ${CFLAGS} -lm -o pool pool.c
${CC} ${CFLAGS} -lm -o testpool testpool.c
${CC} ${CFLAGS} -lm -o benchmark benchmark.c
${CC} ${CFLAGS} -lm -o scoredetail scoredetail.c
