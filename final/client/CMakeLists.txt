# Dans client/CMakeLists.txt

# Recherche de PortAudio via Conan (le générateur CMakeDeps aura créé le fichier de config)
find_package(portaudio REQUIRED CONFIG)

# Recherche d’Opus (fourni également via Conan)
find_package(Opus REQUIRED CONFIG)

find_package(asio REQUIRED CONFIG)

add_executable(VoiceClient
    src/AudioStream.cpp
    src/OpusCodec.cpp
    src/NetworkClient.cpp
    src/main_client.cpp
)

target_include_directories(VoiceClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(VoiceClient PRIVATE
    portaudio::portaudio
    Opus::opus
    asio::asio
)

# Liaisons spécifiques selon la plateforme
if(APPLE)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    target_link_libraries(VoiceClient PRIVATE ${AUDIO_TOOLBOX} ${CORE_AUDIO})
elseif(WIN32)
    target_link_libraries(VoiceClient PRIVATE winmm ws2_32)
else()
    target_link_libraries(VoiceClient PRIVATE pthread asound)
endif()
