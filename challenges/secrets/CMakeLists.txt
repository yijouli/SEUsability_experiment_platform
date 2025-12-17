cmake_minimum_required(VERSION 3.28)
project(secrets C)

set(CMAKE_C_STANDARD 17)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")

add_executable(secrets main.c
        crypto.c
        crypto.h
        sort.c
        sort.h
        structures.h)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "secrets.out")