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
#include "btdelegatedevsecurity.h"
#include "btqtconstants.h"
#include <QModelIndex>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <btdelegatefactory.h>
#include <hbnotificationdialog.h>
#include "btqtconstants.h"
#include "btuiutil.h"
#include <bluetoothuitrace.h>
#include <btengconnman.h>

BtDelegateDevSecurity::BtDelegateDevSecurity(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, 
        QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtEngDevMan(0),
    mDisconnectDelegate(0), mBtEngAddr(0), mRegDevArray(0), mOperation(0), 
    mDevice(0),mNewDev(0), mActiveHandling(false),mAddingDevToRegistry(false)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this ); 
    mRegDevArray = new CBTDeviceArray(1);
    BTUI_ASSERT_X( mRegDevArray, "BtDelegateDevSecurity::BtDelegateDevSecurity()", 
            "can't allocate regdevarray");
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

BtDelegateDevSecurity::~BtDelegateDevSecurity()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    delete mBtEngDevMan;
    delete mDisconnectDelegate;
    delete mRegDevArray;
    delete mNewDev;
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}


/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegateDevSecurity::supportedEditorTypes() const
{
    return BtDelegate::UnpairDevice 
           | BtDelegate::BlockDevice
           | BtDelegate::UnblockDevice
           | BtDelegate::TrustDevice
           | BtDelegate::UntrustDevice;
}

/*!
 * performs operations on remote device:  unpair, authorize/unauthorize, block/unblock
 * params of type QList<QVariant>: 
 *         1) remote device address (QString)
 *         2) DeviceSecurityService operation 
 */
void BtDelegateDevSecurity::exec( const QVariant &params )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    // check if in use already
    if ( mActiveHandling ) {
        emit delegateCompleted(KErrInUse, this);
        BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrInUse );
        return;
    }
    mActiveHandling = true;
    
    // check parameters
    QList<QVariant> paramList = params.value< QList<QVariant> >(); 
    if (paramList.count() != 2) {
        // wrong parameters
        emitCommandComplete( KErrArgument ); 
        BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrArgument );
        return;
    }
    
    int error = 0;
    TRAP( error, {
        if( !mBtEngDevMan) {
            mBtEngDevMan = CBTEngDevMan::NewL( this );
        }
    });
    if (error) {
        emitCommandComplete( KErrNoMemory ); 
        BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrNoMemory );
        return;
    }
    
    mStrBtAddr = paramList.at(0).value<QString>();  // device to operate on
    mOperation = paramList.at(1).toInt();                   // operation
    
    addrReadbleStringToSymbian( mStrBtAddr, mBtEngAddr );
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", mBtEngAddr );
    

    // get device from registry since it is needed for all operations
    mSearchPattern.FindAddress( mBtEngAddr );
    mRegDevArray->ResetAndDestroy();
    mBtEngDevMan->GetDevices(mSearchPattern, mRegDevArray); // callback is HandleGetDevicesComplete()
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}


void BtDelegateDevSecurity::unpair()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    int error = KErrNone;
    
    // unpair first since malicious device might try to connect/pair again 
    // immediately after disconnecting
    mDevice->SetPaired(EFalse);
    mDevice->DeleteLinkKey();

    // untrust the device also
    TBTDeviceSecurity security = mDevice->GlobalSecurity();
    security.SetNoAuthorise( EFalse );
    mDevice->SetGlobalSecurity( security );

    error = mBtEngDevMan->ModifyDevice( *mDevice );  // see callback for possible disconnect
    
    if ( error ) {
        emitCommandComplete(error);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevSecurity::authorizeOrBlock()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this ); 
    BOstraceExt1( TRACE_NORMAL, DUMMY_DEVLIST, "operation (Unpair|Block|Unblock|Authorize|Unauthorize)=%d", 
            mOperation);
    int error = KErrNone;
    
    TBTDeviceSecurity security = mDevice->GlobalSecurity();
    switch ( mOperation ) {
    case BtAuthorize:
        security.SetNoAuthorise( ETrue ); // set trust status to true
        security.SetBanned( EFalse );
        break;
    case BtUnauthorize:
        security.SetNoAuthorise( EFalse );
        break;
    case BtUnblock:
        security.SetBanned( EFalse );
        break;
    case BtBlock:
        security.SetBanned( ETrue );
        security.SetNoAuthorise( EFalse ); // set trust status to false
        break;
    }

    mDevice->SetGlobalSecurity( security );
    if ( (mOperation == BtBlock) || (mOperation == BtUnblock) ) {
        // deleting link key for executing unblock is safe as no 
        // link key shall exist if the device has been blocked. 
        mDevice->DeleteLinkKey();
        if ( mOperation == BtBlock ) {
            mDevice->SetPaired(EFalse);
        }
    }
    error = mBtEngDevMan->ModifyDevice( *mDevice );
    
    if ( error ) {
        emitCommandComplete(error);
    }
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, error );
}

