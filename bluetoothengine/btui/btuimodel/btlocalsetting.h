/*
* ============================================================================
*  Name        : btuimsettings.h
*  Part of     : BluetoothUI / bluetoothuimodel       *** Info from the SWAD
*  Description : Declaration of the class representing the Bluetooth
*                settings source data.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.2
*/

#ifndef BTLOCALSETTING_H
#define BTLOCALSETTING_H

#include <qglobal.h>
#include <e32base.h>
#include <e32property.h>
#include <btengsettings.h>
#include <btservices/btsimpleactive.h>

#include "btuimodel.h"

/*!
    \class BtuimSettings
    \brief class for handling local Bluetooth setting updates.

    BtLocalSetting class is responsible for providing the latest information
    regarding the local Bluetooth settings such as device name and power state.

    \\sa bluetoothuimodel
 */
class BtLocalSetting : public QObject,
                      public MBTEngSettingsObserver,
                      public MBtSimpleActiveObserver
{
    Q_OBJECT

public:
    explicit BtLocalSetting( BtuiModel& model, QObject *parent = 0 );
    
    virtual ~BtLocalSetting();
    
    bool isValid( int col) const;
    
    int itemCount() const;
        
    void data(QVariant& val, int col, int role ) const;
    
    BtuiModelDataItem itemData( int col ) const;

signals:

    void settingDataChanged( int row, int column, void *parent );    
    
private:
    // from MBTEngSettingsObserver
    
    void PowerStateChanged( TBTPowerStateValue state );
    
    void VisibilityModeChanged( TBTVisibilityMode state );
    
    // from MBtSimpleActiveObserver
    
    void RequestCompletedL( CBtSimpleActive* active, TInt status );

    void CancelRequest( TInt requestId );

    void HandleError( CBtSimpleActive* active, TInt error );
    

    
public slots:
    //void activeRequestCompleted( int status, int id );

private:

    void setVisibilityMode( TBTVisibilityMode state );
    void updateDeviceName( const QString &name );
    
    void setPowerSetting( TBTPowerStateValue state );
    
    //void setOfflineSetting( bool state );
    //void setBtConnectionsSetting( int connections );
    
    void getNameFromRegistry( QString &name );

private:
    BtuiModelDataSource mData;
    
    BtuiModel& mModel;
    
    CBTEngSettings *mBtengSetting;
    
    // For monitoring local device name change
    RProperty mLocalDeviceKey;
    CBtSimpleActive *mLocalDeviceWatcher;
    
    //RProperty mBtLinkCountKey;
    //CBTEngActive *mBtLinkCountWatcher;
    Q_DISABLE_COPY(BtLocalSetting)

};

#endif // BTLOCALSETTING_H
