cmake_minimum_required(VERSION 2.8.6)

# Project name
set(PROJECT_NAME airwave)
project(${PROJECT_NAME})

# Project version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 4)
set(VERSION_PATCH 0)

# Internal communication protocol version. Increase this value, when you break
# backward compatibility.
set(PROTOCOL_VERSION 1)

# Set plugin shared library base name
set(PLUGIN_BASENAME ${PROJECT_NAME}-plugin)

# Set plugin shared library location
set(PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/
	CACHE PATH "Path to the plugin shared library.")

# Set host binary base name
set(HOST_BASENAME ${PROJECT_NAME}-host)

# Set host binary location
set(HOST_PATH ${CMAKE_INSTALL_PREFIX}/bin/
	CACHE PATH "Path to the host binary.")

# Configure logger
option(ENABLE_LOGGER "Enable sending log message to the airwave-utility." ON)

set(LOG_SOCKET_PATH "/tmp/${PROJECT_NAME}.sock"
	CACHE PATH "Path to the log socket.")

# Run cmake with parameter -DBUILD_UTILITY=OFF for disable build the
# GUI utility application.
option(BUILD_UTILITY "Build the GUI utility (Qt4)." ON)


# Create config.h header
configure_file(
	${PROJECT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# Check build type and offer user to set concrete one
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
   message(WARNING "CMAKE_BUILD_TYPE is not set, forcing to RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)

# Configure compiler flags for build profiles
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3 -DDEBUG_BUILD")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g3")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(CMAKE_COMPILER_IS_GNUCXX)

# Setup path, where CMake would search for additional modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})


# Configure the VST SDK path
set(VSTSDK_PATH ${PROJECT_SOURCE_DIR}/vstsdk2.4 CACHE PATH
	"Path to the Steinberg VST 2.4 SDK")

message(STATUS "VSTSDK_PATH is set to " ${VSTSDK_PATH})

find_path(VSTSDK_INCLUDE_DIR NAMES aeffect.h aeffectx.h
	PATHS "${VSTSDK_PATH}/pluginterfaces/vst2.x/")

if(NOT VSTSDK_INCLUDE_DIR)
	message(FATAL_ERROR "VST SDK 2.4 is not found. You should copy the VST SDK "
		"contents to the VSTSDK_PATH location or change the VSTSDK_PATH value.")
endif()

message(STATUS "VST SDK headers are found in ${VSTSDK_INCLUDE_DIR}")


# Run cmake with parameter -DREDEFINE_CDECL=OFF for disable redefinition of
# __cdecl for suppressing compiler warnings. Then the sources will not compile,
# until you remove __cdecl from aeffect.h header manually. This is intended
# for development convenience only.
option(REDEFINE_CDECL "Redefine __cdecl for Steinberg VST SDK code." ON)

if(REDEFINE_CDECL)
	# Hack for VST SDK.
	add_definitions(-D__cdecl=)
endif()


# Set global include directories
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${VSTSDK_INCLUDE_DIR}
)


# Project sources
add_subdirectory(host)
add_subdirectory(plugin)

if(BUILD_UTILITY)
	include_directories(3rdparty)
	add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/qtsingleapplication)

	add_subdirectory(utility)
else()
   message(STATUS "BUILD_UTILITY is OFF, skipping build of GUI utility.")
endif()
