########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(portaudio_COMPONENT_NAMES "")
if(DEFINED portaudio_FIND_DEPENDENCY_NAMES)
  list(APPEND portaudio_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES portaudio_FIND_DEPENDENCY_NAMES)
else()
  set(portaudio_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(portaudio_PACKAGE_FOLDER_RELEASE "/Users/jeancharleshouinato/.conan2/p/b/portaf43935364d7ad/p")
set(portaudio_BUILD_MODULES_PATHS_RELEASE )


set(portaudio_INCLUDE_DIRS_RELEASE "${portaudio_PACKAGE_FOLDER_RELEASE}/include")
set(portaudio_RES_DIRS_RELEASE )
set(portaudio_DEFINITIONS_RELEASE )
set(portaudio_SHARED_LINK_FLAGS_RELEASE )
set(portaudio_EXE_LINK_FLAGS_RELEASE )
set(portaudio_OBJECTS_RELEASE )
set(portaudio_COMPILE_DEFINITIONS_RELEASE )
set(portaudio_COMPILE_OPTIONS_C_RELEASE )
set(portaudio_COMPILE_OPTIONS_CXX_RELEASE )
set(portaudio_LIB_DIRS_RELEASE "${portaudio_PACKAGE_FOLDER_RELEASE}/lib")
set(portaudio_BIN_DIRS_RELEASE )
set(portaudio_LIBRARY_TYPE_RELEASE STATIC)
set(portaudio_IS_HOST_WINDOWS_RELEASE 0)
set(portaudio_LIBS_RELEASE portaudio)
set(portaudio_SYSTEM_LIBS_RELEASE )
set(portaudio_FRAMEWORK_DIRS_RELEASE )
set(portaudio_FRAMEWORKS_RELEASE CoreAudio AudioToolbox AudioUnit CoreServices Carbon)
set(portaudio_BUILD_DIRS_RELEASE )
set(portaudio_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(portaudio_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${portaudio_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${portaudio_COMPILE_OPTIONS_C_RELEASE}>")
set(portaudio_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${portaudio_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${portaudio_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${portaudio_EXE_LINK_FLAGS_RELEASE}>")


set(portaudio_COMPONENTS_RELEASE )