/*
 * UserConfig.h - UserConfig class
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

#include <QJsonArray>

#include "Configuration/Object.h"
#include "Configuration/Property.h"
#include "ComputerMonitoringWidget.h"
#include "VeyonMaster.h"

// clazy:excludeall=ctor-missing-parent-argument,copyable-polymorphic

class UserConfig : public Configuration::Object
{
	Q_OBJECT
public:
	explicit UserConfig( Configuration::Store::Backend backend );

#define FOREACH_PERSONAL_CONFIG_PROPERTY(OP)						\
	OP( UserConfig, VeyonMaster::userConfig(), QJsonArray, checkedNetworkObjects, setCheckedNetworkObjects, "CheckedNetworkObjects", "UI", QJsonArray(), Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), QJsonArray, computerPositions, setComputerPositions, "ComputerPositions", "UI", QJsonArray(), Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), bool, useCustomComputerPositions, setUseCustomComputerPositions, "UseCustomComputerPositions", "UI", false, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), bool, filterPoweredOnComputers, setFilterPoweredOnComputers, "FilterPoweredOnComputers", "UI", false, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), int, monitoringScreenSize, setMonitoringScreenSize, "MonitoringScreenSize", "UI", ComputerMonitoringWidget::DefaultComputerScreenSize, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), int, defaultRole, setDefaultRole, "DefaultRole", "Authentication", 0, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), bool, toolButtonIconOnlyMode, setToolButtonIconOnlyMode, "ToolButtonIconOnlyMode", "UI", false, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), bool, noToolTips, setNoToolTips, "NoToolTips", "UI", false, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), QString, windowState, setWindowState, "WindowState", "UI", QString(), Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), QString, windowGeometry, setWindowGeometry, "WindowGeometry", "UI", QString(), Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), int, width, setWidth, "Width", "Plugin.Record", 1280, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), int, heigth, setHeigth, "Heigth", "Plugin.Record", 720, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), bool, video, setVideo, "Video", "Plugin.Record", true, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), QString, startDateTime, setStartDateTime, "StartDateTime", "Plugin.Record", QString(), Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), QString, endDateTime, setEndDateTime, "EndDateTime", "Plugin.Record", QString(), Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), int, captureIntervalNum, setCaptureIntervalNum, "CaptureIntervalNum", "Plugin.Record", 1, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), int, captureIntervalDen, setCaptureIntervalDen, "CaptureIntervalDen", "Plugin.Record", 1, Configuration::Property::Flag::Standard )	\
	OP( UserConfig, VeyonMaster::userConfig(), QString, savePath, setSavePath, "SavePath", "Plugin.Record", QString(), Configuration::Property::Flag::Standard )	\


	FOREACH_PERSONAL_CONFIG_PROPERTY(DECLARE_CONFIG_PROPERTY)

} ;

