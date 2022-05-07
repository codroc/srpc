cmake_minimum_required(VERSION 3.12)
project(net)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(net STATIC ${sources})
target_link_libraries(net PUBLIC cpputils yaml-cpp dl stdc++fs pthread)
target_include_directories(net PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(gtest_test_env test/gtest_main.cpp test/gtest_test_env.cpp)
target_link_libraries(gtest_test_env gtest pthread)

add_executable(address_test src/address.cpp test/gtest_main.cpp test/address_test.cpp)
target_link_libraries(address_test gtest pthread cpputils)
target_include_directories(address_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(file_descriptor_test src/file_descriptor.cpp test/gtest_main.cpp test/file_descriptor_test.cpp)
target_link_libraries(file_descriptor_test gtest pthread cpputils)
target_include_directories(file_descriptor_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(socket_test src/socket.cpp src/ssl_socket.cpp src/file_descriptor.cpp src/address.cpp test/gtest_main.cpp test/socket_test.cpp)
target_link_libraries(socket_test gtest cpputils yaml-cpp stdc++fs ssl crypto pthread)
target_include_directories(socket_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(buffer_test src/buffer.cpp test/gtest_main.cpp test/buffer_test.cpp)
target_link_libraries(buffer_test gtest pthread)
target_include_directories(buffer_test PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(evl_0 src/eventloop.cpp test/evl_0.cpp)
target_link_libraries(evl_0 pthread cpputils net)
target_include_directories(evl_0 PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(evl_1 src/eventloop.cpp test/evl_1.cpp)
target_link_libraries(evl_1 pthread cpputils net)
target_include_directories(evl_1 PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(evl_2 src/eventloop.cpp test/evl_2.cpp)
target_link_libraries(evl_2 pthread cpputils net)
target_include_directories(evl_2 PUBLIC ${PROJECT_SOURCE_DIR}/include)
