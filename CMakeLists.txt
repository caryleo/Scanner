cmake_minimum_required(VERSION 3.10)
project(Scanner)

set(CMAKE_CXX_STANDARD 11)

add_executable(Scanner main.cpp Scanner.cpp Scanner.h Token.cpp Token.h Position.cpp Position.h)