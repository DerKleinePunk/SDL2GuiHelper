message(STATUS "working on map dir")

FILE (GLOB MAP_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(MAP_SRCS ${MAP_SRCS} PARENT_SCOPE)