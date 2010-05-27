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

#include "btdelegateconnect.h"
#include <QModelIndex>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbnotificationdialog.h>

BtDelegateConnect::BtDelegateConnect(
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtengConnMan(0)
{
    
}

BtDelegateConnect::~BtDelegateConnect()
{
    delete mBtengConnMan;
}

void BtDelegateConnect::exec( const QVariant &params )
{
    int error = KErrNone;
    QModelIndex index = params.value<QModelIndex>();
    
    mdeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
    
    QString strBtAddr = getDeviceModel()->data(index,
            BtDeviceModel::ReadableBdaddrRole).toString();
    int cod = getDeviceModel()->data(index,BtDeviceModel::CoDRole).toInt();
    
    if ( ! mBtengConnMan ){
        TRAP_IGNORE( mBtengConnMan = CBTEngConnMan::NewL(this) );
    }
    Q_CHECK_PTR( mBtengConnMan );
    
    TBTDeviceClass btEngDeviceClass(cod);
    TPtrC ptrName(reinterpret_cast<const TText*>(strBtAddr.constData()));
        
    RBuf16 btName;
    error = btName.Create(ptrName.Length());
    
    if(error == KErrNone) {
        btName.Copy(ptrName);
        mBtEngddr.SetReadable(btName);
        error = mBtengConnMan->Connect(mBtEngddr, btEngDeviceClass);
        btName.Close();
    }
    
    
    if(error) {
        emitCommandComplete(error);
    }
    
}

void BtDelegateConnect::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aConflicts);  
    emitCommandComplete(aErr);
}

void BtDelegateConnect::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
}

void BtDelegateConnect::PairingComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);
}

void BtDelegateConnect::cancel()
{
    mBtengConnMan->CancelConnect(mBtEngddr);
}

void BtDelegateConnect::emitCommandComplete(int error)
{
    QString str(hbTrId("Connected to %1"));
    QString err(hbTrId("Connecting with %1 Failed"));
    
    if(error != KErrNone) {
        HbNotificationDialog::launchDialog(err.arg(mdeviceName));
    }
    else {
        HbNotificationDialog::launchDialog(str.arg(mdeviceName));
    }

    emit commandCompleted(error);
}


