# CMakeLists.txt for the Capturino plugin
# Version 3.13.0 is required for the target_link_libraries() function
# to link libraries against executables in another directory
cmake_minimum_required(VERSION 3.13.0)

project(CapturinoPlugin VERSION 0.1.0 LANGUAGES C)

# gather platform specific source files
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "MSVC compiler will be used for this project")
    message(STATUS "adding the Win folder to the build sources")
    add_compile_options(/std:c11 /W4 /we4020 /we4047)
    set(MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/win_main.c)
    set(COMPATIBILITY_LAYER WinCompatLayer)
    set(COMPATIBILITY_LAYER_DIR Win)
    
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GNU compiler will be used for this project")
    message(STATUS "adding the Posix folder to the build sources")
    # use c11 standard plus GNU extensions (like POSIX)
    add_compile_options(-std=gnu11 -Wall -Wextra)
    set(MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/posix_main.c)
    set(COMPATIBILITY_LAYER PosixCompatLayer)
    set(COMPATIBILITY_LAYER_DIR Posix)

else()
    message(STATUS "CMAKE_C_COMPILER_ID is ${CMAKE_C_COMPILER_ID}")
    message(FATAL_ERROR "Unknown compiler for this project")

endif()

add_subdirectory(generic)
add_subdirectory(lib)
add_subdirectory(${COMPATIBILITY_LAYER_DIR})

# generate the executable
add_executable(CapturinoPlugin ${MAIN_SRC})
set_target_properties(CapturinoPlugin PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(CapturinoPlugin PUBLIC generic)
target_include_directories(CapturinoPlugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/generic)

target_link_libraries(CapturinoPlugin PUBLIC ${COMPATIBILITY_LAYER})
target_include_directories(CapturinoPlugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/compat)

# generate the documentation
# Custom command to convert .svg files to .pdf using Inkscape
file(GLOB SVG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/docsrcs/*.svg)
foreach(SVG_FILE ${SVG_FILES})
    get_filename_component(SVG_FILENAME ${SVG_FILE} NAME_WE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/build/docs/imgs/${SVG_FILENAME}.pdf
        COMMAND inkscape ${SVG_FILE} --export-filename=${CMAKE_CURRENT_SOURCE_DIR}/build/docs/imgs/${SVG_FILENAME}.pdf --export-pdf-version=1.5
        DEPENDS ${SVG_FILE}
        COMMENT "Converting ${SVG_FILE} to PDF using Inkscape"
        VERBATIM
    )
    list(APPEND PDF_FILES ${CMAKE_CURRENT_SOURCE_DIR}/build/docs/imgs/${SVG_FILENAME}.pdf)
endforeach()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/build/docs
    COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}/build/docs/latex make
    DEPENDS ${PDF_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating documentation with Doxygen"
    VERBATIM
)

add_custom_target(docs ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/build/docs)
