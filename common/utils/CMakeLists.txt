message(STATUS "working on utils dir")

FILE (GLOB SRCS "commonutils.cpp" "Tokenizer.cpp" "Geopos.cpp")
IF(OSMSCOUT_FOUND)
    FILE (GLOB SRCS2 "osmsoutlogger.cpp")
ENDIF()

IF(SDL2_FOUND)
    FILE (GLOB SRCS3 "SDL2Helper.cpp")
ENDIF()

message(STATUS "${OSMSCOUT_FOUND} result ${SRCS2}")
message(STATUS "${SDL2_FOUND} result ${SRCS3}")

set(common_utils_SRCS ${SRCS} PARENT_SCOPE)
set(common_utils_SRCS_SDL2 ${SRCS3} PARENT_SCOPE)
set(common_utils_SRCS_OSMSCOUT ${SRCS2} PARENT_SCOPE)