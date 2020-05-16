message(STATUS "working on gui dir")

FILE (GLOB GUI_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(GUI_SRCS ${GUI_SRCS} PARENT_SCOPE)