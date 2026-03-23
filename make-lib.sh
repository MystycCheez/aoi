#!/bin/sh
set -e

echo "clang"
clang -O3 -c src/aoi.c -o build/aoi.o -Iinclude/

echo "make archive"
ar rcs build/libaoi.a build/aoi.o

echo "remove object files"
rm build/*.o

echo "copy includes and archive to local storage"
rsync -varzP build/ /usr/local/lib
rsync -varzP include/ /usr/local/include/aoi