INCLUDE(BuildVeyonPlugin)

build_veyon_plugin(record
	RecordFeaturePlugin.cpp
	RecordFeaturePlugin.h
	record.qrc
)

TARGET_LINK_LIBRARIES(record -lavformat -lavcodec -lavutil -lswscale)