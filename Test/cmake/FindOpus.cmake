# FindOpus.cmake
#
# Ce module recherche la bibliothèque Opus et ses headers.
# Il définit :
#   Opus_FOUND         - TRUE si trouvée.
#   Opus_INCLUDE_DIRS  - Répertoire(s) contenant les headers d'Opus.
#   Opus_LIBRARIES     - La ou les bibliothèques Opus à lier.

# On recherche d'abord le header. Certains packages installent le header dans un sous-répertoire (e.g. opus/opus.h)
find_path(OPUS_INCLUDE_DIR
  NAMES opus/opus.h opus.h
  HINTS
    ${CONAN_OPUS_ROOT}           # Si Conan définit une variable d'environnement ou un cache
    /opt/homebrew/include         # Mac Apple Silicon (et parfois Mac Intel avec Homebrew)
    /usr/local/include            # Mac Intel et Linux
    /usr/include                  # Linux
)

# On recherche ensuite la bibliothèque.
find_library(OPUS_LIBRARY
  NAMES opus opus_static
  HINTS
    ${CONAN_OPUS_ROOT}/lib       # Si Conan définit ce chemin
    /opt/homebrew/lib             # Mac Apple Silicon
    /usr/local/lib                # Mac Intel et Linux
    /usr/lib                      # Linux
)

if(OPUS_INCLUDE_DIR AND OPUS_LIBRARY)
  set(Opus_FOUND TRUE)
  set(Opus_INCLUDE_DIRS ${OPUS_INCLUDE_DIR})
  set(Opus_LIBRARIES ${OPUS_LIBRARY})
else()
  set(Opus_FOUND FALSE)
endif()

mark_as_advanced(Opus_INCLUDE_DIRS Opus_LIBRARIES)