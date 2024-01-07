cmake_minimum_required(VERSION 3.18)
project(SemestralnaPracaKlient)

set(CMAKE_CXX_STANDARD 17)

add_executable(SemestralnaPracaKlient main.cpp
        my_socket.cpp
        MemoryLeak.h
        connect4/structures/Hra.h
        connect4/structures/Hra.cpp
        connect4/structures/ThreadData.h
        connect4/structures/ThreadData.cpp
        connect4/Connect4.h
        connect4/Connect4.cpp

)
target_link_libraries(SemestralnaPracaKlient ws2_32)
