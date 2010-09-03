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
* Description: Server class for handling commands from clients, and the 
*                central class in btnotif thread.
*
*/

#include <e32base.h>
#include <hb/hbcore/hbdevicedialogsymbian.h>
#include <btnotif.h>
#include "btnotifpowernotifier.h"

#include "btnotifserver.h"
#include "btnotifclientserver.h"
#include "bluetoothnotification.h"
#include "btnotificationmanager.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifPowerNotifier::CBTNotifPowerNotifier( CBTNotifServer* aServer )
:   iServer( aServer ), iTurnBTOn(ETrue)
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifPowerNotifier::ConstructL()
    {
    iBTEngSettings = CBTEngSettings::NewL(this);
    }

// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifPowerNotifier* CBTNotifPowerNotifier::NewL( CBTNotifServer* aServer )
    {
    CBTNotifPowerNotifier* self = new( ELeave ) CBTNotifPowerNotifier( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifPowerNotifier::~CBTNotifPowerNotifier()
    {
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    delete iBTEngSettings;
    }

// ---------------------------------------------------------------------------
// Process a client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifPowerNotifier::HandleNotifierRequestL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    TInt opcode = aMessage.Function();
    TInt uid = aMessage.Int0();
    TInt err = KErrNone;
    switch ( opcode ) 
        {
        case EBTNotifCancelNotifier:
            {
            aMessage.Complete( err );
            break;
            }
        case EBTNotifUpdateNotifier:
            {
            aMessage.Complete( err );
            break;
            }
        case EBTNotifStartSyncNotifier:
            {
            TRAP(err,ShowNotificationL(aMessage));
            if(err)
                {
                aMessage.Complete(err);
                }
            break;
            }
        case EBTNotifStartAsyncNotifier:
            {
            TRAP(err,ShowNotificationL(aMessage));
            if(err)
                {
                aMessage.Complete(err);
                }
            break;
            }
        default:
            {
            aMessage.Complete( KErrNotSupported );
            }
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifPowerNotifier::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    if(aData.Keys().MdcaPoint(0).Compare(_L("actionResult")) == 0)
        {
        iTurnBTOn = *(static_cast<TInt*>(aData.Get(_L("actionResult"))->Data()));
        }
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifPowerNotifier::MBRNotificationClosed( TInt aError, const TDesC8& aData  )
    {
    (void) aError;
    (void) aData;

    if(iTurnBTOn)
        {
        //Turn BT on.
        iBTEngSettings->SetPowerState(EBTPowerOn);
        }
    
    else if ( !iNotifierMessage.IsNull() )
        {
        //User choose No. So, complete the request with out turning BT on.
        iClientResponse() = EFalse;
        iNotifierMessage.Write(EBTNotifSrvReplySlot, iClientResponse);
        iNotifierMessage.Complete(KErrNone);
        }
    iNotification->RemoveObserver();
    iNotification = NULL;
    }

// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifPowerNotifier::ShowNotificationL(const RMessage2& aMessage )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    
    iNotifierMessage = aMessage;
    
    if(iNotification)
        {
        iNotification->RemoveObserver();
        iNotification = NULL;
        }
    iNotification = iServer->NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    //iNotification->SetNotificationType( TBluetoothDialogParams::ENote, EGenericInfo );
    
    // read the message parameters
    RBuf8 params;
    params.CreateL( aMessage.GetDesLengthL( EBTNotifSrvParamSlot ) );
    aMessage.ReadL( EBTNotifSrvParamSlot, params );
    TBTGenericInfoNotifierParams notifparams;
    TPckgC<TBTGenericInfoNotifierParams> paramsPckg( notifparams );
    paramsPckg.Set( params );
    TInt notifType = paramsPckg().iMessageType;
    
    TCoreAppUIsNetworkConnectionAllowed offLineMode; 
    TBTEnabledInOfflineMode btEnabled;
    iBTEngSettings->GetOfflineModeSettings(offLineMode, btEnabled);
    
    // Choose user interaction
    //
    if ( (btEnabled == EBTEnabledInOfflineMode) && 
            (offLineMode == ECoreAppUIsNetworkConnectionNotAllowed) )
        {
        // Allowed to enable BT in offline mode and device is currently in offline mode.
        // Launch the offline query notification dialog.
        iNotification->SetNotificationType( TBluetoothDialogParams::EInformationDialog, EIssueOfflineQuery );
        iServer->NotificationManager()->QueueNotificationL( iNotification, CBTNotificationManager::EPriorityHigh );
        }
    else if(offLineMode == ECoreAppUIsNetworkConnectionAllowed)
        {
        //Device not in offline mode, simply turn BT on.
        iBTEngSettings->SetPowerState(EBTPowerOn);
        }
    else
        {
        //Return EFalse, indication that BT should not be switched on.
        if(!iNotifierMessage.IsNull())
            {
            iClientResponse() = EFalse;
            iNotifierMessage.Write(EBTNotifSrvReplySlot, iClientResponse);
            iNotifierMessage.Complete(KErrNone);
            }
        }
    
    params.Close();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

void CBTNotifPowerNotifier::PowerStateChanged( TBTPowerStateValue aState )
    {
    if(!iNotifierMessage.IsNull())
        {
        if(aState == EBTPowerOn)
            {
            //Request to turn BT on was successful.
            iClientResponse() = ETrue;
            iNotifierMessage.Write(EBTNotifSrvReplySlot, iClientResponse);
            iNotifierMessage.Complete(KErrNone);
            }
        else
            {
            //Request to turn BT on was not successful.
            iClientResponse() = EFalse;
            iNotifierMessage.Write(EBTNotifSrvReplySlot, iClientResponse);
            iNotifierMessage.Complete(KErrNone);
            }
        }
    }

void CBTNotifPowerNotifier::VisibilityModeChanged( TBTVisibilityMode aState )
    {
    (void) aState;    
    }


