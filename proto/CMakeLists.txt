cmake_minimum_required(VERSION 3.12)
project(proto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sources src/*.cpp)
add_library(proto STATIC ${sources})
target_link_libraries(proto PUBLIC net ssl crypto)
target_include_directories(proto PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(http_test test/gtest_main.cpp test/http_test.cpp src/http.cpp)
target_link_libraries(http_test PUBLIC gtest cpputils pthread)
target_include_directories(http_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(mwget test/wget.cpp src/http.cpp)
target_link_libraries(mwget PUBLIC gtest net cpputils ssl crypto)
target_include_directories(mwget PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(parser_test test/gtest_main.cpp test/parser_test.cpp src/parser.cpp)
target_link_libraries(parser_test PUBLIC gtest pthread)
target_include_directories(parser_test PUBLIC ${PROJECT_SOURCE_DIR}/include)
