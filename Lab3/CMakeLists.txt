cmake_minimum_required(VERSION 3.21)
project(Lab3 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS -pthread)
add_executable(Lab3 caltrain-runner.c caltrain.c caltrain.h)
