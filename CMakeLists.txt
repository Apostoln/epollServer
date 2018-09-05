cmake_minimum_required(VERSION 3.12)
project(epollServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O0")
add_executable(epollServer main.cpp Socket.h Server.h)