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

#include "btxqsspdebugservice.h"
#include <btxqserviceapi.h>
#include <hbapplication.h>
#include <btengdomainpskeys.h>
#include <e32property.h>


BtxqSspDebugService::BtxqSspDebugService(QObject* parent)
: XQServiceProvider(BluetoothServiceToggleSspDebugMode,parent),
  mCurrentRequestIndex(0),
  mError(0)
{
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), qApp, SLOT(quit()));
}

BtxqSspDebugService::~BtxqSspDebugService()
{

}

void BtxqSspDebugService::toggleSspDebugMode()
{
    mCurrentRequestIndex = setCurrentRequestAsync();
    
    TInt sspDebugMode( EBTSspDebugModeOff );
    
    // Get current status
    TInt err = RProperty::Get( KPSUidBluetoothDutMode, KBTSspDebugmode, 
            sspDebugMode );
    
    if ( !err )
        {
        if( sspDebugMode == EBTDutOff )
            {
            // The Ssp debug mode is turned off, turn it on.
            err = RProperty::Set( KPSUidBluetoothDutMode,
                    KBTSspDebugmode,
                    EBTSspDebugModeOn );
            }
        else
            {
            // The Ssp debug mode is turned on, turn it off.
            err = RProperty::Set( KPSUidBluetoothDutMode,
                    KBTSspDebugmode,
                    EBTSspDebugModeOff );
            }
        }
    
    mError = err;
    
    // Complete the service request with error code, if any.
    QVariant retVal(mError);
    completeRequest(mCurrentRequestIndex, retVal);
}
