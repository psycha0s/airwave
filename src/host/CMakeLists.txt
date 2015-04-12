# Configure base name
set(TARGET_NAME ${HOST_BASENAME})

project(${TARGET_NAME})

find_package(Threads REQUIRED)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${VSTSDK_INCLUDE_DIR}
)

# Set target operating system
set(CMAKE_SYSTEM_NAME Windows)

# Set compilers to use
set(CMAKE_C_COMPILER winegcc)
set(CMAKE_CXX_COMPILER wineg++)

add_definitions(-DNOMINMAX)

if(DEBUG_BINARY_DIR)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEBUG_BINARY_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEBUG_BINARY_DIR})
endif()

# Sources
set(SOURCES
	../common/dataport.cpp
	../common/eventsignal.cpp
	../common/filesystem.cpp
	../common/logger.cpp
	../common/vsteventkeeper.cpp
	host.cpp
	main.cpp
)

if(PLATFORM_64BIT)
	# Set target
	add_executable(${TARGET_NAME}-64 WIN32 ${SOURCES})

	set_target_properties(${TARGET_NAME}-64 PROPERTIES
		COMPILE_FLAGS "-m64"
		LINK_FLAGS "-m64"
	)

	# Link with libraries
	target_link_libraries(${TARGET_NAME}-64
		${CMAKE_THREAD_LIBS_INIT}
	)

	install(PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-64.exe
		${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-64.exe.so
		DESTINATION bin
	)
endif()


# Set target
add_executable(${TARGET_NAME}-32 WIN32 ${SOURCES})

set_target_properties(${TARGET_NAME}-32 PROPERTIES
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)

# Link with libraries
target_link_libraries(${TARGET_NAME}-32
	${CMAKE_THREAD_LIBS_INIT}
)

install(PROGRAMS
	${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-32.exe
	${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-32.exe.so
	DESTINATION bin
)
