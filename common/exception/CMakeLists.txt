message(STATUS "working on exception dir")

FILE (GLOB exception_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(exception_SRCS ${exception_SRCS} PARENT_SCOPE)