#!/bin/bash
set -e

mkdir -p ./bin
g++-4.7 -std=gnu++11 src/test.cpp src/fun.cpp -o ./bin/test && ./bin/test
