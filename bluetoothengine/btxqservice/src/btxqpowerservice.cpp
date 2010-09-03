/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#include "btxqpowerservice.h"
#include <btxqserviceapi.h>
#include <bluetoothdevicedialogs.h>
#include <hbapplication.h>
#include <hbdevicedialog.h>
#include <btnotif.h>

const QString deviceDialogType = "com.nokia.hb.btdevicedialog/1.0";


BtxqPowerService::BtxqPowerService(QObject *parent)
: XQServiceProvider(BluetoothServiceTogglePower, parent),
mSettings(0), mCurrentRequestIndex(0), mNotificationDialog(0), 
mOfflineDialog(0), mOfflineQueryShown(false)
{
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), qApp, SLOT(quit()));
}

BtxqPowerService::~BtxqPowerService()
{
    delete mSettings;
    delete mNotificationDialog;
    delete mOfflineDialog;
}

void BtxqPowerService::togglePower()
{
    mCurrentRequestIndex = setCurrentRequestAsync();
    
    TRAPD(err, mSettings = CBTEngSettings::NewL(this));
    if(err < 0)
        {
        // Error creating CBTEngSettings. Complete the service request with error.
        QVariant retVal(err);
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
    
    // Get the current power state.
    TBTPowerStateValue state; 
    err = mSettings->GetPowerState(state);
    if(err < 0)
        {
        // Error getting the current power state. Complete the service request with error.
        QVariant retVal(err);
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
    
    if (EBTPowerOff == state)
        {
        // Bluetooth is currently turned off.
        bool isInOffline = checkOfflineMode(); 
        
        if (isInOffline) 
            {
            // Device is in offline mode.
            // Ask user if he/she wants Bluetooth to be turned on in offline mode.
            showOfflineDialog();
            }
        
        else 
            {
            // Device is not in offline mode.
            // Turn Bluetooth on.
            changeBtPowerMode(EBTPowerOn);
            }               
        }         
    
    else 
        {  
        // Bluetooth is currently turned on.
        // Switch Bluetooth on.
        changeBtPowerMode(EBTPowerOff);
        }
}

void BtxqPowerService::changeBtPowerMode(TBTPowerStateValue aNewPowerState)
{   
    // Set the new Bluetooth power state.
    TInt error = mSettings->SetPowerState(aNewPowerState);
    if(error != KErrNone)
        {
        // Error setting the new Bluetooth power state.
        // Complete the service request with error.
        QVariant retVal(error);
        completeRequest(mCurrentRequestIndex, retVal);
        }
}

bool BtxqPowerService::checkOfflineMode()
{
    // Check if the device is in offline mode.
    TCoreAppUIsNetworkConnectionAllowed offline = ECoreAppUIsNetworkConnectionAllowed;  
    TBTEnabledInOfflineMode enabledInOffline; 
    mSettings->GetOfflineModeSettings(offline, enabledInOffline); 
    
    //todo: Check if activation of Bluetooth is allowed in offline mode.
    return !offline;  
}

void BtxqPowerService::showBTStatusNotification(TBool powerState)
{
    // Prepare new power state notification dialog.
    if(!mNotificationDialog)
        {
        mNotificationDialog = new HbDeviceDialog();
        }
    
    connect(mNotificationDialog, SIGNAL(deviceDialogClosed()), this, SLOT(handleDialogClosed()));
    QVariantMap parameters;
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogType), 
          QString::number(TBluetoothDialogParams::EGlobalNotif));
    
    parameters.insert(QString::number(TBluetoothDialogParams::EResource),
            QString::number(EGenericInfo));
    
    if(powerState)
        {
        // Show dialog with power on notification.
        parameters.insert(QString::number(TBluetoothDeviceDialog::EAdditionalInt),
              QString::number(EBTSwitchedOn));
        }
    else
        {
        // Show dialog with power off notification.
        parameters.insert(QString::number(TBluetoothDeviceDialog::EAdditionalInt),
              QString::number(EBTSwitchedOff));
        }
    
    mNotificationDialog->show(deviceDialogType, parameters);    
}

void BtxqPowerService::showOfflineDialog()
{
    // Prepare and show the offline query dialog.
    if(!mOfflineDialog)
        {
        mOfflineDialog = new HbDeviceDialog();
        }
    
    connect(mOfflineDialog, SIGNAL(dataReceived(QVariantMap)), 
          this, SLOT(handleDataReceived(QVariantMap)));
    connect(mOfflineDialog, SIGNAL(deviceDialogClosed()), 
          this, SLOT(handleDialogClosed()));
    
    QVariantMap parameters;
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogType), 
          QString::number(TBluetoothDialogParams::EInformationDialog));
    
    parameters.insert(QString::number(TBluetoothDialogParams::EDialogTitle),
          QString::number(TBluetoothDialogParams::EOfflineQuery));
    
    mOfflineQueryShown = true;
    mOfflineDialog->show(deviceDialogType, parameters);    
}

void BtxqPowerService::handleDialogClosed()
{
    // Handle the dialog closed.
    if(mOfflineQueryShown)
        {
        mOfflineDialog->cancel();
        mOfflineQueryShown = false;
        changeBtPowerMode(EBTPowerOn);
        }
    else
        {
        QVariant retVal(0);
        completeRequest(mCurrentRequestIndex, retVal);
        }
}

void BtxqPowerService::handleDataReceived(QVariantMap data)
{
    // Handle the user selection in the dialog.
    mOfflineDialog->cancel();
    if(data.value(QString("actionResult")).toBool())
        {
        changeBtPowerMode(EBTPowerOn);
        }
    else
        {
        QVariant retVal(0);
        completeRequest(mCurrentRequestIndex, retVal);
        }
}

void BtxqPowerService::PowerStateChanged( TBTPowerStateValue aState )
{
    // Call-back from CBTEngSettings confirming the power change to the new power state.
    (aState == EBTPowerOn) ? showBTStatusNotification(true) : showBTStatusNotification(false);
}

void BtxqPowerService::VisibilityModeChanged( TBTVisibilityMode aState )
{
    // This call-back is not used.
    (void) aState;
}

