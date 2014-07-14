cmake_minimum_required(VERSION 2.8.6)

# Configure library base name
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(TARGET_NAME ${PLUGIN_BASENAME})


set(FIND_LIBRARY_USE_LIB64_PATHS OFF)

find_package(LibDl REQUIRED)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)


include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${X11_INCLUDE_DIR}
)


# Configure socket filename for logger
add_definitions(-DLOG_SOCK_PATH="/tmp/${TARGET_NAME}.sock")


# Common sources
set(SOURCES
	../common/dataport.cpp
	../common/eventsignal.cpp
	../common/filesystem.cpp
	../common/linkmanager.cpp
	../common/logger.cpp
	../common/vsteventkeeper.cpp
	masterunit.cpp
	plugin.cpp
)


include_directories(${LIBDL_INCLUDE_DIR})

# Set target
add_library(${TARGET_NAME}-32 SHARED ${SOURCES})


set_target_properties(${TARGET_NAME}-32 PROPERTIES
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)

# Link with libraries
target_link_libraries(${TARGET_NAME}-32
	${LIBDL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${X11_X11_LIB}
)


install(TARGETS ${TARGET_NAME}-32
	LIBRARY DESTINATION share/${PROJECT_NAME}
)
