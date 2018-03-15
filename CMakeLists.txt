cmake_minimum_required(VERSION 3.1)
project(XORList)

set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

include_directories(include)


enable_testing()

if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()
include(third-party/DownloadProject/DownloadProject.cmake)

# ================================== GOOGLETEST =========================================
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
                )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()
# ================================ GOOGLETEST END =======================================

add_executable(unittesting tests/empty.cpp)
target_link_libraries(unittesting gtest gmock_main)

add_test(NAME do_unittests COMMAND unittesting)

# ================================= TEST COVERAGE =======================================
download_project(PROJ                googletest-coverage
                 GIT_REPOSITORY      https://github.com/NikitkoCent/cmake-modules
                 GIT_TAG             changes
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
                )
include(${googletest-coverage_SOURCE_DIR}/CodeCoverage.cmake)

APPEND_COVERAGE_COMPILER_FLAGS()
SETUP_TARGET_FOR_COVERAGE(NAME collect_coverage
                          EXECUTABLE ctest
                          DEPENDENCIES unittesting)
# ===============================  END TEST COVERAGE ====================================
