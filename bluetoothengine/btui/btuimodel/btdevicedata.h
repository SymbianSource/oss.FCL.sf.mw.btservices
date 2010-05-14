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

#ifndef BTDEVICEDATA_H
#define BTDEVICEDATA_H

#include <qglobal.h>
#include <e32base.h>
#include <btengconnman.h>
#include <btservices/btdevrepository.h>
#include "btuimodel.h"

/*!
    \class BtDeviceData
    \brief class for handling local Bluetooth setting updates.

    BtDeviceData class is responsible for providing the latest information
    regarding the properties of remote devices and the connection status.

    \\sa bluetoothuimodel
 */
class BtDeviceData : public QObject,
                     public MBTEngConnObserver,
                     public MBtDevRepositoryObserver
{
    Q_OBJECT

public:
    BtDeviceData(
            const QSharedPointer<BtuiModelDataSource> &data,
            QObject *parent = 0
            );
    
    virtual ~BtDeviceData();
    
private:
    // from MBTEngConnObserver
    
    void ConnectComplete( TBTDevAddr& addr, TInt err, 
         RBTDevAddrArray* conflicts );

    void DisconnectComplete( TBTDevAddr& addr, TInt err );
    
    // From MBtDeviceRepositoryObserver
    
    void BtDeviceDeleted( const TBTDevAddr& addr );

    void BtDeviceAdded( const CBTDevice& device );
    
    void BtDeviceChanged( const CBTDevice& device, TUint similarity ); 
    
public slots:
    //void activeRequestCompleted( int status, int id );

private:

private:
    QSharedPointer<BtuiModelDataSource> mData;
    
    CBTEngConnMan *mBtengConnMan;
    
    CBtDevRepository* mDeviceRepo;
    
    Q_DISABLE_COPY(BtDeviceData)

};

#endif // BTLOCALSETTING_H
