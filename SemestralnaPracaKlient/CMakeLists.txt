cmake_minimum_required(VERSION 3.18)
project(SemestralnaPracaKlient)

set(CMAKE_CXX_STANDARD 17)

add_executable(SemestralnaPracaKlient main.cpp
        my_socket.cpp)
target_link_libraries(SemestralnaPracaKlient ws2_32)
