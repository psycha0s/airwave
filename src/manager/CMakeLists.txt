set(TARGET_NAME ${PROJECT_NAME}-manager)

project(${TARGET_NAME})

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(LibMagic REQUIRED)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LIBMAGIC_INCLUDE_DIR}
)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

set(SOURCES
	main.cpp
	../common/filesystem.cpp
	../common/json.cpp
	../common/moduleinfo.cpp
	../common/storage.cpp
	core/application.cpp
	core/logsocket.cpp
	core/singleapplication.cpp
	forms/filedialog.cpp
	forms/folderdialog.cpp
	forms/linkdialog.cpp
	forms/loaderdialog.cpp
	forms/mainform.cpp
	forms/prefixdialog.cpp
	forms/settingsdialog.cpp
	models/directorymodel.cpp
	models/linksmodel.cpp
	models/loadersmodel.cpp
	models/prefixesmodel.cpp
	widgets/lineedit.cpp
	widgets/linksview.cpp
	widgets/logview.cpp
	widgets/directoryview.cpp
	widgets/loadersview.cpp
	widgets/nofocusdelegate.cpp
	widgets/prefixesview.cpp
	widgets/separatorlabel.cpp
)

# Help the stupid IDE to consider following headers as a part of the project
set(HEADERS
	../common/config.h
	../common/types.h
	models/generictreemodel.h
	widgets/generictreeview.h
)


set(RESOURCES
	resources/resources.qrc
)

set(LIBRARIES
	Qt5::Widgets
	Qt5::Network
	${LIBMAGIC_LIBRARY}
)

qt5_add_resources(RCC_SOURCES ${RESOURCES})

# Set target
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS} ${RCC_SOURCES})

# Use the Widgets module from Qt 5
target_link_libraries(${TARGET_NAME} ${LIBRARIES})

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
install(FILES resources/${TARGET_NAME}.png DESTINATION share/icons/hicolor/48x48/apps)
install(FILES ${TARGET_NAME}.desktop DESTINATION share/applications)
