message(STATUS "working on database dir")

FILE (GLOB_RECURSE SRCS "*.cpp" "*.cxx" "*.cc" "*.c" "*.h" "*.hpp")

set(common_database_SRCS ${SRCS} PARENT_SCOPE)


