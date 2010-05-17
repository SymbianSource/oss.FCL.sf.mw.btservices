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
#include "btdelegatedisconnect.h"
#include <QModelIndex>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbnotificationdialog.h>

BtDelegateDisconnect::BtDelegateDisconnect(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtengConnMan(0)
{
    
}

BtDelegateDisconnect::~BtDelegateDisconnect()
{
    delete mBtengConnMan;
}

void BtDelegateDisconnect::exec( const QVariant &params )
{
    int error = KErrNone;
    QModelIndex index = params.value<QModelIndex>();
    
    mdeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
    
    QString strBtAddr = getDeviceModel()->data(index,BtDeviceModel::ReadableBdaddrRole).toString();
    
    if ( ! mBtengConnMan ){
        TRAP_IGNORE( mBtengConnMan = CBTEngConnMan::NewL(this) );
    }
    Q_CHECK_PTR( mBtengConnMan );
    
    TPtrC ptrName(reinterpret_cast<const TText*>(strBtAddr.constData()));
        
    RBuf16 btName;
    error = btName.Create(ptrName.Length());
    
    if(error == KErrNone) {
        btName.Copy(ptrName);
        mBtEngddr.SetReadable(btName);
        error = mBtengConnMan->Disconnect(mBtEngddr, EBTDiscGraceful);
        btName.Close();
    }
    
    
    if(error) {
        emitCommandComplete(error);
    }
    
}

void BtDelegateDisconnect::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aConflicts);  
    Q_UNUSED(aErr);
}

void BtDelegateDisconnect::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    emitCommandComplete(aErr);    
}

void BtDelegateDisconnect::PairingComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);
}

void BtDelegateDisconnect::cancel()
{
    
}

void BtDelegateDisconnect::emitCommandComplete(int error)
{
    QString str(hbTrId("Disconnected to %1"));
    QString err(hbTrId("Disconnecting with %1 Failed"));
    
    if(error != KErrNone) {
        HbNotificationDialog::launchDialog(err.arg(mdeviceName));
    }
    else {
        HbNotificationDialog::launchDialog(str.arg(mdeviceName));
    }

    emit commandCompleted(error);
}


