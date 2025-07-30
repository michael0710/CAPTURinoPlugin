# CMakeLists.txt for the compatibility layer for operating systems compliant
# with the POSIX standard
add_subdirectory(diag)

# generate the compatibility layer library
file(GLOB ALL_POSIX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(PosixCompatLayer STATIC ${ALL_POSIX_SRCS})
target_include_directories(PosixCompatLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../compat)
target_include_directories(PosixCompatLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(PosixCompatLayer PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(PosixCompatLayer PUBLIC PosixDiagnosis)

target_link_libraries(PosixCompatLayer PUBLIC libmodules)
target_include_directories(PosixCompatLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

