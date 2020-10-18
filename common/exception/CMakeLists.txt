message(STATUS "working on exception dir")

FILE (GLOB SRCS "ArgumentException.cpp" "FileNotFoundException.cpp" "IllegalStateException.cpp" "NotImplementedException.cpp" "NullPointerException.cpp")
IF(SQLITE3_FOUND)
    FILE (GLOB SRCS2 "SqliteException.cpp")
ENDIF()

set(exception_SRCS ${SRCS} PARENT_SCOPE)
set(exception_SRCS_Sqlite ${SRCS2} PARENT_SCOPE)