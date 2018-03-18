cmake_minimum_required(VERSION 3.1)
project(XORList CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

find_package(Threads)

if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()
include(third-party/DownloadProject/DownloadProject.cmake)

# =============================== CMAKE MODULES =========================================
download_project(PROJ                cmake-modules
                 GIT_REPOSITORY      https://github.com/NikitkoCent/cmake-modules
                 GIT_TAG             changes
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
                )
# =============================== END CMAKE MODULES =====================================

# ================================ SETUP WARNINGS =======================================
include(${cmake-modules_SOURCE_DIR}/EnableExtraCompilerWarnings.cmake)
globally_enable_extra_compiler_warnings()
# ============================== END SETUP WARNINGS =====================================

# ================================== GOOGLETEST =========================================
enable_testing()

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

add_executable(unittesting tests/list.cpp
                           include/xor_list.h
                           tests/iterator.cpp)
target_link_libraries(unittesting gtest gmock_main Threads::Threads)

add_test(NAME do_unittests COMMAND unittesting)


add_executable(sort_performance tests/performance_list_sort.cpp)
add_test(NAME measure_sort_performance COMMAND sort_performance)

# ================================= TEST COVERAGE =======================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(${cmake-modules_SOURCE_DIR}/CodeCoverage.cmake)

    if (CMAKE_CXX_FLAGS_COVERAGE)
        APPEND_COVERAGE_COMPILER_FLAGS()
        SETUP_TARGET_FOR_COVERAGE(NAME collect_coverage
                                  EXECUTABLE ctest
                                  DEPENDENCIES unittesting)
    endif()
endif()
# ===============================  END TEST COVERAGE ====================================

MESSAGE("Build type : ${CMAKE_BUILD_TYPE}")
MESSAGE("Used compiler : ${CMAKE_CXX_COMPILER}")
if (CMAKE_BUILD_TYPE EQUAL "Debug")
    MESSAGE("Compiler flags : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif (CMAKE_BUILD_TYPE EQUAL "Release")
    MESSAGE("Compiler flags : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
elseif (CMAKE_BUILD_TYPE EQUAL "RelWithDebInfo")
    MESSAGE("Compiler flags : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif (CMAKE_BUILD_TYPE EQUAL "MinSizeRel")
    MESSAGE("Compiler flags : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL}")
else()
    MESSAGE("Compiler flags : ${CMAKE_CXX_FLAGS}")
endif()
