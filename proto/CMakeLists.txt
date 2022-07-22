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

add_executable(rpc_test test/rpc_test.cpp test/helloworld.cpp src/rpc/rpc.cpp src/rpc/message.cpp src/rpc/serialize.cpp src/rpc/codeco.cpp src/rpc/service.cpp)
target_link_libraries(rpc_test PUBLIC cpputils net ssl crypto pthread)
target_include_directories(rpc_test PUBLIC ${PROJECT_SOURCE_DIR}/include/rpc)

add_executable(rpc_test_server test/rpc_test_server.cpp src/rpc/rpc.cpp src/rpc/message.cpp src/rpc/serialize.cpp src/rpc/codeco.cpp src/rpc/service.cpp test/helloworld.cpp)
target_link_libraries(rpc_test_server PUBLIC cpputils net ssl crypto pthread)
target_include_directories(rpc_test_server PUBLIC ${PROJECT_SOURCE_DIR}/include/rpc)
