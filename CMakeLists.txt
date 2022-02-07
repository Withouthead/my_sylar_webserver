cmake_minimum_required(VERSION 3.16.3)
project(sylar_web_server)

set(CMAKE_CXX_STANDARD 20)

add_executable(sylar_web_server main.cpp log.cpp log.h)
