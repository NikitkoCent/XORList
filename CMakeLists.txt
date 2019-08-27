cmake_minimum_required(VERSION 3.1)
project(cpp-xor-list CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third-party)

include_directories(include)

# Enable testing for the project
enable_testing()
add_subdirectory(tests)
