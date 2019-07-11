#!/usr/bin/env bash

./configure
make -j -C build
make -j -C build test
