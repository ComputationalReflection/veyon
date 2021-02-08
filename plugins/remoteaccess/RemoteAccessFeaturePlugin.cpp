/*
 * RemoteAccessFeaturePlugin.cpp - implementation of RemoteAccessFeaturePlugin class
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

#include <QApplication>
#include <QInputDialog>

#include "AuthenticationCredentials.h"
#include "RemoteAccessFeaturePlugin.h"
#include "RemoteAccessWidget.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"
#include "FeatureWorkerManager.h"
#include "PlatformCoreFunctions.h"
#include "VeyonConnection.h"



RemoteAccessFeaturePlugin::RemoteAccessFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_remoteViewFeature( QStringLiteral( "RemoteView" ),
						 Feature::Session | Feature::Master,
						 Feature::Uid( "a18e545b-1321-4d4e-ac34-adc421c6e9c8" ),
						 Feature::Uid(),
						 tr( "Remote view" ), {},
						 tr( "Open a remote view for a computer without interaction." ),
						 QStringLiteral(":/remoteaccess/kmag.png") ),
	m_remoteControlFeature( QStringLiteral( "RemoteControl" ),
							Feature::Session | Feature::Master,
							Feature::Uid( "ca00ad68-1709-4abe-85e2-48dff6ccf8a2" ),
							Feature::Uid(),
							tr( "Remote control" ), {},
							tr( "Open a remote control window for a computer." ),
							QStringLiteral(":/remoteaccess/krdc.png") ),
	m_features( { m_remoteViewFeature, m_remoteControlFeature } ),
	m_commands( {
{ QStringLiteral("view"), m_remoteViewFeature.displayName() },
{ QStringLiteral("control"), m_remoteControlFeature.displayName() },
{ QStringLiteral("help"), tr( "Show help about command" ) },
				} )
{
}



const FeatureList &RemoteAccessFeaturePlugin::featureList() const
{
	return m_features;
}



bool RemoteAccessFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
											  const ComputerControlInterfaceList& computerControlInterfaces )
{
	// determine which computer to access and ask if neccessary
	ComputerControlInterface::Pointer remoteAccessComputer;

	if( ( feature.uid() == m_remoteViewFeature.uid() ||
		  feature.uid() == m_remoteControlFeature.uid() ) &&
			computerControlInterfaces.count() != 1 )
	{
		QString hostName = QInputDialog::getText( master.mainWindow(),
												  tr( "Remote access" ),
												  tr( "Please enter the hostname or IP address of the computer to access:" ) );
		if( hostName.isEmpty() )
		{
			return false;
		}

		Computer customComputer;
		customComputer.setHostAddress( hostName );
		customComputer.setName( hostName );
		remoteAccessComputer = ComputerControlInterface::Pointer::create( customComputer );
	}
	else if( computerControlInterfaces.count() >= 1 )
	{
		remoteAccessComputer = computerControlInterfaces.first();
	}

	if( remoteAccessComputer.isNull() )
	{
		return false;
	}

	if( feature.uid() == m_remoteViewFeature.uid() )
	{
		sendFeatureMessage( FeatureMessage( m_remoteViewFeature.uid(), FeatureMessage::DefaultCommand ),
									computerControlInterfaces );
		new RemoteAccessWidget( remoteAccessComputer, this, true );

		return true;
	}
	else if( feature.uid() == m_remoteControlFeature.uid() )
	{
		sendFeatureMessage( FeatureMessage( m_remoteControlFeature.uid(), FeatureMessage::DefaultCommand ),
									computerControlInterfaces );
		new RemoteAccessWidget( remoteAccessComputer, this, false );

		return true;
	}

	return false;
}

bool RemoteAccessFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
													  const MessageContext& messageContext,
													  const FeatureMessage& message )
{
	Q_UNUSED(messageContext)
	auto& featureWorkerManager = server.featureWorkerManager();

	if( message.featureUid() == m_remoteViewFeature.uid() )
	{
		vWarning() << "PASA POR AQUI? VIEW";
		featureWorkerManager.startWorker( m_remoteViewFeature, FeatureWorkerManager::ManagedSystemProcess );
		featureWorkerManager.sendMessage( message );

	}
	else if( message.featureUid() == m_remoteControlFeature.uid() )
	{
		vWarning() << "PASA POR AQUI? CONTROL";
		featureWorkerManager.startWorker( m_remoteControlFeature, FeatureWorkerManager::ManagedSystemProcess );
		featureWorkerManager.sendMessage( message );
	}
	else
	{
		return false;
	}

	return true;
}

bool RemoteAccessFeaturePlugin::handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message )
{
	Q_UNUSED(worker)
	vWarning() << "Mensaje recibido en RemoteAccessFeaturePlugin::handleFeatureMessage";
	vWarning() << "feature Id: " << message.featureUid();

	if( message.featureUid() == m_remoteControlFeature.uid() )
	{
		QMessageBox m( QMessageBox::Question, tr( "A remote user wants to CONTROL you computer" ),
				   tr( "Do you wnat to allow REMOTE CONTROL?" ),
				   QMessageBox::Yes | QMessageBox::No );
		m.show();
		VeyonCore::platform().coreFunctions().raiseWindow( &m, true );

		if( m.exec() == QMessageBox::No )
		{
			return false;
		}
		return true;
	}

	return false;
}

QStringList RemoteAccessFeaturePlugin::commands() const
{
	return m_commands.keys();
}



QString RemoteAccessFeaturePlugin::commandHelp( const QString& command ) const
{
	return m_commands.value( command );
}



CommandLinePluginInterface::RunResult RemoteAccessFeaturePlugin::handle_view( const QStringList& arguments )
{
	if( arguments.count() < 1 )
	{
		return NotEnoughArguments;
	}

	return remoteAccess( arguments.first(), true ) ? Successful : Failed;
}



CommandLinePluginInterface::RunResult RemoteAccessFeaturePlugin::handle_control( const QStringList& arguments )
{
	if( arguments.count() < 1 )
	{
		return NotEnoughArguments;
	}

	return remoteAccess( arguments.first(), false ) ? Successful : Failed;
}



CommandLinePluginInterface::RunResult RemoteAccessFeaturePlugin::handle_help( const QStringList& arguments )
{
	if( arguments.value( 0 ) == QLatin1String("view") )
	{
		printf( "\nremoteaccess view <host>\n\n" );
		return NoResult;
	}
	else if( arguments.value( 0 ) == QLatin1String("control") )
	{
		printf( "\nremoteaccess control <host>\n}n" );
		return NoResult;
	}

	return InvalidCommand;
}



bool RemoteAccessFeaturePlugin::initAuthentication()
{
	if( VeyonCore::instance()->initAuthentication() == false )
	{
		vWarning() << "Could not initialize authentication";
		return false;
	}

	return true;
}



bool RemoteAccessFeaturePlugin::remoteAccess( const QString& hostAddress, bool viewOnly )
{
	if( initAuthentication() == false )
	{
		return false;
	}

	Computer remoteComputer;
	remoteComputer.setName( hostAddress );
	remoteComputer.setHostAddress( hostAddress );

	new RemoteAccessWidget( ComputerControlInterface::Pointer::create( remoteComputer ), this, viewOnly );

	qApp->exec();

	return true;
}


void RemoteAccessFeaturePlugin::notifyRemoteControlRequest(VeyonConnection* connection)
{
    vWarning() << "WIDGET set REMOTE CONTROL";
    connection->sendFeatureMessage( FeatureMessage( m_remoteControlFeature.uid(), FeatureMessage::DefaultCommand ), false);
}


void RemoteAccessFeaturePlugin::notifyRemoteControlRequest(const ComputerControlInterface::Pointer& server)
{
    vWarning() << "WIDGET set REMOTE CONTROL ComputerInterface";
    server->sendFeatureMessage( FeatureMessage( m_remoteControlFeature.uid(), FeatureMessage::DefaultCommand ), false);
}

