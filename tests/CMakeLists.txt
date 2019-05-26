option(COLLECT_CODE_COVERAGE_LCOV "Setup code coverage or not" OFF)

find_package(Threads)

include(${cmake-modules_SOURCE_DIR}/EnableExtraCompilerWarnings.cmake)

enable_testing()

find_package(Threads)

add_executable(unittesting ${CMAKE_SOURCE_DIR}/include/xor_list/xor_list.h
                           list.cpp
                           iterator.cpp)

if (COLLECT_CODE_COVERAGE_LCOV)
    include(${cmake-modules_SOURCE_DIR}/CodeCoverage.cmake)

    append_target_coverage_flags(unittesting)
    setup_target_for_coverage_lcov(NAME collect_coverage
                                   EXECUTABLE unittesting
								   DEPENDENCIES unittesting)
endif (COLLECT_CODE_COVERAGE_LCOV)
enable_extra_compiler_warnings(unittesting)

target_link_libraries(unittesting gtest gmock_main Threads::Threads)

add_test(NAME list_test COMMAND unittesting --gtest_filter=LIST.*)
add_test(NAME iterator_test COMMAND unittesting --gtest_filter=ITERATOR.*)

add_executable(sort_performance ${CMAKE_SOURCE_DIR}/include/xor_list/xor_list.h
                                performance_list_sort.cpp)
