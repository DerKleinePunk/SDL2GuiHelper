message(STATUS "working on exception dir")

FILE (GLOB EXCEPTION_GUI_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(EXCEPTION_GUI_SRCS ${EXCEPTION_GUI_SRCS} PARENT_SCOPE)