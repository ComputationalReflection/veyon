INCLUDE(BuildVeyonApplication)
INCLUDE(WindowsBuildHelpers)

SET(kitemmodels_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/kitemmodels/src/core)
SET(kitemmodels_SOURCES
	${kitemmodels_SOURCE_DIR}/kextracolumnsproxymodel.cpp
	${kitemmodels_SOURCE_DIR}/krecursivefilterproxymodel.cpp
	${kitemmodels_SOURCE_DIR}/kextracolumnsproxymodel.h
	${kitemmodels_SOURCE_DIR}/krecursivefilterproxymodel.h
)

FILE(GLOB master_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
FILE(GLOB master_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui)

SET(master_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/master.qrc)
IF(VEYON_DEBUG)
SET(master_RESOURCES ${master_RESOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/examples.qrc)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src ${kitemmodels_SOURCE_DIR})
build_veyon_application(veyon-master ${master_SOURCES} ${master_INCLUDES} ${master_RESOURCES} ${kitemmodels_SOURCES})
TARGET_LINK_LIBRARIES(veyon-master veyon-core)

ADD_WINDOWS_RESOURCE(veyon-master)
MAKE_GRAPHICAL_APP(veyon-master)

IF(VEYON_BUILD_LINUX)
	XDG_INSTALL(${CMAKE_CURRENT_BINARY_DIR}/data/veyon-master.desktop ${CMAKE_CURRENT_SOURCE_DIR}/data/veyon-master.xpm ${CMAKE_CURRENT_SOURCE_DIR}/data/veyon-master.png ${CMAKE_CURRENT_SOURCE_DIR}/data/veyon-master.svg)
ENDIF()

remove_definitions(-DQT_NO_KEYWORDS)
set_source_files_properties(${master_SOURCES} PROPERTIES COMPILE_DEFINITIONS QT_NO_KEYWORDS)

# enable again once kitemmodels compile with QT_NO_KEYWORDS and the remove_definitions/set_source_files_properties above can be removed
# cotire_veyon(veyon-master)
