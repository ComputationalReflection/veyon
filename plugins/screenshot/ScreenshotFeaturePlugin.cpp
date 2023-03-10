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

#include "ScreenshotFeaturePlugin.h"
#include "ComputerControlInterface.h"
#include "VeyonMasterInterface.h"
#include "Screenshot.h"


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
	m_autoShotEnabled = false;
	m_autoTimer = new QTimer(this);
	connect(m_autoTimer, SIGNAL(timeout()), this, SLOT(saveScreenshots()));
}



const FeatureList &ScreenshotFeaturePlugin::featureList() const
{
	return m_features;
}


void ScreenshotFeaturePlugin::saveScreenshots()
{
	if(m_autoShotEnabled)
	{
		for( const auto& controlInterface : m_lastComputerControlInterfaces )
		{
			Screenshot().take( controlInterface );
		}
	}
}


bool ScreenshotFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
											const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature.uid() == m_screenshotFeature.uid() )
	{
		m_lastComputerControlInterfaces = computerControlInterfaces;
		if( m_autoShotEnabled == false)
		{
			QMessageBox::information( master.mainWindow(),
								  tr( "Starting Auto Screenshots" ),
								  tr( "Screenshot of %1 computer have been taken successfully." ).
								  arg( computerControlInterfaces.count() ) );
			int interval = 1000; //1 second?
			m_autoTimer->start(interval);
			m_autoShotEnabled = true;
		}
		else
		{
			m_autoTimer->stop();
			m_autoShotEnabled = false;
			QMessageBox::information(nullptr, tr("Stopping Auto Screenshots"), tr("Screenshots are now disabled"));
		}
		return true;
	}

	return true;
}


