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


#include "btdelegatepower.h"
#include "btqtconstants.h"
#include <btdelegatedisconnect.h>
#include <btdelegatefactory.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbmessagebox.h>
#include <bluetoothuitrace.h>
#include <hbaction.h>

/*!
    Constructor.
 */
BtDelegatePower::BtDelegatePower(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent )
    : BtAbstractDelegate( settingModel, deviceModel, parent ),
      mDisconnectDelegate(0)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );   
    TRAP_IGNORE( mBtengSettings = CBTEngSettings::NewL(this) );
    Q_CHECK_PTR( mBtengSettings );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
    Destructor.
 */
BtDelegatePower::~BtDelegatePower()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    delete mBtengSettings;
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
    Returns the supported editor types.
    \return the sum of supported editor types
 */
int BtDelegatePower::supportedEditorTypes() const
{
    return BtDelegate::ManagePower;
}

/*!
    Turns BT power on/off
    param powerState is the desired power state and is of type PowerStateQtValue
 */
void BtDelegatePower::exec( const QVariant &powerState )
{   
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    BTUI_ASSERT_X(!isExecuting(), "BtDelegatePower::exec", "operation ongoing!");

    mReqPowerState = BtEngPowerState((PowerStateQtValue)powerState.toInt());
    BTUI_ASSERT_X( (mReqPowerState == EBTPowerOff) || (mReqPowerState == EBTPowerOn), 
            "BtDelegatePower::exec()", "wrong power state value" );
    
    // get current power status
    TBTPowerStateValue curPowerState(EBTPowerOff);
    int err = mBtengSettings->GetPowerState( curPowerState );
    
    if (!err && mReqPowerState != curPowerState) {
        setExecuting(true);
        // perform power on/off operation
        if ( mReqPowerState == EBTPowerOff ){ 
            switchBTOff();     
        }
        else if ( mReqPowerState == EBTPowerOn ) {
            switchBTOn();
        }
    } else {
        // no need to do anything
        completeDelegateExecution(err);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::switchBTOn()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    int err = 0;

    //check if device is in OFFLINE mode first
    bool btEnabledInOffline = false;
    if (checkOfflineMode(btEnabledInOffline)){  // offline mode is active
        if (btEnabledInOffline){
            // BT is allowed to be enabled in offline mode, show query.
            HbMessageBox::question( hbTrId("txt_bt_info_trun_bluetooth_on_ini_offline_mode" ),this, 
				SLOT(btOnQuestionClose(int)), HbMessageBox::Yes | HbMessageBox::No );
        } else {
            //if BT is not allowed to be enabled in offline mode, show message and complete
            HbMessageBox::warning( hbTrId("txt_bt_info_bluetooth_not_allowed_to_be_turned_on" ),this, 
				SLOT(btOnWarningClose()));
        }
    }
    else { // offline mode is not active
        err = mBtengSettings->SetPowerState(EBTPowerOn);
    }    
    if ( err ) {
        completeDelegateExecution(err);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::btOnQuestionClose(int action)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    int err = 0;
    if(action == HbMessageBox::Yes) {
        //user chooses "yes" for using BT in offline 
        err = mBtengSettings->SetPowerState(EBTPowerOn);
    } else {
        err = KErrCancel;
    }
    if ( err ) {
        completeDelegateExecution(err);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::btOnWarningClose()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    completeDelegateExecution(KErrPermissionDenied);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::switchBTOff()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    int err = 0;
    
    CBTEngConnMan *btengConnMan(0);
    TRAP(err, btengConnMan = CBTEngConnMan::NewL());
    RBTDevAddrArray devAddrArray;
    if (!err) {
        err = btengConnMan->GetConnectedAddresses(devAddrArray);
    }
    if (!err && devAddrArray.Count()) {
        disconnectConnections(); 
    } else if ( !err ) {
        err = mBtengSettings->SetPowerState(EBTPowerOff);
    }
    devAddrArray.Close();
    delete btengConnMan;
    
    if (err) {
        completeDelegateExecution(err);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::disconnectConnections()
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    if (! mDisconnectDelegate) {
        mDisconnectDelegate = new BtDelegateDisconnect(
                settingModel(), deviceModel(), this);
        (void) connect(mDisconnectDelegate, 
                SIGNAL(delegateCompleted(int,BtAbstractDelegate*)), 
                this, SLOT(disconnectCompleted(int,BtAbstractDelegate*)));
    }
    QList<QVariant> list;
    list.append(QVariant( AllOngoingConnections ));
    mDisconnectDelegate->exec(QVariant(list));
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::disconnectCompleted(int err, BtAbstractDelegate *delegate)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    // We will turn off power even an error was returned from disconnection.
    Q_UNUSED(delegate);
    err = mBtengSettings->SetPowerState(EBTPowerOff);
    if ( err ) {
        completeDelegateExecution(err);
    }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

void BtDelegatePower::PowerStateChanged( TBTPowerStateValue aPowerState )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    // It is possible that others change power: no handling for these cases.
    if ( isExecuting() ) {
        int err = ( mReqPowerState == aPowerState ) ? KErrNone : KErrGeneral;
        completeDelegateExecution(err);
    } 
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

//Method derived from MBTEngSettingsObserver, no need to be implemented here
void BtDelegatePower::VisibilityModeChanged( TBTVisibilityMode aState )
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    Q_UNUSED( aState );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
}

/*!
   Returns true if offline mode is on, parameter returns true if BT is allowed 
   in offline mode
 */
bool BtDelegatePower::checkOfflineMode(bool& btEnabledInOffline)
{
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TCoreAppUIsNetworkConnectionAllowed offLineMode; 
    TBTEnabledInOfflineMode btEnabled;
   
    mBtengSettings->GetOfflineModeSettings(offLineMode, btEnabled);
    
    btEnabledInOffline = (btEnabled == EBTEnabledInOfflineMode);
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, offLineMode == ECoreAppUIsNetworkConnectionNotAllowed);
    return (offLineMode == ECoreAppUIsNetworkConnectionNotAllowed);
}

