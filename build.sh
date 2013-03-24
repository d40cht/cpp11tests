#!/bin/bash
set -e

mkdir -p ./bin
g++-4.7 -DBOOST_RESULT_OF_USE_DECLTYPE -std=c++11 src/test.cpp src/fun.cpp src/hashtable.cpp -o ./bin/test && ./bin/test
