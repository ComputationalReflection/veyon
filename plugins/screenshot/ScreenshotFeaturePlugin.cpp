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
	connect(m_recordTimer, SIGNAL(timeout()), this, SLOT(record_video()));
	m_lastMaster = nullptr;

	
	QString codec_name = tr("libx264");

	av_register_all();
	m_codec = avcodec_find_encoder_by_name(codec_name.toLocal8Bit().data());
	if (!m_codec) {
	    QTextStream(stdout) << tr("Codec ") << codec_name << tr(" not found") << endl;
	}
	else {
	    QTextStream(stdout) << tr("Codec ") << codec_name << tr(" found") << endl;
	}
}



const FeatureList &ScreenshotFeaturePlugin::featureList() const
{
	return m_features;
}

void ScreenshotFeaturePlugin::record_video()
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
	int width = m_lastMaster->userConfigurationObject()->value(tr("VideoResX"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	int heigth = m_lastMaster->userConfigurationObject()->value(tr("VideoResY"), tr("Uniovi.Reflection"), QVariant(0)).toInt();
	qDebug() << tr("x:") << m_lastMaster->userConfigurationObject()->value(tr("VideoResX"), tr("Uniovi.Reflection"), QVariant(0)).toString() << endl;
	qDebug() << tr("y:") << m_lastMaster->userConfigurationObject()->value(tr("VideoResY"), tr("Uniovi.Reflection"), QVariant(0)).toString() << endl;

	if(m_lastMaster->userConfigurationObject()->value(tr("SaveVideo"), tr("Uniovi.Reflection"), QVariant(false)).toBool())
	    qDebug() << "Save Video" << endl;
	else
    	    qDebug() << "Do not save Video" << endl;


	if(m_lastMaster->userConfigurationObject()->value(tr("SaveScreenshots"), tr("Uniovi.Reflection"), QVariant(false)).toBool())
	{
		for( const auto& controlInterface : m_lastComputerControlInterfaces )
		{
			//This is a simplified version of the code in Screenshot::take(). In this case no label is added to png image
			const auto dir = VeyonCore::filesystem().expandPath( VeyonCore::config().screenshotDirectory() );
			QString fileName = dir + QDir::separator() + Screenshot::constructFileName( controlInterface->userLoginName(), controlInterface->computer().hostAddress() );
			QImage image = controlInterface->screen();
			if(width != 0 && heigth != 0)
				image = image.scaled(width, heigth, Qt::IgnoreAspectRatio, Qt::FastTransformation);
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
		m_lastMaster = &master;
		m_lastComputerControlInterfaces = computerControlInterfaces;
		if( m_recordEnabled == false)
		{
			m_recordEnabled = true;
			int interval = m_lastMaster->userConfigurationObject()->value(tr("VideoFrameInterval"), tr("Uniovi.Reflection"), QVariant(10000)).toInt();
			qDebug() << tr("VideoFrameInterval") << interval << endl;
			m_recordTimer->start(interval);
			QMessageBox::information(nullptr, tr("recording"), tr("recording enabled"));
		}
		else
		{
			m_recordEnabled = false;
			m_recordTimer->stop();
			QMessageBox::information(nullptr, tr("recording"), tr("recording disabled"));
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
