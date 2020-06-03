message(STATUS "working on sound dir")

FILE (GLOB SOUND_MINI_SRCS "MiniAudioManager.cpp" "IAudioManager.cpp")

set(SOUND_MINI_SRCS ${SOUND_MINI_SRCS} PARENT_SCOPE)