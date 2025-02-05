# FindPortAudio.cmake
#
# This module searches for the PortAudio library and its header.
# It defines:
#   PortAudio_FOUND         - TRUE if found.
#   PortAudio_INCLUDE_DIRS  - Directory(ies) for headers.
#   PortAudio_LIBRARIES     - The PortAudio library to link against.

find_path(PORTAUDIO_INCLUDE_DIR
  NAMES portaudio.h
  HINTS
    /opt/homebrew/include
    /usr/local/include
    /usr/include
  PATH_SUFFIXES portaudio)

find_library(PORTAUDIO_LIBRARY
  NAMES portaudio
  HINTS
    /opt/homebrew/lib
    /usr/local/lib
    /usr/lib)

if(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)
  set(PortAudio_FOUND TRUE)
  set(PortAudio_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})
  set(PortAudio_LIBRARIES ${PORTAUDIO_LIBRARY})
else()
  set(PortAudio_FOUND FALSE)
endif()

mark_as_advanced(PortAudio_INCLUDE_DIRS PortAudio_LIBRARIES)
