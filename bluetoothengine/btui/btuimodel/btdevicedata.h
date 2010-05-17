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

#include "btuimodeltypes.h"
#include <btdevicemodel.h>

#include <e32base.h>
#include <btservices/btdevrepository.h>
#include <btservices/devdiscoveryobserver.h>


class CAdvanceDevDiscoverer;

/*!
    \class BtDeviceData
    \brief class for providing remote device data.

    BtDeviceData class is responsible for providing the latest information
    regarding the properties of remote devices.

    \\sa bluetoothuimodel
 */
class BtDeviceData : public QObject,
                     public MBtDevRepositoryObserver,
                     public MDevDiscoveryObserver
{
    Q_OBJECT

public:
    explicit BtDeviceData( BtDeviceModel& model, QObject *parent = 0 );
    
    virtual ~BtDeviceData();
    
    bool isValid( int row, int col ) const;
    
    int rowCount() const;
    
    int columnCount() const;
        
    void data(QVariant& val, int row, int col, int role ) const;
    
    BtuiModelDataItem itemData( int row, int col ) const;
    
    bool searchDevice();
    
    void cancelSearchDevice();
    
    void removeTransientDevices();
    
private:
    // From MBtDeviceRepositoryObserver
    
    void RepositoryInitialized();
    
    void DeletedFromRegistry( const TBTDevAddr& addr );
    
    void AddedToRegistry( const CBtDevExtension& dev );
    
    void ChangedInRegistry( const CBtDevExtension& dev, TUint similarity  ); 

    void ServiceConnectionChanged(
            const CBtDevExtension& dev, TBool connected );
    
    // from MDevDiscoveryObserver
    void HandleNextDiscoveryResultL( 
            const TInquirySockAddr& inqAddr, const TDesC& name );

    void HandleDiscoveryCompleted( TInt error );
    
public slots:
    //void activeRequestCompleted( int status, int id );

private:

    void initializeDataStore();

    void updateDeviceProperty(BtuiModelDataItem& qtdev,
            const CBtDevExtension& dev, TUint similarity );
    
    int indexOf( const TBTDevAddr& addr ) const;
    
    void updateRssi(BtuiModelDataItem& qtdev, int rssi );
    
    void setMajorProperty( BtuiModelDataItem& qtdev, int prop, bool addto);
    
    bool isDeviceInRange( const BtuiModelDataItem& qtdev );
    
    bool isDeviceInRegistry( const BtuiModelDataItem& qtdev );
    
private:

    BtuiModelDataSource mData;
    
    BtDeviceModel& mModel;
    
    CBtDevRepository* mDeviceRepo;
    
    CAdvanceDevDiscoverer* mDiscover;
    
    bool isSearchingDevice;
    
    Q_DISABLE_COPY(BtDeviceData)

};

#endif // BTLOCALSETTING_H
