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
 * Description:  
 *
 */

#include "btxqaddrservice.h"
#include <btxqserviceapi.h>
#include <bttypes.h>
#include <bt_subscribe.h>
#include <xqconversions.h>
#include <hbapplication.h>
#include <bluetoothdevicedialogs.h>
#include <hbdevicedialog.h>

const QString deviceDialogType = "com.nokia.hb.btdevicedialog/1.0";


BtxqAddrService::BtxqAddrService(QObject* parent)
: XQServiceProvider(BluetoothServiceShowLocalAddress, parent), mDialog(0), 
mCurrentRequestIndex(0),
mError(0)
{
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), qApp, SLOT(quit()));
}

BtxqAddrService::~BtxqAddrService()
{
    delete mDialog;    
}

void BtxqAddrService::showBTAddr()
{
    mCurrentRequestIndex = setCurrentRequestAsync();
    
    TBuf<KBTDevAddrSize * 3> btAddr;
    TPckgBuf<TBTDevAddr> aDevAddrPckg;
    // Get the local device's Bluetooth address
    TInt err = RProperty::Get(KUidSystemCategory, 
    KPropertyKeyBluetoothGetLocalDeviceAddress, aDevAddrPckg);
 
    aDevAddrPckg().GetReadable(btAddr, KNullDesC, _L(":"), KNullDesC);
    btAddr.UpperCase();
    
    mError = err;
    
    // Prepare and show the dialog.
    if(!mDialog)
        {
        mDialog = new HbDeviceDialog();
        }
 
    connect(mDialog, SIGNAL(deviceDialogClosed()), this, SLOT(handleshowBTAddrCompleted()));
    QVariantMap parameters;
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogType), 
          QString::number(TBluetoothDialogParams::EInformationDialog));
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogTitle),
          QString::number(TBluetoothDialogParams::EBTAddress));
    
    parameters.insert(QString::number(TBluetoothDeviceDialog::ELocalAddress),
          XQConversions::s60DescToQString(btAddr));
    
    mDialog->show(deviceDialogType, parameters);    
}

void BtxqAddrService::handleshowBTAddrCompleted()
{
    // The dialog has been closed.
    // Complete the service request.
    QVariant retVal(mError);
    completeRequest(mCurrentRequestIndex, retVal);
}
