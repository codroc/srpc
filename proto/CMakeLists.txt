cmake_minimum_required(VERSION 3.12)
project(proto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(http_test test/gtest_main.cpp test/http_test.cpp src/http.cpp)
target_link_libraries(http_test PUBLIC gtest cpputils pthread)
target_include_directories(http_test PUBLIC ${PROJECT_SOURCE_DIR}/include)
