# gather platform specific source files
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/std:c11 /W4)
else()
    message(STATUS "CMAKE_C_COMPILER_ID is ${CMAKE_C_COMPILER_ID}")
    message(FATAL_ERROR "Unknown compiler for this project")
endif()

file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
# generate the compatibility layer library
add_library(WinDiagnosis STATIC ${ALL_SRCS})
set_target_properties(WinDiagnosis PROPERTIES LINKER_LANGUAGE C)
target_include_directories(WinDiagnosis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(WinDiagnosis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../compat)
