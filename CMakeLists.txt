cmake_minimum_required(VERSION 3.10)
project(TCP_Async_Client_Server)

# Inclure les fichiers générés par Conan
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Charger les dépendances Conan
find_package(asio REQUIRED)

# Serveur
add_executable(server
    ServerTCP/server.cpp
    ServerTCP/Main_server.cpp
)

# Client
add_executable(client
    ClientTCP/Client.cpp
    ClientTCP/Main_Client.cpp
)


# Lier les dépendances Conan
target_link_libraries(server asio::asio)
target_link_libraries(client asio::asio)