cmake_minimum_required(VERSION 3.20)
project(VoiceChat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Conan integration: Use toolchain via command line for Conan 2.x
# (Don't include conan_toolchain.cmake here if passing via command line)

# Add custom module path so CMake can find FindPortAudio.cmake
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Find system PortAudio using the custom module.
find_package(PortAudio REQUIRED)

# Find Conan-managed dependencies.
find_package(asio REQUIRED)
find_package(Opus REQUIRED)

# Server executable
add_executable(server
    server.cpp
    main_server.cpp
    client_session.cpp
)
target_link_libraries(server PRIVATE 
    asio::asio 
    Opus::opus
)

# Client executable
add_executable(client
    client.cpp
    main_client.cpp
)
target_link_libraries(client PRIVATE 
    asio::asio 
    Opus::opus 
    ${PortAudio_LIBRARIES}  # Use the variable set by FindPortAudio.cmake
)
target_include_directories(client PRIVATE ${PortAudio_INCLUDE_DIRS})
