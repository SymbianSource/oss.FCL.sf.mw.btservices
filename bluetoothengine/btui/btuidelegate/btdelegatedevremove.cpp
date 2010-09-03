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
#include "btdelegatedevremove.h"
#include "btqtconstants.h"
#include "btuiutil.h"
#include "btuidevtypemap.h"
#include <QModelIndex>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <btdelegatefactory.h>
#include "btqtconstants.h"
#include "btuiutil.h"
#include <bluetoothuitrace.h>

BtDelegateDevRemove::BtDelegateDevRemove(BtSettingModel* settingModel,
        BtDeviceModel* deviceModel, QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtEngDevMan(0),
            mOngoingBlockExec(false)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

BtDelegateDevRemove::~BtDelegateDevRemove()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    if (mBtEngDevMan) {
        delete mBtEngDevMan;
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}


/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegateDevRemove::supportedEditorTypes() const
{
    return BtDelegate::RemoveAllRegistryDevices
           | BtDelegate::RemovePairedDevices
           | BtDelegate::RemoveBlockedDevices;
}

/*!
 * Performs device deletion operation from registry based on
 * params passed.
 * params of type int holds the value of type DeviceRemovalType enum 
 */
void BtDelegateDevRemove::exec( const QVariant &params )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    
    int error = 0;
    
    if( !mBtEngDevMan) {
        TRAP( error, (mBtEngDevMan = CBTEngDevMan::NewL( this )));
        if (error) {
            emitCommandComplete( KErrNoMemory ); 
            BOstraceFunctionExitExt( DUMMY_DEVLIST, this, KErrNoMemory );
            return;
        }
    }
    BtDelegate::EditorType editor = (BtDelegate::EditorType) params.toInt();
    TBTRegistrySearch searchPattern; 
    switch(editor) {
        case BtDelegate::RemoveAllRegistryDevices:
            searchPattern.FindAll();
            error = mBtEngDevMan->DeleteDevices(searchPattern);
            break;
        case BtDelegate::RemovePairedDevices:
            searchPattern.FindBonded();
            error = mBtEngDevMan->DeleteDevices(searchPattern);
            break;
        case BtDelegate::RemoveBlockedDevices:
            mOngoingBlockExec = true;
            if(isBlockedDeviceExists()) {
                error = removeBlockedDevice();
            }
            else {
                //No Blocked Device exists return success.
                emitCommandComplete(0);
            }
            break;
        default:
            // error
            BTUI_ASSERT_X(false, "BtDelegateDevRemove::exec", "Invalid Parameter");
    }
    if (error) {
        // complete command with error
        emitCommandComplete(error);
    }


    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

int BtDelegateDevRemove::removeBlockedDevice()
{
    //todo:This is temporary solution, TBTRegistrySearch should support
    //fecthing blocked devices.
    int error = 0;

    QModelIndex index = mBlockedindexList.at(0);
    TBTDevAddr btDeviceAddr;
    QString addrStr = (index.data(BtDeviceModel::ReadableBdaddrRole)).toString(); 
    addrReadbleStringToSymbian( addrStr, btDeviceAddr );  
    TBTRegistrySearch searchPattern; 
    
    searchPattern.FindAddress(btDeviceAddr);
    error = mBtEngDevMan->DeleteDevices(searchPattern);
    
    return error;
}

bool BtDelegateDevRemove::isBlockedDeviceExists()
{
    QModelIndex start = deviceModel()->index(0,0);
    mBlockedindexList.clear();
    mBlockedindexList = deviceModel()->match(start,
            BtDeviceModel::MajorPropertyRole, BtuiDevProperty::Blocked,
            Qt::MatchContains);
    if(mBlockedindexList.count()) {
        return true;
    }
    return false;
}

void BtDelegateDevRemove::cancel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this ); 
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevRemove::HandleDevManComplete( TInt aErr )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aErr );
    
    if(mOngoingBlockExec) {
        //In this case aErr is not checked, as device removal is not
        //updated immediately in the model. This causes unnecessary 
        //function calls as it is repeated until model is updated with 
        //blocked devices.
        if(isBlockedDeviceExists()) {
            int error = 0;
            error = removeBlockedDevice();
            if(error) {
                emitCommandComplete(error);
                mOngoingBlockExec = false;
            }
            return;
        }
        else {
            mOngoingBlockExec = false;
        }
    }
    emitCommandComplete(aErr);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    
}

void BtDelegateDevRemove::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aErr );  
    Q_UNUSED(aErr);
    Q_UNUSED(aDeviceArray);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegateDevRemove::emitCommandComplete(int error)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, error );  
    
    emit delegateCompleted(error, this);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}





