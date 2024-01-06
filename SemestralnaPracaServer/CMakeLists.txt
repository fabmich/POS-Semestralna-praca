cmake_minimum_required(VERSION 3.18)
project(SemestralnaPracaServer C)

set(CMAKE_C_STANDARD 11)

add_executable(SemestralnaPracaServer main.c pos_sockets/passive_socket.c pos_sockets/char_buffer.c pos_sockets/active_socket.c
)
target_link_libraries(SemestralnaPracaServer pthread)

