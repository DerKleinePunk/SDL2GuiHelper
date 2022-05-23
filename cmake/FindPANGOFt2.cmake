# pangoft2
PKG_CHECK_MODULES(PC_PANGOFT2 QUIET pangoft2)
FIND_PATH(PANGOFt2_INCLUDE_DIRS
    NAMES pango/pangoft2.h
    HINTS ${PC_PANGOFT2_INCLUDEDIR}
          ${PC_PANGOFT2_INCLUDE_DIRS}
          $ENV{PANGOFT2_HOME}/include
          $ENV{PANGOFT2_ROOT}/include
          /pangoft2/include
          ${PANGO_INCLUDE_HINTS}
    PATH_SUFFIXES pango pango-1.0 pangoft2 libpangoft2-1.0 pangoft21.0
)

FIND_LIBRARY(PANGOFt2_LIBRARIES
    NAMES pangoft2 pangoft2-1.0 libpangoft2-1.0
    HINTS ${PC_PANGOFT2_LIBDIR}
          ${PC_PANGOFT2_LIBRARY_DIRS}
          $ENV{PANGOFT2_HOME}/lib
          $ENV{PANGOFT2_ROOT}/lib
          ${PANGO_LIBRARY_HINTS}
    PATH_SUFFIXES pango pangoft2 libpangoft2-1.0 pangoft21.0
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PANGOFt2 DEFAULT_MSG PANGOFt2_INCLUDE_DIRS PANGOFt2_LIBRARIES)
MARK_AS_ADVANCED(PANGOFt2_INCLUDE_DIRS PANGOFt2_LIBRARIES)