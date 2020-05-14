INCLUDE(BuildVeyonPlugin)

find_path( AVCODEC_INCLUDE_DIR libavcodec/avcodec.h )
find_library( AVCODEC_LIBRARY avcodec )

build_veyon_plugin(screenshot
	ScreenshotFeaturePlugin.cpp
	ScreenshotFeaturePlugin.h
	screenshot.qrc
)

TARGET_LINK_LIBRARIES(screenshot -lavformat -lavcodec -lavutil)

