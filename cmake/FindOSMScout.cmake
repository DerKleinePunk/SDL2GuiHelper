
SET(OSMSCOUT_SEARCH_PATHS
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    ${OSMSCOUT_PATH}
    $ENV{OSMSCOUT_PATH}
    F:/Mine/OpenSource/libosmscout-code/build/MinGW/Debug
    F:/Mine/OpenSource/libosmscout-code/build/MinGW/Release
)

MESSAGE(STATUS "OSMScout ${OSMSCOUT_SEARCH_PATHS}")

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(PATH_SUFFIXES lib64 lib/x64 lib)
else() 
	set(PATH_SUFFIXES lib/x86 lib)
endif() 

FIND_PATH(OSMSCOUT_INCLUDE_DIR osmscout/Database.h
    HINTS
    PATH_SUFFIXES include
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

FIND_LIBRARY(OSMSCOUT_LIBRARY_DEBUG
    NAMES osmscoutd
    HINTS
        ENV OSMSCOUT_PATH
    PATH_SUFFIXES lib lib64
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

FIND_LIBRARY(OSMSCOUT_LIBRARY_RELEASE
    NAMES osmscout
    HINTS
        ENV OSMSCOUT_PATH
    PATH_SUFFIXES lib lib64
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

FIND_LIBRARY(OSMSCOUT_LIBRARY_MAP_DEBUG
    NAMES osmscout_mapd
    HINTS
        ENV OSMSCOUT_PATH
    PATH_SUFFIXES lib lib64
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

FIND_LIBRARY(OSMSCOUT_LIBRARY_MAP_RELEASE
    NAMES osmscout_map
    HINTS
        ENV OSMSCOUT_PATH
    PATH_SUFFIXES lib lib64
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

FIND_LIBRARY(OSMSCOUT_LIBRARY_MAP_CAIRO_DEBUG
    NAMES osmscout_map_cairod
    HINTS
        ENV OSMSCOUT_PATH
    PATH_SUFFIXES lib lib64
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

FIND_LIBRARY(OSMSCOUT_LIBRARY_MAP_CAIRO_RELEASE
    NAMES osmscout_map_cairo
    HINTS
        ENV OSMSCOUT_PATH
    PATH_SUFFIXES lib lib64
    PATHS ${OSMSCOUT_SEARCH_PATHS}
)

INCLUDE(FindPackageHandleStandardArgs)

SET(OSMSCOUT_LIBRARIES_DEBUG ${OSMSCOUT_LIBRARY_DEBUG} ${OSMSCOUT_LIBRARY_MAP_DEBUG} ${OSMSCOUT_LIBRARY_MAP_CAIRO_DEBUG})
if(NOT OSMSCOUT_LIBRARY_DEBUG AND OSMSCOUT_LIBRARY_RELEASE)
    set(OSMSCOUT_LIBRARY_DEBUG ${OSMSCOUT_LIBRARY_RELEASE})
endif()
if(NOT OSMSCOUT_LIBRARY_MAP_DEBUG AND OSMSCOUT_LIBRARY_MAP_RELEASE)
    set(OSMSCOUT_LIBRARY_MAP_DEBUG ${OSMSCOUT_LIBRARY_MAP_RELEASE})
endif()
if(NOT OSMSCOUT_LIBRARY_MAP_CAIRO_DEBUG AND OSMSCOUT_LIBRARY_MAP_CAIRO_RELEASE)
    set(OSMSCOUT_LIBRARY_MAP_CAIRO_DEBUG ${OSMSCOUT_LIBRARY_MAP_CAIRO_RELEASE})
endif()
MESSAGE(STATUS "OSMScout debug ${OSMSCOUT_LIBRARIES_DEBUG}")
SET(OSMSCOUT_LIBRARIES_RELEASE ${OSMSCOUT_LIBRARY_RELEASE} ${OSMSCOUT_LIBRARY_MAP_RELEASE} ${OSMSCOUT_LIBRARY_MAP_CAIRO_RELEASE})
MESSAGE(STATUS "OSMScout release ${OSMSCOUT_LIBRARIES_RELEASE}")
set(OSMSCOUT_INCLUDE_DIRS ${OSMSCOUT_INCLUDE_DIR})

MESSAGE(STATUS "OSMScout Buildtype ${CMAKE_BUILD_TYPE}")

set(OSMSCOUT_LIBRARIES
        debug ${OSMSCOUT_LIBRARY_DEBUG} 
        debug ${OSMSCOUT_LIBRARY_MAP_DEBUG}
        debug ${OSMSCOUT_LIBRARY_MAP_CAIRO_DEBUG}
        optimized ${OSMSCOUT_LIBRARY_RELEASE}
        optimized ${OSMSCOUT_LIBRARY_MAP_RELEASE}
        optimized ${OSMSCOUT_LIBRARY_MAP_CAIRO_RELEASE}
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSMSCOUT REQUIRED_VARS OSMSCOUT_LIBRARIES OSMSCOUT_INCLUDE_DIRS)
