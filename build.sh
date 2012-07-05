#!/bin/bash
set -e

mkdir -p ./bin
g++-4.7 -DBOOST_RESULT_OF_USE_DECLTYPE -std=gnu++11 src/test.cpp src/fun.cpp -o ./bin/test && ./bin/test
