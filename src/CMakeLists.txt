message(STATUS "working on src dir")

FILE (GLOB MAIN_GUI_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(exception)

set(GUI_SRCS ${GUI_SRCS} ${MAIN_GUI_SRCS} ${EXCEPTION_GUI_SRCS} PARENT_SCOPE)