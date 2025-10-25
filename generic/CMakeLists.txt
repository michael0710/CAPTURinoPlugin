# CMakeLists.txt for the OS independent application components
# add the sources from the generic directory to a library
add_subdirectory(capturelib)

file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(generic STATIC ${ALL_SRCS})
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../compat)
set_target_properties(generic PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(generic PUBLIC libmodules)
target_link_libraries(generic PUBLIC capturelib)
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/capturelib)
