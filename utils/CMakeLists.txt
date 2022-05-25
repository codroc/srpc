cmake_minimum_required(VERSION 3.12)
project(utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)

# add_library(utils STATIC ${sources})
# target_link_libraries(utils PUBLIC pthread)
# target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include/*.h)

add_executable(test_thread_pool test/test_thread_pool.cpp)
target_link_libraries(test_thread_pool net pthread)
target_include_directories(test_thread_pool PUBLIC ${PROJECT_SOURCE_DIR}/include)

