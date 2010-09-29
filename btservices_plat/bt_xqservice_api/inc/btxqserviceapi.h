/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description: Bluetooth-services APIs through Qt Service Framework
 *
 */

#ifndef BTXQSERVICEAPI_H
#define BTXQSERVICEAPI_H

#include <qstring.h>

/*!
 todo: this API may need to move to plat_ API folder of its destination package.
 */

/*!
 The Bluetooth service name.
 */
static const QString BluetoothServiceName("btxqservice");

/*!
 The interface of showing the local Bluetooth device address in a dialog
 */
static const QString BluetoothInterfaceShowLocalAddress("com.nokia.symbian.IShowLocalDeviceAddress");

/*!
 The operation used for showing the local Bluetooth device address in a dialog
 */
static const QString BluetoothShowLocalAddress("showBTAddr()");

/*!
 The full service name of showing the local Bluetooth device address in a dialog
 */
static const QString BluetoothServiceShowLocalAddress = 
        BluetoothServiceName + "." + BluetoothInterfaceShowLocalAddress;

/*!
 The interface of toggling Bluetooth power
 */
static const QString BluetoothInterfaceTogglePower("com.nokia.symbian.ITogglePower");

/*!
 The operation used for toggling the Bluetooth power
 */
static const QString BluetoothTogglePower("togglePower()");

/*!
 The full service name of toggling Bluetooth power
 */
static const QString BluetoothServiceTogglePower = 
        BluetoothServiceName + "." + BluetoothInterfaceTogglePower;

/*!
 The interface of activating Bluetooth device test mode (DUT mode)
 */
static const QString BluetoothInterfaceActivateDutMode("com.nokia.symbian.IActivateDutMode");

/*!
 The operation used for activating the Bluetooth device test mode (DUT mode) 
 */
static const QString BluetoothActivateDutMode("activateDeviceTestMode()");

/*!
 The operation used for activating the Bluetooth device test mode (DUT mode) 
 
 This API is deprecated and should be removed when the client has migrated to new API.
 */
static const QString ActivateDutMode("activateDeviceTestMode()");

/*!
 The full service name of activating Bluetooth device test mode (DUT mode)
 */
static const QString BluetoothServiceActivateDutMode = 
        BluetoothServiceName + "." + BluetoothInterfaceActivateDutMode;

/*!
 The interface of toggling Bluetooth SSP debug mode
 */
static const QString BluetoothInterfaceToggleSspDebugMode("com.nokia.symbian.IToggleSspDebugMode");

/*!
 The operation used for toggling the Bluetooth SSP debug mode 
 */
static const QString BluetoothToggleSspDebugMode("toggleSspDebugMode()");

/*!
 The operation used for toggling the Bluetooth SSP debug mode.
  
 This API is deprecated and should be removed when the client has migrated to new API.
 */
static const QString ToggleSspDebugMode("toggleSspDebugMode()");

/*!
 The full service name of toggling Bluetooth SSP debug mode
 */
static const QString BluetoothServiceToggleSspDebugMode = 
        BluetoothServiceName + "." + BluetoothInterfaceToggleSspDebugMode;

#endif // BTXQSERVICEAPI_H