void BtDelegateDevSecurity::cancel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this ); 
    if ( mActiveHandling ) {
        emitCommandComplete(KErrNone);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevSecurity::disconnectDelegateCompleted( int err )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );  
    if (mDisconnectDelegate) {
        delete mDisconnectDelegate;
        mDisconnectDelegate = 0;
    }
    emitCommandComplete(err);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevSecurity::HandleDevManComplete( TInt err )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );  

    if ( !mActiveHandling ) {
        BOstraceFunctionExit1( DUMMY_DEVLIST, this );
        return;
    }
    if ( !err ) {
        if ( mAddingDevToRegistry ) {
            // blocked a device which was not in the registry originally
            mAddingDevToRegistry = false;
            delete mNewDev;
            mNewDev = 0;
        }
        else if ( mOperation == BtBlock || mOperation == BtUnpair) {  
            // disconnect after blocking/unpairing if device is connected;
            // disconnect done after block/unpair, instead of before, to prevent a malicious device 
            // from reconnecting/"re-pairing"
            CBTEngConnMan *connMan(0);
            TRAP( err, connMan = CBTEngConnMan::NewL() );
            TBTEngConnectionStatus connstatus(EBTEngNotConnected);
            if (!err) {
                err = connMan->IsConnected(mBtEngAddr, connstatus );
                delete connMan;
            }
            if ( !err && connstatus == EBTEngConnected ) {
                if (! mDisconnectDelegate){
                    mDisconnectDelegate = BtDelegateFactory::newDelegate(
                                            BtDelegate::DisconnectService, settingModel(), deviceModel()); 
                    connect( mDisconnectDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, 
                            SLOT(disconnectDelegateCompleted(int)) );
                }
                QList<QVariant>list;
                list.append(QVariant(ServiceLevel));
                list.append(QVariant(mStrBtAddr));
                mDisconnectDelegate->exec(QVariant(list)); // see callback for continuation
                return;
            }
        }
    }
    emitCommandComplete(err);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevSecurity::HandleGetDevicesComplete( TInt err, CBTDeviceArray* aDeviceArray )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, err );  
    
    if ( mActiveHandling ) {
        if ( !err && aDeviceArray->Count() ) { 
            mDevice = aDeviceArray->At( 0 );
            switch ( mOperation ) {
            case BtUnpair:
                unpair();
                break;
            case BtAuthorize:
            case BtUnauthorize:
            case BtUnblock:
            case BtBlock:
                authorizeOrBlock();
                break;
            default:
                // wrong parameter
                emitCommandComplete( KErrArgument );
            }
        }
        else if ( err == KErrNotFound && ((mOperation == BtBlock) ||
                (mOperation == BtAuthorize))) {  // device not in registry, need to add it
            mAddingDevToRegistry = true;
            TRAP( err, {
                    mNewDev = CBTDevice::NewL( mBtEngAddr );
            });
            if ( !err ) {
                // get needed info about device from model, e.g. name, cod
                QString btStringAddr;
                addrSymbianToReadbleString(btStringAddr, mBtEngAddr);
                QModelIndex start = deviceModel()->index(0,0);
                QModelIndexList indexList = deviceModel()->match(start,
                        BtDeviceModel::ReadableBdaddrRole, btStringAddr);
                // ToDo:  can we be sure that device will always be found in the model?
                QModelIndex index = indexList.at(0);
                QString devName = deviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
                BtTraceQString1( TRACE_DEBUG, DUMMY_DEVLIST, "device name=", devName);
                TBuf<KMaxBCBluetoothNameLen> buf( devName.utf16() );
                TRAP( err, mNewDev->SetDeviceNameL( BTDeviceNameConverter::ToUTF8L( buf ) ));
                if( !err ) {
                    int cod = (index.data(BtDeviceModel::CoDRole)).toInt();
                    mNewDev->SetDeviceClass(cod);
                    TBTDeviceSecurity security = mNewDev->GlobalSecurity();
                    if (mOperation == BtBlock) {
                        security.SetBanned( ETrue );
                        security.SetNoAuthorise( EFalse ); // set trust status to false
                        mNewDev->DeleteLinkKey();
                        mNewDev->SetPaired(EFalse);
                    } else {  // BtAuthorize
                        security.SetNoAuthorise( ETrue ); // set trust status to true
                        security.SetBanned( EFalse );
                    }
                    mNewDev->SetGlobalSecurity( security );
                    err = mBtEngDevMan->AddDevice( *mNewDev );  // see callback HandleDevManComplete()
                }
            }
        }
        if (err) {
            emitCommandComplete( err );
        }
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevSecurity::emitCommandComplete(int error)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, error );  
    // no dialogs here since stack provides "unpaired to %1" dialog
    // and failures are not reported
    mActiveHandling = false;
    mAddingDevToRegistry = false;
    if ( mNewDev ) {
        delete mNewDev;
        mNewDev = 0;
    }
    emit delegateCompleted(error, this);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}




