cmake_minimum_required(VERSION 2.8.11)

set(PROJECT_NAME airwave)
project(${PROJECT_NAME})

# Project version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_PATCH 1)

# Set plugin shared library base name
set(PLUGIN_BASENAME ${PROJECT_NAME}-plugin)

# Set host binary base name
set(HOST_BASENAME ${PROJECT_NAME}-host)

# Set installation path
set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "")

# Check for 64-bit platform
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(PLATFORM_64BIT 1)
endif()

# Generate config header
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/src/common/config.h
)


# Check the build type and ask the user to set concrete one
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
   message(WARNING "CMAKE_BUILD_TYPE is not set, forcing to RelWithDebInfo")
endif()


# Set compiler flags
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g3")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
endif()


# Setup path, where CMake would search for additional modules
set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

# Configure the VST SDK path
set(VSTSDK_PATH ${PROJECT_SOURCE_DIR}/VST3\ SDK CACHE PATH
	"Path to the Steinberg VST Audio Plugins SDK")

message(STATUS "VSTSDK_PATH is set to " ${VSTSDK_PATH})

find_path(VSTSDK_INCLUDE_DIR NAMES aeffect.h aeffectx.h
	PATHS "${VSTSDK_PATH}/pluginterfaces/vst2.x/")

if(NOT VSTSDK_INCLUDE_DIR)
	message(FATAL_ERROR "VST SDK is not found. You should set the VSTSDK_PATH variable "
			"to the directory, where your copy of the VST SDK is located.")
endif()

message(STATUS "VST SDK headers are found in ${VSTSDK_INCLUDE_DIR}")

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(src/plugin)
add_subdirectory(src/host)
add_subdirectory(src/manager)
