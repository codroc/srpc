cmake_minimum_required(VERSION 3.12)
project(net)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

add_library(net STATIC ${sources})
target_link_libraries(net PUBLIC cpputils yaml-cpp ssl crypto dl stdc++fs pthread)
target_include_directories(net PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(TARGETS net DESTINATION /usr/local/bin)
install(FILES ${HEADERS} DESTINATION /usr/local/include/srpc/net)

add_executable(gtest_test_env test/gtest_main.cpp test/gtest_test_env.cpp)
target_link_libraries(gtest_test_env gtest pthread)

add_executable(address_test test/gtest_main.cpp test/address_test.cpp)
target_link_libraries(address_test gtest net)
target_include_directories(address_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(file_descriptor_test test/gtest_main.cpp test/file_descriptor_test.cpp)
target_link_libraries(file_descriptor_test gtest net)
target_include_directories(file_descriptor_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(socket_test test/gtest_main.cpp test/socket_test.cpp)
target_link_libraries(socket_test gtest net)
target_include_directories(socket_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(buffer_test test/gtest_main.cpp test/buffer_test.cpp)
target_link_libraries(buffer_test gtest net)
target_include_directories(buffer_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(evl_0 test/evl_0.cpp)
target_link_libraries(evl_0 pthread net)
target_include_directories(evl_0 PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(evl_1 test/evl_1.cpp)
target_link_libraries(evl_1 pthread net)
target_include_directories(evl_1 PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(evl_2 test/evl_2.cpp)
target_link_libraries(evl_2 pthread net)
target_include_directories(evl_2 PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(timer_test test/timer_test.cpp)
target_link_libraries(timer_test pthread net)
target_include_directories(timer_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(timer_resolution_test test/timer_resolution_test.cpp)
target_link_libraries(timer_resolution_test net)
target_include_directories(timer_resolution_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(timer_multithread_test test/timer_multithread_test.cpp)
target_link_libraries(timer_multithread_test net)
target_include_directories(timer_multithread_test PUBLIC ${PROJECT_SOURCE_DIR}/include)
