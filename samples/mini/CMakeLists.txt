message(STATUS "working on mini dir")

project("SampleMini")

ADD_SUBDIRECTORY(${MAINFOLDER}/common ${MAINFOLDER}/build/${TARGET}/mini)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (project_BIN ${PROJECT_NAME})
else()
    SET (project_BIN ${PROJECT_NAME}.bin)
endif()

INCLUDE_DIRECTORIES("${MAINFOLDER}/samples/mini")
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

FILE(GLOB SRCS "*.cpp")

add_executable(${project_BIN} ${SRCS} ${GUI_SRCS} ${MAIN_common_SRCS} ${common_utils_SRCS_SDL2} ${common_SDL2_gfx_SRCS})

TARGET_LINK_LIBRARIES(${project_BIN} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

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
set(MAINFONTSOURCE "${MAINFOLDER}/resources/images/map_menu.png")
add_custom_command(TARGET ${project_BIN} POST_BUILD # Adds a post-build event to MyTest
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
                        "${MAINFONTSOURCE}"      # <--this is in-file
                        $<TARGET_FILE_DIR:${project_BIN}>)