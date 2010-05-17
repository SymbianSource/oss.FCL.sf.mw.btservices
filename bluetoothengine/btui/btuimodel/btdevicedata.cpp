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
#include <QDateTime>
#include <btservices/advancedevdiscoverer.h>
#include "btuiutil.h"
#include "btqtconstants.h"

class DevTypeIconMapping {
public:    
    int majorDevClass; // major device class value from CoD
    int minorDevClass; // minor device class value from CoD
    int majorProperty;   // one of major properties defined in BtDeviceModel
    int minorProperty;   // one of minor properties defined in BtDeviceModel
    char* connectedIcon; // the icon name for connected device
    char* unconnectedIcon; // the icon name for not connected device.
};

// mapping table from major and minor Device Classes to device types and icons
// which are specifically defined in btapplication namespace.
// (Note audio device mapping is not in this table due to its complex logic)
static const DevTypeIconMapping DeviceTypeIconMappingTable[] =
{
{EMajorDeviceComputer, 0, BtDeviceModel::Computer, 0, 
        ":/icons/qgn_prop_bt_computer_connect.svg", ":/icons/qgn_prop_bt_computer.svg" },
{EMajorDevicePhone,    0, BtDeviceModel::Phone,    0, 
        ":/icons/qgn_prop_bt_phone_connect.svg", ":/icons/qgn_prop_bt_phone.svg"},
{EMajorDeviceLanAccessPoint, 0, BtDeviceModel::LANAccessDev, 0, 
        ":/icons/qgn_prop_bt_misc.svg", ":/icons/qgn_prop_bt_misc.svg" },
{EMajorDevicePeripheral, EMinorDevicePeripheralKeyboard, 
        BtDeviceModel::Peripheral, BtDeviceModel::Keyboard,
        ":/icons/qgn_prop_bt_keyboard_connect.svg", ":/icons/qgn_prop_bt_keyboard.svg"},
{EMajorDevicePeripheral, EMinorDevicePeripheralPointer, 
        BtDeviceModel::Peripheral, BtDeviceModel::Mouse,
        ":/icons/qgn_prop_bt_mouse_connect.svg", ":/icons/qgn_prop_bt_mouse.svg"},
{EMajorDeviceImaging, 0, BtDeviceModel::ImagingDev, 0, 
        ":/icons/qgn_prop_bt_printer_connect.svg", ":/icons/qgn_prop_bt_printer.svg"},
{EMajorDeviceWearable, 0, BtDeviceModel::WearableDev, 0, 
        ":/icons/qgn_prop_bt_misc.svg", ":/icons/qgn_prop_bt_misc.svg"},
{EMajorDeviceToy, 0, BtDeviceModel::Toy, 0,
        ":/icons/qgn_prop_bt_misc.svg", ":/icons/qgn_prop_bt_misc.svg"},
};

static const int DeviceTypeIconMappingTableSize = 
        sizeof( DeviceTypeIconMappingTable ) / sizeof( DevTypeIconMapping );

/*!
    Constructor.
 */
BtDeviceData::BtDeviceData( BtDeviceModel& model, QObject *parent )
    : QObject( parent ), mModel( model ), mDiscover( 0 )
{
    mDeviceRepo = 0;
    isSearchingDevice = false;
    TRAP_IGNORE({
        mDeviceRepo = CBtDevRepository::NewL();
    });
    Q_CHECK_PTR( mDeviceRepo );
    TRAP_IGNORE( mDeviceRepo->AddObserverL( this ) );
    
    if ( mDeviceRepo->IsInitialized() ) {
        initializeDataStore();
    }
}

/*!
    Destructor.
 */
BtDeviceData::~BtDeviceData()
{
    delete mDeviceRepo;
    delete mDiscover;
}


/*!
    Tells whether the given column is in the range of the setting list.
    
    \param row the row number to be checked
    \param col the column number to be checked
    
    \return true if the given row and column are valid; false otherwise.
*/
bool BtDeviceData::isValid( int row, int column) const
{
    return row >= 0 && row < mData.count() && column == 0;
}

/*!
    \return the total amount of rows.
    
*/
int BtDeviceData::rowCount() const
{
    return mData.count();
}

/*!
    \return the total amount of columns.
    
*/
int BtDeviceData::columnCount() const
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
void BtDeviceData::data(QVariant& val, int row,  int col, int role ) const
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
BtuiModelDataItem BtDeviceData::itemData( int row, int col ) const
{
    if ( isValid( row, col ) ) {
        return mData.at( row );
    }
    return BtuiModelDataItem();
}


