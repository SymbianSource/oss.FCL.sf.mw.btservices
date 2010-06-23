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
    
}

BtDelegateConnect::~BtDelegateConnect()
{
    delete mBtengConnMan;
}

void BtDelegateConnect::exec( const QVariant &params )
{
    if ( mActiveHandling ) {
        emit commandCompleted( KErrAlreadyExists );
        return;
    }
    mIndex = params.value<QModelIndex>();
    mActiveHandling = true;
    // save needed values from model
    mDeviceName = (mIndex.data(BtDeviceModel::NameAliasRole)).toString();
    QString addrStr = (mIndex.data(BtDeviceModel::ReadableBdaddrRole)).toString(); 
    addrReadbleStringToSymbian( addrStr, mAddr );  
    
    mCod = (mIndex.data(BtDeviceModel::CoDRole)).toInt();
    mMajorProperty = (mIndex.data(BtDeviceModel::MajorPropertyRole)).toInt();
    
    // first turn on power if needed
    QModelIndex powerIndex = getSettingModel()->index(BtSettingModel::PowerStateRow, 0);
    PowerStateQtValue powerState = (PowerStateQtValue)getSettingModel()->data(powerIndex, Qt::EditRole).toInt();
    if (powerState == BtPowerOff) {
        if (!mAbstractDelegate) //if there is no other delegate running
        { 
            mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::ManagePower, 
                    getSettingModel(), getDeviceModel() ); 
            connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(powerDelegateCompleted(int)) );
            mAbstractDelegate->exec(QVariant(BtPowerOn));
        }
    } 
    else {
        // power is already on
        exec_connect();
    }
}

void BtDelegateConnect::powerDelegateCompleted(int status)
{
    //ToDo: Error handling here 
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
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
}

void BtDelegateConnect::exec_connect()
{
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
}

void BtDelegateConnect::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    // It is possible that another audio device has just connected to phone when we are
    // connecting to this audio device. Or a device is connected while this command
    // is idle. No handling for these cases.
    if ( mAddr != aAddr || !mActiveHandling ) {  
        return;
    }
    
    // bteng calls ConnectComplete even if cancelConnect is called,
    // we won't signal the interested party in this case.
    // ToDo: is this needed?
    if ( aErr == KErrCancel ) {
        mActiveHandling = false;
        return;
    }
    
    // conflict could occur as well if another audio device is already connected
    // since currently we don't support multiple audio device connections.
    if ( aErr && aConflicts && aConflicts->Count() ) {
        // get the display name of the device that is 
        // causing the conflict 
        QString conflictDevAddr;
        addrSymbianToReadbleString(conflictDevAddr, (*aConflicts)[0] );
        QModelIndex start = getDeviceModel()->index(0,0);
        QModelIndexList indexList = getDeviceModel()->match(start, BtDeviceModel::ReadableBdaddrRole, conflictDevAddr);
        BTUI_ASSERT_X(indexList.count(), "BtDelegateConnect::ConnectComplete()", "device missing from model!");
        mConflictDevIndex = indexList.at(0);
      
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
            QString conflictDevName = (mConflictDevIndex.data(BtDeviceModel::NameAliasRole)).toString();    
    
            QString questionText(hbTrId("txt_bt_info_to_connect_1_2_needs_to_be_disconnec")
                    .arg(mDeviceName).arg(conflictDevName));
            
            HbMessageBox::question( questionText, this, SLOT(handleUserAnswer(HbAction*)), 
                    hbTrId("txt_common_button_continue"), hbTrId("txt_common_button_cancel") );        
        }
    }
    else {
        // command is finished
        emitCommandComplete(aErr);
    }
}

void BtDelegateConnect::handleUserAnswer( HbAction* answer )
{
    HbMessageBox* dlg = static_cast<HbMessageBox*>( sender() );
    if( dlg->actions().first() == answer ) { 
        // Continue, ie. disconnect conflict device and then try reconnecting again
        if (!mAbstractDelegate) //if there is no other delegate running
        { 
            QList<QVariant>list;
            QVariant paramFirst;
            paramFirst.setValue(mConflictDevIndex);    
            QVariant paramSecond(ServiceLevel);
            list.append(paramFirst);
            list.append(paramSecond);
            QVariant paramsList(list);
            mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::Disconnect, 
                    getSettingModel(), getDeviceModel() ); 
            connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(disconnectDelegateCompleted(int)) );
            mAbstractDelegate->exec(paramsList);
        }
    }
    else {
        // Cancel
        emitCommandComplete(KErrCancel);
    }
}
/*!
 * returns true if call is ongoing
 */
bool BtDelegateConnect::callOngoing()
{
    int callState;
    int err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
    if (!err && (callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized)) {
        return false;
    }
    else {
        return true;
    }
}
void BtDelegateConnect::disconnectDelegateCompleted(int status)
{
    // finished disconnecting conflict device, now reconnect to original device
    if ( status == KErrNone ) {
        exec_connect();
    }
    else {
        // disconnect failed, abort
        emitCommandComplete( status );
    }
}

void BtDelegateConnect::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
}


void BtDelegateConnect::cancel()
{
    if ( mBtengConnMan ) {
        mBtengConnMan->CancelConnect(mAddr);
    }
}

void BtDelegateConnect::emitCommandComplete(int error)
{
    if ( error == KErrNone ) {
        // success, show indicator with connection status
        
        HbIcon icon = getBadgedDeviceTypeIcon( mCod, mMajorProperty, 0);  // no badging required, only icon
        QString str(hbTrId("txt_bt_dpopinfo_connected_to_1"));
        HbNotificationDialog::launchDialog( icon, hbTrId("txt_bt_dpophead_connected"), 
            str.arg(mDeviceName) );  
    }
    else if ( error == KErrCancel ) {
        // no user note, return success
        error = KErrNone;
    }
    else {
        // failure to connect, show user note
        QString err(hbTrId("txt_bt_info_unable_to_connect_with_bluetooth"));
        HbMessageBox::warning(err.arg(mDeviceName));
    }
    mActiveHandling = false;
    
    emit commandCompleted(error);
}


