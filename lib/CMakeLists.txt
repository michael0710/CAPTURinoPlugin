# CMakeLists.txt for OS independent library modules
# add the sources from the lib directory to a library with no other dependencies
file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(libmodules STATIC ${ALL_SRCS})
set_target_properties(libmodules PROPERTIES LINKER_LANGUAGE C)
target_include_directories(libmodules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libmodules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../compat)
