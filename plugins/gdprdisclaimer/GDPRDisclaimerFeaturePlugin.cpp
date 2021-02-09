/*
 * TextMessageFeaturePlugin.cpp - implementation of TextMessageFeaturePlugin class
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

#include "GDPRDisclaimerFeaturePlugin.h"
#include "FeatureWorkerManager.h"
#include "ComputerControlInterface.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"


GDPRDisclaimerFeaturePlugin::GDPRDisclaimerFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_gdprDisclaimerFeature( Feature( QStringLiteral( "GDPRDisclaimer" ),
								   Feature::Action | Feature::AllComponents,
								   Feature::Uid( "5ce410fc-3c2b-4852-a00f-37373d947b73" ),
								   Feature::Uid(),
								   tr( "GDPR disclaimer" ), {},
								   tr( "Use this function to send a privacy disclaimer to all" ),
								   QStringLiteral(":/gdprdisclaimer/dialog-information.png") ) ),
	m_features( { m_gdprDisclaimerFeature } )
{
}



const FeatureList &GDPRDisclaimerFeaturePlugin::featureList() const
{
	return m_features;
}



bool GDPRDisclaimerFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
											 const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature.uid() != m_gdprDisclaimerFeature.uid() )
	{
		return false;
	}

	QString textMessage;

    FeatureMessage featureMessage( m_gdprDisclaimerFeature.uid(), ShowDisclaimerMessage);
    featureMessage.addArgument( MessageTextArgument, textMessage );
    featureMessage.addArgument( MessageIcon, QMessageBox::Information );

    sendFeatureMessage( featureMessage, computerControlInterfaces );

	return true;
}



bool GDPRDisclaimerFeaturePlugin::stopFeature( VeyonMasterInterface& master, const Feature& feature,
											const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(master);
	Q_UNUSED(feature);
	Q_UNUSED(computerControlInterfaces);

	return false;
}



bool GDPRDisclaimerFeaturePlugin::handleFeatureMessage( VeyonMasterInterface& master, const FeatureMessage& message,
													 ComputerControlInterface::Pointer computerControlInterface )
{
	Q_UNUSED(master);
	Q_UNUSED(message);
	Q_UNUSED(computerControlInterface);

	return false;
}



bool GDPRDisclaimerFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
													 const MessageContext& messageContext,
													 const FeatureMessage& message )
{
	Q_UNUSED(messageContext)

	if( m_gdprDisclaimerFeature.uid() == message.featureUid() )
	{
		// forward message to worker
		if( server.featureWorkerManager().isWorkerRunning( m_gdprDisclaimerFeature ) == false )
		{
			server.featureWorkerManager().startWorker( m_gdprDisclaimerFeature, FeatureWorkerManager::UnmanagedSessionProcess );
		}
		server.featureWorkerManager().sendMessage( message );

		return true;
	}

	return false;
}



bool GDPRDisclaimerFeaturePlugin::handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message )
{
	Q_UNUSED(worker);

	if( message.featureUid() == m_gdprDisclaimerFeature.uid() )
	{
		QMessageBox* messageBox = new QMessageBox( static_cast<QMessageBox::Icon>( message.argument( MessageIcon ).toInt() ),
												   tr( "Message from teacher" ),
												   message.argument( MessageTextArgument ).toString() );
		messageBox->show();

		connect( messageBox, &QMessageBox::accepted, messageBox, &QMessageBox::deleteLater );

		return true;
	}

	return true;
}
