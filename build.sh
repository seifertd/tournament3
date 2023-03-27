#!/bin/bash -xe

rm -rf pool testpool benchmark
cc -O3 -o pool pool.c
cc -O3 -o testpool testpool.c
cc -O3 -o benchmark benchmark.c


