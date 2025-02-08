# Trouver PortAudio
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h
    PATHS
    /usr/include
    /usr/local/include
    /opt/homebrew/include # Apple Silicon
    /opt/local/include # MacPorts
    "C:/portaudio/include" # Windows
)

find_library(PORTAUDIO_LIBRARY
    NAMES portaudio portaudio_x64
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/homebrew/lib # Apple Silicon
    /opt/local/lib # MacPorts
    "C:/portaudio/lib" # Windows
)

if(NOT PORTAUDIO_INCLUDE_DIR)
    message(FATAL_ERROR "PortAudio headers not found! Install with:\n"
        "  macOS: brew install portaudio\n"
        "  Linux: sudo apt install portaudio19-dev\n"
        "  Windows: Download from http://files.portaudio.com/archives/pa_stable_v190700_20210406.tgz")
endif()

if(NOT PORTAUDIO_LIBRARY)
    message(FATAL_ERROR "PortAudio library not found! Follow installation instructions above.")
endif()

# Trouver Opus via Conan
find_package(Opus REQUIRED CONFIG)

add_executable(VoiceClient
    src/AudioStream.cpp
    src/OpusCodec.cpp
    src/NetworkClient.cpp
    src/main_client.cpp
)

target_include_directories(VoiceClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PORTAUDIO_INCLUDE_DIR}
)

target_link_libraries(VoiceClient PRIVATE
    ${PORTAUDIO_LIBRARY}
    Opus::opus
)

# Dépendances spécifiques
if(APPLE)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    target_link_libraries(VoiceClient PRIVATE
        ${AUDIO_TOOLBOX}
        ${CORE_AUDIO}
    )
elseif(WIN32)
    target_link_libraries(VoiceClient PRIVATE winmm ws2_32)
else()
    target_link_libraries(VoiceClient PRIVATE pthread asound)
endif()
