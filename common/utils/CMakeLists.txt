message(STATUS "working on utils dir")

FILE (GLOB_RECURSE SRCS "*.cpp")

set(common_utils_SRCS ${SRCS} PARENT_SCOPE)