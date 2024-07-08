
cmake_minimum_required (VERSION 3.4.3)

#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(SMTG_VST3_SDK)

#-------------------------------------------------------------------------------
# SDK Project
#-------------------------------------------------------------------------------
project(vstsdk)

setupPlatformToolset()

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Here you can define where the VST 3 SDK is located
set(SDK_ROOT "${ROOT}")
set(public_sdk_SOURCE_DIR ${SDK_ROOT}/public.sdk)
set(pluginterfaces_SOURCE_DIR ${SDK_ROOT}/pluginterfaces)

include_directories(${ROOT} ${SDK_ROOT})
set(SDK_IDE_HOSTING_EXAMPLES_FOLDER FOLDER "HostingExamples")
add_subdirectory(public.sdk/samples/vst-hosting)

#-------------------------------------------------------------------------------
# Projects
#-------------------------------------------------------------------------------
set(SDK_IDE_LIBS_FOLDER FOLDER "Libraries")

#---Add base libraries---------------------------
set(VST_SDK TRUE) # used for Base module which provides only a subset of Base for VST-SDK
add_subdirectory(pluginterfaces)
add_subdirectory(base)
add_subdirectory(public.sdk)
add_subdirectory(public.sdk/source/vst/interappaudio)


#-------------------------------------------------------------------------------
# Add our folder to build
#-------------------------------------------------------------------------------

add_subdirectory(melb_inst_vst3)



set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Add c++ wave reading lib
add_subdirectory(submodules/AudioFile)

# Add rapidJson library
option(RAPIDJSON_BUILD_EXAMPLES OFF)
add_subdirectory(submodules/rapidjson)

#-------------------------------------------------------------------------------
# IDE sorting
#-------------------------------------------------------------------------------
set_target_properties(sdk PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(base PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(pluginterfaces PROPERTIES ${SDK_IDE_LIBS_FOLDER})