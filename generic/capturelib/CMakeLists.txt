# CMakeLists.txt for the OS independent capture library components
# add the sources from the capturelib directory to a library
file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(capturelib STATIC ${ALL_SRCS})
target_include_directories(capturelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(capturelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(capturelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../compat)
set_target_properties(capturelib PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(capturelib PUBLIC libmodules)
target_include_directories(capturelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
