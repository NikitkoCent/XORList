# Possible values:
#   * LCOV  - add coverage flags to compiler/linker
#             and setup target for code coverage collection via lcov
#   * GCOV  - add coverage flags to compiler/linker
#             and setup target for code coverage collection via gcov
#   * OTHER - add coverage flags to compiler/linker
#   * OFF   - do nothing
set(COLLECT_CODE_COVERAGE "OFF" CACHE STRING "Code coverage mode (LCOV|GCOV|OTHER|OFF)")

find_package(Threads)

include(${cmake-modules_SOURCE_DIR}/EnableExtraCompilerWarnings.cmake)

add_executable(unittesting ${CMAKE_SOURCE_DIR}/include/xor_list/xor_list.h
                           list.cpp
                           iterator.cpp)

if (COLLECT_CODE_COVERAGE STREQUAL "LCOV")
    include(${cmake-modules_SOURCE_DIR}/CodeCoverage.cmake)

    append_target_coverage_flags(unittesting)
    setup_target_for_coverage_lcov(NAME collect_coverage
                                   EXECUTABLE unittesting
								   DEPENDENCIES unittesting)
elseif (COLLECT_CODE_COVERAGE STREQUAL "GCOV")
    message(FATAL_ERROR "GCOV value for COLLECT_CODE_COVERAGE variable isn't supported yet")
elseif (COLLECT_CODE_COVERAGE STREQUAL "OTHER")
    include(${cmake-modules_SOURCE_DIR}/CodeCoverage.cmake)

    append_target_coverage_flags(unittesting)
endif ()

enable_extra_compiler_warnings(unittesting)

target_link_libraries(unittesting gtest gmock_main Threads::Threads)

add_test(NAME list_test COMMAND unittesting --gtest_filter=LIST.*)
add_test(NAME iterator_test COMMAND unittesting --gtest_filter=ITERATOR.*)

add_executable(sort_performance ${CMAKE_SOURCE_DIR}/include/xor_list/xor_list.h
                                performance_list_sort.cpp)
