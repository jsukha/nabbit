#!/usr/bin/env bash
# Configure
if [ -z $1 ]; then
    TOOLCHAIN=gcc
else
    TOOLCHAIN=$1
fi

echo "Using toolchain: ${TOOLCHAIN}"   
BUILD_DIR=build_${TOOLCHAIN}

./configure ${TOOLCHAIN} ${BUILD_DIR}
make -j -C${BUILD_DIR}
make -j -C${BUILD_DIR} test
make -C${BUILD_DIR} sw_btest_run
