cmake_minimum_required(VERSION 3.19)
project(sampvoice-server)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/omp-sdk)
add_subdirectory(server)