/*!
    Requests the model to searching Bluetooth devices.
    \return true if the request is accepted; false otherwise
 */
bool BtDeviceData::searchDevice()
{
    int err ( 0 );
    removeTransientDevices();
    if ( !mDiscover ) {
        TRAP(err, mDiscover = CAdvanceDevDiscoverer::NewL( *mDeviceRepo, *this) );
    }
    if ( !err ) {
        TRAP(err, mDiscover->DiscoverDeviceL() );
    }
    isSearchingDevice = true;
    return err == 0;
}

/*!
    Cancels a possible outstanding device search request.
 */
void BtDeviceData::cancelSearchDevice()
{
    if ( mDiscover ) {
        isSearchingDevice = false;
        mDiscover->CancelDiscovery();
    }
}

/*!
    Removes transient (not-in-registry) devices 
    (added as the result of device search).
 */
void BtDeviceData::removeTransientDevices()
{
    // clear in-range property for all device items in this model.
    int cnt = mData.count();
    for ( int i = mData.count() - 1; i > -1; --i)
        {
        const BtuiModelDataItem& qtdev = mData.at(i);
        if(isDeviceInRange(qtdev)) {
            if(isDeviceInRegistry(qtdev)) {
                // we cannot remove this device as it is in registry.
                // remove it in-range property.
                setMajorProperty(mData[i], BtDeviceModel::InRange, false);
                updateRssi(mData[i], RssiInvalid);
                mModel.emitDataChanged( i, 0, this );
            }
            else {
                // this device is not in-registry. Delete it from local
                // store.
                mModel.beginRemoveRows(QModelIndex(), i, i);
                mData.removeAt( i );
                mModel.endRemoveRows();
            }
        }
    }
}

/*!
    callback from repository.
    re-initialize our store.
 */
