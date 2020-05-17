/*
 * ScreenshotFeaturePlugin.cpp - implementation of ScreenshotFeaturePlugin class
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

#include <QMessageBox>
#include <QTimer>
#include <QImage>
#include <QDate>

#include "RecordFeaturePlugin.h"
#include "ComputerControlInterface.h"
#include "VeyonMasterInterface.h"
#include "Screenshot.h"
#include "VeyonConfiguration.h"
#include "Computer.h"
#include "ComputerControlInterface.h"
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
	connect(m_recordTimer, SIGNAL(timeout()), this, SLOT(record()));
	m_lastMaster = nullptr;
}

void RecordFeaturePlugin::initializeRecordingParameters()
{
	//initialize libav
	av_register_all();
	m_videoCodecName = tr("libx264");
	
	m_recordingWidth = m_lastMaster->userConfigurationObject()->value(tr("VideoResX"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingHeight = m_lastMaster->userConfigurationObject()->value(tr("VideoResY"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingVideo = m_lastMaster->userConfigurationObject()->value(tr("SaveVideo"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingFrameInterval = m_lastMaster->userConfigurationObject()->value(tr("VideoFrameInterval"), tr("Uniovi.Reflection"), QVariant(false)).toBool();

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
			currentRecording.videoRecording.videoCodecContext->bit_rate = 400000;
			currentRecording.videoRecording.videoCodecContext->width = m_recordingWidth;
			currentRecording.videoRecording.videoCodecContext->height = m_recordingHeight;
			//currentRecording.videoRecording.videoCodecContext->time_base = (AVRational){1, m_recordingFrameInterval/1000.0};
			//currentRecording.videoRecording.videoCodecContext->framerate = (AVRational){m_recordingFrameInterval/1000.0, 1};
			currentRecording.videoRecording.videoCodecContext->time_base = (AVRational){1, 25};
			currentRecording.videoRecording.videoCodecContext->framerate = (AVRational){25, 1};
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
			currentRecording.videoRecording.outFilePath = currentRecording.computer->computer().name() + QDateTime::currentDateTime().toString( Qt::ISODate ) + tr(".mp4");
			currentRecording.videoRecording.outFile = fopen(currentRecording.videoRecording.outFilePath.toLocal8Bit().data(), "wb");

			//if (!m_outputFormat)
				//QTextStream(stdout) << tr("Error av_guess_format.") << endl;

			//if (avformat_alloc_output_context2(&m_outputContext, NULL, NULL, "Stadyn_user.mp4") < 0)
				//QTextStream(stdout) << tr("Error avformat_alloc_output_context2()") << endl;
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
					QTextStream(stdout) << tr("AVERROR_EOF") << endl;
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

void RecordFeaturePlugin::recordFrame()
{
	//Debugging configured values
	qDebug() << tr("x:") << m_recordingWidth << endl;
	qDebug() << tr("y:") << m_recordingHeight << endl;

	if(m_recordingVideo)
	{
		for( auto& currentRecording : m_recordingSessions )
		{
			QImage image = currentRecording.computer->screen();
			image = image.scaled(m_recordingWidth, m_recordingHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);

			avpicture_fill((AVPicture*)currentRecording.videoRecording.screenshotVideoFrame, image.bits(), AV_PIX_FMT_RGB32, image.width(), image.height());
			sws_scale(currentRecording.videoRecording.swsResizeContext, currentRecording.videoRecording.screenshotVideoFrame->data, currentRecording.videoRecording.screenshotVideoFrame->linesize, 0, image.height(), currentRecording.videoRecording.currentVideoframe->data, currentRecording.videoRecording.currentVideoframe->linesize);

			//currentRecording.videoRecording.currentVideoframe->data
			currentRecording.videoRecording.currentVideoframe->pts = currentRecording.videoRecording.frameCount++;
			QTextStream(stdout) << tr("current frame ") << currentRecording.videoRecording.currentVideoframe->pts << endl;

			int ret = avcodec_send_frame(currentRecording.videoRecording.videoCodecContext, currentRecording.videoRecording.currentVideoframe);
			if (ret < 0)
				QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

			while (ret >= 0)
			{
				ret = avcodec_receive_packet(currentRecording.videoRecording.videoCodecContext, currentRecording.videoRecording.pkt);
				if (ret == AVERROR(EAGAIN))
					;//QTextStream(stdout) << tr("AVERROR") << endl;
				else 
				{
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
			const auto dir = VeyonCore::filesystem().expandPath( VeyonCore::config().screenshotDirectory() );
			QString fileName = dir + QDir::separator() + Screenshot::constructFileName( controlInterface->computer().name(), controlInterface->computer().hostAddress() );
			QImage image = controlInterface->screen();
			if(m_recordingWidth != 0 && m_recordingHeight != 0)
				image = image.scaled(m_recordingWidth, m_recordingHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
			image.save( fileName, "PNG", 50 );
			QTextStream(stdout) << fileName << endl;
		}
	}
//	m_lastMaster->userConfigurationObject()->data()[tr("Uniovi.Reflection")];
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
		if(m_lastMaster == nullptr)
		{
			m_lastMaster = &master;
			initializeRecordingParameters();
		}

		if( m_recordEnabled == false)
		{
			m_recordEnabled = true;
			int interval = m_lastMaster->userConfigurationObject()->value(tr("VideoFrameInterval"), tr("Uniovi.Reflection"), QVariant(10000)).toInt();
			qDebug() << tr("VideoFrameInterval") << interval << endl;
			m_recordTimer->start(interval);
			QMessageBox::information(nullptr, tr("Starting recording"), tr("Recording parameters: TO BE DONE"));
		}
		else
		{
			m_recordEnabled = false;
			m_recordTimer->stop();
			stopRecording();
			QMessageBox::information(nullptr, tr("Stopping recording"), tr("Recording is now disabled"));
		}
		return true;
	}
	return true;
}
