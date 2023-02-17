#!/bin/bash

rm -r build
mkdir build

cmake -B build/
make -C build/