void BtDeviceData::RepositoryInitialized() 
{
    initializeDataStore();
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceData::DeletedFromRegistry( const TBTDevAddr& addr ) 
{
    int i = indexOf( addr );
    if ( i > -1 ) {
        if ( isSearchingDevice && isDeviceInRange( mData.at(i) ) ) {
            // device searching is ongoing, and it is in-range. we can not 
            // remore it from model now.
            // clear-registry related properties, so that
            // we get a chance to clean it after device searching later.
            setMajorProperty(mData[i], BtDeviceModel::RegistryProperties, false);
            mModel.emitDataChanged( i, 0, this );
        }
        else {
            mModel.beginRemoveRows(QModelIndex(), i, i);
            mData.removeAt( i );
            mModel.endRemoveRows();
        }
    }
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceData::AddedToRegistry( const CBtDevExtension& dev ) 
{
    ChangedInRegistry( dev, 0 );
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceData::ChangedInRegistry( 
        const CBtDevExtension& dev, TUint similarity )
{
    int i = indexOf( dev.Addr() );
    if ( i == -1 ) {
        BtuiModelDataItem devData;
        if ( !isSearchingDevice ) {
            // Rssi is only available at device inquiry stage. 
            // We initialize this property to an invalid value
            updateRssi(devData, RssiInvalid);
        }
        // add device-in-registry property:
        setMajorProperty(devData, BtDeviceModel::InRegistry, true);
        updateDeviceProperty(devData, dev, 0 );
        mModel.beginInsertRows( QModelIndex(), mData.count(), mData.count() );
        mData.append( devData );
        mModel.endInsertRows();
    }
    else {
        updateDeviceProperty(mData[i], dev, similarity );
        setMajorProperty(mData[i], BtDeviceModel::InRegistry, true);
        mModel.emitDataChanged( i, 0, this );
    }
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceData::ServiceConnectionChanged(
        const CBtDevExtension& dev, TBool connected )
{
    int i = indexOf( dev.Addr() );
    if ( i > -1 ) {
        int preconn =  BtDeviceModel::Connected 
                & mData[i][BtDeviceModel::MajorPropertyRole].toInt();
        // we only update and signal if connection status is really
        // changed:
        if ( ( preconn != 0 && !connected )
            || ( preconn == 0 && connected ) ) {
            setMajorProperty(mData[i], BtDeviceModel::Connected, connected );
            mModel.emitDataChanged( i, 0, this );
        }
    }
    // it is impossible that a device has connected but it is not in
    // our local store according to current bteng services.
    // need to take more care in future when this becomes possible.
}

/*!
    callback from device search.
    update our store.
 */
void BtDeviceData::HandleNextDiscoveryResultL( 
        const TInquirySockAddr& inqAddr, const TDesC& name )
{
    int pos = indexOf( inqAddr.BTAddr() );
    const CBtDevExtension* dev = mDeviceRepo->Device( inqAddr.BTAddr() );
    
    //RssiRole
    int rssi( RssiInvalid ); // initialize to an invalid value.
    if( inqAddr.ResultFlags() & TInquirySockAddr::ERssiValid ) {
        rssi = inqAddr.Rssi();
    }
    
    if ( pos == -1 ) {
        BtuiModelDataItem devData;
        setMajorProperty(devData, BtDeviceModel::InRange, true);
        updateRssi(devData, rssi);
        CBtDevExtension* devExt(NULL);
        TRAP_IGNORE( {
            devExt = CBtDevExtension::NewLC( inqAddr, name );
            CleanupStack::Pop(); });
        updateDeviceProperty(devData, *devExt, 0);
        delete devExt;
        mModel.beginInsertRows( QModelIndex(), mData.count(), mData.count() );
        mData.append( devData );
        mModel.endInsertRows();
    }
    else {
        setMajorProperty(mData[pos], BtDeviceModel::InRange, true);
        updateRssi(mData[pos], rssi);
        mModel.emitDataChanged( pos, 0, this );
    }
}

/*!
    callback from device search.
    inform client.
 */
void BtDeviceData::HandleDiscoveryCompleted( TInt error )
{
    isSearchingDevice = false;
    mModel.emitdeviceSearchCompleted( (int) error );
}

void BtDeviceData::initializeDataStore()
    {
    // it is possible that we are searching devices.
    // We use a simple but not-so-efficient method to update the model.
    
    // If the device store is not empty, we clear
    // registry property from these devices first.
    for (int i = 0; i < mData.count(); ++i) {
        setMajorProperty(mData[i], BtDeviceModel::RegistryProperties, false);
    }
    if ( mData.count() ) {
        // need to update view because we have changed device properties.
        QModelIndex top = mModel.createIndex(0, 0, this);
        QModelIndex bottom = mModel.createIndex(mData.count() - 1, 0, this);
        mModel.emitDataChanged( top, bottom );
    }

    const RDevExtensionArray& devs = mDeviceRepo->AllDevices();
    for (int i = 0; i < devs.Count(); ++i) {
        int pos = indexOf( devs[i]->Addr() );
        if ( pos > -1 ) {
            // add device-in-registry property:
            setMajorProperty(mData[pos], BtDeviceModel::InRegistry, true);            
            updateDeviceProperty(mData[pos], *(devs[i]), 0);
            mModel.emitDataChanged( pos, 0, this );
        }
        else {
            BtuiModelDataItem devData;
            // add device-in-registry property:
            setMajorProperty(devData, BtDeviceModel::InRegistry, true);
            updateDeviceProperty(devData, *( devs[i] ), 0 );
            mModel.beginInsertRows(QModelIndex(), mData.count(), mData.count() );
            mData.append( devData );
            mModel.endInsertRows();
        }
    }
}

void BtDeviceData::updateDeviceProperty(BtuiModelDataItem& qtdev,
        const CBtDevExtension& dev, TUint similarity )
{
    // similarity is not used currently. 
    // It is possible to gain better performance
    // with this info to avoid re-manipulate
    // unchanged properties.
    Q_UNUSED(similarity);
    
    //DevDisplayNameRole
    QString str = QString::fromUtf16( 
            dev.Alias().Ptr(), dev.Alias().Length() );
    qtdev[BtDeviceModel::NameAliasRole] = QVariant( str );

    //DevAddrReadableRole
    addrSymbianToReadbleString( str, dev.Addr() );
    qtdev[BtDeviceModel::ReadableBdaddrRole] = QVariant( str );

    //LastUsedTimeRole
    TDateTime symDt = dev.Device().Used().DateTime();
    QDate date( symDt.Year(), symDt.Month(), symDt.Day() );
    QTime time( symDt.Hour(), symDt.Minute(), symDt.MicroSecond() / 1000 );
    QDateTime qdt(date, time);
    qtdev[BtDeviceModel::LastUsedTimeRole] = QVariant(qdt);
 
    // set paired status:
    setMajorProperty(qtdev, BtDeviceModel::Bonded, isBonded( dev.Device() ));
    
    // set blocked status:
    setMajorProperty(qtdev, BtDeviceModel::Blocked, 
            dev.Device().GlobalSecurity().Banned() );
    // set trusted status:
    setMajorProperty(qtdev, BtDeviceModel::Trusted, 
            dev.Device().GlobalSecurity().NoAuthorise() );

    //CoDRole
    //MinorPropertyRole
    int cod = static_cast<int>( dev.Device().DeviceClass().DeviceClass() );
    qtdev[BtDeviceModel::CoDRole] = QVariant(cod);
    
    // Initially, clear CoD related properties:
    int majorProperty = qtdev[BtDeviceModel::MajorPropertyRole].toInt() & 
        ~BtDeviceModel::CodProperties;
    
    int minorProperty = BtDeviceModel::NullProperty;
    
    // device type must be mapped according to CoD:
    int majorDevCls = dev.Device().DeviceClass().MajorDeviceClass();
    int minorDevCls = dev.Device().DeviceClass().MinorDeviceClass();

    int i;
    for (i = 0; i < DeviceTypeIconMappingTableSize; ++i ) {
        if ( DeviceTypeIconMappingTable[i].majorDevClass == majorDevCls &&
             ( DeviceTypeIconMappingTable[i].minorDevClass == 0 || 
               DeviceTypeIconMappingTable[i].minorDevClass == minorDevCls ) ) {
             // device classes match a item in table, get the mapping:
            majorProperty |= DeviceTypeIconMappingTable[i].majorProperty;
            minorProperty |= DeviceTypeIconMappingTable[i].minorProperty;
            break;
         }
    }
    
    // AV device mapping are not defined in the table, do mapping here if no device
    // type mapped so far:
    if ( i == DeviceTypeIconMappingTableSize) {
        // audio device, carkit, headset or speaker:
        if( ( majorDevCls == EMajorDeviceAV) 
            || (dev.Device().DeviceClass().MajorServiceClass() == EMajorServiceRendering 
            && majorDevCls != EMajorDeviceImaging) ) {
            
            majorProperty |= BtDeviceModel::AVDev;
            
            if( minorDevCls == EMinorDeviceAVCarAudio ) {
                // carkit:
                minorProperty |= BtDeviceModel::Carkit; 
            }
            else {
                // headset:
                minorProperty |= BtDeviceModel::Headset;
            }
        }
    }
    
    qtdev[BtDeviceModel::MajorPropertyRole] = QVariant( majorProperty );
    qtdev[BtDeviceModel::MinorPropertyRole] = QVariant( minorProperty );
}

int BtDeviceData::indexOf( const TBTDevAddr& addr ) const
{
    QString addrStr;
    addrSymbianToReadbleString( addrStr, addr );
    for (int i = 0; i < mData.count(); ++i ) {
        if ( mData.at( i ).value( BtDeviceModel::ReadableBdaddrRole ) 
                == addrStr ) {
            return i;
        }
    }
    return -1;
}

void BtDeviceData::updateRssi(BtuiModelDataItem& qtdev, int rssi )
    {
    qtdev[BtDeviceModel::RssiRole] = QVariant( rssi );
    }

/*!
    Add the specified major property to the device if addto is true.
    Otherwise the property is removed from the device. 
 */
void BtDeviceData::setMajorProperty(
        BtuiModelDataItem& qtdev, int prop, bool addto)
{
    if ( addto ) {
        qtdev[BtDeviceModel::MajorPropertyRole] = 
            QVariant( qtdev[BtDeviceModel::MajorPropertyRole].toInt() | prop);
    }
    else {
        qtdev[BtDeviceModel::MajorPropertyRole] = 
            QVariant( qtdev[BtDeviceModel::MajorPropertyRole].toInt() & ~prop);
    }
}

bool BtDeviceData::isDeviceInRange( const BtuiModelDataItem& qtdev )
{   
    return BtDeviceModel::InRange & qtdev[BtDeviceModel::MajorPropertyRole].toInt();
}

bool BtDeviceData::isDeviceInRegistry( const BtuiModelDataItem& qtdev )
{
    return BtDeviceModel::InRegistry & qtdev[BtDeviceModel::MajorPropertyRole].toInt();
}
