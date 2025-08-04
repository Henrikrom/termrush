#! /bin/bash -e

mkdir -p build

gcc termrush.c -o build/termrush

./build/termrush
