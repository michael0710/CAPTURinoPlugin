# CMakeLists.txt for the compatibility layer for the Windows operating system
add_subdirectory(diag)

# generate the compatibility layer library
file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(WinCompatLayer STATIC ${ALL_SRCS})
target_include_directories(WinCompatLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../compat)
target_include_directories(WinCompatLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(WinCompatLayer PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(WinCompatLayer PUBLIC WinDiagnosis)

target_link_libraries(WinCompatLayer PUBLIC libmodules)
target_include_directories(WinCompatLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

