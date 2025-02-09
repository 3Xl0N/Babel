########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(portaudio_FIND_QUIETLY)
    set(portaudio_MESSAGE_MODE VERBOSE)
else()
    set(portaudio_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/portaudioTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${portaudio_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(portaudio_VERSION_STRING "19.7.0")
set(portaudio_INCLUDE_DIRS ${portaudio_INCLUDE_DIRS_RELEASE} )
set(portaudio_INCLUDE_DIR ${portaudio_INCLUDE_DIRS_RELEASE} )
set(portaudio_LIBRARIES ${portaudio_LIBRARIES_RELEASE} )
set(portaudio_DEFINITIONS ${portaudio_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${portaudio_BUILD_MODULES_PATHS_RELEASE} )
    message(${portaudio_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


