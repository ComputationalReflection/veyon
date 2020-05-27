/*
 * RecordFeaturePlugin.cpp - implementation of RecordFeaturePlugin class
 *
 * Copyright (c) 2020 Jose Quiroga <quirogajose@uniovi.es>, Miguel Garcia <garciarmiguel@uniovi.es>
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

#include <QMessageBox>
#include <QTimer>
#include <QImage>
#include <QDate>
#include <QDir>
#include <QFileInfo>

#include "RecordFeaturePlugin.h"
#include "ComputerControlInterface.h"
#include "VeyonMasterInterface.h"
#include "Screenshot.h"
#include "VeyonConfiguration.h"
#include "Computer.h"
#include "Filesystem.h"

#ifdef __cplusplus
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#endif


RecordFeaturePlugin::RecordFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_screenshotFeature( Feature( QStringLiteral( "Record" ),
								  Feature::Action | Feature::Master,
								  Feature::Uid( "d5ee3aac-2a87-4d05-b827-0c20344490be" ),
								  Feature::Uid(),
								  tr( "Record" ), {},
								  tr( "Use this function to record the selected computers." ),
								  QStringLiteral(":/record/record.png") ) ),
	m_features( { m_screenshotFeature } )
{
	m_recordEnabled = false;
	m_recordTimer = new QTimer(this);
	connect(m_recordTimer, SIGNAL(timeout()), this, SLOT(saveFrame()));
	m_lastMaster = nullptr;

	//initialize libav
	av_register_all();
	m_videoCodecName = tr("libx264");
}

void RecordFeaturePlugin::updateConfigFile()
{
	if( VeyonCore::filesystem().ensurePathExists( m_savePath ) == false )
	{
		if (m_savePath == tr("%APPDATA%/Record"))
		{
			const auto dir = VeyonCore::filesystem().expandPath( m_savePath );
			QDir().mkdir(dir);
		}
		else
		{
			const auto dir = VeyonCore::filesystem().expandPath( m_savePath );
			QMessageBox::critical( nullptr, tr( "Recording" ), tr( "Directory %1 doesn't exist and couldn't be created. Videos should be saved in default video folder." ).arg( dir ) );
			const auto defaultdir = VeyonCore::filesystem().expandPath( tr("%APPDATA%/Record") );
			QDir().mkdir(defaultdir);
		}
	}

	//Update config file values. It writes default values if not set.
	m_lastMaster->userConfigurationObject()->setValue(tr("Width"), QVariant(m_recordingWidth), tr("Plugin.Record"));
	m_lastMaster->userConfigurationObject()->setValue(tr("Height"), QVariant(m_recordingHeight), tr("Plugin.Record"));
	m_lastMaster->userConfigurationObject()->setValue(tr("Video"), QVariant(m_recordingVideo), tr("Plugin.Record"));
	m_lastMaster->userConfigurationObject()->setValue(tr("CaptureIntervalNum"), QVariant(m_recordingFrameIntervalNum), tr("Plugin.Record"));
	m_lastMaster->userConfigurationObject()->setValue(tr("CaptureIntervalDen"), QVariant(m_recordingFrameIntervalDen), tr("Plugin.Record"));
	m_lastMaster->userConfigurationObject()->setValue(tr("SavePath"), QVariant(m_savePath), tr("Plugin.Record"));

}

void RecordFeaturePlugin::initializeRecordingParameters()
{
	m_recordingWidth = m_lastMaster->userConfigurationObject()->value(tr("Width"), tr("Plugin.Record"), QVariant(1280)).toInt();
	m_recordingHeight = m_lastMaster->userConfigurationObject()->value(tr("Height"), tr("Plugin.Record"), QVariant(720)).toInt();
	m_recordingVideo = m_lastMaster->userConfigurationObject()->value(tr("Video"), tr("Plugin.Record"), QVariant(true)).toBool();
	m_recordingFrameIntervalNum = m_lastMaster->userConfigurationObject()->value(tr("CaptureIntervalNum"), tr("Plugin.Record"), QVariant(1000)).toInt();
	m_recordingFrameIntervalDen = m_lastMaster->userConfigurationObject()->value(tr("CaptureIntervalDen"), tr("Plugin.Record"), QVariant(1000)).toInt();
	m_savePath = m_lastMaster->userConfigurationObject()->value(tr("SavePath"), tr("Plugin.Record"), tr("%APPDATA%/Record")).toString();
	
	updateConfigFile();
	
	m_recordingSessions.clear();
	for( const auto& controlInterface : m_lastComputerControlInterfaces )
	{
		RecordingComputer recordingData;
		recordingData.computer = controlInterface;
		m_recordingSessions.append(recordingData);
	}
	startRecording();
}

void RecordFeaturePlugin::startRecording()
{
	if(m_recordingVideo)
	{
		for( auto& currentRecording : m_recordingSessions )
		{
			//currentRecording.videoRecording.videoCodec
			currentRecording.videoRecording.videoCodec = avcodec_find_encoder_by_name(m_videoCodecName.toLocal8Bit().data());
			if (!currentRecording.videoRecording.videoCodec) {
				QTextStream(stdout) << tr("Codec ") << m_videoCodecName << tr(" not found") << endl;
			}
			else {
				QTextStream(stdout) << tr("Codec ") << m_videoCodecName << tr(" found") << endl;
			}
			currentRecording.videoRecording.videoCodecContext = avcodec_alloc_context3(currentRecording.videoRecording.videoCodec);
			if (!currentRecording.videoRecording.videoCodecContext)
				QTextStream(stdout) << tr("Codec Context couldn't be allocated.") << endl;

			currentRecording.videoRecording.pkt = av_packet_alloc();
			if (!currentRecording.videoRecording.pkt)
				QTextStream(stdout) << tr("AV packet couldn't be allocated.") << endl;

			//Initilize basic encoding context: based on https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/encode_video.c
			currentRecording.videoRecording.videoCodecContext->bit_rate = 4000000;
			currentRecording.videoRecording.videoCodecContext->width = m_recordingWidth;
			currentRecording.videoRecording.videoCodecContext->height = m_recordingHeight;
			currentRecording.videoRecording.videoCodecContext->time_base = (AVRational){m_recordingFrameIntervalNum, m_recordingFrameIntervalDen};
			currentRecording.videoRecording.videoCodecContext->framerate = (AVRational){m_recordingFrameIntervalDen, m_recordingFrameIntervalNum};
			currentRecording.videoRecording.videoCodecContext->gop_size = 10;
			currentRecording.videoRecording.videoCodecContext->max_b_frames = 1;
			currentRecording.videoRecording.videoCodecContext->frame_size = 1;
			currentRecording.videoRecording.videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

			av_opt_set(currentRecording.videoRecording.videoCodecContext->priv_data, "preset", "slow", 0);

			if (avcodec_open2(currentRecording.videoRecording.videoCodecContext, currentRecording.videoRecording.videoCodec, NULL) < 0)
				QTextStream(stdout) << tr("Could not open codec.") << endl;

			currentRecording.videoRecording.currentVideoframe = av_frame_alloc();
			if (!currentRecording.videoRecording.currentVideoframe)
				QTextStream(stdout) << tr("Could not allocate video frame.") << endl;
			currentRecording.videoRecording.currentVideoframe->format = currentRecording.videoRecording.videoCodecContext->pix_fmt;
			currentRecording.videoRecording.currentVideoframe->width  = currentRecording.videoRecording.videoCodecContext->width;
			currentRecording.videoRecording.currentVideoframe->height = currentRecording.videoRecording.videoCodecContext->height;


			if (av_frame_get_buffer(currentRecording.videoRecording.currentVideoframe, 32) < 0) 
				QTextStream(stdout) << tr("Could not allocate the video frame data.") << endl;

			if (av_frame_make_writable(currentRecording.videoRecording.currentVideoframe) < 0) 
				QTextStream(stdout) << tr("Could not made the video frame data writable.") << endl;

			currentRecording.videoRecording.screenshotVideoFrame = av_frame_alloc();

			currentRecording.videoRecording.swsResizeContext = sws_getContext(m_recordingWidth, m_recordingHeight, AV_PIX_FMT_RGB32, m_recordingWidth, m_recordingHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0 );

			currentRecording.videoRecording.frameCount = 0;
			currentRecording.videoRecording.pktCount = 0;
			currentRecording.videoRecording.outFilePath = VeyonCore::filesystem().expandPath( m_savePath ) + QDir::separator() + currentRecording.computer->computer().name() + tr("_") + QDateTime::currentDateTime().toString( Qt::ISODate ) + tr(".h264");
			currentRecording.videoRecording.outFile = fopen(currentRecording.videoRecording.outFilePath.toLocal8Bit().data(), "wb");
		}
	}
}

void RecordFeaturePlugin::stopRecording()
{
	if (m_recordingVideo)
	{
		for( auto& currentRecording : m_recordingSessions )
		{
			/* flush the encoder */
			//encode(c, NULL, pkt, f);
			int ret = avcodec_send_frame(currentRecording.videoRecording.videoCodecContext, NULL);
			if (ret < 0)
				QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

			while (ret >= 0)
			{
				ret = avcodec_receive_packet(currentRecording.videoRecording.videoCodecContext, currentRecording.videoRecording.pkt);
				if (ret == AVERROR_EOF)
					;//QTextStream(stdout) << tr("AVERROR_EOF") << endl;
				else if (ret < 0)
					QTextStream(stdout) << tr("Error during encoding") << endl;
				else {
					fwrite(currentRecording.videoRecording.pkt->data, 1, currentRecording.videoRecording.pkt->size, currentRecording.videoRecording.outFile);
					av_packet_unref(currentRecording.videoRecording.pkt);
				}
			}
			fclose(currentRecording.videoRecording.outFile);
			avcodec_free_context(&(currentRecording.videoRecording.videoCodecContext));
			av_frame_free(&(currentRecording.videoRecording.currentVideoframe));
			av_frame_free(&(currentRecording.videoRecording.screenshotVideoFrame));
			av_packet_free(&(currentRecording.videoRecording.pkt));
		}
	}
}

