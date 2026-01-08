cmake_minimum_required(VERSION 4.1)
project(termin_all C)

set(CMAKE_C_STANDARD 11)

add_executable(termin_all server.c
        client.c)
