cmake_minimum_required(VERSION 2.8.6)

set(TARGET_NAME qtsingleapplication)

# Find packages
find_package(Qt4 REQUIRED)

set(TARGET_SOURCES
    qtlocalpeer.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
    qtlockedfile.cpp
)

if(WIN32)
    add_definitions(-DUNICODE)
    set(TARGET_SOURCES ${TARGET_SOURCES} qtlockedfile_win.cpp)
elseif(UNIX)
    set(TARGET_SOURCES ${TARGET_SOURCES} qtlockedfile_unix.cpp)
endif(WIN32)

set(QT_USE_QTNETWORK true)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Set target
add_library(${TARGET_NAME} STATIC ${TARGET_SOURCES})

# Enable automatic handling of sources by Qt meta object compiler
set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC true)

# Link with libraries
target_link_libraries(${TARGET_NAME} ${QT_LIBRARIES})
