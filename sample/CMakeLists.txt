message(STATUS "working on sample dir")

project("SDL2GuiHelperSample")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (project_BIN ${PROJECT_NAME})
else()
    SET (project_BIN ${PROJECT_NAME}.bin)
endif()

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

FILE(GLOB SRCS "*.cpp")

add_executable(${project_BIN} ${SRCS} ${GUI_SRCS} ${MAIN_common_SRCS})

TARGET_LINK_LIBRARIES(${project_BIN} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
