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


#include "btlocalsetting.h"
#include <btdevice.h>
//#include <QStringList>
#include <btmanclient.h>
#include <bt_subscribe.h>
//#include <centralrepository.h>
//#include <coreapplicationuissdkcrkeys.h>
#include "btqtconstants.h"

const int KLocalDeviceNameWatcher = 10;
const int KBtLinkCountWatcher = 11;

/*!
    Constructor.
 */
BtLocalSetting::BtLocalSetting( BtuiModel& model, QObject *parent )
    : QObject( parent ), mModel(model), mLocalDeviceWatcher(0)
    {
    int err( 0 );
    if (!err ) {
        err = mLocalDeviceKey.Attach( KPropertyUidBluetoothCategory, 
                    KPropertyKeyBluetoothGetRegistryTableChange );
    }
    
    Q_CHECK_PTR( !err ); // other proper alternative?

    TRAP_IGNORE({
        mBtengSetting = CBTEngSettings::NewL( this );
        mLocalDeviceWatcher = CBtSimpleActive::NewL(*this, KLocalDeviceNameWatcher );
    });
    
    Q_CHECK_PTR( mBtengSetting );
    Q_CHECK_PTR( mLocalDeviceWatcher );

    for ( int i = 0; i < BtuiModel::LocalSettingColCount; ++i ) {
        // Initialize the list with empty values.
        mData.append( BtuiModelDataItem() );
    }
    
    // subscribe to local device table change:
    mLocalDeviceKey.Subscribe( mLocalDeviceWatcher->RequestStatus() );
    mLocalDeviceWatcher->GoActive();
    
    // Get the device name
    TBTDeviceName  deviceName;
    (void) mBtengSetting->GetLocalName( deviceName );
    updateDeviceName( QString::fromUtf16( deviceName.Ptr(), deviceName.Length() ) );
    
    // Get the power setting.
    TBTPowerStateValue power( EBTPowerOff );
    (void) mBtengSetting->GetPowerState( power );
    setPowerSetting( power );
    
    // Get the visibility mode
    TBTVisibilityMode visibilityMode( EBTVisibilityModeNoScans );
    (void) mBtengSetting->GetVisibilityMode( visibilityMode );
    setVisibilityMode( visibilityMode );
}

/*!
    Destructor.
 */
BtLocalSetting::~BtLocalSetting()
{
    // delete main data structure
    delete mBtengSetting;
    delete mLocalDeviceWatcher;
    mLocalDeviceKey.Close();
    
    // delete mBtLinkCountWatcher;
    //mBtLinkCountKey.Close();
}


bool BtLocalSetting::isValid( int column) const
{
    return column < mData.count();
}

int BtLocalSetting::itemCount() const
{
    return mData.count();
}

void BtLocalSetting::data(QVariant& val, int col, int role ) const
{
    if ( isValid( col ) ) {
        val = mData.at( col ).value( role );
    }
    else {
        val = QVariant( QVariant::Invalid );
    }
}

BtuiModelDataItem BtLocalSetting::itemData( int col ) const
{
    if ( isValid( col ) ) {
        return mData.at( col );
    }
    return BtuiModelDataItem();
}


/*!
    Provides notification of changes in the power state
    of the Bluetooth hardware.

    @param state EBTPowerOff if the BT hardware has been turned off,
                 EBTPowerOn if it has been turned on.
 */
void BtLocalSetting::PowerStateChanged( TBTPowerStateValue state ) 
{
    setPowerSetting( state );
    emit settingDataChanged( BtuiModel::LocalSettingRow, BtuiModel::PowerStateCol, this );
}

/*!
    Provides notification of changes in the discoverability
    mode of the Bluetooth hardware.
    @param state EBTDiscModeHidden if the BT hardware is in hidden mode,
                  EBTDiscModeGeneral if it is in visible mode.
 */
void BtLocalSetting::VisibilityModeChanged( TBTVisibilityMode state )
{
    setVisibilityMode( state );
    emit settingDataChanged( BtuiModel::LocalSettingRow, BtuiModel::VisibilityCol, this );
}

