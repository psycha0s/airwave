cmake_minimum_required(VERSION 2.8.6)

# Target name
set(TARGET_NAME ${PROJECT_NAME}-utility)

# Find packages
find_package(Qt4 REQUIRED)
find_package(LibMagic REQUIRED)

include(${QT_USE_FILE})


include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LIBMAGIC_INCLUDE_DIR}
)


# Common sources
set(SOURCES
	logsocket.cpp
	main.cpp
	moduleinfo.cpp
	../common/filesystem.cpp
	../common/logger.cpp
	../common/linkmanager.cpp
	forms/linkeditdialog.cpp
	forms/mainform.cpp
	models/dirlistmodel.cpp
	models/linksmodel.cpp
	widgets/elidedlabel.cpp
	widgets/logview.cpp
	widgets/nofocusdelegate.cpp
	widgets/simpletreeview.cpp
)

# User interface files
set(FORMS
	forms/linkeditdialog.ui
	forms/mainform.ui
)

# Resource files
set(RESOURCES
	icons.qrc
)

# Generate addition sources
qt4_wrap_ui(UIC_SOURCES ${FORMS})
qt4_add_resources(RCC_SOURCES ${RESOURCES})

# Set target
add_executable(${TARGET_NAME} ${SOURCES} ${UIC_SOURCES} ${RCC_SOURCES})

# Enable automatic handling of sources by Qt meta object compiler
set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC true)

# Link with libraries
target_link_libraries(${TARGET_NAME}
	${QT_LIBRARIES}
	${LIBMAGIC_LIBRARY}
	qtsingleapplication
)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

install(FILES icons/${TARGET_NAME}.png
	DESTINATION share/icons/hicolor/48x48/apps)

install(FILES ${TARGET_NAME}.desktop DESTINATION share/applications)
