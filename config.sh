# rm -rf build/
cmake -B build -GNinja -DCMAKE_BUILD_TYPE=DEBUG
cmake --build build
