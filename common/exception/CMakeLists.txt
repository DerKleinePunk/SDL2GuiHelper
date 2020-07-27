message(STATUS "working on exception dir")

FILE (GLOB SRCS "ArgumentException.cpp" "FileNotFoundException.cpp" "IllegalStateException.cpp" "NotImplementedException.cpp" "NullPointerException.cpp")
IF(SQLite3_FOUND)
    FILE (GLOB SRCS2 "SqliteException.cpp")
ENDIF()

set(exception_SRCS ${SRCS} ${SRCS2} PARENT_SCOPE)