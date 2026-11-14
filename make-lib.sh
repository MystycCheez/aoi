#!/bin/sh
set -e

echo "------------------------------------------------------------------"
echo "clang"
echo "------------------------------------------------------------------"
clang -fsanitize=address -g -O0 -c src/aoi.c -o build/aoi.o -Iinclude/
clang -fsanitize=address -g -O0 -c src/aoi_action.c -o build/aoi_action.o -Iinclude/ 
clang -fsanitize=address -g -O0 -c src/aoi_userdata.c -o build/aoi_userdata.o -Iinclude/
clang -fsanitize=address -g -O0 -c src/aoi_binding.c -o build/aoi_binding.o -Iinclude/

echo "------------------------------------------------------------------"
echo "make archive"
echo "------------------------------------------------------------------"
ar rcs build/libaoi.a build/*.o

echo "------------------------------------------------------------------"
echo "remove object files"
echo "------------------------------------------------------------------"
rm build/*.o

echo "------------------------------------------------------------------"
echo "copy includes and archive to local storage"
echo "------------------------------------------------------------------"
rsync -varzP build/ /usr/local/lib
rsync -varzP include/ /usr/local/include/aoi

echo "------------------------------------------------------------------"
echo "remove lib files"
echo "------------------------------------------------------------------"
rm build/*.a