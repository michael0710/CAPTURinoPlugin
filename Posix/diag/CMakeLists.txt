# add compiler specific options
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # use c11 standard plus GNU extensions (like POSIX)
    add_compile_options(-std=gnu11 -Wall -Wextra)
else()
    message(STATUS "CMAKE_C_COMPILER_ID is ${CMAKE_C_COMPILER_ID}")
    message(FATAL_ERROR "Unknown compiler for this project")
endif()

file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
# generate the compatibility layer library
add_library(PosixDiagnosis STATIC ${ALL_SRCS})
set_target_properties(PosixDiagnosis PROPERTIES LINKER_LANGUAGE C)
target_include_directories(PosixDiagnosis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(PosixDiagnosis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../compat)
