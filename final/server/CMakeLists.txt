add_executable(VoiceServer
    src/ClientSession.cpp
    src/Server.cpp
    src/main_server.cpp
)

# Ajout de la dépendance ASIO via Conan
find_package(asio REQUIRED CONFIG)

target_include_directories(VoiceServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Lien avec ASIO (header-only mais nécessaire pour les includes)
target_link_libraries(VoiceServer PRIVATE
    asio::asio
)

# Dépendances système
if(WIN32)
    target_link_libraries(VoiceServer PRIVATE ws2_32 mswsock)
else()
    target_link_libraries(VoiceServer PRIVATE pthread)
endif()