void RecordFeaturePlugin::saveFrame()
{
	if(m_recordingVideo)
	{
		for( auto& currentRecording : m_recordingSessions )
		{
			QImage image = currentRecording.computer->screen();
			image = image.scaled(m_recordingWidth, m_recordingHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);

			avpicture_fill((AVPicture*)currentRecording.videoRecording.screenshotVideoFrame, image.bits(), AV_PIX_FMT_RGB32, image.width(), image.height());
			sws_scale(currentRecording.videoRecording.swsResizeContext, currentRecording.videoRecording.screenshotVideoFrame->data, currentRecording.videoRecording.screenshotVideoFrame->linesize, 0, image.height(), currentRecording.videoRecording.currentVideoframe->data, currentRecording.videoRecording.currentVideoframe->linesize);

			currentRecording.videoRecording.currentVideoframe->pts = currentRecording.videoRecording.frameCount++;

			int ret = avcodec_send_frame(currentRecording.videoRecording.videoCodecContext, currentRecording.videoRecording.currentVideoframe);
			if (ret < 0)
				QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

			while (ret >= 0)
			{
				ret = avcodec_receive_packet(currentRecording.videoRecording.videoCodecContext, currentRecording.videoRecording.pkt);
				if (ret != AVERROR(EAGAIN))
				{
					currentRecording.videoRecording.pkt->pts = currentRecording.videoRecording.pktCount++;
					fwrite(currentRecording.videoRecording.pkt->data, 1, currentRecording.videoRecording.pkt->size, currentRecording.videoRecording.outFile);
					av_packet_unref(currentRecording.videoRecording.pkt);
				}
			}
		}
	}
	else
	{
		for( const auto& controlInterface : m_lastComputerControlInterfaces )
		{
			//This is a simplified version of the code in Screenshot::take(). In this case no label is added to png image
			const auto dir = VeyonCore::filesystem().expandPath( m_savePath );
			QString fileName = dir + QDir::separator() + Screenshot::constructFileName( controlInterface->computer().name(), controlInterface->computer().hostAddress(), QDateTime::currentDateTime().date(), QDateTime::currentDateTime().time() );
			QImage image = controlInterface->screen();
			if(m_recordingWidth != 0 && m_recordingHeight != 0)
				image = image.scaled(m_recordingWidth, m_recordingHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
			image.save( fileName, "PNG", 50 );
		}
	}
}
const FeatureList &RecordFeaturePlugin::featureList() const
{
	return m_features;
}
bool RecordFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
											const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature.uid() == m_screenshotFeature.uid() )
	{
		m_lastComputerControlInterfaces = computerControlInterfaces;
		m_lastMaster = &master;

		if( m_recordEnabled == false)
		{
			initializeRecordingParameters();
			m_recordEnabled = true;
			int interval = (int)((m_recordingFrameIntervalNum * 1000.0) / (m_recordingFrameIntervalDen * 1.0));
			m_recordTimer->start(interval);
			if (m_recordingVideo)
				QMessageBox::information(nullptr, tr("Starting recording"), tr("Recording parameters:\nVideo with resolution %1x%2\nInterval: %3 frames per second").arg( QString::number(m_recordingWidth),
					QString::number(m_recordingHeight), QString::number(1000.0/interval)));
			else
				QMessageBox::information(nullptr, tr("Starting recording"), tr("Recording parameters:\nScreenshots with resolution %1x%2\nInterval: %3 screenshots per second").arg( QString::number(m_recordingWidth),
					QString::number(m_recordingHeight), QString::number(1000.0/interval)));
		}
		else
		{
			m_recordTimer->stop();
			stopRecording();
			m_recordEnabled = false;
			QMessageBox::information(nullptr, tr("Stopping recording"), tr("Recording is now disabled"));
		}
		return true;
	}
	return true;
}



