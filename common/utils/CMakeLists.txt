message(STATUS "working on utils dir")

FILE (GLOB SRCS "commonutils.cpp" "Tokenizer.cpp" "Geopos.cpp")
IF(OSMSCOUT_FOUND)
    FILE (GLOB SRCS2 "osmsoutlogger.cpp")
ENDIF()

message(STATUS "${OSMSCOUT_FOUND} result ${SRCS2}")

set(common_utils_SRCS "${SRCS};${SRCS2}" PARENT_SCOPE)