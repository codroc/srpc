cmake_minimum_required(VERSION 3.12)
project(server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(https_hello https_hello/https_hello.cpp)
target_link_libraries(https_hello proto)

add_executable(https_ft https_ft/https_ft.cpp)
target_link_libraries(https_ft proto stdc++fs)

add_executable(mt_discard https_mt_echo/discard.cpp)
target_link_libraries(mt_discard net proto)

add_executable(mt_echo https_mt_echo/echo.cpp)
target_link_libraries(mt_echo net proto)

add_executable(mt_time mt/time.cpp)
target_link_libraries(mt_time net proto)
