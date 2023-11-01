cmake_minimum_required(VERSION 3.10)
project(information-compressor)

# set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(SOURCES
    src/main.cc
    src/bitarray.cc
    src/bitarray.h
    src/encoding.cc
    src/encoding.h
    src/huffman.cc
    src/huffman.h
)

# Specify the include directories
include_directories(src)

# Create the executable target
add_executable(information-compressor ${SOURCES})
