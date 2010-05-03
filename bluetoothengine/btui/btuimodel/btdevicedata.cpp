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


#include "btdevicedata.h"

/*!
    Constructor.
 */
BtDeviceData::BtDeviceData(
        const QSharedPointer<BtuiModelDataSource> &data,
        QObject *parent)
    : QObject( parent ), mData( data ), mBtengConnMan(0)
{
    TRAP_IGNORE({
        mBtengConnMan = CBTEngConnMan::NewL( this );
        mDeviceRepo = CBtDevRepository::NewL( );
    });
    
    Q_CHECK_PTR( mBtengConnMan );
    Q_CHECK_PTR( mDeviceRepo );
}

/*!
    Destructor.
 */
BtDeviceData::~BtDeviceData()
{
    delete mBtengConnMan;
    delete mDeviceRepo;
}

void BtDeviceData::ConnectComplete( TBTDevAddr& addr, TInt err, 
        RBTDevAddrArray* conflicts ) 
{
    Q_UNUSED( addr );
    Q_UNUSED( err );
    Q_UNUSED( conflicts );
}

void BtDeviceData::DisconnectComplete( TBTDevAddr& addr, TInt err ) 
{
    Q_UNUSED( addr );
    Q_UNUSED( err );
}

void BtDeviceData::BtDeviceDeleted( const TBTDevAddr& addr ) 
{
    Q_UNUSED( addr );
}

void BtDeviceData::BtDeviceAdded( const CBTDevice& device ) 
{
    Q_UNUSED( device );
}

void BtDeviceData::BtDeviceChanged( const CBTDevice& device, TUint similarity )
{
    Q_UNUSED( device );
    Q_UNUSED( similarity );
}

