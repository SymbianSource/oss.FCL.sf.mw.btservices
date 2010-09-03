/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  
 *
 */

#include "btxqdutservice.h"
#include <btxqserviceapi.h>
#include <hbapplication.h>
#include <hbdevicedialog.h>
#include <bluetoothdevicedialogs.h>
#include <btengdomainpskeys.h>
#include <e32property.h>

const QString deviceDialogType = "com.nokia.hb.btdevicedialog/1.0";


BtxqDutService::BtxqDutService(QObject* parent)
: XQServiceProvider(BluetoothServiceActivateDutMode,parent), mNotificationDialog(0), 
  mCurrentRequestIndex(0),
  mError(0)
{
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), qApp, SLOT(quit()));
}

BtxqDutService::~BtxqDutService()
{
    delete mNotificationDialog;
}

void BtxqDutService::activateDeviceTestMode()
{
    mCurrentRequestIndex = setCurrentRequestAsync();
    
    TInt currentDUTModeStatus( EBTDutOff );
    
    // Get current status of BT loopback
    TInt err = RProperty::Get( KPSUidBluetoothDutMode, KBTDutEnabled, 
            currentDUTModeStatus );
    
    // If the DUT mode is not already activated, activate it.
    if ( currentDUTModeStatus == EBTDutOff && err == KErrNone )
        {
        err = RProperty::Set( KPSUidBluetoothDutMode,
                KBTDutEnabled,
                EBTDutOn );
        }
    
    mError = err;
    
    if(!mError)
        {
        // Prepare and show the dialog.
        if(!mNotificationDialog)
            {
            mNotificationDialog = new HbDeviceDialog();
            }
        
        connect(mNotificationDialog, SIGNAL(deviceDialogClosed()), this, SLOT(dutModeActivated()));
        QVariantMap parameters;
        
        parameters.insert(QString::number(TBluetoothDialogParams::EDialogType), 
              QString::number(TBluetoothDialogParams::EGlobalNotif));
        
        parameters.insert(QString::number(TBluetoothDialogParams::EResource),
                QString::number(EBluetoothTestMode));
        
        mNotificationDialog->show(deviceDialogType, parameters);    
        }
    else
        {
        // If there is an error in activating the DUT mode, complete the service request with error code.
        dutModeActivated();
        }
}

void BtxqDutService::dutModeActivated()
{
    // Completes the service request.
    QVariant retVal(mError);
    completeRequest(mCurrentRequestIndex, retVal);
}
