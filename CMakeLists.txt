cmake_minimum_required(VERSION 3.10)
project(dining_philosophers)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(dining_philosophers src/main.cpp src/spinlocklock.cpp)
