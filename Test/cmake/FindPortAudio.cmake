# FindPortAudio.cmake
#
# Ce module recherche la bibliothèque PortAudio ainsi que son header.
# Il définit :
#   PortAudio_FOUND         - TRUE si trouvée.
#   PortAudio_INCLUDE_DIRS  - Répertoire(s) des headers.
#   PortAudio_LIBRARIES     - La bibliothèque PortAudio à lier.

# Définir une liste d'emplacements de recherche pour les headers.
set(PORTAUDIO_SEARCH_PATHS
    /opt/homebrew/include   # Pour Mac Apple Silicon (et parfois Mac Intel avec Homebrew)
    /usr/local/include      # Pour Mac Intel et Linux
    /usr/include            # Pour Linux
)

# Sur Windows, ajouter des indices basés sur les variables d'environnement.
if(WIN32)
    list(APPEND PORTAUDIO_SEARCH_PATHS "$ENV{ProgramFiles}/PortAudio/include")
    list(APPEND PORTAUDIO_SEARCH_PATHS "$ENV{ProgramFiles(x86)}/PortAudio/include")
endif()

find_path(PORTAUDIO_INCLUDE_DIR
    NAMES portaudio.h
    HINTS ${PORTAUDIO_SEARCH_PATHS}
    PATH_SUFFIXES portaudio
)

# Définir les emplacements de recherche pour la bibliothèque.
set(PORTAUDIO_SEARCH_LIB_PATHS
    /opt/homebrew/lib      # Pour Mac Apple Silicon
    /usr/local/lib         # Pour Mac Intel et Linux
    /usr/lib               # Pour Linux
)

if(WIN32)
    list(APPEND PORTAUDIO_SEARCH_LIB_PATHS "$ENV{ProgramFiles}/PortAudio/lib")
    list(APPEND PORTAUDIO_SEARCH_LIB_PATHS "$ENV{ProgramFiles(x86)}/PortAudio/lib")
endif()

find_library(PORTAUDIO_LIBRARY
    NAMES portaudio portaudio_static
    HINTS ${PORTAUDIO_SEARCH_LIB_PATHS}
)

if(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)
    set(PortAudio_FOUND TRUE)
    set(PortAudio_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})
    set(PortAudio_LIBRARIES ${PORTAUDIO_LIBRARY})
else()
    set(PortAudio_FOUND FALSE)
endif()

mark_as_advanced(PortAudio_INCLUDE_DIRS PortAudio_LIBRARIES)