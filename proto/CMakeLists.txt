cmake_minimum_required(VERSION 3.12)
project(proto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sources src/*.cpp src/rpc/*.cpp)
file(GLOB BASIC_HEADERS include/*.h)
file(GLOB RPC_HEADERS include/rpc/*.h)
add_library(proto STATIC ${sources})
target_link_libraries(proto PUBLIC net ssl crypto)
target_include_directories(proto PUBLIC ${PROJECT_SOURCE_DIR}/include)

install (TARGETS proto DESTINATION /usr/local/bin)
install (FILES ${BASIC_HEADERS} DESTINATION /usr/local/include/srpc/proto)
install (FILES ${RPC_HEADERS} DESTINATION /usr/local/include/srpc/proto/rpc)

# add_executable(http_test test/gtest_main.cpp test/http_test.cpp src/http.cpp)
# target_link_libraries(http_test PUBLIC gtest cpputils pthread)
# target_include_directories(http_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(http_test test/gtest_main.cpp test/http_test.cpp)
target_link_libraries(http_test PUBLIC gtest cpputils proto pthread)
target_include_directories(http_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(mwget test/wget.cpp)
target_link_libraries(mwget PUBLIC gtest net cpputils proto ssl crypto)
target_include_directories(mwget PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(parser_test test/gtest_main.cpp test/parser_test.cpp)
target_link_libraries(parser_test PUBLIC gtest proto pthread)
target_include_directories(parser_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(rpc_test test/rpc_test.cpp test/helloworld.cpp)
target_link_libraries(rpc_test PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(rpc_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(rpc_test_server test/rpc_test_server.cpp test/helloworld.cpp)
target_link_libraries(rpc_test_server PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(rpc_test_server PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(stub_generator_test test/stub_generator_test.cpp)
target_link_libraries(stub_generator_test PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(stub_generator_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(rpc_sayhi_test test/rpc_sayhi_test.cpp test/sayhi.srpc.cpp)
target_link_libraries(rpc_sayhi_test  PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(rpc_sayhi_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(rpc_sayhi_test_server test/rpc_sayhi_test_server.cpp test/sayhi.srpc.cpp)
target_link_libraries(rpc_sayhi_test_server  PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(rpc_sayhi_test_server PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(rpc_algorithm_test test/rpc_algorithm_test.cpp test/algorithm.srpc.cpp)
target_link_libraries(rpc_algorithm_test PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(rpc_algorithm_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(rpc_algorithm_test_server test/rpc_algorithm_test_server.cpp test/algorithm.srpc.cpp)
target_link_libraries(rpc_algorithm_test_server PUBLIC cpputils net ssl crypto proto pthread)
target_include_directories(rpc_algorithm_test_server PUBLIC ${PROJECT_SOURCE_DIR}/include)
