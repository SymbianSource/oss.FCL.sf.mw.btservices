/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#include "btabstractdelegate.h"
#include "btqtconstants.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <btengsettings.h>
#include <bluetoothuitrace.h>

/*!
    Constructor.
 */
BtAbstractDelegate::BtAbstractDelegate( BtSettingModel *settingModel, 
        BtDeviceModel *deviceModel, QObject *parent )
    : QObject( parent ), mSettingModel(settingModel), mDeviceModel(deviceModel),
      mExecuting(false)
{
}

/*!
    Destructor.
 */
BtAbstractDelegate::~BtAbstractDelegate()
{
}

bool BtAbstractDelegate::isExecuting()
{
    return mExecuting;
}

BtSettingModel *BtAbstractDelegate::settingModel()
{
    return mSettingModel;
}

BtDeviceModel *BtAbstractDelegate::deviceModel()
{
    return mDeviceModel;
}

void BtAbstractDelegate::cancel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

bool BtAbstractDelegate::isBtPowerOn()
{
    TBTPowerStateValue state(EBTPowerOff);
    CBTEngSettings *settings(0);
    TRAP_IGNORE(settings = CBTEngSettings::NewL());
    if (settings) {
        // error on function call is treated as BT OFF
        (void) settings->GetPowerState(state);
        delete settings;
    }
   BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, state);
   return (EBTPowerOn == state);
}

void BtAbstractDelegate::completeDelegateExecution(int error)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, error);
    setExecuting(false);
    emit delegateCompleted(error, this);
    BOstraceFunctionExit0( DUMMY_DEVLIST );
}

void BtAbstractDelegate::setExecuting(bool status)
{
    mExecuting = status;
}
