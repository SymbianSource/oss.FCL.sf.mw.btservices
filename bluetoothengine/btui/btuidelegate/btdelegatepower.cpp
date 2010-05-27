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
    : BtAbstractDelegate( settingModel, deviceModel, parent )
{
    TRAP_IGNORE( mBtengSettings = CBTEngSettings::NewL(this) );
    Q_CHECK_PTR( mBtengSettings );
    
}

/*!
    Destructor.
 */
BtDelegatePower::~BtDelegatePower()
{
    delete mBtengSettings;
    
}

void BtDelegatePower::exec( const QVariant &params )
{   
    if (params.toInt()){//turn power OFF
        
        switchBTOff();     
    }
    else{//turn power ON  
    
        switchBTOn();
    }
}
       
    

void BtDelegatePower::switchBTOn()
{
    int err = 0;
    
    //check if device is in OFFLINE mode first
    TBTEnabledInOfflineMode enabledInOffline = EBTDisabledInOfflineMode;
    if (checkOfflineMode(enabledInOffline)){
    //if (1){
        if (enabledInOffline){
        //if (1){
            // BT is allowed to be enabled in offline mode, show query.
            HbMessageBox::question( tr("Turn Bluetooth on in offline mode?"),this, 
				SLOT(btOnQuestionClose(HbAction*)));

        }
        else{
            //if BT is not allowed to be enabled in offline mode, show message and complete
            HbMessageBox::warning(tr("Bluetooth not allowed to be turned on in offline mode"),this, 
				SLOT(btOnWarningClose()));
        }
        
    }
    else{
        //set BT on if the not in offline mode
        err = mBtengSettings->SetPowerState((TBTPowerStateValue)(1));
    }
    
    if ( err ) {
        QString info = "Unable to switch BT power ON" ;
        emit commandCompleted(KErrGeneral);
    }
    
}

void BtDelegatePower::btOnQuestionClose(HbAction *action)
{
    HbMessageBox *dlg = static_cast<HbMessageBox*>(sender());
    int err = 0;
    if(action == dlg->actions().at(0)) 
    {
        //user chooses "yes" for using BT in offline 
        err = mBtengSettings->SetPowerState((TBTPowerStateValue)(1));
    }
    else
    {
        //if user chooses "NO", emits the signal
        emit commandCompleted(KErrNone);
           
    }     
    if ( err ) {
        QString info = "Unable to switch BT power ON" ;
        emit commandCompleted(KErrGeneral);
    }
}

void BtDelegatePower::btOnWarningClose()
{
    emit commandCompleted(KErrNone);        
}



void BtDelegatePower::switchBTOff()
{
    int err = 0;
    err = mBtengSettings->SetPowerState((TBTPowerStateValue)(0));
    
    if ( err ) {
        QString info = "Unable to switch BT power OFF" ;
        emit commandCompleted(KErrGeneral);
    }
        
}

void BtDelegatePower::btOffDialogClose(HbAction *action)
{
    Q_UNUSED( action );
    
}

void BtDelegatePower::PowerStateChanged( TBTPowerStateValue aState )
{
    Q_UNUSED( aState );
    emit commandCompleted(KErrNone);
}

//Method derived from MBTEngSettingsObserver, no need to be implemented here
void BtDelegatePower::VisibilityModeChanged( TBTVisibilityMode aState )
{
    Q_UNUSED( aState );
}

bool BtDelegatePower::checkOfflineMode(TBTEnabledInOfflineMode& aEnabledInOffline)
{
    TCoreAppUIsNetworkConnectionAllowed offline = ECoreAppUIsNetworkConnectionAllowed;  
   
    mBtengSettings->GetOfflineModeSettings(offline, aEnabledInOffline);
    return (!offline);
    
}

/*if (params.toBool()) {  // turning power on

        // find out if local device is in offline mode
        QModelIndex idx = mModel->index( Btuim::OfflineMode, 0);
        QVariant var = mModel->data( idx, Qt::EditRole );
        bool offlineMode = var.toBool();

        // find out whether BT is allowed in offline mode
        var = mModel->data( idx, Btuim::SettingAdditionalParam );
        bool activationAllowed = var.toBool();
        
        if (offlineMode) {
            // in offline mode
            if (activationAllowed) {
                HbMessageBox *messageBox = new HbMessageBox(); 
                // BT is allowed to be enabled in offline mode, show query.
                if (messageBox->question( tr("Activate Bluetooth in offline mode?") )) {
                    ret = mModel->setData(index, value, role);
                }
                delete messageBox;
            }
            else {
                // BT is not allowed to be activated in offline mode, show note.

                HbDialog *mShowOnlyPopup = new HbDialog();
                mShowOnlyPopup->setAttribute(Qt::WA_DeleteOnClose);
                mShowOnlyPopup->setModal(false);
                mShowOnlyPopup->setBackgroundFaded(false);
                mShowOnlyPopup->setDismissPolicy( HbPopup::NoDismiss  );
                mShowOnlyPopup->setTimeout( 5000 );  // 5 sec
                HbLabel *label = new HbLabel( tr("Bluetooth is not allowed in offline mode") );
                label->setAlignment(Qt::AlignCenter);
                QSizeF popupSize(350,100);
                mShowOnlyPopup->setMinimumSize(popupSize);
                mShowOnlyPopup->setContentWidget(label);                
                mShowOnlyPopup->show();
            }
        } 
        else {
            // not in offline mode, forward the request to model. 
            ret =  mModel->setData(index, value, role);    
        }
    }
    else {   // turning power off
        // first check if existing connections
        QModelIndex idx = mModel->index(Btuim::BtConnections, 0);
        QVariant var = mModel->data(idx, Qt::EditRole);
        bool ok;
        TInt connNum = var.toInt( &ok );
        BTUI_ASSERT_X( ok, "BtUiSettingsDelegate::setData", "wrong qvariant type");
        if (connNum) {
            // there is at least 1 active connection, show query.
            HbMessageBox *messageBox = new HbMessageBox(); 
            if (messageBox->question( tr("Turn Bluetooth off even though connections exist?") )) {
                ret = mModel->setData(index, value, role);
            }
            delete messageBox;
        } 
        else {  
            // no active connections exist, forward the request to model.
            ret =  mModel->setData(index, value, role);
        }
    }*/
    //emit commandCompleted(err);
        //return ret;
    //return false;
