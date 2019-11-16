# For CLion use, don't touch if you don't need it pls
cmake_minimum_required(VERSION 3.15)
project(compiler C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -pedantic -Wextra")

include_directories(.)

add_executable(compiler
        src/main.c # building the compiler
        src/err.h
        src/scanner.c src/scanner.h
        src/my_string.c src/my_string.h
        src/parser.c src/parser.h)

add_executable(compiler_tests
        src/test.c
        src/err.h
        src/scanner.c src/scanner.h
        src/my_string.c src/my_string.h
        src/parser.c src/parser.h)

