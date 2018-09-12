#1/usr/bin/env bash

# Use this script to build the package manager if you do not have an existing instance of particlepm

set -x

CC=${CC:-cc}
CXX=${CXX:-c++}
LD=${LD:-ld}

$CXX $PWD/particlepm.cpp "-I${PWD}" -ldl -o libparticlepm.so -std=c++17 -shared -g -fPIC
$CXX $PWD/main.cpp "-I${PWD}" -o particlepm -std=c++17 -L. -lparticlepm -Wl,-rpath=./ -g -fPIC