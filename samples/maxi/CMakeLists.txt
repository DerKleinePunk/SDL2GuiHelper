message(STATUS "working on maxi dir")

project("SampleMaxi")

FIND_PACKAGE(OSMScout REQUIRED)
if(OSMScout_FOUND)
    FIND_PACKAGE(Cairo REQUIRED)
    FIND_PACKAGE(PANGO REQUIRED)
    MESSAGE(STATUS "OSMScout ${OSMSCOUT_INCLUDE_DIRS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBOSMSCOUT")
endif()
FIND_PACKAGE(SQLITE3 REQUIRED)

ADD_SUBDIRECTORY(${MAINFOLDER}/common ${MAINFOLDER}/build/${TARGET}/maxi)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (project_BIN ${PROJECT_NAME})
else()
    SET (project_BIN ${PROJECT_NAME}.bin)
endif()

INCLUDE_DIRECTORIES("${MAINFOLDER}/samples/maxi")
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

if(OSMScout_FOUND)
    INCLUDE_DIRECTORIES(${OSMSCOUT_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${PANGO_INCLUDE_DIRS})
ENDIF()

FILE(GLOB SRCS "*.cpp")

add_executable(${project_BIN} ${SRCS} ${GUI_SRCS} ${MAIN_common_SRCS} ${MAP_SRCS} ${common_utils_SRCS_OSMSCOUT} ${common_database_SRCS} ${common_utils_SRCS_SDL2} ${common_SDL2_gfx_SRCS})

TARGET_LINK_LIBRARIES(${project_BIN} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

if(OSMSCOUT_FOUND)
    TARGET_LINK_LIBRARIES(${project_BIN} ${OSMScout_LIBRARIES})
    TARGET_LINK_LIBRARIES(${project_BIN} ${CAIRO_LIBRARIES})
    TARGET_LINK_LIBRARIES(${project_BIN} ${PANGO_LIBRARIES})
endif()

if(SQLITE3_FOUND)
    TARGET_LINK_LIBRARIES(${project_BIN} ${SQLITE3_LIBRARIES})
endif()

IF("${TARGET}" STREQUAL "PI")
    # Why i need this ?!? Cross Compile
    TARGET_LINK_LIBRARIES(${project_BIN} ${RPIROOT}/opt/vc/lib/libmmal_core.so)
    TARGET_LINK_LIBRARIES(${project_BIN} ${RPIROOT}/opt/vc/lib/libmmal_util.so)
    TARGET_LINK_LIBRARIES(${project_BIN} ${RPIROOT}/opt/vc/lib/libmmal_vc_client.so)
    TARGET_LINK_LIBRARIES(${project_BIN} ${RPIROOT}/opt/vc/lib/libvcos.so)
    TARGET_LINK_LIBRARIES(${project_BIN} ${RPIROOT}/opt/vc/lib/libvchiq_arm.so)
    TARGET_LINK_LIBRARIES(${project_BIN} ${RPIROOT}/opt/vc/lib/libvcsm.so)
ENDIF()

message(STATUS "Generating Copy Rule for Resources")
set(MAINFONTSOURCE "${MAINFOLDER}/resources/font/Inconsolata-Bold.ttf")
add_custom_command(TARGET ${project_BIN} POST_BUILD # Adds a post-build event to MyTest
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
                       "${MAINFONTSOURCE}"      # <--this is in-file
                       $<TARGET_FILE_DIR:${project_BIN}>)
set(MAINFONTSOURCE "${MAINFOLDER}/resources/font/Inconsolata-Regular.ttf")
add_custom_command(TARGET ${project_BIN} POST_BUILD # Adds a post-build event to MyTest
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
                       "${MAINFONTSOURCE}"      # <--this is in-file
                       $<TARGET_FILE_DIR:${project_BIN}>)
