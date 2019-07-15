#!/usr/bin/env bash

./configure
make -j -C build
make -j -C build test
make -C build sw_btest_run
