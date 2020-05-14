/*
 * ScreenshotFeaturePlugin.h - declaration of ScreenshotFeature class
 *
 * Copyright (c) 2017-2020 Tobias Junghans <tobydox@veyon.io>
 *
 * This file is part of Veyon - https://veyon.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#pragma once

#include <QTimer>
#include "Feature.h"
#include "SimpleFeatureProvider.h"

#ifdef __cplusplus
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#endif


class ScreenshotFeaturePlugin : public QObject, SimpleFeatureProvider, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.Screenshot")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit ScreenshotFeaturePlugin( QObject* parent = nullptr );
	~ScreenshotFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return QStringLiteral("ee322521-f4fb-482d-b082-82a79003afa7");
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 1 );
	}

	QString name() const override
	{
		return QStringLiteral("Screenshot");
	}

	QString description() const override
	{
		return tr( "Take screenshots of computers and save them locally." );
	}

	QString vendor() const override
	{
		return QStringLiteral("Veyon Community");
	}

	QString copyright() const override
	{
		return QStringLiteral("Tobias Junghans");
	}

	const FeatureList& featureList() const override;

	bool startFeature( VeyonMasterInterface& master, const Feature& feature,
					   const ComputerControlInterfaceList& computerControlInterfaces ) override;

public Q_SLOTS:
	void record();

private:
	const Feature m_screenshotFeature;
	const FeatureList m_features;
	bool m_recordEnabled;
	QTimer *m_recordTimer;
	VeyonMasterInterface* m_lastMaster;
	ComputerControlInterfaceList m_lastComputerControlInterfaces;
	AVCodec *m_codec;
	AVCodecContext *m_codecContext;
	AVPacket *m_pkt;
	FILE *m_outFile;
	AVFrame *m_currentVideoframe;
	SwsContext *m_swsContext;

	//Reflection.Uniovi configuration parameters
	int m_recordingWidth;
	int m_recordingHeight;
	bool m_recordingVideo;
	int m_recordingFrameInterval;
	long m_frameCount;
	void initializeRecordingParameters();
	
};
