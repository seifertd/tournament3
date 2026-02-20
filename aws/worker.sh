#!/bin/bash
set -euo pipefail

BATCH_INDEX=${AWS_BATCH_JOB_ARRAY_INDEX}
NUM_BATCHES=${POOL_NUM_BATCHES:-4096}
BUCKET=${POOL_S3_BUCKET}
PREFIX=${POOL_S3_PREFIX:-pool}

WORK_DIR=$(mktemp -d)
trap "rm -rf ${WORK_DIR}" EXIT

echo "Worker ${BATCH_INDEX} of ${NUM_BATCHES} starting..."

aws s3 sync "s3://${BUCKET}/${PREFIX}/data/" "${WORK_DIR}/"

pool -d "${WORK_DIR}" -b "${BATCH_INDEX}" -n "${NUM_BATCHES}" -f bin poss

BIN_FILE="${WORK_DIR}/poss_${BATCH_INDEX}_of_${NUM_BATCHES}.bin"
aws s3 cp "${BIN_FILE}" "s3://${BUCKET}/${PREFIX}/bins/poss_${BATCH_INDEX}_of_${NUM_BATCHES}.bin"

echo "Worker ${BATCH_INDEX} complete."
