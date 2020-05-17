message(STATUS "common")

FILE (GLOB MAIN_common_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

ADD_SUBDIRECTORY(easylogging)
ADD_SUBDIRECTORY(exception)
ADD_SUBDIRECTORY(SDL2_gfx)
ADD_SUBDIRECTORY(utils)

set(MAIN_common_SRCS ${MAIN_common_SRCS} ${easylogging_SRCS} ${exception_SRCS} ${common_SDL2_gfx_SRCS} ${common_utils_SRCS} PARENT_SCOPE)