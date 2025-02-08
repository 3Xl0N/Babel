cmake_minimum_required(VERSION 3.20)
project(VoiceChat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option 2 : Inclure le toolchain Conan depuis le répertoire de build
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Ajouter le chemin du module personnalisé pour que CMake trouve FindPortAudio.cmake et FindOpus.cmake
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Recherche de PortAudio (via votre module personnalisé)
find_package(PortAudio REQUIRED)

# Recherche d'Opus (via votre module personnalisé FindOpus.cmake)
find_package(Opus REQUIRED)

# Recherche d'asio (géré par Conan)
find_package(asio REQUIRED)

# Définition de la cible serveur
add_executable(server
    server.cpp
    main_server.cpp
    client_session.cpp
)
target_link_libraries(server PRIVATE 
    asio::asio 
    ${Opus_LIBRARIES}
)

# Définition de la cible client
add_executable(client
    client.cpp
    main_client.cpp
)
target_link_libraries(client PRIVATE 
    asio::asio 
    ${Opus_LIBRARIES} 
    ${PortAudio_LIBRARIES}
)
target_include_directories(client PRIVATE ${PortAudio_INCLUDE_DIRS} ${Opus_INCLUDE_DIRS})
