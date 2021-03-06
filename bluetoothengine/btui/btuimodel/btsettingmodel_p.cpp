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

#include "btsettingmodel_p.h"
#include <btdevice.h>
#include <btmanclient.h>
#include <bt_subscribe.h>
#include "btqtconstants.h"

const int KLocalDeviceNameWatcher = 10;
const int KBtLinkCountWatcher = 11;

/*!
    Constructor.
 */
BtSettingModelPrivate::BtSettingModelPrivate( BtSettingModel& model, QObject *parent )
    : QObject( parent), mModel( model ), mLocalDeviceWatcher(0)
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

    for ( int i = 0; i < BtSettingModel::LocalSettingRowCount; ++i ) {
        // Initialize the list with empty values.
        mData.append( BtuiModelDataItem() );
    }
    
    // subscribe to local device table change:
    mLocalDeviceKey.Subscribe( mLocalDeviceWatcher->RequestStatus() );
    mLocalDeviceWatcher->GoActive();
    
    // At initialization, we do not need to handle the return value
    (void) updateLocalDeviceName();
    
    // Initialize the power setting.
    TBTPowerStateValue power( EBTPowerOff );
    (void) mBtengSetting->GetPowerState( power );
    setPowerSetting( power );
    
    // Initialize the visibility mode
    TBTVisibilityMode visibilityMode( EBTVisibilityModeNoScans );
    (void) mBtengSetting->GetVisibilityMode( visibilityMode );
    setVisibilityMode( visibilityMode );
}

/*!
    Destructor.
 */
BtSettingModelPrivate::~BtSettingModelPrivate()
{
    // delete main data structure
    delete mBtengSetting;
    delete mLocalDeviceWatcher;
    mLocalDeviceKey.Close();
    
    // delete mBtLinkCountWatcher;
    //mBtLinkCountKey.Close();
}


/*!
    Tells whether the given column is in the range of the setting list.
    
    \param row the row number to be checked
    \param col the column number to be checked
    
    \return true if the given row and column are valid; false otherwise.
*/
bool BtSettingModelPrivate::isValid( int row, int column) const
{
    return row >= 0 && row < mData.count() && column == 0;
}

/*!
    \return the total amount of rows.
    
*/
int BtSettingModelPrivate::rowCount() const
{
    return mData.count();
}

/*!
    \return the total amount of columns.
    
*/
int BtSettingModelPrivate::columnCount() const
{
    return 1;
}

/*!
    Gets the value within a data item.
    \param val contains the value at return.
    \param row the row number which the value is from
    \param col the column number which the value is from
    \param role the role idenfier of the value.
 */
void BtSettingModelPrivate::data(QVariant& val, int row,  int col, int role ) const
{
    if ( isValid( row, col ) ) {
        val = mData.at( row ).value( role );
    }
    else {
        val = QVariant( QVariant::Invalid );
    }
}

/*!
    Gets the whole item data at the specified column
    \param row the row number of the item data to be returned
    \param col the column number of the item data to be returned
    \return the item data
 */
BtuiModelDataItem BtSettingModelPrivate::itemData( int row, int col ) const
{
    if ( isValid( row, col ) ) {
        return mData.at( row );
    }
    return BtuiModelDataItem();
}

/*!
    Provides notification of changes in the power state
    of the Bluetooth hardware.

    \param state EBTPowerOff if the BT hardware has been turned off,
                 EBTPowerOn if it has been turned on.
 */
void BtSettingModelPrivate::PowerStateChanged( TBTPowerStateValue state ) 
{
    setPowerSetting( state );
    emit settingDataChanged( BtSettingModel::PowerStateRow, this );
}

/*!
    Provides notification of changes in the discoverability
    mode of the Bluetooth hardware.
    \param state EBTDiscModeHidden if the BT hardware is in hidden mode,
                  EBTDiscModeGeneral if it is in visible mode.
 */
void BtSettingModelPrivate::VisibilityModeChanged( TBTVisibilityMode state )
{
    setVisibilityMode( state );
    emit settingDataChanged( BtSettingModel::VisibilityRow, this );
}

void BtSettingModelPrivate::RequestCompletedL( CBtSimpleActive* active, TInt status ) {
    Q_UNUSED( active );
    Q_UNUSED( status );
    if ( active->RequestId() == KLocalDeviceNameWatcher ) {
        mLocalDeviceKey.Subscribe( mLocalDeviceWatcher->RequestStatus() );
        mLocalDeviceWatcher->GoActive();
        // Refresh local name of this model only when the local device table is changed.
        TInt changedTable;
        TInt err = mLocalDeviceKey.Get( changedTable );
        if( !err && changedTable == KRegistryChangeLocalTable ) {
            bool updated = updateLocalDeviceName();
            if (updated) {
                emit settingDataChanged( BtSettingModel::LocalBtNameRow, this );
            }
        }
    }
}

void BtSettingModelPrivate::CancelRequest( TInt requestId ) {
    if ( requestId == KLocalDeviceNameWatcher ) {
        mLocalDeviceKey.Cancel();
    }
    else if ( requestId == KBtLinkCountWatcher ) {
        //mBtLinkCountKey.Cancel();
    }
}

/*!
    AO's RunL() cannot leave in Qt applications. No handling.
 */
void BtSettingModelPrivate::HandleError( CBtSimpleActive* active, TInt error ) {
    Q_UNUSED( active );
    Q_UNUSED( error );
}

/*!
    Update local Bluetooth device name in the data store.
    \return true if the local name is really updated in this model; false, otherwise.
 */
bool BtSettingModelPrivate::updateLocalDeviceName() 
{
    QString nameInReg;
    getNameFromRegistry( nameInReg );
    QString currentName = mData.at(BtSettingModel::LocalBtNameRow).value(
                    BtSettingModel::settingDisplayRole).toString();
    if ( nameInReg != currentName ) {
        mData[BtSettingModel::LocalBtNameRow][BtSettingModel::settingDisplayRole] = QVariant(nameInReg);
        mData[BtSettingModel::LocalBtNameRow][BtSettingModel::SettingValueRole] = QVariant(nameInReg);
        return true;
    }
    return false;
}

/*!
    Updates all values related to the power setting.
 */
void BtSettingModelPrivate::setPowerSetting( TBTPowerStateValue state )
{
    mData[BtSettingModel::PowerStateRow][BtSettingModel::SettingValueRole] = QVariant(QtPowerMode(state));
}

void BtSettingModelPrivate::setVisibilityMode( TBTVisibilityMode state )
{
    mData[BtSettingModel::VisibilityRow][BtSettingModel::SettingValueRole] = QVariant(QtVisibilityMode(state));
}

/*!
    Get local Bluetooth device name from BTRegistry.
 */
void BtSettingModelPrivate::getNameFromRegistry( QString &name )
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
