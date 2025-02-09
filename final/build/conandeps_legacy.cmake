message(STATUS "Conan: Using CMakeDeps conandeps_legacy.cmake aggregator via include()")
message(STATUS "Conan: It is recommended to use explicit find_package() per dependency instead")

find_package(Opus)
find_package(asio)
find_package(portaudio)

set(CONANDEPS_LEGACY  Opus::opus  asio::asio  portaudio::portaudio )