#!/bin/sh
set -e

echo "clang"
clang -fsanitize=address -g -O0 -c src/aoi.c -o build/aoi.o -Iinclude/
clang -fsanitize=address -g -O0 -c src/aoi_action.c -o build/aoi_action.o -Iinclude/ 
clang -fsanitize=address -g -O0 -c src/aoi_userdata.c -o build/aoi_userdata.o -Iinclude/
clang -fsanitize=address -g -O0 -c src/aoi_binding.c -o build/aoi_binding.o -Iinclude/

echo "make archive"
ar rcs build/libaoi.a build/*.o

echo "remove object files"
rm build/*.o

echo "copy includes and archive to local storage"
rsync -varzP build/ /usr/local/lib
rsync -varzP include/ /usr/local/include/aoi

echo "remove lib files"
rm build/*.a