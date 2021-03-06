INCLUDE(BuildVeyonApplication)
INCLUDE(WindowsBuildHelpers)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(cli_SOURCES src/main.cpp)

build_veyon_application(veyon-cli ${cli_SOURCES})
TARGET_LINK_LIBRARIES(veyon-cli veyon-core Qt5::Network)

ADD_WINDOWS_RESOURCE(veyon-cli ${CMAKE_CURRENT_BINARY_DIR}/veyon-cli.rc)
MAKE_CONSOLE_APP(veyon-cli)

IF(VEYON_BUILD_WIN32)
build_veyon_application(veyon-wcli ${cli_SOURCES})
TARGET_LINK_LIBRARIES(veyon-wcli veyon-core Qt5::Network)

ADD_WINDOWS_RESOURCE(veyon-wcli ${CMAKE_CURRENT_BINARY_DIR}/veyon-wcli.rc)
MAKE_GRAPHICAL_APP(veyon-wcli)
ENDIF()
