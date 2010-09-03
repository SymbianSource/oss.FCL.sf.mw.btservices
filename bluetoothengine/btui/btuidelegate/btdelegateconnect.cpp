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

#include "btdelegateconnect.h"
#include "btuiutil.h"
#include "btqtconstants.h"
#include <QModelIndex>
#include <hblabel.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbnotificationdialog.h>
#include <hbmessagebox.h>
#include "btuiiconutil.h"
#include "btdelegatefactory.h"
#include <bluetoothuitrace.h>
#include <e32property.h>
#include <ctsydomainpskeys.h>  

BtDelegateConnect::BtDelegateConnect(
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtengConnMan(0),
    mAbstractDelegate(0), mActiveHandling(false)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

BtDelegateConnect::~BtDelegateConnect()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    delete mBtengConnMan;
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegateConnect::supportedEditorTypes() const
{
    return BtDelegate::ConnectService;
}

/*!
 * execute connect operation
 *    first check if power is on
 */
void BtDelegateConnect::exec( const QVariant &params )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    if ( mActiveHandling ) {
        emit delegateCompleted( KErrAlreadyExists, this );
        BOstraceFunctionExit1( DUMMY_DEVLIST, this );
        return;
    }
    mIndex = params.value<QModelIndex>();
    mActiveHandling = true;
    // save needed values from model
    mDeviceName = (mIndex.data(BtDeviceModel::NameAliasRole)).toString();
    BtTraceQString1( TRACE_DEBUG, DUMMY_DEVLIST, "device name=", mDeviceName);
    QString addrStr = (mIndex.data(BtDeviceModel::ReadableBdaddrRole)).toString(); 
    addrReadbleStringToSymbian( addrStr, mAddr );  
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", mAddr );
    
    mCod = (mIndex.data(BtDeviceModel::CoDRole)).toInt();
    mMajorProperty = (mIndex.data(BtDeviceModel::MajorPropertyRole)).toInt();
    
    // first turn on power if needed
    if (!isBtPowerOn()) {
        if (!mAbstractDelegate) //if there is no other delegate running
        { 
            mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::ManagePower, 
                    settingModel(), deviceModel() ); 
            connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, SLOT(powerDelegateCompleted(int)) );
            mAbstractDelegate->exec(QVariant(BtPowerOn));
        }
    } 
    else {
        // power is already on
        exec_connect();
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * power delegate has completed, continue processing
 */
void BtDelegateConnect::powerDelegateCompleted(int status)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, status );  
    if (mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    if ( status == KErrNone ) {
        // continue connecting
        exec_connect();
    } 
    else {
        // error
        emitCommandComplete(status);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * execute connect operation
 */
void BtDelegateConnect::exec_connect()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    int error = KErrNone;
    
    if ( ! mBtengConnMan ){
        TRAP( error, mBtengConnMan = CBTEngConnMan::NewL(this) );
    }

    if ( !error ) {
        TBTDeviceClass btEngDeviceClass(mCod);
        error = mBtengConnMan->Connect(mAddr, btEngDeviceClass);
    }
    
    if( error ) {
        emitCommandComplete(error);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * connect callback from CBTengConnMan
 */
void BtDelegateConnect::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aErr );  
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", aAddr );
    // It is possible that another audio device has just connected to phone when we are
    // connecting to this audio device. Or a device is connected while this command
    // is idle. No handling for these cases.
    if ( mAddr != aAddr || !mActiveHandling ) {  
        BOstraceFunctionExit1( DUMMY_DEVLIST, this );
        return;
    }
    
    QModelIndex conflictDevIndex;
    // conflict could occur as well if another audio device is already connected
    // since currently we don't support multiple audio device connections.
    if ( aErr && aConflicts && aConflicts->Count() ) {
        // get the display name of the device that is 
        // causing the conflict 
        
        addrSymbianToReadbleString(mConflictDevAddr, (*aConflicts)[0] );
        QModelIndex start = deviceModel()->index(0,0);
        QModelIndexList indexList = deviceModel()->match(start, BtDeviceModel::ReadableBdaddrRole, 
                QVariant(mConflictDevAddr));
        // ToDo:  change assert to normal error handling;  
        // is it possible that device is not in model or registry?
        BTUI_ASSERT_X(indexList.count(), "BtDelegateConnect::ConnectComplete()", "device missing from model!");
        conflictDevIndex = indexList.at(0);
      
        // check if conflict device is being used in a call
        // Note:  actually only checking if *any* audio device is involved in a call, not necessarily the
        // one we are concerned with here.  Btaudioman does not currently support finding out the actual 
        // device involved in a call. 
        if (callOngoing()) {
            HbMessageBox::warning(hbTrId("txt_bt_info_not_possible_during_a_call"));
            emitCommandComplete(KErrCancel);
        }
        else {
            // no call, check if user wants to disconnect conflict device 
            QString conflictDevName = (conflictDevIndex.data(BtDeviceModel::NameAliasRole)).toString();    
    
            QString questionText(hbTrId("txt_bt_info_to_connect_1_2_needs_to_be_disconnec")
                    .arg(mDeviceName).arg(conflictDevName));
            
            HbMessageBox::question( questionText, this, SLOT(handleUserAnswer(int)), 
                    HbMessageBox::Continue | HbMessageBox::Cancel );
        }
    }
    else {
        // command is finished
        emitCommandComplete(aErr);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * handle user response to query about disconnecting conflict device
 */
void BtDelegateConnect::handleUserAnswer( int answer )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    if( answer == HbMessageBox::Continue ) { 
        // Continue, ie. disconnect conflict device and then try reconnecting again
        if (!mAbstractDelegate) //if there is no other delegate running
        { 
            QList<QVariant>list;
            QVariant paramSecond;
            paramSecond.setValue(mConflictDevAddr);    
            list.append(QVariant(ServiceLevel));
            list.append(paramSecond);
            mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::DisconnectService, 
                    settingModel(), deviceModel() ); 
            connect( mAbstractDelegate, SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), this, SLOT(disconnectDelegateCompleted(int)) );
            mAbstractDelegate->exec(QVariant(list));
        }
    }
    else {
        // Cancel connect operation
        emitCommandComplete(KErrCancel);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * returns true if phone call is ongoing
 */
bool BtDelegateConnect::callOngoing()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    // ToDo:  check if there exists Qt PS key for ongoing call
    int callState;
    bool retVal;
    int err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
    if (!err && (callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized)) {
        retVal = false;
    }
    else {
        retVal =  true;
    }
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, retVal );
    return retVal;
}

/*!
 * disconnecting conflict device has completed, continue connecting
 */
void BtDelegateConnect::disconnectDelegateCompleted(int status)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    if (mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    // finished disconnecting conflict device, now reconnect to original device
    if ( status == KErrNone ) {
        exec_connect();
    }
    else {
        // disconnect failed, abort
        emitCommandComplete( status );
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * not used here
 */
void BtDelegateConnect::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aErr );  
    BtTraceBtAddr1( TRACE_DEBUG, DUMMY_DEVLIST, "device addr=", aAddr );
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * cancel connect operation
 *   ConnectComplete() callback will be called upon completion of cancel with KErrCancel
 */
void BtDelegateConnect::cancel()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );  
    if ( mBtengConnMan ) {
        mBtengConnMan->CancelConnect(mAddr);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
 * shows user notes with connection success/failure information
 *    cancel operation is handled without a user note
 */
void BtDelegateConnect::emitCommandComplete(int error)
{
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, error );  
    if ( error == KErrCancel ) {
        // no user note, return success since cancel operation completed successfully
        error = KErrNone;
    }
    mActiveHandling = false;
    
    emit delegateCompleted(error, this);
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, error );
}


