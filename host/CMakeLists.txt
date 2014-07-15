cmake_minimum_required(VERSION 2.8.6)

set(TARGET_NAME ${HOST_BASENAME})


set(FIND_LIBRARY_USE_LIB64_PATHS OFF)
find_package(Threads REQUIRED)


include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)


# Set target operating system
set(CMAKE_SYSTEM_NAME Windows)


# Set compilers to use
set(CMAKE_C_COMPILER winegcc)
set(CMAKE_CXX_COMPILER wineg++)


# Sources
set(SOURCES
	../common/dataport.cpp
	../common/eventsignal.cpp
	../common/filesystem.cpp
	../common/logger.cpp
	../common/vsteventkeeper.cpp
	host.cpp
	slaveunit.cpp
)


# Set target
add_executable(${TARGET_NAME}-32 WIN32 ${SOURCES})

set_target_properties(${TARGET_NAME}-32 PROPERTIES
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)

# Link with libraries
target_link_libraries(${TARGET_NAME}-32 ${CMAKE_THREAD_LIBS_INIT})


install(PROGRAMS
	${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-32.exe
	${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-32.exe.so
	DESTINATION bin
)
