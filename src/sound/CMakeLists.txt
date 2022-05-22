message(STATUS "working on sound dir")

FILE (GLOB SOUND_MINI_SRCS "MiniAudioManager.cpp")
FILE (GLOB SOUND_MUSIK_SRCS "MusikAudioManager.cpp" "MediaStream.cpp")

set(SOUND_MINI_SRCS ${SOUND_MINI_SRCS} PARENT_SCOPE)
set(SOUND_MUSIK_SRCS ${SOUND_MUSIK_SRCS} PARENT_SCOPE)