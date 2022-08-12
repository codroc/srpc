# rm -rf build/
cmake -B build -DCMAKE_BUILD_TYPE=DEBUG
cmake --build build -j 8 #--target install
