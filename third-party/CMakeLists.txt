cmake_minimum_required(VERSION 2.8.8)

# =================================== INITIALIZE SUBMODULES ===================================
message(STATUS "Setting up Git submodules...")

find_package(Git)
if (NOT GIT_FOUND)
    message(FATAL_ERROR "Cannot find git.")
endif(NOT GIT_FOUND)

execute_process(COMMAND
                ${GIT_EXECUTABLE} submodule init
                RESULT_VARIABLE GIT_RESULT
                ERROR_VARIABLE GIT_ERROR_MESSAGE)

if (NOT ${GIT_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to init git submodules because of \"${GIT_ERROR_MESSAGE}\"")
endif(NOT ${GIT_RESULT} EQUAL 0)

execute_process(COMMAND
                ${GIT_EXECUTABLE} submodule update
                RESULT_VARIABLE GIT_RESULT
                ERROR_VARIABLE GIT_ERROR_MESSAGE)

if (NOT ${GIT_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to update git submodules because of \"${GIT_ERROR_MESSAGE}\"")
endif(NOT ${GIT_RESULT} EQUAL 0)

set(_googletest_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
set(_cmake-modules_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

set(googletest_SOURCE_DIR ${_googletest_SOURCE_DIR} PARENT_SCOPE)
set(cmake-modules_SOURCE_DIR ${_cmake-modules_SOURCE_DIR} PARENT_SCOPE)

message(STATUS "Git submodules has been set up successfully.")
# ================================= END INITIALIZE SUBMODULES =================================

# ======================================== GOOGLETEST =========================================
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${_googletest_SOURCE_DIR})

if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()
# ====================================== END GOOGLETEST =======================================
