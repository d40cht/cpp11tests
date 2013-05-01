#!/bin/bash

set -e

./sbt "native-build-configuration Release_Gcc_LinuxPC" compile test
./sbt "native-build-configuration Release_Clang_LinuxPC" compile test
