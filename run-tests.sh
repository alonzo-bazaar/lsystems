#!/usr/bin/env sh

# ensure later commands are ran having pwd = the folder this script's in
cd $(dirname "$0")

# gcc, I love you dearly, but your error messages are ass
export CC=clang
export CXX=clang++

cd build
ctest --output-on-failure
