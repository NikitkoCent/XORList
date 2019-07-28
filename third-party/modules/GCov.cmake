function(get_target_gcov_components TARGET_NAME)
    get_target_property(TARGET_SRC_DIR ${TARGET_NAME} SOURCE_DIR)
    get_target_property(TARGET_BIN_DIR ${TARGET_NAME} BINARY_DIR)
    get_target_property(TARGET_SOURCES ${TARGET_NAME} SOURCES)

    set(TARGET_GCOV_ROOT "${TARGET_BIN_DIR}/CMakeFiles/${TARGET_NAME}.dir")

    set(TARGET_GCOV_OBJECTS)

    foreach(SRC_PATH ${TARGET_SOURCES})
        if (NOT IS_ABSOLUTE ${SRC_PATH})
            set(SRC_PATH "${TARGET_SRC_DIR}/${SRC_PATH}")            
        endif (NOT IS_ABSOLUTE ${SRC_PATH})

        file(RELATIVE_PATH SRC_PATH "${TARGET_SRC_DIR}" "${SRC_PATH}")

        string(REPLACE "/../" "/__/" SRC_PATH "${SRC_PATH}")

        string(REGEX REPLACE "^../" "__/" SRC_PATH "${SRC_PATH}")

        list(APPEND TARGET_GCOV_OBJECTS "${TARGET_GCOV_ROOT}/${SRC_PATH}")
    endforeach(SRC_PATH ${TARGET_SOURCES})

    set(${TARGET_NAME}_GCOV_ROOT "${TARGET_GCOV_ROOT}" PARENT_SCOPE)
    set(${TARGET_NAME}_GCOV_OBJECTS "${TARGET_GCOV_OBJECTS}" PARENT_SCOPE)
endfunction(get_target_gcov_components TARGET_NAME)

function(setup_target_for_coverage_gcov)
    #set(options NONE)
    set(oneValueArgs TARGET NAME)
    #set(multiValueArgs NONE)

    cmake_parse_arguments(GCOV "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_target_gcov_components(${GCOV_TARGET})

    set(GCOV_SOURCES)

    foreach(SRC_PATH ${${GCOV_TARGET}_GCOV_OBJECTS})
        list(APPEND GCOV_SOURCES "${SRC_PATH}.")
    endforeach(SRC_PATH ${${GCOV_TARGET}_GCOV_OBJECTS})

    add_custom_target(${GCOV_NAME}
                      COMMAND $<TARGET_FILE:${GCOV_TARGET}>
                      COMMAND gcov -lp ${GCOV_SOURCES}
                      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                      DEPENDS ${GCOV_TARGET}
                      COMMENT "Collecting gcov coverage...")

endfunction()

function(print_target_info)

    #set(options NONE)
    set(oneValueArgs TARGET)
    #set(multiValueArgs NONE)

    cmake_parse_arguments(PTI "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "project build dir: ${CMAKE_BINARY_DIR}")

    get_target_property(TARGET_SRC_DIR ${PTI_TARGET} SOURCE_DIR)
    message(STATUS "${PTI_TARGET} source dir: ${TARGET_SRC_DIR}")

    get_target_property(TARGET_BIN_DIR ${PTI_TARGET} BINARY_DIR)
    message(STATUS "${PTI_TARGET} binary dir: ${TARGET_BIN_DIR}")

    set(TARGET_OBJ_DIR "${TARGET_BIN_DIR}/CMakeFiles/${PTI_TARGET}.dir")
    message(STATUS "${PTI_TARGET} objects dir: ${TARGET_OBJ_DIR}")

    get_target_property(TARGET_SOURCES ${PTI_TARGET} SOURCES)

    set(TARGET_FULL_PATH_SOURCES)

    foreach(SRC_FILE_PATH ${TARGET_SOURCES})
        if (NOT IS_ABSOLUTE ${SRC_FILE_PATH})
            set(SRC_FILE_PATH "${TARGET_SRC_DIR}/${SRC_FILE_PATH}")            
        endif (NOT IS_ABSOLUTE ${SRC_FILE_PATH})

        file(RELATIVE_PATH SRC_FILE_PATH "${TARGET_SRC_DIR}" "${SRC_FILE_PATH}")

        string(REPLACE "/../" "/__/" SRC_FILE_PATH "${SRC_FILE_PATH}")

        string(REGEX REPLACE "^../" "__/" SRC_FILE_PATH "${SRC_FILE_PATH}")

        list(APPEND TARGET_FULL_PATH_SOURCES ${SRC_FILE_PATH})
    endforeach(SRC_FILE_PATH ${TARGET_SOURCES})

    message(STATUS "${PTI_TARGET} full path sources: ${TARGET_FULL_PATH_SOURCES}")

endfunction(print_target_info)

