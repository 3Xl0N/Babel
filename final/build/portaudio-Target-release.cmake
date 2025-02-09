# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(portaudio_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(portaudio_FRAMEWORKS_FOUND_RELEASE "${portaudio_FRAMEWORKS_RELEASE}" "${portaudio_FRAMEWORK_DIRS_RELEASE}")

set(portaudio_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET portaudio_DEPS_TARGET)
    add_library(portaudio_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET portaudio_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${portaudio_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${portaudio_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### portaudio_DEPS_TARGET to all of them
conan_package_library_targets("${portaudio_LIBS_RELEASE}"    # libraries
                              "${portaudio_LIB_DIRS_RELEASE}" # package_libdir
                              "${portaudio_BIN_DIRS_RELEASE}" # package_bindir
                              "${portaudio_LIBRARY_TYPE_RELEASE}"
                              "${portaudio_IS_HOST_WINDOWS_RELEASE}"
                              portaudio_DEPS_TARGET
                              portaudio_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "portaudio"    # package_name
                              "${portaudio_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${portaudio_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET portaudio::portaudio
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${portaudio_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${portaudio_LIBRARIES_TARGETS}>
                 )

    if("${portaudio_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET portaudio::portaudio
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     portaudio_DEPS_TARGET)
    endif()

    set_property(TARGET portaudio::portaudio
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${portaudio_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET portaudio::portaudio
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${portaudio_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET portaudio::portaudio
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${portaudio_LIB_DIRS_RELEASE}>)
    set_property(TARGET portaudio::portaudio
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${portaudio_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET portaudio::portaudio
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${portaudio_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(portaudio_LIBRARIES_RELEASE portaudio::portaudio)
