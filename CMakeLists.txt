cmake_minimum_required(VERSION 3.1)
project(XORList CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_REQUIRED ON)

add_subdirectory(third-party)

include_directories(include)

add_subdirectory(tests)
