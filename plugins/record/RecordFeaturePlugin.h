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

typedef struct VideoRecording {
	
	AVCodec *videoCodec;
	AVCodecContext *videoCodecContext;
	
	
	AVFrame *currentVideoframe;
	AVFrame *screenshotVideoFrame;
	SwsContext *swsResizeContext;
	long frameCount;
	long pktCount;
	
	AVPacket *pkt;
	FILE *outFile;
	QString outFilePath;
    
} VideoRecording;

typedef struct RecordingComputer {
	
	VideoRecording videoRecording;
	QSharedPointer<ComputerControlInterface> computer;    
} RecordingComputer;

class RecordFeaturePlugin : public QObject, SimpleFeatureProvider, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.Record")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit RecordFeaturePlugin( QObject* parent = nullptr );
	~RecordFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return QStringLiteral("ee322521-f4fb-482d-b082-82a79003afa8");
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 1 );
	}

	QString name() const override
	{
		return QStringLiteral("Record");
	}

	QString description() const override
	{
		return tr( "Record the selected computers and save them locally." );
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
	void saveFrame();

private:
	const Feature m_screenshotFeature;
	const FeatureList m_features;
	bool m_recordEnabled;
	QTimer *m_recordTimer;
	VeyonMasterInterface* m_lastMaster;
	ComputerControlInterfaceList m_lastComputerControlInterfaces;

	QVector<RecordingComputer> m_recordingSessions;
	QString m_videoCodecName;
	
	//Reflection.Uniovi configuration parameters
	int m_recordingWidth;
	int m_recordingHeight;
	bool m_recordingVideo;
	int m_recordingFrameInterval;
	long m_packetCount;
	//
	void initializeRecordingParameters();
	void startRecording();
	void stopRecording();
};
