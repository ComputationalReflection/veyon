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

#include "RecordFeaturePlugin.h"
#include "ComputerControlInterface.h"
#include "VeyonMasterInterface.h"
#include "Screenshot.h"
#include "VeyonConfiguration.h"
#include "Computer.h"
#include "ComputerControlInterface.h"
#include "Filesystem.h"


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

void ScreenshotFeaturePlugin::initializeRecordingParameters()
{
	//initialize libav
	av_register_all();
	QString m_videoCodecName = tr("libx264");
	
	m_recordingWidth = m_lastMaster->userConfigurationObject()->value(tr("VideoResX"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingHeight = m_lastMaster->userConfigurationObject()->value(tr("VideoResY"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingVideo = m_lastMaster->userConfigurationObject()->value(tr("SaveVideo"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingFrameInterval = m_lastMaster->userConfigurationObject()->value(tr("VideoFrameInterval"), tr("Uniovi.Reflection"), QVariant(false)).toBool();
	if(m_recordingVideo)
	{
		initializeVideo();
	}
}

void ScreenshotFeaturePlugin::startRecording()
{	
	if(m_recordingVideo)
	{
		//m_videoEncoder[0]->videoCodec
		m_videoEncoder[0]->videoCodec = avcodec_find_encoder_by_name(m_videoCodecName.toLocal8Bit().data());
		if (!m_videoEncoder[0]->videoCodec) {
		    QTextStream(stdout) << tr("Codec ") << m_videoCodecName << tr(" not found") << endl;
		}
		else {
		    QTextStream(stdout) << tr("Codec ") << m_videoCodecName << tr(" found") << endl;
		}
		m_videoEncoder[0]->videoCodecContext = avcodec_alloc_context3(m_videoEncoder[0]->videoCodec);
		if (!m_videoEncoder[0]->videoCodecContext)
		    QTextStream(stdout) << tr("Codec Context couldn't be allocated.") << endl;

		m_videoEncoder[0]->pkt = av_packet_alloc();
		if (!m_videoEncoder[0]->pkt)
		    QTextStream(stdout) << tr("AV packet couldn't be allocated.") << endl;

		//Initilize basic encoding context: based on https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/encode_video.c
		m_videoEncoder[0]->videoCodecContext->bit_rate = 400000;
		m_videoEncoder[0]->videoCodecContext->width = m_recordingWidth;
		m_videoEncoder[0]->videoCodecContext->height = m_recordingHeight;
//		m_videoEncoder[0]->videoCodecContext->time_base = (AVRational){1, m_recordingFrameInterval/1000.0};
//		m_videoEncoder[0]->videoCodecContext->framerate = (AVRational){m_recordingFrameInterval/1000.0, 1};
		m_videoEncoder[0]->videoCodecContext->time_base = (AVRational){1, 25};
		m_videoEncoder[0]->videoCodecContext->framerate = (AVRational){25, 1};
		m_videoEncoder[0]->videoCodecContext->gop_size = 10;
		m_videoEncoder[0]->videoCodecContext->max_b_frames = 1;
		m_videoEncoder[0]->videoCodecContext->frame_size = 1;
		m_videoEncoder[0]->videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

		av_opt_set(m_videoEncoder[0]->videoCodecContext->priv_data, "preset", "slow", 0);

		if (avcodec_open2(m_videoEncoder[0]->videoCodecContext, m_videoEncoder[0]->videoCodec, NULL) < 0)
			QTextStream(stdout) << tr("Could not open codec.") << endl;

		m_videoEncoder[0]->currentVideoframe = av_frame_alloc();
		if (!m_videoEncoder[0]->currentVideoframe)
			QTextStream(stdout) << tr("Could not allocate video frame.") << endl;
		m_videoEncoder[0]->currentVideoframe->format = m_videoEncoder[0]->videoCodecContext->pix_fmt;
		m_videoEncoder[0]->currentVideoframe->width  = m_videoEncoder[0]->videoCodecContext->width;
		m_videoEncoder[0]->currentVideoframe->height = m_videoEncoder[0]->videoCodecContext->height;


		if (av_frame_get_buffer(m_videoEncoder[0]->currentVideoframe, 32) < 0) 
			QTextStream(stdout) << tr("Could not allocate the video frame data.") << endl;

		if (av_frame_make_writable(m_videoEncoder[0]->currentVideoframe) < 0) 
			QTextStream(stdout) << tr("Could not made the video frame data writable.") << endl;

		m_videoEncoder[0]->screenshotVideoFrame = av_frame_alloc();

		m_videoEncoder[0]->swsResizeContext = sws_getContext(m_recordingWidth, m_recordingHeight, AV_PIX_FMT_RGB32, m_recordingWidth, m_recordingHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0 );

		m_frameCount = 0;
		//m_videoEncoder[0]->outFile = fopen(tr("Stadyn_user.mp4").toLocal8Bit().data(), "wb");
		//m_outputFormat = av_guess_format(NULL, tr("Stadyn_user.mp4").toLocal8Bit().data(), NULL);
		//if (!m_outputFormat)
			//QTextStream(stdout) << tr("Error av_guess_format.") << endl;
		
		//if (avformat_alloc_output_context2(&m_outputContext, NULL, NULL, "Stadyn_user.mp4") < 0)
			//QTextStream(stdout) << tr("Error avformat_alloc_output_context2()") << endl;

	}
	
}

void ScreenshotFeaturePlugin::stopRecording()
{
	if (m_recordingVideo)
	{
		/* flush the encoder */
		//encode(c, NULL, pkt, f);
		int ret = avcodec_send_frame(m_videoEncoder[0]->videoCodecContext, NULL);
		if (ret < 0)
			QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

		while (ret >= 0)
		{
			ret = avcodec_receive_packet(m_videoEncoder[0]->videoCodecContext, m_videoEncoder[0]->pkt);
			if (ret == AVERROR_EOF)
				QTextStream(stdout) << tr("AVERROR_EOF") << endl;
			else if (ret < 0)
				QTextStream(stdout) << tr("Error during encoding") << endl;
			else {
				fwrite(m_videoEncoder[0]->pkt->data, 1, m_videoEncoder[0]->pkt->size, m_videoEncoder[0]->outFile);
				av_packet_unref(m_videoEncoder[0]->pkt);
			}
		}

		fclose(m_videoEncoder[0]->outFile);

		avcodec_free_context(&m_videoEncoder[0]->videoCodecContext);
		av_frame_free(&m_videoEncoder[0]->currentVideoframe);
		av_frame_free(&m_videoEncoder[0]->screenshotVideoFrame);
		av_packet_free(&m_videoEncoder[0]->pkt);
		//avformat_free_context(m_outputContext);
	}	
}

void RecordFeaturePlugin::recordFrame()
{
	//Debugging configured values
	qDebug() << tr("x:") << m_recordingWidth << endl;
	qDebug() << tr("y:") << m_recordingHeight << endl;

	if(m_recordingVideo)
	{
		for( const auto& controlInterface : m_lastComputerControlInterfaces )
		{
			QImage image = controlInterface->screen();
			image = image.scaled(m_recordingWidth, m_recordingHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);


			avpicture_fill((AVPicture*)m_videoEncoder[0]->screenshotVideoFrame, image.bits(), AV_PIX_FMT_RGB32, image.width(), image.height());
			sws_scale(m_videoEncoder[0]->swsResizeContext, m_videoEncoder[0]->screenshotVideoFrame->data, m_videoEncoder[0]->screenshotVideoFrame->linesize, 0, image.height(), m_videoEncoder[0]->currentVideoframe->data, m_videoEncoder[0]->currentVideoframe->linesize);

			//m_videoEncoder[0]->currentVideoframe->data
			m_videoEncoder[0]->currentVideoframe->pts = m_frameCount++;
			QTextStream(stdout) << tr("current frame ") << m_videoEncoder[0]->currentVideoframe->pts << endl;

			int ret = avcodec_send_frame(m_videoEncoder[0]->videoCodecContext, m_videoEncoder[0]->currentVideoframe);
			if (ret < 0)
        			QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

			while (ret >= 0)
			{
				ret = avcodec_receive_packet(m_videoEncoder[0]->videoCodecContext, m_videoEncoder[0]->pkt);
				if (ret == AVERROR(EAGAIN))
					;//QTextStream(stdout) << tr("AVERROR") << endl;
				else 
				{
					fwrite(m_videoEncoder[0]->pkt->data, 1, m_videoEncoder[0]->pkt->size, m_videoEncoder[0]->outFile);
					av_packet_unref(m_videoEncoder[0]->pkt);
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
		if(m_lastMaster == nullptr)
		{
			m_lastMaster = &master;
			initializeRecordingParameters();
		}

		m_lastComputerControlInterfaces = computerControlInterfaces;
		if( m_recordEnabled == false)
		{
			m_recordEnabled = true;
			int interval = m_lastMaster->userConfigurationObject()->value(tr("VideoFrameInterval"), tr("Uniovi.Reflection"), QVariant(10000)).toInt();
			qDebug() << tr("VideoFrameInterval") << interval << endl;
			m_recordTimer->start(interval);
			QMessageBox::information(nullptr, tr("Starting recording"), tr("Recording parameters: TODO"));
		}
		else
		{
			m_recordEnabled = false;
			stopRecording();
			m_recordTimer->stop();
			QMessageBox::information(nullptr, tr("Stopping recording"), tr("Recording is now disabled"));
		}
		return true;
	}
	return true;
}
