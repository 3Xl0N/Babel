cmake_minimum_required(VERSION 3.15)
project(VoiceChat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Conan-generated toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

add_subdirectory(client)
add_subdirectory(server)
