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


#include "btdelegateinquiry.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <bluetoothuitrace.h>


BtDelegateInquiry::BtDelegateInquiry(
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject* parent )
    :BtAbstractDelegate( settingModel, deviceModel, parent )
{
}

BtDelegateInquiry::~BtDelegateInquiry()
{

}

void BtDelegateInquiry::exec( const QVariant& params )
{
    Q_UNUSED(params);
    
    bool err = getDeviceModel()->searchDevice();
    if (!err) {
        // TODO - Verify the error code to be used.
        // Complete command with error
        emit commandCompleted(KErrNotSupported);
        return;
    }
    
    emit commandCompleted(KErrNone);
}

void BtDelegateInquiry::cancel()
{
    getDeviceModel()->cancelSearchDevice();
}
