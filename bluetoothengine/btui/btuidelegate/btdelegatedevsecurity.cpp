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

#include <QModelIndex>

#include "btdelegatedevsecurity.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbnotificationdialog.h>

BtDelegateDevSecurity::BtDelegateDevSecurity(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, 
        QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtEngDevMan(0)
{
    
}

BtDelegateDevSecurity::~BtDelegateDevSecurity()
{
    delete mBtEngDevMan;
}


void BtDelegateDevSecurity::exec( const QVariant &params )
{
    int error = KErrNone;
    QModelIndex index = params.value<QModelIndex>();
    
    QString strBtAddr = getDeviceModel()->data(index,
            BtDeviceModel::ReadableBdaddrRole).toString();
    
    mdeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
    
    TBTDevAddr symaddr;
    TBuf<KBTDevAddrSize * 2> buffer(strBtAddr.utf16());
    symaddr.SetReadable( buffer );

    CBTDevice *symBtDevice = 0;
    TRAP( error, {
            symBtDevice = CBTDevice::NewL( symaddr );
            if( !mBtEngDevMan) {
                mBtEngDevMan = CBTEngDevMan::NewL( this );
            }
    });
    
    if ( !error ) {
        symBtDevice->SetPaired(EFalse);
        // deleting link key for executing unpair is safe as no 
        // link key shall exist if the device has been unpaired. 
        symBtDevice->DeleteLinkKey();
        error = mBtEngDevMan->ModifyDevice( *symBtDevice );
    }
    delete symBtDevice;
    
    if ( error ) {
        emitCommandComplete(error);
    }
}

void BtDelegateDevSecurity::cancel()
{
    
}

void BtDelegateDevSecurity::HandleDevManComplete( TInt aErr )
{
    emitCommandComplete(aErr);
}

void BtDelegateDevSecurity::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray )
{
    Q_UNUSED(aErr);
    Q_UNUSED(aDeviceArray);
}

void BtDelegateDevSecurity::emitCommandComplete(int error)
{
    QString str(hbTrId("Unpaired to %1"));
    QString err(hbTrId("Unpairing with %1 Failed"));
    
    if(error != KErrNone) {
        HbNotificationDialog::launchDialog(err.arg(mdeviceName));
    }
    else {
        HbNotificationDialog::launchDialog(str.arg(mdeviceName));
    }

    emit commandCompleted(error);
}





