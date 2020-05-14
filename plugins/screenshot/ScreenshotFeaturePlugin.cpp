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

#include "ScreenshotFeaturePlugin.h"
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
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#endif


ScreenshotFeaturePlugin::ScreenshotFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_screenshotFeature( Feature( QStringLiteral( "Screenshot" ),
								  Feature::Action | Feature::Master,
								  Feature::Uid( "d5ee3aac-2a87-4d05-b827-0c20344490bd" ),
								  Feature::Uid(),
								  tr( "Screenshot" ), {},
								  tr( "Use this function to take a screenshot of selected computers." ),
								  QStringLiteral(":/screenshot/camera-photo.png") ) ),
	m_features( { m_screenshotFeature } )
{
	m_recordEnabled = false;
	m_recordTimer = new QTimer(this);
	connect(m_recordTimer, SIGNAL(timeout()), this, SLOT(record()));
	m_lastMaster = nullptr;

}

void ScreenshotFeaturePlugin::initializeRecordingParameters()
{
	m_recordingWidth = m_lastMaster->userConfigurationObject()->value(tr("VideoResX"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingHeight = m_lastMaster->userConfigurationObject()->value(tr("VideoResY"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingVideo = m_lastMaster->userConfigurationObject()->value(tr("VideoResX"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	m_recordingFrameInterval = m_lastMaster->userConfigurationObject()->value(tr("SaveVideo"), tr("Uniovi.Reflection"), QVariant(false)).toBool();

	
	if(m_recordingVideo)
	{
		QString codec_name = tr("libx264");

		av_register_all();
		m_codec = avcodec_find_encoder_by_name(codec_name.toLocal8Bit().data());
		if (!m_codec) {
		    QTextStream(stdout) << tr("Codec ") << codec_name << tr(" not found") << endl;
		}
		else {
		    QTextStream(stdout) << tr("Codec ") << codec_name << tr(" found") << endl;
		}
		m_codecContext = avcodec_alloc_context3(m_codec);
		if (!m_codecContext)
		    QTextStream(stdout) << tr("Codec Context couldn't be allocated.") << endl;

		m_pkt = av_packet_alloc();
		if (!m_pkt)
		    QTextStream(stdout) << tr("AV packet couldn't be allocated.") << endl;

		//Initilize basic encoding context: based on https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/encode_video.c
		m_codecContext->bit_rate = 400000;
		m_codecContext->width = m_recordingWidth;
		m_codecContext->height = m_recordingHeight;
//		m_codecContext->time_base = (AVRational){1, m_recordingFrameInterval/1000.0};
//		m_codecContext->framerate = (AVRational){m_recordingFrameInterval/1000.0, 1};
		m_codecContext->time_base = (AVRational){1, 1};
		m_codecContext->framerate = (AVRational){1, 1};
		m_codecContext->gop_size = 10;
		m_codecContext->max_b_frames = 1;
		m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

		if (avcodec_open2(m_codecContext, m_codec, NULL) < 0)
			QTextStream(stdout) << tr("Could not open codec.") << endl;

		m_currentVideoframe = av_frame_alloc();
		if (!m_currentVideoframe)
			QTextStream(stdout) << tr("Could not allocate video frame.") << endl;
		m_currentVideoframe->format = m_codecContext->pix_fmt;
		m_currentVideoframe->width  = m_codecContext->width;
		m_currentVideoframe->height = m_codecContext->height;


		if (av_frame_get_buffer(m_currentVideoframe, 32) < 0) 
			QTextStream(stdout) << tr("Could not allocate the video frame data.") << endl;

		if (av_frame_make_writable(m_currentVideoframe) < 0) 
			QTextStream(stdout) << tr("Could not made the video frame data writable.") << endl;


		m_swsContext = sws_getContext(m_recordingWidth, m_recordingHeight, AV_PIX_FMT_ARGB, m_recordingWidth, m_recordingHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0 );

		m_frameCount = 0;
		m_outFile = fopen(tr("Stadyn_user.mp4").toLocal8Bit().data(), "wb");
	}
	
}

const FeatureList &ScreenshotFeaturePlugin::featureList() const
{
	return m_features;
}

void ScreenshotFeaturePlugin::record()
{
	//QMessageBox::information(nullptr, tr("hola"), tr("mundo"));
	//qDebug() << "recording event";

/*	//iteration on Json categories (incomplete)
	QMapIterator<QString, QVariant> i(m_lastMaster->userConfigurationObject()->data());
	while (i.hasNext())
	{
	    i.next();
	    QTextStream(stdout) << i.key() << ": " << i.value().toString() << endl;
	}
*/
	//Debugging configured values
	qDebug() << tr("x:") << m_recordingWidth << endl;
	qDebug() << tr("y:") << m_recordingHeight << endl;

	if(m_recordingVideo)
	{
		for( const auto& controlInterface : m_lastComputerControlInterfaces )
		{
			QImage image = controlInterface->screen();
			image = image.scaled(m_recordingWidth, m_recordingHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);


			AVFrame* inpic = av_frame_alloc();
			avpicture_fill((AVPicture*)inpic, image.bits(), AV_PIX_FMT_ARGB, image.width(), image.height());
			sws_scale(m_swsContext, inpic->data, inpic->linesize, 0, image.height(), m_currentVideoframe->data, m_currentVideoframe->linesize);


			//avpicture_fill((AVPicture *)m_currentVideoframe, image.bits(), AV_PIX_FMT_RGB24, m_codecContext->width, m_codecContext->height);

			//av_frame_copy(copyFrame, frame);
			//int rgb_stride[3]={3, 0, 0};
			//sws_scale(m_swsContext, (const uint8_t * const *)image.bits(), rgb_stride, 0, m_codecContext->height, m_currentVideoframe->data, m_currentVideoframe->linesize);

			//m_currentVideoframe->data
			m_currentVideoframe->pts = m_frameCount++;			
			int ret = avcodec_send_frame(m_codecContext, m_currentVideoframe);
			if (ret < 0)
        			QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

			while (ret >= 0)
			{
        			ret = avcodec_receive_packet(m_codecContext, m_pkt);
        			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					QTextStream(stdout) << tr("AVERROR or AVERROR_EOF") << endl;
        			else if (ret < 0)
					QTextStream(stdout) << tr("Error during encoding") << endl;

				fwrite(m_pkt->data, 1, m_pkt->size, m_outFile);
				QTextStream(stdout) << tr("Size written: ") << m_pkt->pts << endl;
				av_packet_unref(m_pkt);
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

bool ScreenshotFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
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
			m_recordTimer->stop();
			QMessageBox::information(nullptr, tr("Stopping recording"), tr("Recording is now disabled"));
			if (m_recordingVideo)
			{
				/* flush the encoder */
				//encode(c, NULL, pkt, f);
				int ret = avcodec_send_frame(m_codecContext, NULL);
				if (ret < 0)
					QTextStream(stdout) << tr("Error sending a frame for encoding") << endl;

				while (ret >= 0)
				{
					ret = avcodec_receive_packet(m_codecContext, m_pkt);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
						QTextStream(stdout) << tr("AVERROR or AVERROR_EOF") << endl;
					else if (ret < 0)
						QTextStream(stdout) << tr("Error during encoding") << endl;

					fwrite(m_pkt->data, 1, m_pkt->size, m_outFile);
					av_packet_unref(m_pkt);
				}

				fclose(m_outFile);

				avcodec_free_context(&m_codecContext);
				av_frame_free(&m_currentVideoframe);
				av_packet_free(&m_pkt);
			}
		}


		
//QMessageBox::information(nullptr, tr("hola"), tr("mundo"));

		//for( const auto& controlInterface : computerControlInterfaces )
//		{
//			Screenshot().take( controlInterface );
//
//		}

		//QMessageBox::information( master.mainWindow(),
		//						  tr( "Screenshots taken" ),
		//						  tr( "Screenshot of %1 computer have been taken successfully." ).
		//						  arg( computerControlInterfaces.count() ) );

		return true;
	}

	return true;
}
