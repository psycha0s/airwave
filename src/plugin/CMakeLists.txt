set(TARGET_NAME ${PLUGIN_BASENAME})

project(${TARGET_NAME})

find_package(LibDl REQUIRED)
find_package(LibMagic REQUIRED)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LIBDL_INCLUDE_DIR}
	${X11_INCLUDE_DIR}
	${VSTSDK_INCLUDE_DIR}
)

# Workaround for VST 2.4 SDK on Linux
add_definitions(-D__cdecl=)

if(DEBUG_BINARY_DIR)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEBUG_BINARY_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEBUG_BINARY_DIR})
endif()

# Common sources
set(SOURCES
	main.cpp
	plugin.cpp
	../common/dataport.cpp
	../common/event.cpp
	../common/filesystem.cpp
	../common/json.cpp
	../common/logger.cpp
	../common/moduleinfo.cpp
	../common/storage.cpp
	../common/vsteventkeeper.cpp
)

set(HEADERS
	../common/json.h
	../common/protocol.h
	../common/vst24.h
)

# Configure library base name
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Set target
add_library(${TARGET_NAME} SHARED ${SOURCES})

# Link with libraries
target_link_libraries(${TARGET_NAME}
	${LIBDL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${X11_X11_LIB}
	${LIBMAGIC_LIBRARY}
)

install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION bin)
