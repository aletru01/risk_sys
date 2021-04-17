cmake_minimum_required(VERSION 3.10)

project(tcp_server)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-std=c++17 -pedantic -Wall -Wextra -Werror -g -pthread")


set(SRC
    src/server.cc
    src/main.cc
    src/update_table.cc
    src/parse_msg.cc
)

add_executable(tcp_server ${SRC})
