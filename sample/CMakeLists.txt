message(STATUS "working on sample dir")

project("SDL2GuiHelperSample")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (project_BIN ${PROJECT_NAME})
else()
    SET (project_BIN ${PROJECT_NAME}.bin)
endif()

INCLUDE_DIRECTORIES("${MAINFOLDER}/sample/")
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

FILE(GLOB SRCS "*.cpp")

add_executable(${project_BIN} ${SRCS} ${GUI_SRCS} ${MAIN_common_SRCS})

TARGET_LINK_LIBRARIES(${project_BIN} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})

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