void BtLocalSetting::RequestCompletedL( CBtSimpleActive* active, TInt status ) {
    Q_UNUSED( active );
    Q_UNUSED( status );
    if ( active->RequestId() == KLocalDeviceNameWatcher ) {
        mLocalDeviceKey.Subscribe( mLocalDeviceWatcher->RequestStatus() );
        mLocalDeviceWatcher->GoActive();
        updateDeviceName( QString() );
    }
}

void BtLocalSetting::CancelRequest( TInt requestId ) {
    if ( requestId == KLocalDeviceNameWatcher ) {
        mLocalDeviceKey.Cancel();
    }
    else if ( requestId == KBtLinkCountWatcher ) {
        //mBtLinkCountKey.Cancel();
    }
}

void BtLocalSetting::HandleError( CBtSimpleActive* active, TInt error ) {
    Q_UNUSED( active );
    Q_UNUSED( error );
}

/*!
    Update local Bluetooth device name in the data store.
    @param name the latest Bluetooth name.
 */
void BtLocalSetting::updateDeviceName( const QString &name ) 
{
    // To-do: the data structure initialization is not impled yet in the model
    BtuiModelDataItem& item = 
            mData[ BtuiModel::BluetoothNameCol ];
    
    if ( item.isEmpty() ) {
        // Initialize with additional information on the setting
        item[ BtuiModel::SettingIdentity ] = QVariant( tr( "Local Bluetooth name" ) );
    }
    
    bool setByUser = !name.isEmpty();
    
    // The additional parameter is the flag indicating whether the 
    // Bluetooth name has been set by the user.
    // The flag is set to true if the name has been set.    
    // requirement does not 
    //nitem[ BtuiModel::SettingValueParam ] = QVariant( setByUser );
    
    QString resolvedName( name );
    if ( resolvedName.isEmpty() ) {
        // We get the default name as suggestion for the user to set.
        getNameFromRegistry( resolvedName );
    }
    item[ BtuiModel::settingDisplay ] = QVariant( resolvedName );
    item[ BtuiModel::SettingValue ] = QVariant( resolvedName );
}

/*!
    Updates all values related to the power setting.
 */
void BtLocalSetting::setPowerSetting( TBTPowerStateValue state )
{
    BtuiModelDataItem& item = 
            mData[ BtuiModel::PowerStateCol ];
    if ( item.isEmpty() ) {
        // Initialize with additional information on the setting
        item[ BtuiModel::SettingIdentity ] = QVariant( tr( "Bluetooth power" ) );
    }
    
    bool powerOn = ( state == EBTPowerOn );

    item[ BtuiModel::settingDisplay ] = 
            powerOn ? QVariant( tr( "On" ) ) : QVariant( tr( "Off" ) );
    item[ BtuiModel::SettingValue ] = QVariant( powerOn );
}

void BtLocalSetting::setVisibilityMode( TBTVisibilityMode state )
{
    BtuiModelDataItem& item = mData[ BtuiModel::VisibilityCol ];

    if ( item.isEmpty() ) {
        item[ BtuiModel::SettingIdentity ] = QVariant( tr( "Phone visibility" ) );
    }
    
    if ( state == EBTVisibilityModeHidden )
        {
        item [ BtuiModel::settingDisplay ] = QVariant( tr( "Hidden" ) );
        }
    else if ( state == EBTVisibilityModeGeneral )
        {
        item [ BtuiModel::settingDisplay ] = QVariant( tr( "Visible" ) );
        }
    else
        {
        item [ BtuiModel::settingDisplay ] = QVariant( tr( "Temporarily visible" ) );
        }
    item [ BtuiModel::SettingValue ] = QVariant( QtVisibilityMode(state) );
}

/*!
    Get local Bluetooth device name from BTRegistry.
 */
void BtLocalSetting::getNameFromRegistry( QString &name )
{
    RBTRegServ btRegServ;   // Session with BTMan
    RBTLocalDevice btReg;   // Subsession with local device table
    TBTLocalDevice localDev;// Data structure holding local device information

    TInt err = btRegServ.Connect();
    if ( !err ) {
        err = btReg.Open( btRegServ );
    }
    if ( !err ) {
        // Read the BT local name from BT Registry.
        err = btReg.Get( localDev );
    }
    if ( !err ) {
        name = QString::fromUtf8( (const char*) localDev.DeviceName().Ptr(), (int) localDev.DeviceName().Length() );
    }
    btReg.Close();
    btRegServ.Close();
}
