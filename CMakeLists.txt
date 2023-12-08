cmake_minimum_required(VERSION 3.27)
project(external_memory_sort)

set(CMAKE_CXX_STANDARD 20)

add_executable(external_memory_sort main.cpp
        TmpFile.h
        ExternalMemorySort.cpp
        ExternalMemorySort.hpp)
add_executable(write_file write_file.cpp)
