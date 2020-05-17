INCLUDE(BuildVeyonPlugin)

find_path( AVCODEC_INCLUDE_DIR libavcodec/avcodec.h )
find_library( AVCODEC_LIBRARY avcodec )

build_veyon_plugin(record
	RecordFeaturePlugin.cpp
	RecordFeaturePlugin.h
	record.qrc
)

TARGET_LINK_LIBRARIES(record -lavformat -lavcodec -lavutil -lswscale)