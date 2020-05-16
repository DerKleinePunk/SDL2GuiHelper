message(STATUS "working on SDL2_gfx dir")

FILE (GLOB_RECURSE SRCS "*.c")

set(common_SDL2_gfx_SRCS ${SRCS} PARENT_SCOPE)