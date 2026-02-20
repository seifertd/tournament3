FROM public.ecr.aws/amazonlinux/amazonlinux:2023

RUN dnf install -y gcc unzip && dnf clean all && \
    curl -fsSL "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o /tmp/aws.zip && \
    unzip -q /tmp/aws.zip -d /tmp && \
    /tmp/aws/install && \
    rm -rf /tmp/aws.zip /tmp/aws

WORKDIR /build
COPY pool.c pool.h ./
RUN gcc -Wall -Wextra -Wunused-variable -Wno-stringop-truncation -pedantic -std=c99 -O3 \
        -lm -o /usr/local/bin/pool pool.c

WORKDIR /app
COPY aws/worker.sh ./
RUN chmod +x worker.sh

ENTRYPOINT ["/app/worker.sh"]
