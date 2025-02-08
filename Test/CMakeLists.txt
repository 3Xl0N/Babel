cmake_minimum_required(VERSION 3.20)
project(VoiceChat LANGUAGES CXX)

# Spécifiez la norme C++ désirée
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# IMPORTANT : Le fichier toolchain Conan (conan_toolchain.cmake) doit être passé
# en ligne de commande via -DCMAKE_TOOLCHAIN_FILE et non inclus ici.

# Ajout des répertoires contenant vos modules CMake personnalisés.
# Placez vos fichiers FindPortAudio.cmake et éventuellement FindOpus.cmake dans le dossier "cmake" du projet.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Recherche de PortAudio via un module FindPortAudio (à créer)
find_package(PortAudio REQUIRED)

# Recherche d'Opus.
# Selon la recette Conan, Opus peut fournir un fichier de configuration CMake (si c'est le cas, utilisez CONFIG)
# ou vous pouvez utiliser votre module FindOpus si vous l'avez écrit.
find_package(Opus REQUIRED)

# Recherche d'asio fourni par Conan via le générateur CMakeDeps (mode CONFIG)
find_package(asio CONFIG REQUIRED)

# Définition de la cible serveur
add_executable(server
    server.cpp
    main_server.cpp
    client_session.cpp
)
target_link_libraries(server PRIVATE 
    asio::asio 
    ${Opus_LIBRARIES}  # Ces variables doivent être définies par le package Opus ou votre module FindOpus.
)

# Définition de la cible client
add_executable(client
    client.cpp
    main_client.cpp
)
target_link_libraries(client PRIVATE 
    asio::asio 
    ${Opus_LIBRARIES} 
    ${PortAudio_LIBRARIES}  # Variables définies dans votre module FindPortAudio.
)
target_include_directories(client PRIVATE 
    ${PortAudio_INCLUDE_DIRS} 
    ${Opus_INCLUDE_DIRS}       # Encore une fois, ces variables doivent être correctement définies.
)
